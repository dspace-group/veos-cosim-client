// Copyright dSPACE GmbH. All rights reserved.

#include <cstdint>
#include <cstring>
#include <functional>
#include <string>
#include <thread>
#include <vector>

#include "ClientServerTestHelper.h"
#include "CoSimClient.h"
#include "CoSimTypes.h"
#include "Helper.h"
#include "LogHelper.h"
#include "Logger.h"

using namespace DsVeosCoSim;

namespace {

std::unique_ptr<CoSimClient> g_coSimClient;

RunTimeInfo g_runTimeInfo;

std::thread g_simulationThread;

void OnSimulationPostStepCallback(DsVeosCoSim_SimulationTime simulationTime) {
    (void)SendSomeData(simulationTime, g_runTimeInfo);
}

void StartSimulationThread(const std::function<void()>& function) {
    g_simulationThread = std::thread(function);
    g_simulationThread.detach();
}

void OnSimulationStartedCallback(DsVeosCoSim_SimulationTime simulationTime) {
    LogInfo("Simulation started at {} s.", SimulationTimeToSeconds(simulationTime));
}

void OnSimulationStoppedCallback(DsVeosCoSim_SimulationTime simulationTime) {
    LogInfo("Simulation stopped at {} s.", SimulationTimeToSeconds(simulationTime));
}

void OnSimulationTerminatedCallback(DsVeosCoSim_SimulationTime simulationTime, DsVeosCoSim_TerminateReason reason) {
    LogInfo("Simulation terminated with reason {} at {} s.", ToString(reason), SimulationTimeToSeconds(simulationTime));
}

void OnSimulationPausedCallback(DsVeosCoSim_SimulationTime simulationTime) {
    LogInfo("Simulation paused at {} s.", SimulationTimeToSeconds(simulationTime));
}

void OnSimulationContinuedCallback(DsVeosCoSim_SimulationTime simulationTime) {
    LogInfo("Simulation continued at {} s.", SimulationTimeToSeconds(simulationTime));
}

[[nodiscard]] bool Connect(std::string_view host, std::string_view serverName) {
    LogInfo("Connecting ...");
    if (g_coSimClient->GetConnectionState() == DsVeosCoSim_ConnectionState_Connected) {
        LogInfo("Already connected.");
        return true;
    }

    ConnectConfig connectConfig{};
    connectConfig.clientName = "Example Test Client";
    connectConfig.serverName = serverName;
    connectConfig.remoteIpAddress = host;
    CheckResultWithMessage(g_coSimClient->Connect(connectConfig), "Could not connect.");

    g_runTimeInfo.canControllers = g_coSimClient->GetCanControllers();
    if (!g_runTimeInfo.canControllers.empty()) {
        LogTrace("Found the following CAN controllers:");
        for (const CanController& controller : g_runTimeInfo.canControllers) {
            LogCanController(controller);
        }

        LogTrace("");
    }

    g_runTimeInfo.ethControllers = g_coSimClient->GetEthControllers();
    if (!g_runTimeInfo.ethControllers.empty()) {
        LogTrace("Found the following ethernet controllers:");
        for (const EthController& controller : g_runTimeInfo.ethControllers) {
            LogEthController(controller);
        }

        LogTrace("");
    }

    g_runTimeInfo.linControllers = g_coSimClient->GetLinControllers();
    if (!g_runTimeInfo.linControllers.empty()) {
        LogTrace("Found the following LIN controllers:");
        for (const LinController& controller : g_runTimeInfo.linControllers) {
            LogLinController(controller);
        }

        LogTrace("");
    }

    g_runTimeInfo.incomingSignals = g_coSimClient->GetIncomingSignals();
    if (!g_runTimeInfo.incomingSignals.empty()) {
        LogTrace("Found the following read signals:");
        for (const IoSignal& signal : g_runTimeInfo.incomingSignals) {
            LogIoSignal(signal);
        }

        LogTrace("");
    }

    g_runTimeInfo.outgoingSignals = g_coSimClient->GetOutgoingSignals();
    if (!g_runTimeInfo.outgoingSignals.empty()) {
        LogTrace("Found the following write signals:");
        for (const IoSignal& signal : g_runTimeInfo.outgoingSignals) {
            LogIoSignal(signal);
        }

        LogTrace("");
    }

    g_runTimeInfo.transmitCan = [&](const DsVeosCoSim_CanMessage& message) {
        return g_coSimClient->Transmit(message);
    };
    g_runTimeInfo.transmitEth = [&](const DsVeosCoSim_EthMessage& message) {
        return g_coSimClient->Transmit(message);
    };
    g_runTimeInfo.transmitLin = [&](const DsVeosCoSim_LinMessage& message) {
        return g_coSimClient->Transmit(message);
    };
    g_runTimeInfo.write = [&](DsVeosCoSim_IoSignalId signalId, uint32_t length, const void* value) {
        return g_coSimClient->Write(signalId, length, value);
    };

    LogInfo("Connected.");
    return true;
}

void Disconnect() {
    LogInfo("Disconnecting ...");
    g_coSimClient->Disconnect();
    LogInfo("Disconnected.");
}

void RunCallbackBasedCoSimulation() {
    try {
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
        if (g_coSimClient->RunCallbackBasedCoSimulation(callbacks)) {
            exit(0);
        }

        exit(1);
    } catch (const std::exception& e) {
        LogError(e.what());
    }
}

void HostClient(std::string_view host, std::string_view serverName) {
    if (!Connect(host, serverName)) {
        return;
    }

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
                g_coSimClient->Start();
                break;
            case 'o':
                g_coSimClient->Stop();
                break;
            case 'p':
                g_coSimClient->Pause();
                break;
            case 't':
                g_coSimClient->Terminate(DsVeosCoSim_TerminateReason_Error);
                break;
            case 'n':
                g_coSimClient->Continue();
                break;
            default:
                LogError("Unknown key.");
                break;
        }
    }
}

}  // namespace

int32_t main(int32_t argc, char** argv) {
    if (!StartUp()) {
        return 1;
    }

    std::string host;
    std::string serverName = "CoSimProxy";

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
                serverName = argv[i];
            } else {
                LogError("No name specified.");
                return 1;
            }
        }
    }

    g_coSimClient = std::make_unique<CoSimClient>();

    HostClient(host, serverName);

    Disconnect();
    g_coSimClient.reset();

    return 0;
}
