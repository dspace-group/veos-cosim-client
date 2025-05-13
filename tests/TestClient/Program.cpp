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

void PrintStatus(const bool value, const std::string& what) {
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

void WriteOutGoingSignal(const IoSignal& ioSignal) {
    const size_t length = GetDataTypeSize(ioSignal.dataType) * ioSignal.length;
    const std::vector<uint8_t> data = GenerateBytes(length);

    Client->Write(ioSignal.id, ioSignal.length, data.data());
}

void TransmitCanMessage(const CanController& controller) {
    CanMessageContainer message{};
    FillWithRandom(message, controller.id);

    (void)Client->Transmit(Convert(message));
}

void TransmitEthMessage(const EthController& controller) {
    EthMessageContainer message{};
    FillWithRandom(message, controller.id);

    (void)Client->Transmit(Convert(message));
}

void TransmitLinMessage(const LinController& controller) {
    LinMessageContainer message{};
    FillWithRandom(message, controller.id);

    (void)Client->Transmit(Convert(message));
}

void SendSomeData(const SimulationTime simulationTime) {
    static SimulationTime lastHalfSecond = -1s;
    static int64_t counter = 0;
    const SimulationTime currentHalfSecond = simulationTime / 500000000;
    if (currentHalfSecond == lastHalfSecond) {
        return;
    }

    lastHalfSecond = currentHalfSecond;
    counter++;

    if (SendIoData && ((counter % 4) == 0)) {
        for (const IoSignal& signal : Client->GetOutgoingSignals()) {
            WriteOutGoingSignal(signal);
        }
    }

    if (SendCanMessages && ((counter % 4) == 1)) {
        for (const CanController& controller : Client->GetCanControllers()) {
            TransmitCanMessage(controller);
        }
    }

    if (SendEthMessages && ((counter % 4) == 2)) {
        for (const EthController& controller : Client->GetEthControllers()) {
            TransmitEthMessage(controller);
        }
    }

    if (SendLinMessages && ((counter % 4) == 3)) {
        for (const LinController& controller : Client->GetLinControllers()) {
            TransmitLinMessage(controller);
        }
    }
}

void OnSimulationPostStepCallback(const SimulationTime simulationTime) {
    SendSomeData(simulationTime);
}

void StartSimulationThread(const std::function<void()>& function) {
    SimulationThread = std::thread(function);
    SimulationThread.detach();
}

void OnSimulationStartedCallback(const SimulationTime simulationTime) {
    LogInfo("Simulation started at {} s.", SimulationTimeToString(simulationTime));
}

void OnSimulationStoppedCallback(const SimulationTime simulationTime) {
    LogInfo("Simulation stopped at {} s.", SimulationTimeToString(simulationTime));
}

void OnSimulationTerminatedCallback(const SimulationTime simulationTime, const TerminateReason reason) {
    LogInfo("Simulation terminated with reason {} at {} s.", ToString(reason), SimulationTimeToString(simulationTime));
}

void OnSimulationPausedCallback(const SimulationTime simulationTime) {
    LogInfo("Simulation paused at {} s.", SimulationTimeToString(simulationTime));
}

void OnSimulationContinuedCallback(const SimulationTime simulationTime) {
    LogInfo("Simulation continued at {} s.", SimulationTimeToString(simulationTime));
}

void Connect(const std::string_view host, const std::string_view serverName) {
    LogInfo("Connecting ...");

    const ConnectionState connectionState = Client->GetConnectionState();
    if (connectionState == ConnectionState::Connected) {
        LogInfo("Already connected.");
        return;
    }

    ConnectConfig connectConfig{};
    connectConfig.clientName = "Example Test Client";
    connectConfig.serverName = serverName;
    connectConfig.remoteIpAddress = host;

    if (!Client->Connect(connectConfig)) {
        LogError("Could not connect.");
        return;
    }

    LogTrace("");

    const SimulationTime stepSize = Client->GetStepSize();
    LogTrace("Step size: {} s", SimulationTimeToString(stepSize));
    LogTrace("");

    const std::vector<CanController>& canControllers = Client->GetCanControllers();
    if (!canControllers.empty()) {
        LogTrace("Found the following CAN controllers:");
        for (const CanController& controller : canControllers) {
            LogTrace("  {}", ToString(controller));
        }

        LogTrace("");
    }

    const std::vector<EthController>& ethControllers = Client->GetEthControllers();
    if (!ethControllers.empty()) {
        LogTrace("Found the following ETH controllers:");
        for (const EthController& controller : ethControllers) {
            LogTrace("  {}", ToString(controller));
        }

        LogTrace("");
    }

    const std::vector<LinController>& linControllers = Client->GetLinControllers();
    if (!linControllers.empty()) {
        LogTrace("Found the following LIN controllers:");
        for (const LinController& controller : linControllers) {
            LogTrace("  {}", ToString(controller));
        }

        LogTrace("");
    }

    const std::vector<IoSignal>& incomingSignals = Client->GetIncomingSignals();
    if (!incomingSignals.empty()) {
        LogTrace("Found the following incoming signals:");
        for (const IoSignal& signal : incomingSignals) {
            LogTrace("  {}", ToString(signal));
        }

        LogTrace("");
    }

    const std::vector<IoSignal>& outgoingSignals = Client->GetOutgoingSignals();
    if (!incomingSignals.empty()) {
        LogTrace("Found the following outgoing signals:");
        for (const IoSignal& signal : outgoingSignals) {
            LogTrace("  {}", ToString(signal));
        }

        LogTrace("");
    }

    LogInfo("Connected.");
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
    callbacks.incomingSignalChangedCallback = LogIoData;
    callbacks.canMessageReceivedCallback = LogCanMessage;
    callbacks.ethMessageReceivedCallback = LogEthMessage;
    callbacks.linMessageReceivedCallback = LogLinMessage;

    LogInfo("Running callback-based co-simulation ...");
    if (Client->RunCallbackBasedCoSimulation(callbacks)) {
        LogError("Callback-based co-simulation finished successfully.");
        exit(0);
    }

    LogError("Callback-based co-simulation finished with an error.");
    exit(1);
}

void HostClient(const std::string_view host, const std::string_view name) {
    Connect(host, name);

    StartSimulationThread(RunCallbackBasedCoSimulation);

    while (true) {
        switch (GetChar()) {
            case CTRL('c'):
                Disconnect();
                return;
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
                Client->Start();
                break;
            case 'o':
                Client->Stop();
                break;
            case 'p':
                Client->Pause();
                break;
            case 't':
                Client->Terminate(TerminateReason::Error);
                break;
            case 'n':
                Client->Continue();
                break;
            default:
                LogError("Unknown key.");
                break;
        }
    }
}

}  // namespace

int32_t main(const int32_t argc, char** argv) {
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

    Client = CreateClient();
    if (!Client) {
        LogError("Could not create handle.");
        return 1;
    }

    HostClient(host, name);

    Client.reset();

    return 0;
}
