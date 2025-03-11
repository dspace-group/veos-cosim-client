// Copyright dSPACE GmbH. All rights reserved.

#include <mutex>

#include "Generator.h"
#include "Helper.h"
#include "LogHelper.h"

#include <cstring>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "CoSimServer.h"
#include "CoSimTypes.h"

using namespace DsVeosCoSim;
using namespace std::chrono_literals;

namespace {

class ServerWrapper final {  // NOLINT
public:
    ServerWrapper() : _server(CreateServer()) {
    }

    ~ServerWrapper() {
        StopBackgroundThread();
    }

    void Load(const CoSimServerConfig& config) {
        _config = config;
        std::lock_guard lock(_mutex);
        _server->Load(config);
    }

    [[nodiscard]] SimulationTime Step(const SimulationTime simulationTime) {
        std::lock_guard lock(_mutex);
        return _server->Step(simulationTime);
    }

    void Start(const SimulationTime simulationTime) {
        std::lock_guard lock(_mutex);
        _server->Start(simulationTime);
    }

    void Stop(const SimulationTime simulationTime) {
        std::lock_guard lock(_mutex);
        _server->Stop(simulationTime);
    }

    void Pause(const SimulationTime simulationTime) {
        std::lock_guard lock(_mutex);
        _server->Pause(simulationTime);
    }

    void Continue(const SimulationTime simulationTime) {
        std::lock_guard lock(_mutex);
        _server->Continue(simulationTime);
    }

    void Terminate(const SimulationTime simulationTime) {
        std::lock_guard lock(_mutex);
        _server->Terminate(simulationTime, TerminateReason::Error);
    }

    void Write(const IoSignalId signalId, const uint32_t length, const std::vector<uint8_t>& value) {
        std::lock_guard lock(_mutex);
        _server->Write(signalId, length, value.data());
    }

    void Transmit(const CanMessage& message) {
        std::lock_guard lock(_mutex);
        (void)_server->Transmit(message);
    }

    void Transmit(const EthMessage& message) {
        std::lock_guard lock(_mutex);
        (void)_server->Transmit(message);
    }

    void Transmit(const LinMessage& message) {
        std::lock_guard lock(_mutex);
        (void)_server->Transmit(message);
    }

    void StartBackgroundThread() {
        _stopBackgroundThreadFlag = false;
        _backgroundThread = std::thread([&] {
            while (!_stopBackgroundThreadFlag) {
                std::this_thread::sleep_for(1ms);
                try {
                    std::lock_guard lock(_mutex);
                    _server->BackgroundService();
                } catch (const std::exception& e) {
                    LogError(e.what());
                }
            }
        });
    }

    void StopBackgroundThread() {
        if (!_backgroundThread.joinable()) {
            return;
        }

        _stopBackgroundThreadFlag = true;
        if (std::this_thread::get_id() == _backgroundThread.get_id()) {
            _backgroundThread.detach();
        } else {
            _backgroundThread.join();
        }
    }

    [[nodiscard]] const std::vector<IoSignalContainer>& GetIncomingSignals() const {
        return _config.incomingSignals;
    }

    [[nodiscard]] const std::vector<CanControllerContainer>& GetCanControllers() const {
        return _config.canControllers;
    }

    [[nodiscard]] const std::vector<EthControllerContainer>& GetEthControllers() const {
        return _config.ethControllers;
    }

    [[nodiscard]] const std::vector<LinControllerContainer>& GetLinControllers() const {
        return _config.linControllers;
    }

private:
    std::unique_ptr<CoSimServer> _server;
    std::mutex _mutex;
    CoSimServerConfig _config;

    bool _stopBackgroundThreadFlag{};
    std::thread _backgroundThread;
};

bool SendIoData;
bool SendCanMessages;
bool SendEthMessages;
bool SendLinMessages;

bool StopSimulationThreadFlag;
std::thread SimulationThread;
std::thread::id SimulationThreadId;

SimulationTime CurrentTime;

std::unique_ptr<ServerWrapper> Server;
SimulationState State;

void PrintStatus(const bool value, const std::string& what) {
    if (value) {
        LogInfo("Enabled sending " + what);
    } else {
        LogInfo("Disabled sending " + what);
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

void WriteOutGoingSignal(const IoSignalContainer& ioSignal) {
    const size_t length = GetDataTypeSize(ioSignal.dataType) * ioSignal.length;
    const std::vector<uint8_t> data = GenerateBytes(length);

    Server->Write(ioSignal.id, ioSignal.length, data);
}

void TransmitCanMessage(const CanControllerContainer& controller) {
    CanMessageContainer message{};
    FillWithRandom(message, controller.id);

    Server->Transmit(static_cast<CanMessage>(message));
}

void TransmitEthMessage(const EthControllerContainer& controller) {
    EthMessageContainer message{};
    FillWithRandom(message, controller.id);

    Server->Transmit(static_cast<EthMessage>(message));
}

void TransmitLinMessage(const LinControllerContainer& controller) {
    LinMessageContainer message{};
    FillWithRandom(message, controller.id);

    Server->Transmit(static_cast<LinMessage>(message));
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
        for (const IoSignalContainer& signal : Server->GetIncomingSignals()) {
            WriteOutGoingSignal(signal);
        }
    }

    if (SendCanMessages && ((counter % 4) == 1)) {
        for (const CanControllerContainer& controller : Server->GetCanControllers()) {
            TransmitCanMessage(controller);
        }
    }

    if (SendEthMessages && ((counter % 4) == 2)) {
        for (const EthControllerContainer& controller : Server->GetEthControllers()) {
            TransmitEthMessage(controller);
        }
    }

    if (SendLinMessages && ((counter % 4) == 3)) {
        for (const LinControllerContainer& controller : Server->GetLinControllers()) {
            TransmitLinMessage(controller);
        }
    }
}

void DoSimulation() {
    Server->StopBackgroundThread();

    SimulationThreadId = std::this_thread::get_id();
    while (!StopSimulationThreadFlag) {
        SendSomeData(CurrentTime);

        SimulationTime nextSimulationTime = Server->Step(CurrentTime);

        if (nextSimulationTime > CurrentTime) {
            CurrentTime = nextSimulationTime;
        } else {
            CurrentTime += 1ms;
        }
    }

    Server->StartBackgroundThread();
}

void StopSimulationThread() {
    StopSimulationThreadFlag = true;

    if (SimulationThreadId == std::this_thread::get_id()) {
        // It's called from inside the simulation thread. That won't work. So let the next simulation thread starter
        // join this thread
        return;
    }

    if (SimulationThread.joinable()) {
        SimulationThread.join();
    }

    SimulationThread = {};
    SimulationThreadId = {};
}

void StartSimulationThread() {
    StopSimulationThread();

    StopSimulationThreadFlag = false;
    SimulationThread = std::thread(DoSimulation);
}

void StartSimulation() {
    if (State == SimulationState::Running) {
        return;
    }

    if (State != SimulationState::Stopped) {
        LogError("Could not start in state " + ToString(State) + ".");
        return;
    }

    CurrentTime = 0ns;
    LogInfo("Starting ...");

    Server->Start(CurrentTime);

    StartSimulationThread();
    State = SimulationState::Running;

    LogInfo("Started.");
}

void StopSimulation() {
    if (State == SimulationState::Stopped) {
        return;
    }

    if ((State != SimulationState::Running) && (State != SimulationState::Paused)) {
        LogError("Could not stop in state " + ToString(State) + ".");
        return;
    }

    LogInfo("Stopping ...");

    StopSimulationThread();

    Server->Stop(CurrentTime);

    State = SimulationState::Stopped;

    LogInfo("Stopped.");
}

void PauseSimulation() {
    if (State == SimulationState::Paused) {
        return;
    }

    if (State != SimulationState::Running) {
        LogError("Could not pause in state " + ToString(State) + ".");
        return;
    }

    LogInfo("Pausing ...");

    StopSimulationThread();

    Server->Pause(CurrentTime);

    State = SimulationState::Paused;

    LogInfo("Paused.");
}

void ContinueSimulation() {
    if (State == SimulationState::Running) {
        return;
    }

    if (State != SimulationState::Paused) {
        LogError("Could not start in state " + ToString(State) + ".");
        return;
    }

    LogInfo("Continuing ...");

    Server->Continue(CurrentTime);

    StartSimulationThread();
    State = SimulationState::Running;

    LogInfo("Continued.");
}

void TerminateSimulation() {
    if (State == SimulationState::Terminated) {
        return;
    }

    if (State == SimulationState::Unloaded) {
        LogError("Could not terminate in state " + ToString(State) + ".");
        return;
    }

    LogInfo("Terminating ...");

    StopSimulationThread();

    Server->Terminate(CurrentTime);

    State = SimulationState::Terminated;

    LogInfo("Terminated.");
}

void OnSimulationStartedCallback([[maybe_unused]] SimulationTime simulationTime) {
    LogInfo("Received simulation started event.");
    std::thread(StartSimulation).detach();
}

void OnSimulationStoppedCallback([[maybe_unused]] SimulationTime simulationTime) {
    LogInfo("Received simulation stopped event.");
    std::thread(StopSimulation).detach();
}

void OnSimulationPausedCallback([[maybe_unused]] SimulationTime simulationTime) {
    LogInfo("Received simulation paused event.");
    std::thread(PauseSimulation).detach();
}

void OnSimulationContinuedCallback([[maybe_unused]] SimulationTime simulationTime) {
    LogInfo("Received simulation continued event.");
    std::thread(ContinueSimulation).detach();
}

void OnSimulationTerminatedCallback([[maybe_unused]] SimulationTime simulationTime,
                                    [[maybe_unused]] TerminateReason terminateReason) {
    LogInfo("Received simulation continued event.");
    std::thread(TerminateSimulation).detach();
}

void LoadSimulation(const bool isClientOptional, const std::string_view name) {
    LogInfo("Loading ...");

    if (State != SimulationState::Unloaded) {
        LogError("Could not load in state " + ToString(State) + ".");
        return;
    }

    CoSimServerConfig config{};
    config.serverName = name;
    config.logCallback = OnLogCallback;
    config.isClientOptional = isClientOptional;
    config.stepSize = 1ms;
    config.startPortMapper = true;
    config.simulationStartedCallback = OnSimulationStartedCallback;
    config.simulationStoppedCallback = OnSimulationStoppedCallback;
    config.simulationPausedCallback = OnSimulationPausedCallback;
    config.simulationContinuedCallback = OnSimulationContinuedCallback;
    config.simulationTerminatedCallback = OnSimulationTerminatedCallback;
    config.canMessageReceivedCallback = LogCanMessage;
    config.ethMessageReceivedCallback = LogEthMessage;
    config.linMessageReceivedCallback = LogLinMessage;
    config.canControllers = CreateCanControllers(2);
    config.ethControllers = CreateEthControllers(2);
    config.linControllers = CreateLinControllers(2);
    config.incomingSignals = CreateSignals(2);
    config.outgoingSignals = CreateSignals(2);

    Server = std::make_unique<ServerWrapper>();
    Server->Load(config);

    State = SimulationState::Stopped;

    Server->StartBackgroundThread();

    LogInfo("Loaded.");
}

void UnloadSimulation() {
    LogInfo("Unloading ...");

    StopSimulationThread();
    Server.reset();

    State = SimulationState::Unloaded;

    LogInfo("Unloaded.");
}

void HostServer(const bool isClientOptional, const std::string_view name) {
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

int32_t main(const int32_t argc, char** argv) {
    InitializeOutput();

    std::string name = "CoSimTest";
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
