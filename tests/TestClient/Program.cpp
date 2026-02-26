// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <cstdint>
#include <cstring>
#include <ctime>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "DsVeosCoSim/DsVeosCoSim.h"
#include "Helper.hpp"
#include "Logger.hpp"

#ifdef _WIN32

#include <chrono>

#endif

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

DsVeosCoSim_SimulationState State;
std::mutex LockState;

std::thread SimulationThread;

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
bool PrintTurnAroundTime;

time_t lastTimeTurnaroundTimePrinted;

[[nodiscard]] DsVeosCoSim_Result Disconnect();

void PrintTurnaroundTime() {
    time_t now{};
    time(&now);

    if (now <= lastTimeTurnaroundTimePrinted) {
        return;
    }

    lastTimeTurnaroundTimePrinted = now;

    if (!PrintTurnAroundTime) {
        return;
    }

    int64_t roundTripTimeInNanoseconds{};
    if (DsVeosCoSim_GetRoundTripTime(Client, &roundTripTimeInNanoseconds) != DsVeosCoSim_Result_Ok) {
        LogError("Could not get round trip time.");
        (void)Disconnect();
    }

    if (roundTripTimeInNanoseconds > 0) {
        LogTrace("Round trip time: {} ns.", DsVeosCoSim_SimulationTimeToString(roundTripTimeInNanoseconds));
    }
}

void PrintStatus(bool value, const std::string& what) {
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
    PrintStatus(SendEthMessages, "ETH messages");
}

void SwitchSendingLinMessages() {
    SendLinMessages = !SendLinMessages;
    PrintStatus(SendLinMessages, "LIN messages");
}

void SwitchSendingFrMessages() {
    SendFrMessages = !SendFrMessages;
    PrintStatus(SendFrMessages, "FR messages");
}

void SwitchPrintingTurnAroundTime() {
    PrintTurnAroundTime = !PrintTurnAroundTime;
    if (PrintTurnAroundTime) {
        LogInfo("Enabled Printing Turnaround Time.");
    } else {
        LogInfo("Disabled Printing Turnaround Time.");
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
    PrintTurnaroundTime();

    static DsVeosCoSim_SimulationTime lastSecond = -1;
    static int64_t counter = 0;
    DsVeosCoSim_SimulationTime currentSecond = simulationTime / 1'000'000'000;
    if (currentSecond == lastSecond) {
        return DsVeosCoSim_Result_Ok;
    }

    lastSecond = currentSecond;
    counter++;

    if (SendIoData && ((counter % 5) == 0)) {
        for (const DsVeosCoSim_IoSignal& signal : OutgoingSignals) {
            CheckDsVeosCoSimResult(WriteOutGoingSignal(signal));
        }
    }

    if (SendCanMessages && ((counter % 5) == 1)) {
        for (const DsVeosCoSim_CanController& controller : CanControllers) {
            CheckDsVeosCoSimResult(TransmitCanMessage(controller));
        }
    }

    if (SendEthMessages && ((counter % 5) == 2)) {
        for (const DsVeosCoSim_EthController& controller : EthControllers) {
            CheckDsVeosCoSimResult(TransmitEthMessage(controller));
        }
    }

    if (SendLinMessages && ((counter % 5) == 3)) {
        for (const DsVeosCoSim_LinController& controller : LinControllers) {
            CheckDsVeosCoSimResult(TransmitLinMessage(controller));
        }
    }

    if (SendFrMessages && ((counter % 5) == 4)) {
        for (const DsVeosCoSim_FrController& controller : FrControllers) {
            CheckDsVeosCoSimResult(TransmitFrMessage(controller));
        }
    }

    return DsVeosCoSim_Result_Ok;
}

void OnSimulationPostStepCallback(DsVeosCoSim_SimulationTime simulationTime, [[maybe_unused]] void* userData) {
    if (SendSomeData(simulationTime) != DsVeosCoSim_Result_Ok) {
        LogError("Could not send data.");
    }
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

void StartSimulationThread(const std::function<void()>& function) {
    SimulationThread = std::thread(function);
}

void OnSimulationStartedCallback(DsVeosCoSim_SimulationTime simulationTime, [[maybe_unused]] void* userData) {
    std::lock_guard lock(LockState);
    LogInfo("Simulation started at {} s.", DsVeosCoSim_SimulationTimeToString(simulationTime));
    State = DsVeosCoSim_SimulationState_Running;
}

void OnSimulationStoppedCallback(DsVeosCoSim_SimulationTime simulationTime, [[maybe_unused]] void* userData) {
    std::lock_guard lock(LockState);
    LogInfo("Simulation stopped at {} s.", DsVeosCoSim_SimulationTimeToString(simulationTime));
    State = DsVeosCoSim_SimulationState_Stopped;
}

void OnSimulationTerminatedCallback(DsVeosCoSim_SimulationTime simulationTime, DsVeosCoSim_TerminateReason reason, [[maybe_unused]] void* userData) {
    std::lock_guard lock(LockState);
    LogInfo("Simulation terminated with reason {} at {} s.", DsVeosCoSim_TerminateReasonToString(reason), DsVeosCoSim_SimulationTimeToString(simulationTime));
    State = DsVeosCoSim_SimulationState_Terminated;
}

void OnSimulationPausedCallback(DsVeosCoSim_SimulationTime simulationTime, [[maybe_unused]] void* userData) {
    std::lock_guard lock(LockState);
    LogInfo("Simulation paused at {} s.", DsVeosCoSim_SimulationTimeToString(simulationTime));
    State = DsVeosCoSim_SimulationState_Paused;
}

void OnSimulationContinuedCallback(DsVeosCoSim_SimulationTime simulationTime, [[maybe_unused]] void* userData) {
    std::lock_guard lock(LockState);
    LogInfo("Simulation continued at {} s.", DsVeosCoSim_SimulationTimeToString(simulationTime));
    State = DsVeosCoSim_SimulationState_Running;
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

    CheckDsVeosCoSimResult(DsVeosCoSim_GetSimulationState(Client, &State));

    // This can happen with old clients. In that case we assume the state stopped, so that at least the simulation start
    // can be passed to the server
    if (State == DsVeosCoSim_SimulationState_Unloaded) {
        State = DsVeosCoSim_SimulationState_Stopped;
    }

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
        LogTrace("Found the following ETH controllers:");
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
        LogTrace("Found the following FR controllers:");
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

[[noreturn]] void RunCallbackBasedCoSimulation() {
    DsVeosCoSim_Callbacks callbacks{};
    callbacks.simulationStartedCallback = OnSimulationStartedCallback;
    callbacks.simulationStoppedCallback = OnSimulationStoppedCallback;
    callbacks.simulationTerminatedCallback = OnSimulationTerminatedCallback;
    callbacks.simulationPausedCallback = OnSimulationPausedCallback;
    callbacks.simulationContinuedCallback = OnSimulationContinuedCallback;
    callbacks.simulationEndStepCallback = OnSimulationPostStepCallback;
    callbacks.incomingSignalChangedCallback = OnIncomingSignalChanged;
    callbacks.canMessageContainerReceivedCallback = OnCanMessageContainerReceived;
    callbacks.ethMessageContainerReceivedCallback = OnEthMessageContainerReceived;
    callbacks.linMessageContainerReceivedCallback = OnLinMessageContainerReceived;
    callbacks.frMessageContainerReceivedCallback = OnFrMessageContainerReceived;

    LogInfo("Running callback-based co-simulation ...");
    if (DsVeosCoSim_RunCallbackBasedCoSimulation(Client, callbacks) != DsVeosCoSim_Result_Disconnected) {
        LogError("Callback-based co-simulation finished with an error.");
        State = DsVeosCoSim_SimulationState_Unloaded;
        SimulationThread.detach();
        exit(1);
    }

    LogInfo("Callback-based co-simulation finished successfully.");
    SimulationThread.detach();
    exit(0);
}

[[nodiscard]] DsVeosCoSim_Result Start() {
    std::lock_guard lock(LockState);

    DsVeosCoSim_SimulationState state = State;
    switch (state) {
        case DsVeosCoSim_SimulationState_Running:
            break;
        case DsVeosCoSim_SimulationState_Stopped:
            LogInfo("Starting simulation ...");
            CheckDsVeosCoSimResult(DsVeosCoSim_StartSimulation(Client));
            break;
        case DsVeosCoSim_SimulationState_Paused:
            LogInfo("Continuing simulation ...");
            CheckDsVeosCoSimResult(DsVeosCoSim_ContinueSimulation(Client));
            break;
        default:
            LogError("Cannot start in state {}.", DsVeosCoSim_SimulationStateToString(State));
            break;
    }

    return DsVeosCoSim_Result_Ok;
}

[[nodiscard]] DsVeosCoSim_Result Pause() {
    std::lock_guard lock(LockState);

    DsVeosCoSim_SimulationState state = State;
    switch (state) {
        case DsVeosCoSim_SimulationState_Paused:
            break;
        case DsVeosCoSim_SimulationState_Running:
            LogInfo("Pausing simulation ...");
            CheckDsVeosCoSimResult(DsVeosCoSim_PauseSimulation(Client));
            break;
        default:
            LogError("Cannot pause in state {}.", DsVeosCoSim_SimulationStateToString(State));
            break;
    }

    return DsVeosCoSim_Result_Ok;
}

[[nodiscard]] DsVeosCoSim_Result Stop() {
    std::lock_guard lock(LockState);

    DsVeosCoSim_SimulationState state = State;
    switch (state) {
        case DsVeosCoSim_SimulationState_Stopped:
            break;
        case DsVeosCoSim_SimulationState_Running:
        case DsVeosCoSim_SimulationState_Paused:
            LogInfo("Stopping simulation ...");
            CheckDsVeosCoSimResult(DsVeosCoSim_StopSimulation(Client));
            break;
        default:
            LogError("Cannot stop in state {}.", DsVeosCoSim_SimulationStateToString(State));
            break;
    }

    return DsVeosCoSim_Result_Ok;
}

[[nodiscard]] DsVeosCoSim_Result Terminate() {
    std::lock_guard lock(LockState);

    DsVeosCoSim_SimulationState state = State;
    switch (state) {
        case DsVeosCoSim_SimulationState_Terminated:
            break;
        case DsVeosCoSim_SimulationState_Running:
        case DsVeosCoSim_SimulationState_Paused:
        case DsVeosCoSim_SimulationState_Stopped:
            LogInfo("Terminating simulation ...");
            CheckDsVeosCoSimResult(DsVeosCoSim_TerminateSimulation(Client, DsVeosCoSim_TerminateReason_Error));
            break;
        default:
            LogError("Cannot terminate in state {}.", DsVeosCoSim_SimulationStateToString(State));
            break;
    }

    return DsVeosCoSim_Result_Ok;
}

[[nodiscard]] DsVeosCoSim_Result HandleUserInput() {
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
                SwitchPrintingTurnAroundTime();
                break;
            case 's':
                CheckDsVeosCoSimResult(Start());
                break;
            case 'p':
                CheckDsVeosCoSimResult(Pause());
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

    StartSimulationThread(RunCallbackBasedCoSimulation);

    bool connected = true;

    std::thread roundTripTimeThread([&] {
        while (connected) {
            std::this_thread::sleep_for(100ms);

            {
                std::lock_guard lock(LockState);
                if (State == DsVeosCoSim_SimulationState_Running) {
                    continue;
                }
            }

            PrintTurnaroundTime();
        }
    });

    DsVeosCoSim_Result result = HandleUserInput();
    connected = false;
    roundTripTimeThread.join();

    return result;
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
    if (SimulationThread.joinable()) {
        SimulationThread.join();
    }

    DsVeosCoSim_Destroy(Client);

    return (result == DsVeosCoSim_Result_Ok) ? 0 : 1;
}
