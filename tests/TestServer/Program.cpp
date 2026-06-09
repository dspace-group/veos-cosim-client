// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include <CoSimServer.hpp>
#include <CoSimTypes.hpp>
#include <Logger.hpp>
#include <Result.hpp>

#include "Helper.hpp"

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
        _config = config;
        _server = std::make_unique<CoSimServer>();
        std::scoped_lock lock(_mutex);
        return _server->Load(config);
    }

    [[nodiscard]] Result Step(SimulationTime simulationTime, SimulationTime& nextSimulationTime) {
        std::scoped_lock lock(_mutex);
        return _server->Step(simulationTime, nextSimulationTime);
    }

    [[nodiscard]] Result Start(SimulationTime simulationTime) {
        std::scoped_lock lock(_mutex);
        return _server->Start(simulationTime);
    }

    [[nodiscard]] Result Stop(SimulationTime simulationTime) {
        std::scoped_lock lock(_mutex);
        return _server->Stop(simulationTime);
    }

    [[nodiscard]] Result Pause(SimulationTime simulationTime) {
        std::scoped_lock lock(_mutex);
        return _server->Pause(simulationTime);
    }

    [[nodiscard]] Result Continue(SimulationTime simulationTime) {
        std::scoped_lock lock(_mutex);
        return _server->Continue(simulationTime);
    }

    [[nodiscard]] Result Terminate(SimulationTime simulationTime) {
        std::scoped_lock lock(_mutex);
        return _server->Terminate(simulationTime, TerminateReason::Error);
    }

    [[nodiscard]] Result Write(IoSignalId signalId, uint32_t length, const std::vector<uint8_t>& value) {
        std::scoped_lock lock(_mutex);
        return _server->Write(signalId, length, value.data());
    }

    [[nodiscard]] Result Transmit(const CanMessageContainer& messageContainer) {
        std::scoped_lock lock(_mutex);
        return _server->Transmit(messageContainer);
    }

    [[nodiscard]] Result Transmit(const EthMessageContainer& messageContainer) {
        std::scoped_lock lock(_mutex);
        return _server->Transmit(messageContainer);
    }

    [[nodiscard]] Result Transmit(const LinMessageContainer& messageContainer) {
        std::scoped_lock lock(_mutex);
        return _server->Transmit(messageContainer);
    }

    [[nodiscard]] Result Transmit(const FrMessageContainer& messageContainer) {
        std::scoped_lock lock(_mutex);
        return _server->Transmit(messageContainer);
    }

    void StartBackgroundThread() {
        _stopBackgroundThreadFlag = false;
        _backgroundThread = std::thread([&] {
            while (!_stopBackgroundThreadFlag) {
                std::this_thread::sleep_for(100ms);
                std::scoped_lock lock(_mutex);
                SimulationTime roundTripTime{};
                if (!IsOk(_server->BackgroundService(roundTripTime))) {
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

    [[nodiscard]] const std::vector<FrControllerContainer>& GetFrControllers() const {
        return _config.frControllers;
    }

private:
    std::unique_ptr<CoSimServer> _server;
    std::mutex _mutex;
    CoSimServerConfig _config;

    std::atomic<bool> _stopBackgroundThreadFlag{};
    std::thread _backgroundThread;
};

struct ServerData {
    bool sendIoData{};
    bool sendCanMessages{};
    bool sendEthMessages{};
    bool sendLinMessages{};
    bool sendFrMessages{};
    bool printStepsPerSecond{};

    int64_t performanceStepCount{};
    std::chrono::steady_clock::time_point performanceMeasurementStart;
    bool performanceMeasurementActive{};
    std::chrono::steady_clock::time_point lastPerformancePrintTime;

    bool stopSimulationThreadFlag{};
    std::thread simulationThread;
    std::thread::id simulationThreadId;

    SimulationTime currentTime;

    SimulationTime sendLastHalfSecond = -1s;
    int64_t sendCounter{};

    std::unique_ptr<ServerWrapper> server;

    std::mutex lockState;
    SimulationState state{};
};

void PrintStatus(bool value, std::string_view what) {
    if (value) {
        LogInfo("Enabled sending {}.", what);
    } else {
        LogInfo("Disabled sending {}.", what);
    }
}

void SwitchSendingIoSignals(ServerData& serverData) {
    serverData.sendIoData = !serverData.sendIoData;
    PrintStatus(serverData.sendIoData, "IO data");
}

void SwitchSendingCanMessages(ServerData& serverData) {
    serverData.sendCanMessages = !serverData.sendCanMessages;
    PrintStatus(serverData.sendCanMessages, "CAN messages");
}

void SwitchSendingEthMessages(ServerData& serverData) {
    serverData.sendEthMessages = !serverData.sendEthMessages;
    PrintStatus(serverData.sendEthMessages, "Ethernet messages");
}

void SwitchSendingLinMessages(ServerData& serverData) {
    serverData.sendLinMessages = !serverData.sendLinMessages;
    PrintStatus(serverData.sendLinMessages, "LIN messages");
}

void SwitchSendingFrMessages(ServerData& serverData) {
    serverData.sendFrMessages = !serverData.sendFrMessages;
    PrintStatus(serverData.sendFrMessages, "FlexRay messages");
}

void StartPerformanceMeasurement(ServerData& serverData) {
    serverData.performanceStepCount = 0;
    serverData.performanceMeasurementStart = std::chrono::steady_clock::now();
    serverData.performanceMeasurementActive = true;
    serverData.lastPerformancePrintTime = {};
    serverData.sendLastHalfSecond = -1s;
    serverData.sendCounter = 0;
}

void StopPerformanceMeasurement(ServerData& serverData) {
    if (!serverData.performanceMeasurementActive) {
        return;
    }

    serverData.performanceMeasurementActive = false;

    double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - serverData.performanceMeasurementStart).count();
    if (seconds > 0.0) {
        LogInfo("Performance: {:.1f} steps per second average ({} steps in {:.3f} s).",
                static_cast<double>(serverData.performanceStepCount) / seconds,
                serverData.performanceStepCount,
                seconds);
    }
}

void PrintCurrentStepsPerSecond(ServerData& serverData) {
    if (!serverData.performanceMeasurementActive || !serverData.printStepsPerSecond) {
        return;
    }

    auto now = std::chrono::steady_clock::now();

    if (now - serverData.lastPerformancePrintTime < 1s) {
        return;
    }

    serverData.lastPerformancePrintTime = now;

    double seconds = std::chrono::duration<double>(now - serverData.performanceMeasurementStart).count();
    if (seconds > 0.0) {
        LogInfo("Steps per second: {:.1f} ({} steps in {:.1f} s).",
                static_cast<double>(serverData.performanceStepCount) / seconds,
                serverData.performanceStepCount,
                seconds);
    }
}

void SwitchPrintingStepsPerSecond(ServerData& serverData) {
    serverData.printStepsPerSecond = !serverData.printStepsPerSecond;
    if (serverData.printStepsPerSecond) {
        LogInfo("Enabled printing steps per second.");
    } else {
        LogInfo("Disabled printing steps per second.");
    }
}

[[nodiscard]] Result WriteOutGoingSignal(const ServerData& serverData, const IoSignalContainer& ioSignal) {
    size_t length = GetDataTypeSize(ioSignal.dataType) * ioSignal.length;
    std::vector<uint8_t> data = GenerateBytes(length);

    return serverData.server->Write(ioSignal.id, ioSignal.length, data);
}

[[nodiscard]] Result TransmitCanMessage(const ServerData& serverData, const CanControllerContainer& controller) {
    CanMessageContainer messageContainer{};
    FillWithRandom(messageContainer, controller.id);

    return serverData.server->Transmit(messageContainer);
}

[[nodiscard]] Result TransmitEthMessage(const ServerData& serverData, const EthControllerContainer& controller) {
    EthMessageContainer messageContainer{};
    FillWithRandom(messageContainer, controller.id);

    return serverData.server->Transmit(messageContainer);
}

[[nodiscard]] Result TransmitLinMessage(const ServerData& serverData, const LinControllerContainer& controller) {
    LinMessageContainer messageContainer{};
    FillWithRandom(messageContainer, controller.id);

    return serverData.server->Transmit(messageContainer);
}

[[nodiscard]] Result TransmitFrMessage(const ServerData& serverData, const FrControllerContainer& controller) {
    FrMessageContainer messageContainer{};
    FillWithRandom(messageContainer, controller.id);

    return serverData.server->Transmit(messageContainer);
}

[[nodiscard]] Result SendSomeData(ServerData& serverData, SimulationTime simulationTime) {
    SimulationTime currentHalfSecond = simulationTime / 500000000;
    if (currentHalfSecond == serverData.sendLastHalfSecond) {
        return CreateOk();
    }

    serverData.sendLastHalfSecond = currentHalfSecond;
    serverData.sendCounter++;

    if (serverData.sendIoData && ((serverData.sendCounter % 5) == 0)) {
        for (const IoSignalContainer& signal : serverData.server->GetIncomingSignals()) {
            CheckResult(WriteOutGoingSignal(serverData, signal));
        }
    }

    if (serverData.sendCanMessages && ((serverData.sendCounter % 5) == 1)) {
        for (const CanControllerContainer& controller : serverData.server->GetCanControllers()) {
            CheckResult(TransmitCanMessage(serverData, controller));
        }
    }

    if (serverData.sendEthMessages && ((serverData.sendCounter % 5) == 2)) {
        for (const EthControllerContainer& controller : serverData.server->GetEthControllers()) {
            CheckResult(TransmitEthMessage(serverData, controller));
        }
    }

    if (serverData.sendLinMessages && ((serverData.sendCounter % 5) == 3)) {
        for (const LinControllerContainer& controller : serverData.server->GetLinControllers()) {
            CheckResult(TransmitLinMessage(serverData, controller));
        }
    }

    if (serverData.sendFrMessages && ((serverData.sendCounter % 5) == 4)) {
        for (const FrControllerContainer& controller : serverData.server->GetFrControllers()) {
            CheckResult(TransmitFrMessage(serverData, controller));
        }
    }

    return CreateOk();
}

[[nodiscard]] Result DoSimulationLoop(ServerData& serverData) {
    while (!serverData.stopSimulationThreadFlag) {
        CheckResult(SendSomeData(serverData, serverData.currentTime));

        SimulationTime nextSimulationTime{};
        CheckResult(serverData.server->Step(serverData.currentTime, nextSimulationTime));

        serverData.performanceStepCount++;
        PrintCurrentStepsPerSecond(serverData);

        if (nextSimulationTime > serverData.currentTime) {
            serverData.currentTime = nextSimulationTime;
        } else {
            serverData.currentTime += 1ms;
        }
    }

    return CreateOk();
}

[[nodiscard]] Result DoSimulation(ServerData& serverData) {
    StartPerformanceMeasurement(serverData);

    serverData.simulationThreadId = std::this_thread::get_id();

    Result result = DoSimulationLoop(serverData);

    StopPerformanceMeasurement(serverData);

    return result;
}

void StopSimulationThread(ServerData& serverData) {
    serverData.stopSimulationThreadFlag = true;

    if (serverData.simulationThreadId == std::this_thread::get_id()) {
        // It's called from inside the simulation thread. That won't work. So let the next simulation thread starter
        // join this thread
        return;
    }

    if (serverData.simulationThread.joinable()) {
        serverData.simulationThread.join();
    }

    serverData.simulationThread = {};
    serverData.simulationThreadId = {};
}

void StartSimulationThread(ServerData& serverData) {
    StopSimulationThread(serverData);

    serverData.stopSimulationThreadFlag = false;
    serverData.simulationThread = std::thread([&serverData] {
        (void)DoSimulation(serverData);
    });
}

[[nodiscard]] Result StartOrPauseSimulation(ServerData& serverData) {
    std::scoped_lock lock(serverData.lockState);

    switch (SimulationState state = serverData.state; state) {
        case SimulationState::Running:
            LogInfo("Pausing ...");

            StopSimulationThread(serverData);
            CheckResult(serverData.server->Pause(serverData.currentTime));
            serverData.state = SimulationState::Paused;

            LogInfo("Paused.");
            break;
        case SimulationState::Stopped:
            LogInfo("Starting ...");

            serverData.currentTime = 0s;
            CheckResult(serverData.server->Start(serverData.currentTime));
            serverData.state = SimulationState::Running;
            StartSimulationThread(serverData);

            LogInfo("Started.");
            break;
        case SimulationState::Paused:
            LogInfo("Continuing ...");

            CheckResult(serverData.server->Continue(serverData.currentTime));
            serverData.state = SimulationState::Running;
            StartSimulationThread(serverData);

            LogInfo("Continued.");
            break;
        default:
            LogError("Could not start or pause in state {}.", state);
            break;
    }

    return CreateOk();
}

void OnSimulationStarted(ServerData& serverData) {
    std::scoped_lock lock(serverData.lockState);

    switch (SimulationState state = serverData.state; state) {
        case SimulationState::Stopped:
            serverData.currentTime = 0s;
            if (!IsOk(serverData.server->Start(serverData.currentTime))) {
                LogError("Could not start.");
                return;
            }

            serverData.state = SimulationState::Running;
            StartSimulationThread(serverData);
            break;
        case SimulationState::Paused:
            if (!IsOk(serverData.server->Continue(serverData.currentTime))) {
                LogError("Could not continue.");
                return;
            }

            serverData.state = SimulationState::Running;
            StartSimulationThread(serverData);
            break;
        default:
            // Ignored
            break;
    }
}

[[nodiscard]] Result StopSimulation(ServerData& serverData) {
    std::scoped_lock lock(serverData.lockState);

    switch (SimulationState state = serverData.state; state) {
        case SimulationState::Stopped:
            break;
        case SimulationState::Running:
            LogInfo("Stopping ...");

            StopSimulationThread(serverData);
            CheckResult(serverData.server->Stop(serverData.currentTime));
            serverData.state = SimulationState::Stopped;

            LogInfo("Stopped.");
            break;
        case SimulationState::Paused:
            LogInfo("Stopping ...");

            CheckResult(serverData.server->Stop(serverData.currentTime));
            serverData.state = SimulationState::Stopped;

            LogInfo("Stopped.");
            break;
        default:
            LogError("Could not stop in state {}.", state);
            break;
    }

    return CreateOk();
}

void OnSimulationStopped(ServerData& serverData) {
    std::scoped_lock lock(serverData.lockState);

    switch (SimulationState state = serverData.state; state) {
        case SimulationState::Running:
            StopSimulationThread(serverData);
            if (!IsOk(serverData.server->Stop(serverData.currentTime))) {
                LogError("Could not stop.");
            }

            serverData.state = SimulationState::Stopped;
            break;
        case SimulationState::Paused:
            if (!IsOk(serverData.server->Stop(serverData.currentTime))) {
                LogError("Could not stop.");
            }

            serverData.state = SimulationState::Stopped;
            break;
        default:
            // Ignored
            break;
    }
}

void OnSimulationPaused(ServerData& serverData) {
    std::scoped_lock lock(serverData.lockState);

    switch (SimulationState state = serverData.state; state) {
        case SimulationState::Running:
            if (!IsOk(serverData.server->Pause(serverData.currentTime))) {
                LogError("Could not pause.");
            }

            StopSimulationThread(serverData);
            serverData.state = SimulationState::Paused;
            break;
        default:
            // Ignored
            break;
    }
}

[[nodiscard]] Result TerminateSimulation(ServerData& serverData) {
    std::scoped_lock lock(serverData.lockState);

    switch (SimulationState state = serverData.state; state) {
        case SimulationState::Terminated:
            break;
        case SimulationState::Paused:
        case SimulationState::Stopped:
        case SimulationState::Running:
            LogInfo("Terminating ...");

            StopSimulationThread(serverData);
            CheckResult(serverData.server->Terminate(serverData.currentTime));
            serverData.state = SimulationState::Terminated;

            LogInfo("Terminated.");
            break;
        default:
            LogError("Could not terminate in state {}.", state);
            break;
    }

    return CreateOk();
}

void OnSimulationTerminated(ServerData& serverData) {
    std::scoped_lock lock(serverData.lockState);

    switch (SimulationState state = serverData.state; state) {
        case SimulationState::Paused:
        case SimulationState::Stopped:
        case SimulationState::Running:
            if (!IsOk(serverData.server->Terminate(serverData.currentTime))) {
                LogError("Could not terminate.");
            }

            StopSimulationThread(serverData);
            serverData.state = SimulationState::Terminated;
            break;
        default:
            // Ignored
            break;
    }
}

void OnSimulationStartedCallback(ServerData& serverData) {
    LogInfo("Received simulation started event.");
    std::thread([&serverData] {
        OnSimulationStarted(serverData);
    }).detach();
}

void OnSimulationStoppedCallback(ServerData& serverData) {
    LogInfo("Received simulation stopped event.");
    std::thread([&serverData] {
        OnSimulationStopped(serverData);
    }).detach();
}

void OnSimulationPausedCallback(ServerData& serverData) {
    LogInfo("Received simulation paused event.");
    std::thread([&serverData] {
        OnSimulationPaused(serverData);
    }).detach();
}

void OnSimulationContinuedCallback(ServerData& serverData) {
    LogInfo("Received simulation continued event.");
    std::thread([&serverData] {
        OnSimulationStarted(serverData);
    }).detach();
}

void OnSimulationTerminatedCallback(ServerData& serverData) {
    LogInfo("Received simulation terminated event.");
    std::thread([&serverData] {
        OnSimulationTerminated(serverData);
    }).detach();
}

void OnIncomingSignalChanged([[maybe_unused]] SimulationTime simulationTime, const IoSignal& signal, uint32_t length, const void* value) {
    LogIoData(IoDataToString(signal, length, value));
}

void OnCanMessageContainerReceived([[maybe_unused]] SimulationTime simulationTime,
                                   [[maybe_unused]] const CanController& controller,
                                   const CanMessageContainer& messageContainer) {
    LogCanMessage(format_as(messageContainer));
}

void OnEthMessageContainerReceived([[maybe_unused]] SimulationTime simulationTime,
                                   [[maybe_unused]] const EthController& controller,
                                   const EthMessageContainer& messageContainer) {
    LogEthMessage(format_as(messageContainer));
}

void OnLinMessageContainerReceived([[maybe_unused]] SimulationTime simulationTime,
                                   [[maybe_unused]] const LinController& controller,
                                   const LinMessageContainer& messageContainer) {
    LogLinMessage(format_as(messageContainer));
}

void OnFrMessageContainerReceived([[maybe_unused]] SimulationTime simulationTime,
                                  [[maybe_unused]] const FrController& controller,
                                  const FrMessageContainer& messageContainer) {
    LogFrMessage(format_as(messageContainer));
}

[[nodiscard]] Result LoadSimulation(ServerData& serverData, bool isClientOptional, const std::string& name) {
    LogInfo("Loading ...");

    if (serverData.state != SimulationState::Unloaded) {
        LogError("Could not load in state {}.", serverData.state);
        return CreateOk();
    }

    CoSimServerConfig config{};
    config.serverName = name;
    config.isClientOptional = isClientOptional;
    config.stepSize = 1ms;
    config.startPortMapper = true;
    config.simulationStartedCallback = [&serverData](SimulationTime) {
        OnSimulationStartedCallback(serverData);
    };
    config.simulationStoppedCallback = [&serverData](SimulationTime) {
        OnSimulationStoppedCallback(serverData);
    };
    config.simulationPausedCallback = [&serverData](SimulationTime) {
        OnSimulationPausedCallback(serverData);
    };
    config.simulationContinuedCallback = [&serverData](SimulationTime) {
        OnSimulationContinuedCallback(serverData);
    };
    config.simulationTerminatedCallback = [&serverData](SimulationTime, TerminateReason) {
        OnSimulationTerminatedCallback(serverData);
    };
    config.canMessageContainerReceivedCallback = OnCanMessageContainerReceived;
    config.ethMessageContainerReceivedCallback = OnEthMessageContainerReceived;
    config.linMessageContainerReceivedCallback = OnLinMessageContainerReceived;
    config.frMessageContainerReceivedCallback = OnFrMessageContainerReceived;
    config.incomingSignalChangedCallback = OnIncomingSignalChanged;
    config.canControllers = CreateCanControllers(2);
    config.ethControllers = CreateEthControllers(2);
    config.linControllers = CreateLinControllers(2);
    config.frControllers = CreateFrControllers(2);
    config.incomingSignals = CreateSignals(2);
    config.outgoingSignals = CreateSignals(2);

    serverData.server = std::make_unique<ServerWrapper>();
    CheckResult(serverData.server->Load(config));

    serverData.state = SimulationState::Stopped;

    serverData.server->StartBackgroundThread();

    LogInfo("Loaded.");
    return CreateOk();
}

void UnloadSimulation(ServerData& serverData) {
    LogInfo("Unloading ...");

    StopSimulationThread(serverData);
    serverData.server.reset();

    serverData.state = SimulationState::Unloaded;

    LogInfo("Unloaded.");
}

[[nodiscard]] Result HostServer(ServerData& serverData, bool isClientOptional, const std::string& name) {
    CheckResult(LoadSimulation(serverData, isClientOptional, name));

    while (true) {
        switch (GetChar()) {
            case CTRL('c'):
                return CreateOk();
            case 'l':
                CheckResult(LoadSimulation(serverData, isClientOptional, name));
                break;
            case 'u':
                UnloadSimulation(serverData);
                break;
            case 's':
            case 'p':
            case 'k':
            case ' ':
                CheckResult(StartOrPauseSimulation(serverData));
                break;
            case 't':
                CheckResult(StopSimulation(serverData));
                break;
            case CTRL('t'):
                CheckResult(TerminateSimulation(serverData));
                break;
            case '1':
                SwitchSendingIoSignals(serverData);
                break;
            case '2':
                SwitchSendingCanMessages(serverData);
                break;
            case '3':
                SwitchSendingEthMessages(serverData);
                break;
            case '4':
                SwitchSendingLinMessages(serverData);
                break;
            case '5':
                SwitchSendingFrMessages(serverData);
                break;
            case '9':
                SwitchPrintingStepsPerSecond(serverData);
                break;
            default:
                LogError("Unknown key.");
                break;
        }
    }
}

}  // namespace

int main(int argc, char** argv) {
    try {
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

        ServerData serverData{};

        Result result = HostServer(serverData, isClientOptional, name);

        UnloadSimulation(serverData);

        return IsOk(result) ? 0 : 1;
    } catch (...) {
        return 1;
    }
}
