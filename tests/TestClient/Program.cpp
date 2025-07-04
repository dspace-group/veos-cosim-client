// Copyright dSPACE GmbH. All rights reserved.

#include <chrono>
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "DsVeosCoSim/CoSimClient.h"
#include "DsVeosCoSim/CoSimTypes.h"
#include "Generator.h"
#include "Helper.h"
#include "LogHelper.h"

using namespace DsVeosCoSim;
using namespace std::chrono_literals;

namespace {

std::unique_ptr<CoSimClient> Client;

std::thread SimulationThread;

bool SendIoData;
bool SendCanMessages;
bool SendEthMessages;
bool SendLinMessages;

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
    PrintStatus(SendEthMessages, "ETH messages");
}

void SwitchSendingLinMessages() {
    SendLinMessages = !SendLinMessages;
    PrintStatus(SendLinMessages, "LIN messages");
}

[[nodiscard]] Result WriteOutGoingSignal(const IoSignal& ioSignal) {
    size_t length = GetDataTypeSize(ioSignal.dataType) * ioSignal.length;
    std::vector<uint8_t> data = GenerateBytes(length);

    return Client->Write(ioSignal.id, ioSignal.length, data.data());
}

[[nodiscard]] Result TransmitCanMessage(const CanController& controller) {
    CanMessageContainer messageContainer{};
    FillWithRandom(messageContainer, controller.id);

    return Client->Transmit(messageContainer);
}

[[nodiscard]] Result TransmitEthMessage(const EthController& controller) {
    EthMessageContainer messageContainer{};
    FillWithRandom(messageContainer, controller.id);

    return Client->Transmit(messageContainer);
}

[[nodiscard]] Result TransmitLinMessage(const LinController& controller) {
    LinMessageContainer messageContainer{};
    FillWithRandom(messageContainer, controller.id);

    return Client->Transmit(messageContainer);
}

[[nodiscard]] Result SendSomeData(SimulationTime simulationTime) {
    static SimulationTime lastHalfSecond = -1s;
    static int64_t counter = 0;
    SimulationTime currentHalfSecond = simulationTime / 500000000;
    if (currentHalfSecond == lastHalfSecond) {
        return Result::Ok;
    }

    lastHalfSecond = currentHalfSecond;
    counter++;

    if (SendIoData && ((counter % 4) == 0)) {
        std::vector<IoSignal> signals;
        CheckResult(Client->GetOutgoingSignals(signals));
        for (const IoSignal& signal : signals) {
            CheckResult(WriteOutGoingSignal(signal));
        }
    }

    if (SendCanMessages && ((counter % 4) == 1)) {
        std::vector<CanController> controllers;
        CheckResult(Client->GetCanControllers(controllers));
        for (const CanController& controller : controllers) {
            CheckResult(TransmitCanMessage(controller));
        }
    }

    if (SendEthMessages && ((counter % 4) == 2)) {
        std::vector<EthController> controllers;
        CheckResult(Client->GetEthControllers(controllers));
        for (const EthController& controller : controllers) {
            CheckResult(TransmitEthMessage(controller));
        }
    }

    if (SendLinMessages && ((counter % 4) == 3)) {
        std::vector<LinController> controllers;
        CheckResult(Client->GetLinControllers(controllers));
        for (const LinController& controller : controllers) {
            CheckResult(TransmitLinMessage(controller));
        }
    }

    return Result::Ok;
}

void OnSimulationPostStepCallback(SimulationTime simulationTime) {
    if (!IsOk(SendSomeData(simulationTime))) {
        LogError("Could not send data.");
    }
}

void OnIncomingSignalChanged([[maybe_unused]] SimulationTime simulationTime,
                             const IoSignal& ioSignal,
                             uint32_t length,
                             const void* value) {
    LogIoData(ioSignal, length, value);
}

void OnCanMessageContainerReceived([[maybe_unused]] SimulationTime simulationTime,
                                   [[maybe_unused]] const CanController& controller,
                                   const CanMessageContainer& messageContainer) {
    LogCanMessageContainer(messageContainer);
}

void OnEthMessageContainerReceived([[maybe_unused]] SimulationTime simulationTime,
                                   [[maybe_unused]] const EthController& controller,
                                   const EthMessageContainer& messageContainer) {
    LogEthMessageContainer(messageContainer);
}

void OnLinMessageContainerReceived([[maybe_unused]] SimulationTime simulationTime,
                                   [[maybe_unused]] const LinController& controller,
                                   const LinMessageContainer& messageContainer) {
    LogLinMessageContainer(messageContainer);
}

void StartSimulationThread(const std::function<void()>& function) {
    SimulationThread = std::thread(function);
    SimulationThread.detach();
}

void OnSimulationStartedCallback(SimulationTime simulationTime) {
    LogInfo("Simulation started at {} s.", SimulationTimeToString(simulationTime));
}

void OnSimulationStoppedCallback(SimulationTime simulationTime) {
    LogInfo("Simulation stopped at {} s.", SimulationTimeToString(simulationTime));
}

void OnSimulationTerminatedCallback(SimulationTime simulationTime, TerminateReason reason) {
    LogInfo("Simulation terminated with reason {} at {} s.", ToString(reason), SimulationTimeToString(simulationTime));
}

void OnSimulationPausedCallback(SimulationTime simulationTime) {
    LogInfo("Simulation paused at {} s.", SimulationTimeToString(simulationTime));
}

void OnSimulationContinuedCallback(SimulationTime simulationTime) {
    LogInfo("Simulation continued at {} s.", SimulationTimeToString(simulationTime));
}

[[nodiscard]] Result Connect(std::string_view host, std::string_view serverName) {
    LogInfo("Connecting ...");

    ConnectionState connectionState{};
    CheckResult(Client->GetConnectionState(connectionState));
    if (connectionState == ConnectionState::Connected) {
        LogInfo("Already connected.");
        return Result::Ok;
    }

    ConnectConfig connectConfig{};
    connectConfig.clientName = "Example Test Client";
    connectConfig.serverName = serverName;
    connectConfig.remoteIpAddress = host;

    if (!IsOk(Client->Connect(connectConfig))) {
        LogError("Could not connect.");
        return Result::Error;
    }

    LogTrace("");

    SimulationTime stepSize{};
    CheckResult(Client->GetStepSize(stepSize));
    LogTrace("Step size: {} s", SimulationTimeToString(stepSize));
    LogTrace("");

    std::vector<CanController> canControllers;
    CheckResult(Client->GetCanControllers(canControllers));
    if (!canControllers.empty()) {
        LogTrace("Found the following CAN controllers:");
        for (const CanController& controller : canControllers) {
            LogTrace("  {}", ToString(controller));
        }

        LogTrace("");
    }

    std::vector<EthController> ethControllers;
    CheckResult(Client->GetEthControllers(ethControllers));
    if (!ethControllers.empty()) {
        LogTrace("Found the following ETH controllers:");
        for (const EthController& controller : ethControllers) {
            LogTrace("  {}", ToString(controller));
        }

        LogTrace("");
    }

    std::vector<LinController> linControllers;
    CheckResult(Client->GetLinControllers(linControllers));
    if (!linControllers.empty()) {
        LogTrace("Found the following LIN controllers:");
        for (const LinController& controller : linControllers) {
            LogTrace("  {}", ToString(controller));
        }

        LogTrace("");
    }

    std::vector<IoSignal> incomingSignals;
    CheckResult(Client->GetIncomingSignals(incomingSignals));
    if (!incomingSignals.empty()) {
        LogTrace("Found the following incoming signals:");
        for (const IoSignal& signal : incomingSignals) {
            LogTrace("  {}", ToString(signal));
        }

        LogTrace("");
    }

    std::vector<IoSignal> outgoingSignals;
    CheckResult(Client->GetOutgoingSignals(outgoingSignals));
    if (!incomingSignals.empty()) {
        LogTrace("Found the following outgoing signals:");
        for (const IoSignal& signal : outgoingSignals) {
            LogTrace("  {}", ToString(signal));
        }

        LogTrace("");
    }

    LogInfo("Connected.");
    return Result::Ok;
}

void Disconnect() {
    LogInfo("Disconnecting ...");
    Client->Disconnect();
    LogInfo("Disconnected.");
}

[[noreturn]] void RunCallbackBasedCoSimulation() {
    Callbacks callbacks{};
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

    LogInfo("Running callback-based co-simulation ...");
    if (!IsDisconnected(Client->RunCallbackBasedCoSimulation(callbacks))) {
        LogError("Callback-based co-simulation finished with an error.");
        exit(1);
    }

    LogError("Callback-based co-simulation finished successfully.");
    exit(0);
}

[[nodiscard]] Result HostClient(std::string_view host, std::string_view name) {
    CheckResult(Connect(host, name));

    StartSimulationThread(RunCallbackBasedCoSimulation);

    while (true) {
        switch (GetChar()) {
            case CTRL('c'):
                Disconnect();
                return Result::Ok;
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
            case 's':
                CheckResult(Client->Start());
                break;
            case 'o':
                CheckResult(Client->Stop());
                break;
            case 'p':
                CheckResult(Client->Pause());
                break;
            case 't':
                CheckResult(Client->Terminate(TerminateReason::Error));
                break;
            case 'n':
                CheckResult(Client->Continue());
                break;
            default:
                LogError("Unknown key.");
                break;
        }
    }

    return Result::Ok;
}

}  // namespace

int32_t main(int32_t argc, char** argv) {
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

    if (!IsOk(CreateClient(Client))) {
        LogError("Could not create handle.");
        return 1;
    }

    Result result = HostClient(host, name);

    Client.reset();

    return IsOk(result) ? 0 : 1;
}
