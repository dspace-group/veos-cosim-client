// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <chrono>
#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <fmt/color.h>

#include "DsVeosCoSim/CoSimServer.h"
#include "DsVeosCoSim/CoSimTypes.h"
#include "Helper.h"

using namespace DsVeosCoSim;
using namespace std::chrono;
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

    [[nodiscard]] Result Transmit(const FrMessageContainer& messageContainer) {
        std::lock_guard lock(_mutex);
        return _server->Transmit(messageContainer);
    }

    void StartBackgroundThread() {
        _stopBackgroundThreadFlag = false;
        _backgroundThread = std::thread([&] {
            while (!_stopBackgroundThreadFlag) {
                std::this_thread::sleep_for(1s);
                std::lock_guard lock(_mutex);
                std::chrono::nanoseconds roundTripTime{};
                if (!IsOk(_server->BackgroundService(roundTripTime))) {
                    LogError("Error in background task.");
                    return;
                }

                if (roundTripTime.count() > 0) {
                    LogTrace("Round trip time: {} ns.", roundTripTime.count());
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

    [[nodiscard]] const std::vector<FrControllerContainer>& GetFrControllers() const {
        return _config.frControllers;
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
bool SendFrMessages;

bool StopSimulationThreadFlag;
std::thread SimulationThread;
std::thread::id SimulationThreadId;

SimulationTime CurrentTime;

std::unique_ptr<ServerWrapper> Server;

std::mutex LockState;
SimulationState State;

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

[[nodiscard]] Result TransmitFrMessage(const FrControllerContainer& controller) {
    FrMessageContainer messageContainer{};
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

    if (SendIoData && ((counter % 5) == 0)) {
        for (const IoSignalContainer& signal : Server->GetIncomingSignals()) {
            CheckResult(WriteOutGoingSignal(signal));
        }
    }

    if (SendCanMessages && ((counter % 5) == 1)) {
        for (const CanControllerContainer& controller : Server->GetCanControllers()) {
            CheckResult(TransmitCanMessage(controller));
        }
    }

    if (SendEthMessages && ((counter % 5) == 2)) {
        for (const EthControllerContainer& controller : Server->GetEthControllers()) {
            CheckResult(TransmitEthMessage(controller));
        }
    }

    if (SendLinMessages && ((counter % 5) == 3)) {
        for (const LinControllerContainer& controller : Server->GetLinControllers()) {
            CheckResult(TransmitLinMessage(controller));
        }
    }

    if (SendFrMessages && ((counter % 5) == 4)) {
        for (const FrControllerContainer& controller : Server->GetFrControllers()) {
            CheckResult(TransmitFrMessage(controller));
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
    std::lock_guard lock(LockState);

    SimulationState state = State;
    switch (state) {
        case SimulationState::Running:
            break;
        case SimulationState::Stopped:
            LogInfo("Starting ...");

            CurrentTime = 0ns;
            CheckResult(Server->Start(CurrentTime));
            State = SimulationState::Running;
            StartSimulationThread();

            LogInfo("Started.");
            break;
        case SimulationState::Paused:
            LogInfo("Continuing ...");

            CheckResult(Server->Continue(CurrentTime));
            State = SimulationState::Running;
            StartSimulationThread();

            LogInfo("Continued.");
            break;
        default:
            LogError("Could not start in state {}.", state);
            break;
    }

    return Result::Ok;
}

void OnSimulationStarted() {
    std::lock_guard lock(LockState);

    SimulationState state = State;
    switch (state) {
        case SimulationState::Stopped:
            CurrentTime = 0ns;
            if (!IsOk(Server->Start(CurrentTime))) {
                LogError("Could not start.");
                return;
            }

            State = SimulationState::Running;
            StartSimulationThread();
            break;
        case SimulationState::Paused:
            if (!IsOk(Server->Continue(CurrentTime))) {
                LogError("Could not continue.");
                return;
            }

            State = SimulationState::Running;
            StartSimulationThread();
            break;
        default:
            // Ignored
            break;
    }
}

[[nodiscard]] Result StopSimulation() {
    std::lock_guard lock(LockState);

    SimulationState state = State;
    switch (state) {
        case SimulationState::Stopped:
            break;
        case SimulationState::Running:
            LogInfo("Stopping ...");

            StopSimulationThread();
            CheckResult(Server->Stop(CurrentTime));
            State = SimulationState::Stopped;

            LogInfo("Stopped.");
            break;
        case SimulationState::Paused:
            LogInfo("Stopping ...");

            CheckResult(Server->Stop(CurrentTime));
            State = SimulationState::Stopped;

            LogInfo("Stopped.");
            break;
        default:
            LogError("Could not stop in state {}.", state);
            break;
    }

    return Result::Ok;
}

void OnSimulationStopped() {
    std::lock_guard lock(LockState);

    SimulationState state = State;
    switch (state) {
        case SimulationState::Running:
            StopSimulationThread();
            if (!IsOk(Server->Stop(CurrentTime))) {
                LogError("Could not stop.");
            }

            State = SimulationState::Stopped;
            break;
        case SimulationState::Paused:
            if (!IsOk(Server->Stop(CurrentTime))) {
                LogError("Could not stop.");
            }

            State = SimulationState::Stopped;
            break;
        default:
            // Ignored
            break;
    }
}

[[nodiscard]] Result PauseSimulation() {
    std::lock_guard lock(LockState);

    SimulationState state = State;
    switch (state) {
        case SimulationState::Paused:
            break;
        case SimulationState::Running:
            LogInfo("Pausing ...");

            StopSimulationThread();
            CheckResult(Server->Pause(CurrentTime));
            State = SimulationState::Paused;

            LogInfo("Paused.");
            break;
        default:
            LogError("Could not pause in state {}.", state);
            break;
    }

    return Result::Ok;
}

void OnSimulationPaused() {
    std::lock_guard lock(LockState);

    SimulationState state = State;
    switch (state) {
        case SimulationState::Running:
            if (!IsOk(Server->Pause(CurrentTime))) {
                LogError("Could not pause.");
            }

            StopSimulationThread();
            State = SimulationState::Paused;
            break;
        default:
            // Ignored
            break;
    }
}

[[nodiscard]] Result TerminateSimulation() {
    std::lock_guard lock(LockState);

    SimulationState state = State;
    switch (state) {
        case SimulationState::Terminated:
            break;
        case SimulationState::Paused:
        case SimulationState::Stopped:
        case SimulationState::Running:
            LogInfo("Terminating ...");

            StopSimulationThread();
            CheckResult(Server->Terminate(CurrentTime));
            State = SimulationState::Terminated;

            LogInfo("Terminated.");
            break;
        default:
            LogError("Could not terminate in state {}.", state);
            break;
    }

    return Result::Ok;
}

void OnSimulationTerminated() {
    std::lock_guard lock(LockState);

    SimulationState state = State;
    switch (state) {
        case SimulationState::Paused:
        case SimulationState::Stopped:
        case SimulationState::Running:
            if (!IsOk(Server->Terminate(CurrentTime))) {
                LogError("Could not terminate.");
            }

            StopSimulationThread();
            State = SimulationState::Terminated;
            break;
        default:
            // Ignored
            break;
    }
}

void OnSimulationStartedCallback([[maybe_unused]] SimulationTime simulationTime) {
    LogInfo("Received simulation started event.");
    std::thread(OnSimulationStarted).detach();
}

void OnSimulationStoppedCallback([[maybe_unused]] SimulationTime simulationTime) {
    LogInfo("Received simulation stopped event.");
    std::thread(OnSimulationStopped).detach();
}

void OnSimulationPausedCallback([[maybe_unused]] SimulationTime simulationTime) {
    LogInfo("Received simulation paused event.");
    std::thread(OnSimulationPaused).detach();
}

void OnSimulationContinuedCallback([[maybe_unused]] SimulationTime simulationTime) {
    LogInfo("Received simulation continued event.");
    std::thread(OnSimulationStarted).detach();
}

void OnSimulationTerminatedCallback([[maybe_unused]] SimulationTime simulationTime, [[maybe_unused]] TerminateReason terminateReason) {
    LogInfo("Received simulation continued event.");
    std::thread(OnSimulationTerminated).detach();
}

void OnCanMessageContainerReceived([[maybe_unused]] SimulationTime simulationTime,
                                   [[maybe_unused]] const CanController& controller,
                                   const CanMessageContainer& messageContainer) {
    print(fg(fmt::color::dodger_blue), "{}\n", messageContainer);
}

void OnEthMessageContainerReceived([[maybe_unused]] SimulationTime simulationTime,
                                   [[maybe_unused]] const EthController& controller,
                                   const EthMessageContainer& messageContainer) {
    print(fg(fmt::color::cyan), "{}\n", messageContainer);
}

void OnLinMessageContainerReceived([[maybe_unused]] SimulationTime simulationTime,
                                   [[maybe_unused]] const LinController& controller,
                                   const LinMessageContainer& messageContainer) {
    print(fg(fmt::color::lime), "{}\n", messageContainer);
}

void OnFrMessageContainerReceived([[maybe_unused]] SimulationTime simulationTime,
                                  [[maybe_unused]] const FrController& controller,
                                  const FrMessageContainer& messageContainer) {
    print(fg(fmt::color::lime), "{}\n", messageContainer);
}

[[nodiscard]] Result LoadSimulation(bool isClientOptional, const std::string& name) {
    LogInfo("Loading ...");

    if (State != SimulationState::Unloaded) {
        LogError("Could not load in state {}.", State);
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
    config.frMessageContainerReceivedCallback = OnFrMessageContainerReceived;
    config.canControllers = CreateCanControllers(2);
    config.ethControllers = CreateEthControllers(2);
    config.linControllers = CreateLinControllers(2);
    config.frControllers = CreateFrControllers(2);
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

[[nodiscard]] Result HostServer(bool isClientOptional, const std::string& name) {
    CheckResult(LoadSimulation(isClientOptional, name));

    while (true) {
        switch (GetChar()) {
            case CTRL('c'):
                return Result::Ok;
            case 'l':
                CheckResult(LoadSimulation(isClientOptional, name));
                break;
            case 'u':
                UnloadSimulation();
                break;
            case 's':
                CheckResult(StartSimulation());
                break;
            case 'p':
                CheckResult(PauseSimulation());
                break;
            case 't':
                CheckResult(StopSimulation());
                break;
            case CTRL('t'):
                CheckResult(TerminateSimulation());
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
            case '5':
                SwitchSendingFrMessages();
                break;
            default:
                LogError("Unknown key.");
                break;
        }
    }

    return Result::Ok;
}

}  // namespace

int main(int argc, char** argv) {
    InitializeOutput();

    std::string name = "CoSimTest";
    bool isClientOptional = false;

    for (int i = 1; i < argc; i++) {
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
