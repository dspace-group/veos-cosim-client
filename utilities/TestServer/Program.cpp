// Copyright dSPACE GmbH. All rights reserved.

#include <cstring>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "BackgroundService.h"
#include "ClientServerTestHelper.h"
#include "CoSimHelper.h"
#include "CoSimServer.h"
#include "CoSimTypes.h"
#include "Generator.h"
#include "Helper.h"
#include "LogHelper.h"

using namespace DsVeosCoSim;

using namespace std::chrono_literals;

namespace {

enum class State {
    Unloaded,
    Stopped,
    Running,
    Paused,
    Terminated
};

bool g_stopSimulationThread;
std::thread g_simulationThread;

DsVeosCoSim_SimulationTime g_currentTime;

RunTimeInfo g_runTimeInfo;

std::unique_ptr<CoSimServer> g_server;
std::unique_ptr<BackgroundService> g_backgroundService;
State g_state;

std::thread::id g_simulationThreadId;

std::string format_as(State state) {
    switch (state) {
        case State::Unloaded:
            return "Unloaded";
        case State::Stopped:
            return "Stopped";
        case State::Running:
            return "Running";
        case State::Paused:
            return "Paused";
        case State::Terminated:
            return "Terminated";
        default:  // NOLINT(clang-diagnostic-covered-switch-default)
            return "Unknown";
    }
}

void DoSimulation() {
    g_backgroundService.reset();

    g_simulationThreadId = std::this_thread::get_id();
    while (!g_stopSimulationThread) {
        if (!SendSomeData(g_currentTime, g_runTimeInfo)) {
            break;
        }

        DsVeosCoSim_SimulationTime nextSimulationTime{};
        g_server->Step(g_currentTime, nextSimulationTime);

        if (nextSimulationTime > g_currentTime) {
            g_currentTime = nextSimulationTime;
        } else {
            g_currentTime += 1000000;
        }
    }

    g_backgroundService = std::make_unique<BackgroundService>(*g_server);
}

void StopSimulationThread() {
    g_stopSimulationThread = true;

    if (g_simulationThreadId == std::this_thread::get_id()) {
        // It's called from inside the simulation thread. That won't work. So let the next simulation thread starter
        // join this thread
        return;
    }

    if (g_simulationThread.joinable()) {
        g_simulationThread.join();
    }

    g_simulationThread = {};
    g_simulationThreadId = {};
}

void StartSimulationThread() {
    StopSimulationThread();

    g_stopSimulationThread = false;
    g_simulationThread = std::thread(DoSimulation);
}

void StartSimulation() {
    if (g_state == State::Running) {
        return;
    }

    if (g_state != State::Stopped) {
        LogError("Could not start in state {}.", format_as(g_state));
        return;
    }

    g_currentTime = 0;
    LogInfo("Starting ...");
    g_backgroundService.reset();

    g_server->Start(g_currentTime);

    StartSimulationThread();
    g_state = State::Running;

    LogInfo("Started.");
}

void StopSimulation() {
    if (g_state == State::Stopped) {
        return;
    }

    if ((g_state != State::Running) && (g_state != State::Paused)) {
        LogError("Could not stop in state {}.", g_state);
        return;
    }

    LogInfo("Stopping ...");

    StopSimulationThread();
    g_server->Stop(g_currentTime);
    g_state = State::Stopped;

    LogInfo("Stopped.");
}

void PauseSimulation() {
    if (g_state == State::Paused) {
        return;
    }

    if (g_state != State::Running) {
        LogError("Could not pause in state {}.", g_state);
        return;
    }

    LogInfo("Pausing ...");

    StopSimulationThread();
    g_server->Pause(g_currentTime);
    g_state = State::Paused;

    LogInfo("Paused.");
}

void ContinueSimulation() {
    if (g_state == State::Running) {
        return;
    }

    if (g_state != State::Paused) {
        LogError("Could not start in state {}.", g_state);
        return;
    }

    LogInfo("Continuing ...");
    g_server->Continue(g_currentTime);

    StartSimulationThread();
    g_state = State::Running;

    LogInfo("Continued.");
}

void TerminateSimulation() {
    if (g_state == State::Terminated) {
        return;
    }

    if (g_state == State::Unloaded) {
        LogError("Could not terminate in state {}.", g_state);
        return;
    }

    LogInfo("Terminating ...");

    StopSimulationThread();

    g_server->Terminate(g_currentTime, DsVeosCoSim_TerminateReason_Error);
    g_state = State::Terminated;

    LogInfo("Terminated.");
}

void OnSimulationStartedCallback([[maybe_unused]] DsVeosCoSim_SimulationTime simulationTime) {
    LogInfo("Received simulation started event.");
    StartSimulation();
}

void OnSimulationStoppedCallback([[maybe_unused]] DsVeosCoSim_SimulationTime simulationTime) {
    LogInfo("Received simulation stopped event.");
    StopSimulation();
}

void OnSimulationPausedCallback([[maybe_unused]] DsVeosCoSim_SimulationTime simulationTime) {
    LogInfo("Received simulation paused event.");
    PauseSimulation();
}

void OnSimulationContinuedCallback([[maybe_unused]] DsVeosCoSim_SimulationTime simulationTime) {
    LogInfo("Received simulation continued event.");
    ContinueSimulation();
}

void OnSimulationTerminatedCallback([[maybe_unused]] DsVeosCoSim_SimulationTime simulationTime,
                                    [[maybe_unused]] DsVeosCoSim_TerminateReason terminateReason) {
    LogInfo("Received simulation continued event.");
    TerminateSimulation();
}

void LoadSimulation(bool isClientOptional, std::string_view name) {
    LogInfo("Loading ...");

    if (g_state != State::Unloaded) {
        LogError("Could not load in state {}.", g_state);
        return;
    }

    CoSimServerConfig config{};

    config.serverName = name;
    config.logCallback = OnLogCallback;
    config.isClientOptional = isClientOptional;
    config.stepSize = 1000000;
    config.startPortMapper = true;
    config.simulationStartedCallback = OnSimulationStartedCallback;
    config.simulationStoppedCallback = OnSimulationStoppedCallback;
    config.simulationPausedCallback = OnSimulationPausedCallback;
    config.simulationContinuedCallback = OnSimulationContinuedCallback;
    config.simulationTerminatedCallback = OnSimulationTerminatedCallback;
    config.canMessageReceivedCallback = LogCanMessage;
    config.ethMessageReceivedCallback = LogEthMessage;
    config.linMessageReceivedCallback = LogLinMessage;
    config.canControllers = CreateCanControllers(1);
    config.ethControllers = CreateEthControllers(1);
    config.linControllers = CreateLinControllers(1);
    config.incomingSignals = CreateSignals(1);
    config.outgoingSignals = CreateSignals(1);

    g_server = std::make_unique<CoSimServer>();
    g_server->Load(config);
    g_state = State::Stopped;

    g_runTimeInfo.canControllers = config.canControllers;
    g_runTimeInfo.ethControllers = config.ethControllers;
    g_runTimeInfo.linControllers = config.linControllers;
    g_runTimeInfo.incomingSignals = config.outgoingSignals;
    g_runTimeInfo.outgoingSignals = config.incomingSignals;
    g_runTimeInfo.transmitCan = [&](const DsVeosCoSim_CanMessage& message) {
        return g_server->Transmit(message);
    };
    g_runTimeInfo.transmitEth = [&](const DsVeosCoSim_EthMessage& message) {
        return g_server->Transmit(message);
    };
    g_runTimeInfo.transmitLin = [&](const DsVeosCoSim_LinMessage& message) {
        return g_server->Transmit(message);
    };
    g_runTimeInfo.write = [&](DsVeosCoSim_IoSignalId signalId, uint32_t length, const void* value) {
        return g_server->Write(signalId, length, value);
    };

    g_backgroundService = std::make_unique<BackgroundService>(*g_server);

    LogInfo("Loaded.");
}

void UnloadSimulation() {
    LogInfo("Unloading ...");

    StopSimulationThread();
    g_server.reset();
    g_state = State::Unloaded;

    LogInfo("Unloaded.");
}

void HostServer(bool isClientOptional, std::string_view name) {
    LoadSimulation(isClientOptional, name);

    while (true) {
        switch (GetChar()) {
            case CTRL('c'):
                return;
            case 'l':
                LoadSimulation(isClientOptional, name);
                break;
            case 's':
                StartSimulation();
                break;
            case 'o':
                StopSimulation();
                break;
            case 'p':
                PauseSimulation();
                break;
            case 't':
                TerminateSimulation();
                break;
            case 'n':
                ContinueSimulation();
                break;
            case 'u':
                UnloadSimulation();
                break;
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

    std::string name = "CoSimProxy";
    bool isClientOptional = false;

    for (int32_t i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--name") == 0) {
            if (++i < argc) {
                name = argv[i];
            } else {
                LogError("No name specified.");
                return 1;
            }
        }

        if (strcmp(argv[i], "--client-optional") == 0) {
            isClientOptional = true;
        }
    }

    HostServer(isClientOptional, name);

    UnloadSimulation();

    return 0;
}
