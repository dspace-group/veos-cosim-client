// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <chrono>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "DsVeosCoSim/DsVeosCoSim.h"
#include "Helper.hpp"
#include "Logger.hpp"

using namespace DsVeosCoSim;
using namespace std::chrono_literals;

namespace {

#define CheckDsVeosCoSimResult(result)           \
    do {                                         \
        DsVeosCoSim_Result _result_ = (result);  \
        if (_result_ != DsVeosCoSim_Result_Ok) { \
            return _result_;                     \
        }                                        \
    } while (0)

DsVeosCoSim_Handle Client;

class SimulationState {
public:
    void Set(DsVeosCoSim_SimulationState newState) {
        std::scoped_lock lock(_mutex);
        _state = newState;
    }

    [[nodiscard]] DsVeosCoSim_SimulationState Get() {
        std::scoped_lock lock(_mutex);
        return _state;
    }

    template <typename Func>
    auto WithLock(Func&& func) {
        std::scoped_lock lock(_mutex);
        return std::forward<Func>(func)(_state);
    }

private:
    std::mutex _mutex;
    DsVeosCoSim_SimulationState _state{};
};

SimulationState SimState;

std::vector<DsVeosCoSim_IoSignal> OutgoingSignals;
std::vector<DsVeosCoSim_CanController> CanControllers;
std::vector<DsVeosCoSim_EthController> EthControllers;
std::vector<DsVeosCoSim_LinController> LinControllers;
std::vector<DsVeosCoSim_FrController> FrControllers;

bool SendIoData;
bool SendCanMessages;
bool SendEthMessages;
bool SendLinMessages;
bool SendFrMessages;
bool PrintRoundTripTime;
bool PrintStepsPerSecond;

std::chrono::steady_clock::time_point LastRoundTripTimePrinted;

int64_t PerformanceStepCount;
std::chrono::steady_clock::time_point PerformanceMeasurementStart;
bool PerformanceMeasurementActive;
std::chrono::steady_clock::time_point LastPerformancePrintTime;

DsVeosCoSim_SimulationTime SendLastHalfSecond = -1;
int64_t SendCounter = 0;

[[nodiscard]] DsVeosCoSim_Result Disconnect();

void PrintCurrentRoundTripTime() {
    if (!PrintRoundTripTime) {
        return;
    }

    auto now = std::chrono::steady_clock::now();

    if (now - LastRoundTripTimePrinted < 1s) {
        return;
    }

    LastRoundTripTimePrinted = now;

    int64_t roundTripTimeInNanoseconds{};
    if (DsVeosCoSim_GetRoundTripTime(Client, &roundTripTimeInNanoseconds) != DsVeosCoSim_Result_Ok) {
        LogError("Could not get round trip time.");
        return;
    }

    if (roundTripTimeInNanoseconds > 0) {
        LogTrace("Round trip time: {} ns.", DsVeosCoSim_SimulationTimeToString(roundTripTimeInNanoseconds));
    }
}

void PrintStatus(bool value, std::string_view what) {
    if (value) {
        LogInfo("Enabled sending {}.", what);
    } else {
        LogInfo("Disabled sending {}.", what);
    }
}

void SwitchSendingIoSignals() {
    SendIoData = !SendIoData;
    PrintStatus(SendIoData, "IO data");
}

void SwitchSendingCanMessages() {
    SendCanMessages = !SendCanMessages;
    PrintStatus(SendCanMessages, "CAN messages");
}

void SwitchSendingEthMessages() {
    SendEthMessages = !SendEthMessages;
    PrintStatus(SendEthMessages, "Ethernet messages");
}

void SwitchSendingLinMessages() {
    SendLinMessages = !SendLinMessages;
    PrintStatus(SendLinMessages, "LIN messages");
}

void SwitchSendingFrMessages() {
    SendFrMessages = !SendFrMessages;
    PrintStatus(SendFrMessages, "FlexRay messages");
}

void SwitchPrintingRoundTripTime() {
    PrintRoundTripTime = !PrintRoundTripTime;
    if (PrintRoundTripTime) {
        LogInfo("Enabled Printing Round-Trip Time.");
    } else {
        LogInfo("Disabled Printing Round-Trip Time.");
    }
}

void StartPerformanceMeasurement() {
    PerformanceStepCount = 0;
    PerformanceMeasurementStart = std::chrono::steady_clock::now();
    PerformanceMeasurementActive = true;
    LastPerformancePrintTime = {};
    SendLastHalfSecond = -1;
    SendCounter = 0;
}

void StopPerformanceMeasurement() {
    if (!PerformanceMeasurementActive) {
        return;
    }

    PerformanceMeasurementActive = false;

    double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - PerformanceMeasurementStart).count();
    if (seconds > 0.0) {
        LogInfo("Performance: {:.1f} steps per second average ({} steps in {:.3f} s).",
                static_cast<double>(PerformanceStepCount) / seconds,
                PerformanceStepCount,
                seconds);
    }
}

void PrintCurrentStepsPerSecond() {
    if (!PerformanceMeasurementActive || !PrintStepsPerSecond) {
        return;
    }

    auto now = std::chrono::steady_clock::now();

    if (now - LastPerformancePrintTime < 1s) {
        return;
    }

    LastPerformancePrintTime = now;

    double seconds = std::chrono::duration<double>(now - PerformanceMeasurementStart).count();
    if (seconds > 0.0) {
        LogInfo("Steps per second: {:.1f} ({} steps in {:.1f} s).", static_cast<double>(PerformanceStepCount) / seconds, PerformanceStepCount, seconds);
    }
}

void SwitchPrintingStepsPerSecond() {
    PrintStepsPerSecond = !PrintStepsPerSecond;
    if (PrintStepsPerSecond) {
        LogInfo("Enabled printing steps per second.");
    } else {
        LogInfo("Disabled printing steps per second.");
    }
}

[[nodiscard]] DsVeosCoSim_Result WriteOutGoingSignal(const DsVeosCoSim_IoSignal& ioSignal) {
    size_t length = DsVeosCoSim_GetDataTypeSize(ioSignal.dataType) * ioSignal.length;
    std::vector<uint8_t> data = GenerateBytes(length);

    return DsVeosCoSim_WriteOutgoingSignal(Client, ioSignal.id, ioSignal.length, data.data());
}

[[nodiscard]] DsVeosCoSim_Result TransmitCanMessage(const DsVeosCoSim_CanController& controller) {
    uint32_t length = GenerateRandom(1U, 8U);
    DsVeosCoSim_CanMessageContainer messageContainer{};
    messageContainer.controllerId = controller.id;
    messageContainer.id = GenerateU32();
    messageContainer.timestamp = GenerateI64();
    messageContainer.length = length;
    FillWithRandomData(&messageContainer.data[0], length);

    return DsVeosCoSim_TransmitCanMessageContainer(Client, &messageContainer);
}

[[nodiscard]] DsVeosCoSim_Result TransmitEthMessage(const DsVeosCoSim_EthController& controller) {
    uint32_t length = GenerateRandom(1U, 8U);
    DsVeosCoSim_EthMessageContainer messageContainer{};
    messageContainer.controllerId = controller.id;
    messageContainer.timestamp = GenerateI64();
    messageContainer.length = length;
    FillWithRandomData(messageContainer.data, length);

    return DsVeosCoSim_TransmitEthMessageContainer(Client, &messageContainer);
}

[[nodiscard]] DsVeosCoSim_Result TransmitLinMessage(const DsVeosCoSim_LinController& controller) {
    uint32_t length = GenerateRandom(1U, 8U);
    DsVeosCoSim_LinMessageContainer messageContainer{};
    messageContainer.controllerId = controller.id;
    messageContainer.id = GenerateU32();
    messageContainer.timestamp = GenerateI64();
    messageContainer.length = length;
    FillWithRandomData(messageContainer.data, length);

    return DsVeosCoSim_TransmitLinMessageContainer(Client, &messageContainer);
}

[[nodiscard]] DsVeosCoSim_Result TransmitFrMessage(const DsVeosCoSim_FrController& controller) {
    uint32_t length = GenerateRandom(1U, 8U);
    DsVeosCoSim_FrMessageContainer messageContainer{};
    messageContainer.controllerId = controller.id;
    messageContainer.id = GenerateU32();
    messageContainer.timestamp = GenerateI64();
    messageContainer.length = length;
    FillWithRandomData(messageContainer.data, length);

    return DsVeosCoSim_TransmitFrMessageContainer(Client, &messageContainer);
}

[[nodiscard]] DsVeosCoSim_Result SendSomeData(DsVeosCoSim_SimulationTime simulationTime) {
    DsVeosCoSim_SimulationTime currentHalfSecond = simulationTime / 500000000;
    if (currentHalfSecond == SendLastHalfSecond) {
        return DsVeosCoSim_Result_Ok;
    }

    SendLastHalfSecond = currentHalfSecond;
    SendCounter++;

    if (SendIoData && ((SendCounter % 5) == 0)) {
        for (const DsVeosCoSim_IoSignal& signal : OutgoingSignals) {
            CheckDsVeosCoSimResult(WriteOutGoingSignal(signal));
        }
    }

    if (SendCanMessages && ((SendCounter % 5) == 1)) {
        for (const DsVeosCoSim_CanController& controller : CanControllers) {
            CheckDsVeosCoSimResult(TransmitCanMessage(controller));
        }
    }

    if (SendEthMessages && ((SendCounter % 5) == 2)) {
        for (const DsVeosCoSim_EthController& controller : EthControllers) {
            CheckDsVeosCoSimResult(TransmitEthMessage(controller));
        }
    }

    if (SendLinMessages && ((SendCounter % 5) == 3)) {
        for (const DsVeosCoSim_LinController& controller : LinControllers) {
            CheckDsVeosCoSimResult(TransmitLinMessage(controller));
        }
    }

    if (SendFrMessages && ((SendCounter % 5) == 4)) {
        for (const DsVeosCoSim_FrController& controller : FrControllers) {
            CheckDsVeosCoSimResult(TransmitFrMessage(controller));
        }
    }

    return DsVeosCoSim_Result_Ok;
}

void OnIncomingSignalChanged([[maybe_unused]] DsVeosCoSim_SimulationTime simulationTime,
                             const DsVeosCoSim_IoSignal* ioSignal,
                             uint32_t length,
                             const void* value,
                             [[maybe_unused]] void* userData) {
    LogIoData(DsVeosCoSim_IoDataToString(ioSignal, length, value));
}

void OnCanMessageContainerReceived([[maybe_unused]] DsVeosCoSim_SimulationTime simulationTime,
                                   [[maybe_unused]] const DsVeosCoSim_CanController* controller,
                                   const DsVeosCoSim_CanMessageContainer* messageContainer,
                                   [[maybe_unused]] void* userData) {
    LogCanMessage(DsVeosCoSim_CanMessageContainerToString(messageContainer));
}

void OnEthMessageContainerReceived([[maybe_unused]] DsVeosCoSim_SimulationTime simulationTime,
                                   [[maybe_unused]] const DsVeosCoSim_EthController* controller,
                                   const DsVeosCoSim_EthMessageContainer* messageContainer,
                                   [[maybe_unused]] void* userData) {
    LogEthMessage(DsVeosCoSim_EthMessageContainerToString(messageContainer));
}

void OnLinMessageContainerReceived([[maybe_unused]] DsVeosCoSim_SimulationTime simulationTime,
                                   [[maybe_unused]] const DsVeosCoSim_LinController* controller,
                                   const DsVeosCoSim_LinMessageContainer* messageContainer,
                                   [[maybe_unused]] void* userData) {
    LogLinMessage(DsVeosCoSim_LinMessageContainerToString(messageContainer));
}

void OnFrMessageContainerReceived([[maybe_unused]] DsVeosCoSim_SimulationTime simulationTime,
                                  [[maybe_unused]] const DsVeosCoSim_FrController* controller,
                                  const DsVeosCoSim_FrMessageContainer* messageContainer,
                                  [[maybe_unused]] void* userData) {
    LogFrMessage(DsVeosCoSim_FrMessageContainerToString(messageContainer));
}

[[nodiscard]] DsVeosCoSim_Result Connect(const std::string& host, const std::string& serverName) {
    LogInfo("Connecting ...");

    DsVeosCoSim_ConnectionState connectionState{};
    CheckDsVeosCoSimResult(DsVeosCoSim_GetConnectionState(Client, &connectionState));
    if (connectionState == DsVeosCoSim_ConnectionState_Connected) {
        LogInfo("Already connected.");
        return DsVeosCoSim_Result_Ok;
    }

    DsVeosCoSim_ConnectConfig connectConfig{};
    connectConfig.clientName = "Example Test Client";
    connectConfig.serverName = serverName.c_str();
    connectConfig.remoteIpAddress = host.c_str();

    if (DsVeosCoSim_Connect(Client, connectConfig) != DsVeosCoSim_Result_Ok) {
        LogError("Could not connect.");
        return DsVeosCoSim_Result_Error;
    }

    LogTrace("");

    DsVeosCoSim_SimulationTime stepSize{};
    CheckDsVeosCoSimResult(DsVeosCoSim_GetStepSize(Client, &stepSize));
    LogTrace("Step size: {} s", DsVeosCoSim_SimulationTimeToString(stepSize));
    LogTrace("");

    DsVeosCoSim_SimulationState initialState{};
    CheckDsVeosCoSimResult(DsVeosCoSim_GetSimulationState(Client, &initialState));

    // This can happen with old clients. In that case we assume the state stopped, so that at least the simulation start
    // can be passed to the server
    if (initialState == DsVeosCoSim_SimulationState_Unloaded) {
        initialState = DsVeosCoSim_SimulationState_Stopped;
    }

    SimState.Set(initialState);

    uint32_t tmpCanControllersCount{};
    const DsVeosCoSim_CanController* tmpCanControllers{};
    CheckDsVeosCoSimResult(DsVeosCoSim_GetCanControllers(Client, &tmpCanControllersCount, &tmpCanControllers));
    if (tmpCanControllersCount > 0) {
        CanControllers = std::vector<DsVeosCoSim_CanController>(tmpCanControllers, tmpCanControllers + tmpCanControllersCount);
        LogTrace("Found the following CAN controllers:");
        for (const DsVeosCoSim_CanController& controller : CanControllers) {
            LogTrace("  {}", DsVeosCoSim_CanControllerToString(&controller));
        }

        LogTrace("");
    }

    uint32_t tmpEthControllersCount{};
    const DsVeosCoSim_EthController* tmpEthControllers{};
    CheckDsVeosCoSimResult(DsVeosCoSim_GetEthControllers(Client, &tmpEthControllersCount, &tmpEthControllers));
    if (tmpEthControllersCount > 0) {
        EthControllers = std::vector<DsVeosCoSim_EthController>(tmpEthControllers, tmpEthControllers + tmpEthControllersCount);
        LogTrace("Found the following Ethernet controllers:");
        for (const DsVeosCoSim_EthController& controller : EthControllers) {
            LogTrace("  {}", DsVeosCoSim_EthControllerToString(&controller));
        }

        LogTrace("");
    }

    uint32_t tmpLinControllersCount{};
    const DsVeosCoSim_LinController* tmpLinControllers{};
    CheckDsVeosCoSimResult(DsVeosCoSim_GetLinControllers(Client, &tmpLinControllersCount, &tmpLinControllers));
    if (tmpLinControllersCount > 0) {
        LinControllers = std::vector<DsVeosCoSim_LinController>(tmpLinControllers, tmpLinControllers + tmpLinControllersCount);
        LogTrace("Found the following LIN controllers:");
        for (const DsVeosCoSim_LinController& controller : LinControllers) {
            LogTrace("  {}", DsVeosCoSim_LinControllerToString(&controller));
        }

        LogTrace("");
    }

    uint32_t tmpFrControllersCount{};
    const DsVeosCoSim_FrController* tmpFrControllers{};
    CheckDsVeosCoSimResult(DsVeosCoSim_GetFrControllers(Client, &tmpFrControllersCount, &tmpFrControllers));
    if (tmpFrControllersCount > 0) {
        FrControllers = std::vector<DsVeosCoSim_FrController>(tmpFrControllers, tmpFrControllers + tmpFrControllersCount);
        LogTrace("Found the following FlexRay controllers:");
        for (const DsVeosCoSim_FrController& controller : FrControllers) {
            LogTrace("  {}", DsVeosCoSim_FrControllerToString(&controller));
        }

        LogTrace("");
    }

    uint32_t tmpIncomingSignalsCount{};
    const DsVeosCoSim_IoSignal* tmpIncomingSignals{};
    CheckDsVeosCoSimResult(DsVeosCoSim_GetIncomingSignals(Client, &tmpIncomingSignalsCount, &tmpIncomingSignals));
    if (tmpIncomingSignalsCount > 0) {
        LogTrace("Found the following incoming signals:");
        for (uint32_t i = 0; i < tmpIncomingSignalsCount; i++) {
            const DsVeosCoSim_IoSignal& signal = tmpIncomingSignals[i];
            LogTrace("  {}", DsVeosCoSim_IoSignalToString(&signal));
        }

        LogTrace("");
    }

    uint32_t tmpOutgoingSignalsCount{};
    const DsVeosCoSim_IoSignal* tmpOutgoingSignals{};
    CheckDsVeosCoSimResult(DsVeosCoSim_GetOutgoingSignals(Client, &tmpOutgoingSignalsCount, &tmpOutgoingSignals));
    if (tmpOutgoingSignalsCount > 0) {
        OutgoingSignals = std::vector<DsVeosCoSim_IoSignal>(tmpOutgoingSignals, tmpOutgoingSignals + tmpOutgoingSignalsCount);
        LogTrace("Found the following outgoing signals:");
        for (const DsVeosCoSim_IoSignal& signal : OutgoingSignals) {
            LogTrace("  {}", DsVeosCoSim_IoSignalToString(&signal));
        }

        LogTrace("");
    }

    LogInfo("Connected.");
    return DsVeosCoSim_Result_Ok;
}

[[nodiscard]] DsVeosCoSim_Result Disconnect() {
    LogInfo("Disconnecting ...");
    CheckDsVeosCoSimResult(DsVeosCoSim_Disconnect(Client));
    LogInfo("Disconnected.");

    return DsVeosCoSim_Result_Ok;
}

[[nodiscard]] DsVeosCoSim_Result RunPollingBasedCoSimulation() {
    DsVeosCoSim_Callbacks callbacks{};
    callbacks.incomingSignalChangedCallback = OnIncomingSignalChanged;
    callbacks.canMessageContainerReceivedCallback = OnCanMessageContainerReceived;
    callbacks.ethMessageContainerReceivedCallback = OnEthMessageContainerReceived;
    callbacks.linMessageContainerReceivedCallback = OnLinMessageContainerReceived;
    callbacks.frMessageContainerReceivedCallback = OnFrMessageContainerReceived;

    CheckDsVeosCoSimResult(DsVeosCoSim_StartPollingBasedCoSimulation(Client, callbacks));

    LogInfo("Running polling-based co-simulation ...");

    while (true) {
        DsVeosCoSim_SimulationTime simulationTime{};
        DsVeosCoSim_Command command{};

        DsVeosCoSim_Result result = DsVeosCoSim_PollCommand2(Client, &simulationTime, &command, 10);

        if (result == DsVeosCoSim_Result_Disconnected) {
            StopPerformanceMeasurement();
            LogInfo("Polling-based co-simulation finished successfully.");
            return DsVeosCoSim_Result_Ok;
        }

        PrintCurrentRoundTripTime();

        if (result == DsVeosCoSim_Result_Timeout) {
            continue;
        }

        if (result != DsVeosCoSim_Result_Ok) {
            StopPerformanceMeasurement();
            LogError("Polling-based co-simulation finished with an error.");
            SimState.Set(DsVeosCoSim_SimulationState_Unloaded);
            return DsVeosCoSim_Result_Error;
        }

        switch (command) {
            case DsVeosCoSim_Command_Start:
                LogInfo("Simulation started at {} s.", DsVeosCoSim_SimulationTimeToString(simulationTime));
                SimState.Set(DsVeosCoSim_SimulationState_Running);
                StartPerformanceMeasurement();
                break;
            case DsVeosCoSim_Command_Stop:
                LogInfo("Simulation stopped at {} s.", DsVeosCoSim_SimulationTimeToString(simulationTime));
                StopPerformanceMeasurement();
                SimState.Set(DsVeosCoSim_SimulationState_Stopped);
                break;
            case DsVeosCoSim_Command_Terminate:
            case DsVeosCoSim_Command_TerminateFinished:
                LogInfo("Simulation terminated at {} s.", DsVeosCoSim_SimulationTimeToString(simulationTime));
                StopPerformanceMeasurement();
                SimState.Set(DsVeosCoSim_SimulationState_Terminated);
                break;
            case DsVeosCoSim_Command_Pause:
                LogInfo("Simulation paused at {} s.", DsVeosCoSim_SimulationTimeToString(simulationTime));
                StopPerformanceMeasurement();
                SimState.Set(DsVeosCoSim_SimulationState_Paused);
                break;
            case DsVeosCoSim_Command_Continue:
                LogInfo("Simulation continued at {} s.", DsVeosCoSim_SimulationTimeToString(simulationTime));
                SimState.Set(DsVeosCoSim_SimulationState_Running);
                StartPerformanceMeasurement();
                break;
            case DsVeosCoSim_Command_Step:
                PerformanceStepCount++;
                PrintCurrentStepsPerSecond();

                if (SendSomeData(simulationTime) != DsVeosCoSim_Result_Ok) {
                    LogError("Could not send data.");
                }
                break;
            default:
                break;
        }

        CheckDsVeosCoSimResult(DsVeosCoSim_FinishCommand(Client));
    }
}

[[nodiscard]] DsVeosCoSim_Result ToggleSimulation() {
    return SimState.WithLock([](DsVeosCoSim_SimulationState state) -> DsVeosCoSim_Result {
        switch (state) {
            case DsVeosCoSim_SimulationState_Running:
                LogInfo("Pausing simulation ...");
                return DsVeosCoSim_PauseSimulation(Client);
            case DsVeosCoSim_SimulationState_Stopped:
                LogInfo("Starting simulation ...");
                return DsVeosCoSim_StartSimulation(Client);
            case DsVeosCoSim_SimulationState_Paused:
                LogInfo("Continuing simulation ...");
                return DsVeosCoSim_ContinueSimulation(Client);
            default:
                LogError("Cannot start or pause in state {}.", DsVeosCoSim_SimulationStateToString(state));
                return DsVeosCoSim_Result_Ok;
        }
    });
}

[[nodiscard]] DsVeosCoSim_Result Stop() {
    return SimState.WithLock([](DsVeosCoSim_SimulationState state) -> DsVeosCoSim_Result {
        switch (state) {
            case DsVeosCoSim_SimulationState_Stopped:
                return DsVeosCoSim_Result_Ok;
            case DsVeosCoSim_SimulationState_Running:
            case DsVeosCoSim_SimulationState_Paused:
                LogInfo("Stopping simulation ...");
                return DsVeosCoSim_StopSimulation(Client);
            default:
                LogError("Cannot stop in state {}.", DsVeosCoSim_SimulationStateToString(state));
                return DsVeosCoSim_Result_Ok;
        }
    });
}

[[nodiscard]] DsVeosCoSim_Result Terminate() {
    return SimState.WithLock([](DsVeosCoSim_SimulationState state) -> DsVeosCoSim_Result {
        switch (state) {
            case DsVeosCoSim_SimulationState_Terminated:
                return DsVeosCoSim_Result_Ok;
            case DsVeosCoSim_SimulationState_Running:
            case DsVeosCoSim_SimulationState_Paused:
            case DsVeosCoSim_SimulationState_Stopped:
                LogInfo("Terminating simulation ...");
                return DsVeosCoSim_TerminateSimulation(Client, DsVeosCoSim_TerminateReason_Error);
            default:
                LogError("Cannot terminate in state {}.", DsVeosCoSim_SimulationStateToString(state));
                return DsVeosCoSim_Result_Ok;
        }
    });
}

DsVeosCoSim_Result HandleUserInput() {
    while (true) {
        switch (GetChar()) {
            case CTRL('c'):
                return Disconnect();
            case '1':
                SwitchSendingIoSignals();
                break;
            case '2':
                SwitchSendingCanMessages();
                break;
            case '3':
                SwitchSendingEthMessages();
                break;
            case '4':
                SwitchSendingLinMessages();
                break;
            case '5':
                SwitchSendingFrMessages();
                break;
            case '0':
                SwitchPrintingRoundTripTime();
                break;
            case '9':
                SwitchPrintingStepsPerSecond();
                break;
            case 's':
            case 'p':
            case 'k':
            case ' ':
                CheckDsVeosCoSimResult(ToggleSimulation());
                break;
            case 't':
                CheckDsVeosCoSimResult(Stop());
                break;
            case CTRL('t'):
                CheckDsVeosCoSimResult(Terminate());
                break;
            default:
                LogError("Unknown key.");
                break;
        }
    }
}

[[nodiscard]] DsVeosCoSim_Result HostClient(const std::string& host, const std::string& name) {
    CheckDsVeosCoSimResult(Connect(host, name));

    // Stopping the listening for a character is complicated, so we just detach the thread
    std::thread([] {
        HandleUserInput();
    }).detach();

    return RunPollingBasedCoSimulation();
}

}  // namespace

int main(int argc, char** argv) {
    InitializeOutput();

    std::string host;
    std::string name = "CoSimTest";

    for (int32_t i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--host") == 0) {
            if (++i < argc) {
                host = argv[i];
            } else {
                LogError("No host specified.");
                return 1;
            }
        }

        if (strcmp(argv[i], "--name") == 0) {
            if (++i < argc) {
                name = argv[i];
            } else {
                LogError("No name specified.");
                return 1;
            }
        }
    }

    Client = DsVeosCoSim_Create();
    if (!Client) {
        LogError("Could not create handle.");
        return 1;
    }

    DsVeosCoSim_Result result = HostClient(host, name);

    DsVeosCoSim_Destroy(Client);

    return (result == DsVeosCoSim_Result_Ok) ? 0 : 1;
}
