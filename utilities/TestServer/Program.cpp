// Copyright dSPACE GmbH. All rights reserved.

#include <fmt/color.h>

#ifdef _WIN32
#include <Windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>

#include <cstdlib>
#endif

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

void InitializeOutput() {
#if _WIN32
    (void)SetConsoleOutputCP(CP_UTF8);
    (void)setvbuf(stdout, nullptr, _IONBF, 0);

    const HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

    DWORD dwMode = 0;
    const BOOL result = GetConsoleMode(console, &dwMode);
    if (result != 0) {
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        (void)SetConsoleMode(console, dwMode);
    }
#endif
}

[[nodiscard]] int32_t GetChar() {
#ifdef _WIN32
    return _getch();
#else
    termios oldt{};
    tcgetattr(STDIN_FILENO, &oldt);

    termios newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);

    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    int32_t character = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return character;
#endif
}

#ifndef CTRL
#define CTRL(c) ((c) & 037)
#endif

void OnLogCallback(const Severity severity, std::string_view message) {
    switch (severity) {
        case Severity::Error:
            print(fg(fmt::color::red), "{}\n", message);
            break;
        case Severity::Warning:
            print(fg(fmt::color::yellow), "{}\n", message);
            break;
        case Severity::Info:
            print(fg(fmt::color::white), "{}\n", message);
            break;
        case Severity::Trace:
            print(fg(fmt::color::light_gray), "{}\n", message);
            break;
    }
}

bool SendIoData;
bool SendCanMessages;
bool SendEthMessages;
bool SendLinMessages;

bool StopSimulationThreadFlag;
std::thread SimulationThread;

SimulationTime CurrentTime;

std::unique_ptr<CoSimServer> Server;
CoSimServerConfig Config;
SimulationState State;

Event StopBackgroundThreadFlag;
std::thread BackgroundThread;
std::thread::id SimulationThreadId;

std::mutex Mutex;

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

void LogCanMessage([[maybe_unused]] const SimulationTime simulationTime,
                   [[maybe_unused]] const CanController& controller,
                   const CanMessage& message) {
    print(fg(fmt::color::dodger_blue), "{}\n", ToString(message));
}

void LogEthMessage([[maybe_unused]] const SimulationTime simulationTime,
                   [[maybe_unused]] const EthController& controller,
                   const EthMessage& message) {
    print(fg(fmt::color::cyan), "{}\n", ToString(message));
}

void LogLinMessage([[maybe_unused]] const SimulationTime simulationTime,
                   [[maybe_unused]] const LinController& controller,
                   const LinMessage& message) {
    print(fg(fmt::color::lime), "{}\n", ToString(message));
}

[[nodiscard]] int32_t Random(const int32_t min, const int32_t max) {
    static bool first = true;
    if (first) {
        srand(42);
        first = false;
    }

    const int32_t diff = max + 1 - min;

    return min + (rand() % diff);
}

[[nodiscard]] uint32_t GenerateU32(const uint32_t min, const uint32_t max) {
    return static_cast<uint32_t>(Random(static_cast<int32_t>(min), static_cast<int32_t>(max)));
}

[[nodiscard]] uint8_t GenerateU8() {
    return static_cast<uint8_t>(Random(0U, UINT8_MAX));
}

[[nodiscard]] uint32_t GenerateU32() {
    return GenerateU32(0, INT32_MAX);
}

[[nodiscard]] uint64_t GenerateU64() {
    return (static_cast<uint64_t>(GenerateU32()) << sizeof(uint32_t)) + static_cast<uint64_t>(GenerateU32());
}

[[nodiscard]] BusMessageId GenerateBusMessageId(const uint32_t min, const uint32_t max) {
    return static_cast<BusMessageId>(GenerateU32(min, max));
}

[[nodiscard]] SimulationTime GenerateSimulationTime() {
    return SimulationTime(GenerateU64());
}

[[nodiscard]] std::vector<uint8_t> GenerateBytes(const size_t length) {
    std::vector<uint8_t> data;
    data.resize(length);
    for (size_t i = 0; i < length; i++) {
        data[i] = GenerateU8();
    }

    return data;
}

void WriteOutGoingSignal(const IoSignalContainer& ioSignal) {
    const size_t length = GetDataTypeSize(ioSignal.dataType) * ioSignal.length;
    const std::vector<uint8_t> data = GenerateBytes(length);

    std::lock_guard lock(Mutex);
    Server->Write(ioSignal.id, static_cast<uint32_t>(length), data.data());
}

[[nodiscard]] bool TransmitCanMessage(const CanControllerContainer& controller) {
    const uint32_t length = GenerateU32(1U, 8U);
    const std::vector<uint8_t> data = GenerateBytes(length);

    CanMessage message{};
    message.controllerId = controller.id;
    message.id = GenerateBusMessageId(0U, 127U);
    message.timestamp = GenerateSimulationTime();
    message.length = length;
    message.data = data.data();

    std::lock_guard lock(Mutex);
    return Server->Transmit(message);
}

[[nodiscard]] bool TransmitEthMessage(const EthControllerContainer& controller) {
    const uint32_t length = GenerateU32(15U, 28U);
    const std::vector<uint8_t> data = GenerateBytes(length);

    EthMessage message{};
    message.controllerId = controller.id;
    message.timestamp = GenerateSimulationTime();
    message.length = length;
    message.data = data.data();

    std::lock_guard lock(Mutex);
    return Server->Transmit(message);
}

[[nodiscard]] bool TransmitLinMessage(const LinControllerContainer& controller) {
    const uint32_t length = GenerateU32(1U, LinMessageMaxLength);
    const std::vector<uint8_t> data = GenerateBytes(length);

    LinMessage message{};
    message.controllerId = controller.id;
    message.id = GenerateBusMessageId(0, 63);
    message.timestamp = GenerateSimulationTime();
    message.length = length;
    message.data = data.data();

    std::lock_guard lock(Mutex);
    return Server->Transmit(message);
}

[[nodiscard]] bool SendSomeData(const SimulationTime simulationTime) {
    static SimulationTime lastHalfSecond = -1s;
    static int64_t counter = 0;
    const SimulationTime currentHalfSecond = simulationTime / 500000000;
    if (currentHalfSecond == lastHalfSecond) {
        return true;
    }

    lastHalfSecond = currentHalfSecond;
    counter++;

    if (SendIoData && ((counter % 4) == 0)) {
        for (const IoSignalContainer& signal : Config.incomingSignals) {
            WriteOutGoingSignal(signal);
        }
    }

    if (SendCanMessages && ((counter % 4) == 1)) {
        for (const CanControllerContainer& controller : Config.canControllers) {
            CheckResult(TransmitCanMessage(controller));
        }
    }

    if (SendEthMessages && ((counter % 4) == 2)) {
        for (const EthControllerContainer& controller : Config.ethControllers) {
            CheckResult(TransmitEthMessage(controller));
        }
    }

    if (SendLinMessages && ((counter % 4) == 3)) {
        for (const LinControllerContainer& controller : Config.linControllers) {
            CheckResult(TransmitLinMessage(controller));
        }
    }

    return true;
}

void StartBackgroundThread() {
    BackgroundThread = std::thread([] {
        while (!StopBackgroundThreadFlag.Wait(1)) {
            try {
                std::lock_guard lock(Mutex);
                Server->BackgroundService();
            } catch (const std::exception& e) {
                LogError(e.what());
            }
        }
    });
}

void StopBackgroundThread() {
    if (!BackgroundThread.joinable()) {
        return;
    }

    StopBackgroundThreadFlag.Set();
    if (std::this_thread::get_id() == BackgroundThread.get_id()) {
        BackgroundThread.detach();
    } else {
        BackgroundThread.join();
    }
}

void DoSimulation() {
    StopBackgroundThread();

    SimulationThreadId = std::this_thread::get_id();
    while (!StopSimulationThreadFlag) {
        if (!SendSomeData(CurrentTime)) {
            break;
        }

        SimulationTime nextSimulationTime{};
        std::lock_guard lock(Mutex);
        Server->Step(CurrentTime, nextSimulationTime);

        if (nextSimulationTime > CurrentTime) {
            CurrentTime = nextSimulationTime;
        } else {
            CurrentTime += 1ms;
        }
    }

    StartBackgroundThread();
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

    {
        std::lock_guard lock(Mutex);
        Server->Start(CurrentTime);
    }

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
    {
        std::lock_guard lock(Mutex);
        Server->Stop(CurrentTime);
    }

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

    {
        std::lock_guard lock(Mutex);
        Server->Pause(CurrentTime);
    }

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

    {
        std::lock_guard lock(Mutex);
        Server->Continue(CurrentTime);
    }

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

    {
        std::lock_guard lock(Mutex);
        Server->Terminate(CurrentTime, TerminateReason::Error);
    }

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

[[nodiscard]] std::vector<CanControllerContainer> CreateCanControllers() {
    std::vector<CanControllerContainer> controllers;

    CanControllerContainer controller{};
    controller.id = static_cast<BusControllerId>(1);
    controller.queueSize = 512;
    controller.bitsPerSecond = 1000;
    controller.flexibleDataRateBitsPerSecond = 1000;
    controller.name = "CanController1";
    controller.channelName = "CanChannel1";
    controller.clusterName = "CanCluster1";
    controllers.push_back(controller);

    return controllers;
}

[[nodiscard]] std::vector<EthControllerContainer> CreateEthControllers() {
    std::vector<EthControllerContainer> controllers;

    EthControllerContainer controller{};
    controller.id = static_cast<BusControllerId>(1);
    controller.queueSize = 512;
    controller.bitsPerSecond = 1000;
    controller.macAddress = {0, 1, 2, 3, 4, 5};
    controller.name = "EthController1";
    controller.channelName = "EthChannel1";
    controller.clusterName = "EthCluster1";
    controllers.push_back(controller);

    return controllers;
}

[[nodiscard]] std::vector<LinControllerContainer> CreateLinControllers() {
    std::vector<LinControllerContainer> controllers;

    LinControllerContainer controller{};
    controller.id = static_cast<BusControllerId>(1);
    controller.queueSize = 512;
    controller.bitsPerSecond = 1000;
    controller.type = LinControllerType::Responder;
    controller.name = "LinController1";
    controller.channelName = "LinChannel1";
    controller.clusterName = "LinCluster1";
    controllers.push_back(controller);

    return controllers;
}

[[nodiscard]] std::vector<IoSignalContainer> CreateIncomingSignals() {
    std::vector<IoSignalContainer> signals;

    IoSignalContainer signal{};
    signal.id = static_cast<IoSignalId>(1);
    signal.length = 4;
    signal.dataType = DataType::UInt8;
    signal.sizeKind = SizeKind::Variable;
    signal.name = "IncomingIoSignal1";
    signals.push_back(signal);

    return signals;
}

[[nodiscard]] std::vector<IoSignalContainer> CreateOutgoingSignals() {
    std::vector<IoSignalContainer> signals;

    IoSignalContainer signal{};
    signal.id = static_cast<IoSignalId>(1);
    signal.length = 4;
    signal.dataType = DataType::UInt8;
    signal.sizeKind = SizeKind::Variable;
    signal.name = "OutgoingIoSignal1";
    signals.push_back(signal);

    return signals;
}

void LoadSimulation(const bool isClientOptional, const std::string_view name) {
    LogInfo("Loading ...");

    if (State != SimulationState::Unloaded) {
        LogError("Could not load in state " + ToString(State) + ".");
        return;
    }

    Config.serverName = name;
    Config.logCallback = OnLogCallback;
    Config.isClientOptional = isClientOptional;
    Config.stepSize = 1ms;
    Config.startPortMapper = true;
    Config.simulationStartedCallback = OnSimulationStartedCallback;
    Config.simulationStoppedCallback = OnSimulationStoppedCallback;
    Config.simulationPausedCallback = OnSimulationPausedCallback;
    Config.simulationContinuedCallback = OnSimulationContinuedCallback;
    Config.simulationTerminatedCallback = OnSimulationTerminatedCallback;
    Config.canMessageReceivedCallback = LogCanMessage;
    Config.ethMessageReceivedCallback = LogEthMessage;
    Config.linMessageReceivedCallback = LogLinMessage;
    Config.canControllers = CreateCanControllers();
    Config.ethControllers = CreateEthControllers();
    Config.linControllers = CreateLinControllers();
    Config.incomingSignals = CreateIncomingSignals();
    Config.outgoingSignals = CreateOutgoingSignals();

    {
        std::lock_guard lock(Mutex);
        Server = std::make_unique<CoSimServer>();
        Server->Load(Config);
    }

    State = SimulationState::Stopped;

    StartBackgroundThread();

    LogInfo("Loaded.");
}

void UnloadSimulation() {
    LogInfo("Unloading ...");

    StopSimulationThread();
    StopBackgroundThread();

    {
        std::lock_guard lock(Mutex);
        Server.reset();
    }

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
