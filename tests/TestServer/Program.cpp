// Copyright dSPACE GmbH. All rights reserved.

#include <mutex>

#include <cstring>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "DsVeosCoSim/CoSimServer.h"
#include "DsVeosCoSim/CoSimTypes.h"
#include "Helper.h"

using namespace DsVeosCoSim;
using namespace std::chrono_literals;

namespace {

class ServerWrapper final {
public:
    ServerWrapper() = default;

    ~ServerWrapper() noexcept {
        StopBackgroundThread();
    }

    ServerWrapper(const ServerWrapper&) = delete;
    ServerWrapper& operator=(const ServerWrapper&) = delete;

    ServerWrapper(ServerWrapper&&) = delete;
    ServerWrapper& operator=(ServerWrapper&&) = delete;

    [[nodiscard]] Result Load(const CoSimServerConfig& config) {
        CheckResult(CreateServer(_server));
        _config = config;
        std::lock_guard lock(_mutex);
        return _server->Load(config);
    }

    [[nodiscard]] Result Step(SimulationTime simulationTime, SimulationTime& nextSimulationTime) {
        std::lock_guard lock(_mutex);
        return _server->Step(simulationTime, nextSimulationTime);
    }

    [[nodiscard]] Result Start(SimulationTime simulationTime) {
        std::lock_guard lock(_mutex);
        return _server->Start(simulationTime);
    }

    [[nodiscard]] Result Stop(SimulationTime simulationTime) {
        std::lock_guard lock(_mutex);
        return _server->Stop(simulationTime);
    }

    [[nodiscard]] Result Pause(SimulationTime simulationTime) {
        std::lock_guard lock(_mutex);
        return _server->Pause(simulationTime);
    }

    [[nodiscard]] Result Continue(SimulationTime simulationTime) {
        std::lock_guard lock(_mutex);
        return _server->Continue(simulationTime);
    }

    [[nodiscard]] Result Terminate(SimulationTime simulationTime) {
        std::lock_guard lock(_mutex);
        return _server->Terminate(simulationTime, TerminateReason::Error);
    }

    [[nodiscard]] Result Write(IoSignalId signalId, uint32_t length, const std::vector<uint8_t>& value) {
        std::lock_guard lock(_mutex);
        return _server->Write(signalId, length, value.data());
    }

    [[nodiscard]] Result Transmit(const CanMessageContainer& messageContainer) {
        std::lock_guard lock(_mutex);
        return _server->Transmit(messageContainer);
    }

    [[nodiscard]] Result Transmit(const EthMessageContainer& messageContainer) {
        std::lock_guard lock(_mutex);
        return _server->Transmit(messageContainer);
    }

    [[nodiscard]] Result Transmit(const LinMessageContainer& messageContainer) {
        std::lock_guard lock(_mutex);
        return _server->Transmit(messageContainer);
    }

    void StartBackgroundThread() {
        _stopBackgroundThreadFlag = false;
        _backgroundThread = std::thread([&] {
            while (!_stopBackgroundThreadFlag) {
                std::this_thread::sleep_for(1ms);
                std::lock_guard lock(_mutex);
                if (!IsOk(_server->BackgroundService())) {
                    LogError("Error in background task.");
                    return;
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

[[nodiscard]] Result WriteOutGoingSignal(const IoSignalContainer& ioSignal) {
    size_t length = GetDataTypeSize(ioSignal.dataType) * ioSignal.length;
    std::vector<uint8_t> data = GenerateBytes(length);

    return Server->Write(ioSignal.id, ioSignal.length, data);
}

[[nodiscard]] Result TransmitCanMessage(const CanControllerContainer& controller) {
    CanMessageContainer messageContainer{};
    FillWithRandom(messageContainer, controller.id);

    return Server->Transmit(messageContainer);
}

[[nodiscard]] Result TransmitEthMessage(const EthControllerContainer& controller) {
    EthMessageContainer messageContainer{};
    FillWithRandom(messageContainer, controller.id);

    return Server->Transmit(messageContainer);
}

[[nodiscard]] Result TransmitLinMessage(const LinControllerContainer& controller) {
    LinMessageContainer messageContainer{};
    FillWithRandom(messageContainer, controller.id);

    return Server->Transmit(messageContainer);
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
        for (const IoSignalContainer& signal : Server->GetIncomingSignals()) {
            CheckResult(WriteOutGoingSignal(signal));
        }
    }

    if (SendCanMessages && ((counter % 4) == 1)) {
        for (const CanControllerContainer& controller : Server->GetCanControllers()) {
            CheckResult(TransmitCanMessage(controller));
        }
    }

    if (SendEthMessages && ((counter % 4) == 2)) {
        for (const EthControllerContainer& controller : Server->GetEthControllers()) {
            CheckResult(TransmitEthMessage(controller));
        }
    }

    if (SendLinMessages && ((counter % 4) == 3)) {
        for (const LinControllerContainer& controller : Server->GetLinControllers()) {
            CheckResult(TransmitLinMessage(controller));
        }
    }

    return Result::Ok;
}

[[nodiscard]] Result DoSimulation() {
    Server->StopBackgroundThread();

    SimulationThreadId = std::this_thread::get_id();
    while (!StopSimulationThreadFlag) {
        CheckResult(SendSomeData(CurrentTime));

        SimulationTime nextSimulationTime{};
        CheckResult(Server->Step(CurrentTime, nextSimulationTime));

        if (nextSimulationTime > CurrentTime) {
            CurrentTime = nextSimulationTime;
        } else {
            CurrentTime += 1ms;
        }
    }

    Server->StartBackgroundThread();

    return Result::Ok;
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

[[nodiscard]] Result StartSimulation() {
    if (State == SimulationState::Running) {
        return Result::Ok;
    }

    if (State != SimulationState::Stopped) {
        LogError("Could not start in state {}.", ToString(State));
        return Result::Ok;
    }

    CurrentTime = 0ns;
    LogInfo("Starting ...");

    CheckResult(Server->Start(CurrentTime));

    StartSimulationThread();
    State = SimulationState::Running;

    LogInfo("Started.");
    return Result::Ok;
}

[[nodiscard]] Result StopSimulation() {
    if (State == SimulationState::Stopped) {
        return Result::Ok;
    }

    if ((State != SimulationState::Running) && (State != SimulationState::Paused)) {
        LogError("Could not stop in state {}.", ToString(State));
        return Result::Ok;
    }

    LogInfo("Stopping ...");

    StopSimulationThread();

    CheckResult(Server->Stop(CurrentTime));

    State = SimulationState::Stopped;

    LogInfo("Stopped.");
    return Result::Ok;
}

[[nodiscard]] Result PauseSimulation() {
    if (State == SimulationState::Paused) {
        return Result::Ok;
    }

    if (State != SimulationState::Running) {
        LogError("Could not pause in state {}.", ToString(State));
        return Result::Ok;
    }

    LogInfo("Pausing ...");

    StopSimulationThread();

    CheckResult(Server->Pause(CurrentTime));

    State = SimulationState::Paused;

    LogInfo("Paused.");
    return Result::Ok;
}

[[nodiscard]] Result ContinueSimulation() {
    if (State == SimulationState::Running) {
        return Result::Ok;
    }

    if (State != SimulationState::Paused) {
        LogError("Could not continue in state {}.", ToString(State));
        return Result::Ok;
    }

    LogInfo("Continuing ...");

    CheckResult(Server->Continue(CurrentTime));

    StartSimulationThread();
    State = SimulationState::Running;

    LogInfo("Continued.");
    return Result::Ok;
}

[[nodiscard]] Result TerminateSimulation() {
    if (State == SimulationState::Terminated) {
        return Result::Ok;
    }

    if (State == SimulationState::Unloaded) {
        LogError("Could not terminate in state {}.", ToString(State));
        return Result::Ok;
    }

    LogInfo("Terminating ...");

    StopSimulationThread();

    CheckResult(Server->Terminate(CurrentTime));

    State = SimulationState::Terminated;

    LogInfo("Terminated.");
    return Result::Ok;
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

[[nodiscard]] Result LoadSimulation(bool isClientOptional, std::string_view name) {
    LogInfo("Loading ...");

    if (State != SimulationState::Unloaded) {
        LogError("Could not load in state {}.", ToString(State));
        return Result::Ok;
    }

    CoSimServerConfig config{};
    config.serverName = name;
    config.isClientOptional = isClientOptional;
    config.stepSize = 1ms;
    config.startPortMapper = true;
    config.simulationStartedCallback = OnSimulationStartedCallback;
    config.simulationStoppedCallback = OnSimulationStoppedCallback;
    config.simulationPausedCallback = OnSimulationPausedCallback;
    config.simulationContinuedCallback = OnSimulationContinuedCallback;
    config.simulationTerminatedCallback = OnSimulationTerminatedCallback;
    config.canMessageContainerReceivedCallback = OnCanMessageContainerReceived;
    config.ethMessageContainerReceivedCallback = OnEthMessageContainerReceived;
    config.linMessageContainerReceivedCallback = OnLinMessageContainerReceived;
    config.canControllers = CreateCanControllers(2);
    config.ethControllers = CreateEthControllers(2);
    config.linControllers = CreateLinControllers(2);
    config.incomingSignals = CreateSignals(2);
    config.outgoingSignals = CreateSignals(2);

    Server = std::make_unique<ServerWrapper>();
    CheckResult(Server->Load(config));

    State = SimulationState::Stopped;

    Server->StartBackgroundThread();

    LogInfo("Loaded.");
    return Result::Ok;
}

void UnloadSimulation() {
    LogInfo("Unloading ...");

    StopSimulationThread();
    Server.reset();

    State = SimulationState::Unloaded;

    LogInfo("Unloaded.");
}

[[nodiscard]] Result HostServer(bool isClientOptional, std::string_view name) {
    CheckResult(LoadSimulation(isClientOptional, name));

    while (true) {
        switch (GetChar()) {
            case CTRL('c'):
                return Result::Ok;
            case 'l':
                CheckResult(LoadSimulation(isClientOptional, name));
                break;
            case 's':
                CheckResult(StartSimulation());
                break;
            case 'o':
                CheckResult(StopSimulation());
                break;
            case 'p':
                CheckResult(PauseSimulation());
                break;
            case 't':
                CheckResult(TerminateSimulation());
                break;
            case 'n':
                CheckResult(ContinueSimulation());
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

    return Result::Ok;
}

}  // namespace

int32_t main(int32_t argc, char** argv) {
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

    Result result = HostServer(isClientOptional, name);

    UnloadSimulation();

    return IsOk(result) ? 0 : 1;
}
