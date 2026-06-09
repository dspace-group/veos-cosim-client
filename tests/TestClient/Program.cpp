// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include <DsVeosCoSim/DsVeosCoSim.h>
#include <Logger.hpp>

#include "Helper.hpp"

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

class SimulationState {
public:
    void Set(DsVeosCoSim_SimulationState newState) {
        std::scoped_lock lock(_mutex);
        _state = newState;
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

struct ClientData {
    DsVeosCoSim_Handle client{};

    SimulationState simState;

    std::vector<DsVeosCoSim_IoSignal> outgoingSignals;
    std::vector<DsVeosCoSim_CanController> canControllers;
    std::vector<DsVeosCoSim_EthController> ethControllers;
    std::vector<DsVeosCoSim_LinController> linControllers;
    std::vector<DsVeosCoSim_FrController> frControllers;

    bool sendIoData{};
    bool sendCanMessages{};
    bool sendEthMessages{};
    bool sendLinMessages{};
    bool sendFrMessages{};
    bool printRoundTripTime{};
    bool printStepsPerSecond{};

    std::chrono::steady_clock::time_point lastRoundTripTimePrinted;

    int64_t performanceStepCount{};
    std::chrono::steady_clock::time_point performanceMeasurementStart;
    bool performanceMeasurementActive{};
    std::chrono::steady_clock::time_point lastPerformancePrintTime;

    DsVeosCoSim_SimulationTime sendLastHalfSecond = -1;
    int64_t sendCounter{};
};

[[nodiscard]] DsVeosCoSim_Result Disconnect(const ClientData& clientData);

void PrintCurrentRoundTripTime(ClientData& clientData) {
    if (!clientData.printRoundTripTime) {
        return;
    }

    auto now = std::chrono::steady_clock::now();

    if (now - clientData.lastRoundTripTimePrinted < 1s) {
        return;
    }

    clientData.lastRoundTripTimePrinted = now;

    int64_t roundTripTimeInNanoseconds{};
    if (DsVeosCoSim_GetRoundTripTime(clientData.client, &roundTripTimeInNanoseconds) != DsVeosCoSim_Result_Ok) {
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

void SwitchSendingIoSignals(ClientData& clientData) {
    clientData.sendIoData = !clientData.sendIoData;
    PrintStatus(clientData.sendIoData, "IO data");
}

void SwitchSendingCanMessages(ClientData& clientData) {
    clientData.sendCanMessages = !clientData.sendCanMessages;
    PrintStatus(clientData.sendCanMessages, "CAN messages");
}

void SwitchSendingEthMessages(ClientData& clientData) {
    clientData.sendEthMessages = !clientData.sendEthMessages;
    PrintStatus(clientData.sendEthMessages, "Ethernet messages");
}

void SwitchSendingLinMessages(ClientData& clientData) {
    clientData.sendLinMessages = !clientData.sendLinMessages;
    PrintStatus(clientData.sendLinMessages, "LIN messages");
}

void SwitchSendingFrMessages(ClientData& clientData) {
    clientData.sendFrMessages = !clientData.sendFrMessages;
    PrintStatus(clientData.sendFrMessages, "FlexRay messages");
}

void SwitchPrintingRoundTripTime(ClientData& clientData) {
    clientData.printRoundTripTime = !clientData.printRoundTripTime;
    if (clientData.printRoundTripTime) {
        LogInfo("Enabled Printing Round-Trip Time.");
    } else {
        LogInfo("Disabled Printing Round-Trip Time.");
    }
}

void StartPerformanceMeasurement(ClientData& clientData) {
    clientData.performanceStepCount = 0;
    clientData.performanceMeasurementStart = std::chrono::steady_clock::now();
    clientData.performanceMeasurementActive = true;
    clientData.lastPerformancePrintTime = {};
    clientData.sendLastHalfSecond = -1;
    clientData.sendCounter = 0;
}

void StopPerformanceMeasurement(ClientData& clientData) {
    if (!clientData.performanceMeasurementActive) {
        return;
    }

    clientData.performanceMeasurementActive = false;

    double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - clientData.performanceMeasurementStart).count();
    if (seconds > 0.0) {
        LogInfo("Performance: {:.1f} steps per second average ({} steps in {:.3f} s).",
                static_cast<double>(clientData.performanceStepCount) / seconds,
                clientData.performanceStepCount,
                seconds);
    }
}

void PrintCurrentStepsPerSecond(ClientData& clientData) {
    if (!clientData.performanceMeasurementActive || !clientData.printStepsPerSecond) {
        return;
    }

    auto now = std::chrono::steady_clock::now();

    if (now - clientData.lastPerformancePrintTime < 1s) {
        return;
    }

    clientData.lastPerformancePrintTime = now;

    double seconds = std::chrono::duration<double>(now - clientData.performanceMeasurementStart).count();
    if (seconds > 0.0) {
        LogInfo("Steps per second: {:.1f} ({} steps in {:.1f} s).",
                static_cast<double>(clientData.performanceStepCount) / seconds,
                clientData.performanceStepCount,
                seconds);
    }
}

void SwitchPrintingStepsPerSecond(ClientData& clientData) {
    clientData.printStepsPerSecond = !clientData.printStepsPerSecond;
    if (clientData.printStepsPerSecond) {
        LogInfo("Enabled printing steps per second.");
    } else {
        LogInfo("Disabled printing steps per second.");
    }
}

[[nodiscard]] DsVeosCoSim_Result WriteOutGoingSignal(const ClientData& clientData, const DsVeosCoSim_IoSignal& ioSignal) {
    size_t length = DsVeosCoSim_GetDataTypeSize(ioSignal.dataType) * ioSignal.length;
    std::vector<uint8_t> bytes = GenerateBytes(length);

    return DsVeosCoSim_WriteOutgoingSignal(clientData.client, ioSignal.id, ioSignal.length, bytes.data());
}

[[nodiscard]] DsVeosCoSim_Result TransmitCanMessage(const ClientData& clientData, const DsVeosCoSim_CanController& controller) {
    uint32_t length = GenerateRandom(1U, 8U);
    DsVeosCoSim_CanMessageContainer messageContainer{};
    messageContainer.controllerId = controller.id;
    messageContainer.id = GenerateU32();
    messageContainer.timestamp = GenerateI64();
    messageContainer.length = length;
    FillWithRandomData(&messageContainer.data[0], length);

    return DsVeosCoSim_TransmitCanMessageContainer(clientData.client, &messageContainer);
}

[[nodiscard]] DsVeosCoSim_Result TransmitEthMessage(const ClientData& clientData, const DsVeosCoSim_EthController& controller) {
    uint32_t length = GenerateRandom(1U, 8U);
    DsVeosCoSim_EthMessageContainer messageContainer{};
    messageContainer.controllerId = controller.id;
    messageContainer.timestamp = GenerateI64();
    messageContainer.length = length;
    FillWithRandomData(messageContainer.data, length);

    return DsVeosCoSim_TransmitEthMessageContainer(clientData.client, &messageContainer);
}

[[nodiscard]] DsVeosCoSim_Result TransmitLinMessage(const ClientData& clientData, const DsVeosCoSim_LinController& controller) {
    uint32_t length = GenerateRandom(1U, 8U);
    DsVeosCoSim_LinMessageContainer messageContainer{};
    messageContainer.controllerId = controller.id;
    messageContainer.id = GenerateU32();
    messageContainer.timestamp = GenerateI64();
    messageContainer.length = length;
    FillWithRandomData(messageContainer.data, length);

    return DsVeosCoSim_TransmitLinMessageContainer(clientData.client, &messageContainer);
}

[[nodiscard]] DsVeosCoSim_Result TransmitFrMessage(const ClientData& clientData, const DsVeosCoSim_FrController& controller) {
    uint32_t length = GenerateRandom(1U, 8U);
    DsVeosCoSim_FrMessageContainer messageContainer{};
    messageContainer.controllerId = controller.id;
    messageContainer.id = GenerateU32();
    messageContainer.timestamp = GenerateI64();
    messageContainer.length = length;
    FillWithRandomData(messageContainer.data, length);

    return DsVeosCoSim_TransmitFrMessageContainer(clientData.client, &messageContainer);
}

[[nodiscard]] DsVeosCoSim_Result SendSomeData(ClientData& clientData, DsVeosCoSim_SimulationTime simulationTime) {
    DsVeosCoSim_SimulationTime currentHalfSecond = simulationTime / 500000000;
    if (currentHalfSecond == clientData.sendLastHalfSecond) {
        return DsVeosCoSim_Result_Ok;
    }

    clientData.sendLastHalfSecond = currentHalfSecond;
    clientData.sendCounter++;

    if (clientData.sendIoData && ((clientData.sendCounter % 5) == 0)) {
        for (const DsVeosCoSim_IoSignal& signal : clientData.outgoingSignals) {
            CheckDsVeosCoSimResult(WriteOutGoingSignal(clientData, signal));
        }
    }

    if (clientData.sendCanMessages && ((clientData.sendCounter % 5) == 1)) {
        for (const DsVeosCoSim_CanController& controller : clientData.canControllers) {
            CheckDsVeosCoSimResult(TransmitCanMessage(clientData, controller));
        }
    }

    if (clientData.sendEthMessages && ((clientData.sendCounter % 5) == 2)) {
        for (const DsVeosCoSim_EthController& controller : clientData.ethControllers) {
            CheckDsVeosCoSimResult(TransmitEthMessage(clientData, controller));
        }
    }

    if (clientData.sendLinMessages && ((clientData.sendCounter % 5) == 3)) {
        for (const DsVeosCoSim_LinController& controller : clientData.linControllers) {
            CheckDsVeosCoSimResult(TransmitLinMessage(clientData, controller));
        }
    }

    if (clientData.sendFrMessages && ((clientData.sendCounter % 5) == 4)) {
        for (const DsVeosCoSim_FrController& controller : clientData.frControllers) {
            CheckDsVeosCoSimResult(TransmitFrMessage(clientData, controller));
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

[[nodiscard]] DsVeosCoSim_Result Connect(ClientData& clientData, const std::string& host, const std::string& serverName) {
    LogInfo("Connecting ...");

    DsVeosCoSim_ConnectionState connectionState{};
    CheckDsVeosCoSimResult(DsVeosCoSim_GetConnectionState(clientData.client, &connectionState));
    if (connectionState == DsVeosCoSim_ConnectionState_Connected) {
        LogInfo("Already connected.");
        return DsVeosCoSim_Result_Ok;
    }

    DsVeosCoSim_ConnectConfig connectConfig{};
    connectConfig.clientName = "Example Test Client";
    connectConfig.serverName = serverName.c_str();
    connectConfig.remoteIpAddress = host.c_str();

    if (DsVeosCoSim_Connect(clientData.client, connectConfig) != DsVeosCoSim_Result_Ok) {
        LogError("Could not connect.");
        return DsVeosCoSim_Result_Error;
    }

    LogTrace("");

    DsVeosCoSim_SimulationTime stepSize{};
    CheckDsVeosCoSimResult(DsVeosCoSim_GetStepSize(clientData.client, &stepSize));
    LogTrace("Step size: {} s", DsVeosCoSim_SimulationTimeToString(stepSize));
    LogTrace("");

    DsVeosCoSim_SimulationState initialState{};
    CheckDsVeosCoSimResult(DsVeosCoSim_GetSimulationState(clientData.client, &initialState));

    // This can happen with old clients. In that case we assume the state stopped, so that at least the simulation start
    // can be passed to the server
    if (initialState == DsVeosCoSim_SimulationState_Unloaded) {
        initialState = DsVeosCoSim_SimulationState_Stopped;
    }

    clientData.simState.Set(initialState);

    uint32_t tmpCanControllersCount{};
    const DsVeosCoSim_CanController* tmpCanControllers{};
    CheckDsVeosCoSimResult(DsVeosCoSim_GetCanControllers(clientData.client, &tmpCanControllersCount, &tmpCanControllers));
    if (tmpCanControllersCount > 0) {
        clientData.canControllers = std::vector(tmpCanControllers, tmpCanControllers + tmpCanControllersCount);
        LogTrace("Found the following CAN controllers:");
        for (const DsVeosCoSim_CanController& controller : clientData.canControllers) {
            LogTrace("  {}", DsVeosCoSim_CanControllerToString(&controller));
        }

        LogTrace("");
    }

    uint32_t tmpEthControllersCount{};
    const DsVeosCoSim_EthController* tmpEthControllers{};
    CheckDsVeosCoSimResult(DsVeosCoSim_GetEthControllers(clientData.client, &tmpEthControllersCount, &tmpEthControllers));
    if (tmpEthControllersCount > 0) {
        clientData.ethControllers = std::vector(tmpEthControllers, tmpEthControllers + tmpEthControllersCount);
        LogTrace("Found the following Ethernet controllers:");
        for (const DsVeosCoSim_EthController& controller : clientData.ethControllers) {
            LogTrace("  {}", DsVeosCoSim_EthControllerToString(&controller));
        }

        LogTrace("");
    }

    uint32_t tmpLinControllersCount{};
    const DsVeosCoSim_LinController* tmpLinControllers{};
    CheckDsVeosCoSimResult(DsVeosCoSim_GetLinControllers(clientData.client, &tmpLinControllersCount, &tmpLinControllers));
    if (tmpLinControllersCount > 0) {
        clientData.linControllers = std::vector(tmpLinControllers, tmpLinControllers + tmpLinControllersCount);
        LogTrace("Found the following LIN controllers:");
        for (const DsVeosCoSim_LinController& controller : clientData.linControllers) {
            LogTrace("  {}", DsVeosCoSim_LinControllerToString(&controller));
        }

        LogTrace("");
    }

    uint32_t tmpFrControllersCount{};
    const DsVeosCoSim_FrController* tmpFrControllers{};
    CheckDsVeosCoSimResult(DsVeosCoSim_GetFrControllers(clientData.client, &tmpFrControllersCount, &tmpFrControllers));
    if (tmpFrControllersCount > 0) {
        clientData.frControllers = std::vector(tmpFrControllers, tmpFrControllers + tmpFrControllersCount);
        LogTrace("Found the following FlexRay controllers:");
        for (const DsVeosCoSim_FrController& controller : clientData.frControllers) {
            LogTrace("  {}", DsVeosCoSim_FrControllerToString(&controller));
        }

        LogTrace("");
    }

    uint32_t tmpIncomingSignalsCount{};
    const DsVeosCoSim_IoSignal* tmpIncomingSignals{};
    CheckDsVeosCoSimResult(DsVeosCoSim_GetIncomingSignals(clientData.client, &tmpIncomingSignalsCount, &tmpIncomingSignals));
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
    CheckDsVeosCoSimResult(DsVeosCoSim_GetOutgoingSignals(clientData.client, &tmpOutgoingSignalsCount, &tmpOutgoingSignals));
    if (tmpOutgoingSignalsCount > 0) {
        clientData.outgoingSignals = std::vector(tmpOutgoingSignals, tmpOutgoingSignals + tmpOutgoingSignalsCount);
        LogTrace("Found the following outgoing signals:");
        for (const DsVeosCoSim_IoSignal& signal : clientData.outgoingSignals) {
            LogTrace("  {}", DsVeosCoSim_IoSignalToString(&signal));
        }

        LogTrace("");
    }

    LogInfo("Connected.");
    return DsVeosCoSim_Result_Ok;
}

[[nodiscard]] DsVeosCoSim_Result Disconnect(const ClientData& clientData) {
    LogInfo("Disconnecting ...");
    CheckDsVeosCoSimResult(DsVeosCoSim_Disconnect(clientData.client));
    LogInfo("Disconnected.");

    return DsVeosCoSim_Result_Ok;
}

[[nodiscard]] DsVeosCoSim_Result RunPollingBasedCoSimulation(ClientData& clientData) {
    DsVeosCoSim_Callbacks callbacks{};
    callbacks.incomingSignalChangedCallback = OnIncomingSignalChanged;
    callbacks.canMessageContainerReceivedCallback = OnCanMessageContainerReceived;
    callbacks.ethMessageContainerReceivedCallback = OnEthMessageContainerReceived;
    callbacks.linMessageContainerReceivedCallback = OnLinMessageContainerReceived;
    callbacks.frMessageContainerReceivedCallback = OnFrMessageContainerReceived;

    CheckDsVeosCoSimResult(DsVeosCoSim_StartPollingBasedCoSimulation(clientData.client, callbacks));

    LogInfo("Running polling-based co-simulation ...");

    while (true) {
        DsVeosCoSim_SimulationTime simulationTime{};
        DsVeosCoSim_Command command{};

        DsVeosCoSim_Result result = DsVeosCoSim_PollCommand2(clientData.client, &simulationTime, &command, 10);

        if (result == DsVeosCoSim_Result_Disconnected) {
            StopPerformanceMeasurement(clientData);
            LogInfo("Polling-based co-simulation finished successfully.");
            return DsVeosCoSim_Result_Ok;
        }

        PrintCurrentRoundTripTime(clientData);

        if (result == DsVeosCoSim_Result_Timeout) {
            continue;
        }

        if (result != DsVeosCoSim_Result_Ok) {
            StopPerformanceMeasurement(clientData);
            LogError("Polling-based co-simulation finished with an error.");
            clientData.simState.Set(DsVeosCoSim_SimulationState_Unloaded);
            return DsVeosCoSim_Result_Error;
        }

        switch (command) {
            case DsVeosCoSim_Command_Start:
                LogInfo("Simulation started at {} s.", DsVeosCoSim_SimulationTimeToString(simulationTime));
                clientData.simState.Set(DsVeosCoSim_SimulationState_Running);
                StartPerformanceMeasurement(clientData);
                break;
            case DsVeosCoSim_Command_Stop:
                LogInfo("Simulation stopped at {} s.", DsVeosCoSim_SimulationTimeToString(simulationTime));
                StopPerformanceMeasurement(clientData);
                clientData.simState.Set(DsVeosCoSim_SimulationState_Stopped);
                break;
            case DsVeosCoSim_Command_Terminate:
            case DsVeosCoSim_Command_TerminateFinished:
                LogInfo("Simulation terminated at {} s.", DsVeosCoSim_SimulationTimeToString(simulationTime));
                StopPerformanceMeasurement(clientData);
                clientData.simState.Set(DsVeosCoSim_SimulationState_Terminated);
                break;
            case DsVeosCoSim_Command_Pause:
                LogInfo("Simulation paused at {} s.", DsVeosCoSim_SimulationTimeToString(simulationTime));
                StopPerformanceMeasurement(clientData);
                clientData.simState.Set(DsVeosCoSim_SimulationState_Paused);
                break;
            case DsVeosCoSim_Command_Continue:
                LogInfo("Simulation continued at {} s.", DsVeosCoSim_SimulationTimeToString(simulationTime));
                clientData.simState.Set(DsVeosCoSim_SimulationState_Running);
                StartPerformanceMeasurement(clientData);
                break;
            case DsVeosCoSim_Command_Step:
                clientData.performanceStepCount++;
                PrintCurrentStepsPerSecond(clientData);

                if (SendSomeData(clientData, simulationTime) != DsVeosCoSim_Result_Ok) {
                    LogError("Could not send data.");
                }
                break;
            default:
                break;
        }

        CheckDsVeosCoSimResult(DsVeosCoSim_FinishCommand(clientData.client));
    }
}

[[nodiscard]] DsVeosCoSim_Result ToggleSimulation(ClientData& clientData) {
    return clientData.simState.WithLock([&clientData](DsVeosCoSim_SimulationState state) -> DsVeosCoSim_Result {
        switch (state) {
            case DsVeosCoSim_SimulationState_Running:
                LogInfo("Pausing simulation ...");
                return DsVeosCoSim_PauseSimulation(clientData.client);
            case DsVeosCoSim_SimulationState_Stopped:
                LogInfo("Starting simulation ...");
                return DsVeosCoSim_StartSimulation(clientData.client);
            case DsVeosCoSim_SimulationState_Paused:
                LogInfo("Continuing simulation ...");
                return DsVeosCoSim_ContinueSimulation(clientData.client);
            default:
                LogError("Cannot start or pause in state {}.", DsVeosCoSim_SimulationStateToString(state));
                return DsVeosCoSim_Result_Ok;
        }
    });
}

[[nodiscard]] DsVeosCoSim_Result Stop(ClientData& clientData) {
    return clientData.simState.WithLock([&clientData](DsVeosCoSim_SimulationState state) -> DsVeosCoSim_Result {
        switch (state) {
            case DsVeosCoSim_SimulationState_Stopped:
                return DsVeosCoSim_Result_Ok;
            case DsVeosCoSim_SimulationState_Running:
            case DsVeosCoSim_SimulationState_Paused:
                LogInfo("Stopping simulation ...");
                return DsVeosCoSim_StopSimulation(clientData.client);
            default:
                LogError("Cannot stop in state {}.", DsVeosCoSim_SimulationStateToString(state));
                return DsVeosCoSim_Result_Ok;
        }
    });
}

[[nodiscard]] DsVeosCoSim_Result Terminate(ClientData& clientData) {
    return clientData.simState.WithLock([&clientData](DsVeosCoSim_SimulationState state) -> DsVeosCoSim_Result {
        switch (state) {
            case DsVeosCoSim_SimulationState_Terminated:
                return DsVeosCoSim_Result_Ok;
            case DsVeosCoSim_SimulationState_Running:
            case DsVeosCoSim_SimulationState_Paused:
            case DsVeosCoSim_SimulationState_Stopped:
                LogInfo("Terminating simulation ...");
                return DsVeosCoSim_TerminateSimulation(clientData.client, DsVeosCoSim_TerminateReason_Error);
            default:
                LogError("Cannot terminate in state {}.", DsVeosCoSim_SimulationStateToString(state));
                return DsVeosCoSim_Result_Ok;
        }
    });
}

DsVeosCoSim_Result HandleUserInput(ClientData& clientData) {
    while (true) {
        switch (GetChar()) {
            case CTRL('c'):
                return Disconnect(clientData);
            case '1':
                SwitchSendingIoSignals(clientData);
                break;
            case '2':
                SwitchSendingCanMessages(clientData);
                break;
            case '3':
                SwitchSendingEthMessages(clientData);
                break;
            case '4':
                SwitchSendingLinMessages(clientData);
                break;
            case '5':
                SwitchSendingFrMessages(clientData);
                break;
            case '0':
                SwitchPrintingRoundTripTime(clientData);
                break;
            case '9':
                SwitchPrintingStepsPerSecond(clientData);
                break;
            case 's':
            case 'p':
            case 'k':
            case ' ':
                CheckDsVeosCoSimResult(ToggleSimulation(clientData));
                break;
            case 't':
                CheckDsVeosCoSimResult(Stop(clientData));
                break;
            case CTRL('t'):
                CheckDsVeosCoSimResult(Terminate(clientData));
                break;
            default:
                LogError("Unknown key.");
                break;
        }
    }
}

[[nodiscard]] DsVeosCoSim_Result HostClient(ClientData& clientData, const std::string& host, const std::string& name) {
    CheckDsVeosCoSimResult(Connect(clientData, host, name));

    // Stopping the listening for a character is complicated, so we just detach the thread
    std::thread([&clientData] {
        HandleUserInput(clientData);
    }).detach();

    return RunPollingBasedCoSimulation(clientData);
}

}  // namespace

int main(int argc, char** argv) {
    try {
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

        ClientData clientData{};

        clientData.client = DsVeosCoSim_Create();
        if (!clientData.client) {
            LogError("Could not create handle.");
            return 1;
        }

        DsVeosCoSim_Result result = HostClient(clientData, host, name);

        DsVeosCoSim_Destroy(clientData.client);

        return (result == DsVeosCoSim_Result_Ok) ? 0 : 1;
    } catch (...) {
        return 1;
    }
}
