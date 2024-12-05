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
#include "DsVeosCoSim/DsVeosCoSim.h"

using namespace DsVeosCoSim;
using namespace std::chrono_literals;

namespace {

void InitializeOutput() {
#if _WIN32
    (void)::SetConsoleOutputCP(CP_UTF8);
    (void)::setvbuf(stdout, nullptr, _IONBF, 0);

    HANDLE console = ::GetStdHandle(STD_OUTPUT_HANDLE);

    DWORD dwMode = 0;
    if (::GetConsoleMode(console, &dwMode) != 0) {
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        (void)::SetConsoleMode(console, dwMode);
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

void OnLogCallback(DsVeosCoSim_Severity severity, std::string_view message) {
    switch (severity) {
        case DsVeosCoSim_Severity_Error:
            fmt::print(fmt::fg(fmt::color::red), "{}\n", message);
            break;
        case DsVeosCoSim_Severity_Warning:
            fmt::print(fmt::fg(fmt::color::yellow), "{}\n", message);
            break;
        case DsVeosCoSim_Severity_Info:
            fmt::print(fmt::fg(fmt::color::white), "{}\n", message);
            break;
        case DsVeosCoSim_Severity_Trace:
            fmt::print(fmt::fg(fmt::color::light_gray), "{}\n", message);
            break;
        case DsVeosCoSim_Severity_INT_MAX_SENTINEL_DO_NOT_USE_:
            break;
    }
}

bool sendIoData;
bool sendCanMessages;
bool sendEthMessages;
bool sendLinMessages;

bool stopSimulationThread;
std::thread simulationThread;

DsVeosCoSim_SimulationTime currentTime;

std::unique_ptr<CoSimServer> server;
CoSimServerConfig config;
SimulationState state;

Event stopBackgroundThread;
std::thread backgroundThread;

std::thread::id simulationThreadId;

void PrintStatus(bool value, const std::string& what) {
    if (value) {
        LogInfo("Enabled sending " + what);
    } else {
        LogInfo("Disabled sending " + what);
    }
}

void SwitchSendingIoSignals() {
    sendIoData = !sendIoData;
    PrintStatus(sendIoData, "IO data");
}

void SwitchSendingCanMessages() {
    sendCanMessages = !sendCanMessages;
    PrintStatus(sendCanMessages, "CAN messages");
}

void SwitchSendingEthMessages() {
    sendEthMessages = !sendEthMessages;
    PrintStatus(sendEthMessages, "ETH messages");
}

void SwitchSendingLinMessages() {
    sendLinMessages = !sendLinMessages;
    PrintStatus(sendLinMessages, "LIN messages");
}

void LogCanMessage(DsVeosCoSim_SimulationTime simulationTime,
                   const DsVeosCoSim_CanController& controller,
                   const DsVeosCoSim_CanMessage& message) {
    fmt::print(fmt::fg(fmt::color::dodger_blue), "{}\n", CanMessageToString(simulationTime, controller, message));
}

void LogEthMessage(DsVeosCoSim_SimulationTime simulationTime,
                   const DsVeosCoSim_EthController& controller,
                   const DsVeosCoSim_EthMessage& message) {
    fmt::print(fmt::fg(fmt::color::cyan), "{}\n", EthMessageToString(simulationTime, controller, message));
}

void LogLinMessage(DsVeosCoSim_SimulationTime simulationTime,
                   const DsVeosCoSim_LinController& controller,
                   const DsVeosCoSim_LinMessage& message) {
    fmt::print(fmt::fg(fmt::color::lime), "{}\n", LinMessageToString(simulationTime, controller, message));
}

[[nodiscard]] int32_t Random(int32_t min, int32_t max) {
    static bool first = true;
    if (first) {
        srand(42);
        first = false;
    }

    const int32_t diff = max + 1 - min;

    return min + rand() % diff;
}

[[nodiscard]] uint32_t GenerateU32(uint32_t min, uint32_t max) {
    return static_cast<uint32_t>(Random(static_cast<int32_t>(min), static_cast<int32_t>(max)));
}

[[nodiscard]] uint8_t GenerateU8() {
    return Random(static_cast<uint8_t>(0U), static_cast<uint8_t>(UINT8_MAX));
}

[[nodiscard]] uint32_t GenerateU32() {
    return GenerateU32(0, INT32_MAX);
}

[[nodiscard]] int64_t GenerateI64() {
    return (static_cast<int64_t>(GenerateU32()) << sizeof(uint32_t)) + static_cast<int64_t>(GenerateU32());
}

[[nodiscard]] std::vector<uint8_t> GenerateBytes(size_t length) {
    std::vector<uint8_t> data;
    data.resize(length);
    for (size_t i = 0; i < length; i++) {
        data[i] = GenerateU8();
    }

    return data;
}

void WriteOutGoingSignal(const IoSignal& ioSignal) {
    size_t length = GetDataTypeSize(ioSignal.dataType) * ioSignal.length;
    std::vector<uint8_t> data = GenerateBytes(length);

    server->Write(ioSignal.id, static_cast<uint32_t>(length), data.data());
}

[[nodiscard]] bool TransmitCanMessage(const CanController& controller) {
    const uint32_t length = GenerateU32(1, 8);
    std::vector<uint8_t> data = GenerateBytes(length);

    DsVeosCoSim_CanMessage message{};
    message.controllerId = controller.id;
    message.id = GenerateU32(0, 127);
    message.timestamp = GenerateI64();
    message.length = length;
    message.data = data.data();

    return server->Transmit(message);
}

[[nodiscard]] bool TransmitEthMessage(const DsVeosCoSim_EthController& controller) {
    const uint32_t length = GenerateU32(15, 28);
    std::vector<uint8_t> data = GenerateBytes(length);

    DsVeosCoSim_EthMessage message{};
    message.controllerId = controller.id;
    message.timestamp = GenerateI64();
    message.length = length;
    message.data = data.data();

    return server->Transmit(message);
}

[[nodiscard]] bool TransmitLinMessage(const DsVeosCoSim_LinController& controller) {
    const uint32_t length = GenerateU32(1, DSVEOSCOSIM_LIN_MESSAGE_MAX_LENGTH);
    std::vector<uint8_t> data = GenerateBytes(length);

    DsVeosCoSim_LinMessage message{};
    message.controllerId = controller.id;
    message.id = GenerateU32(0, 63);
    message.timestamp = GenerateI64();
    message.length = length;
    message.data = data.data();

    return server->Transmit(message);
}

[[nodiscard]] bool SendSomeData(DsVeosCoSim_SimulationTime simulationTime) {
    static int64_t lastHalfSecond = -1;
    static int64_t counter = 0;
    const int64_t currentHalfSecond = simulationTime / 500000000;
    if (currentHalfSecond == lastHalfSecond) {
        return true;
    }

    lastHalfSecond = currentHalfSecond;
    counter++;

    if (sendIoData && ((counter % 4) == 0)) {
        for (const IoSignal& signal : config.incomingSignals) {
            WriteOutGoingSignal(signal);
        }
    }

    if (sendCanMessages && ((counter % 4) == 1)) {
        for (const CanController& controller : config.canControllers) {
            CheckResult(TransmitCanMessage(controller));
        }
    }

    if (sendEthMessages && ((counter % 4) == 2)) {
        for (const EthController& controller : config.ethControllers) {
            CheckResult(TransmitEthMessage(controller));
        }
    }

    if (sendLinMessages && ((counter % 4) == 3)) {
        for (const LinController& controller : config.linControllers) {
            CheckResult(TransmitLinMessage(controller));
        }
    }

    return true;
}

void StartBackgroundThread() {
    backgroundThread = std::thread([] {
        while (!stopBackgroundThread.Wait(1)) {
            try {
                server->BackgroundService();
            } catch (const std::exception& e) {
                LogError(e.what());
            }
        }
    });
}

void StopBackgroundThread() {
    if (!backgroundThread.joinable()) {
        return;
    }

    stopBackgroundThread.Set();
    if (std::this_thread::get_id() == backgroundThread.get_id()) {
        backgroundThread.detach();
    } else {
        backgroundThread.join();
    }
}

void DoSimulation() {
    StopBackgroundThread();

    simulationThreadId = std::this_thread::get_id();
    while (!stopSimulationThread) {
        if (!SendSomeData(currentTime)) {
            break;
        }

        DsVeosCoSim_SimulationTime nextSimulationTime{};
        server->Step(currentTime, nextSimulationTime);

        if (nextSimulationTime > currentTime) {
            currentTime = nextSimulationTime;
        } else {
            currentTime += 1000000;
        }
    }

    StartBackgroundThread();
}

void StopSimulationThread() {
    stopSimulationThread = true;

    if (simulationThreadId == std::this_thread::get_id()) {
        // It's called from inside the simulation thread. That won't work. So let the next simulation thread starter
        // join this thread
        return;
    }

    if (simulationThread.joinable()) {
        simulationThread.join();
    }

    simulationThread = {};
    simulationThreadId = {};
}

void StartSimulationThread() {
    StopSimulationThread();

    stopSimulationThread = false;
    simulationThread = std::thread(DoSimulation);
}

void StartSimulation() {
    if (state == SimulationState::Running) {
        return;
    }

    if (state != SimulationState::Stopped) {
        LogError("Could not start in state " + ToString(state) + ".");
        return;
    }

    currentTime = 0;
    LogInfo("Starting ...");
    StopBackgroundThread();

    server->Start(currentTime);

    StartSimulationThread();
    state = SimulationState::Running;

    LogInfo("Started.");
}

void StopSimulation() {
    if (state == SimulationState::Stopped) {
        return;
    }

    if ((state != SimulationState::Running) && (state != SimulationState::Paused)) {
        LogError("Could not stop in state " + ToString(state) + ".");
        return;
    }

    LogInfo("Stopping ...");

    StopSimulationThread();
    server->Stop(currentTime);
    state = SimulationState::Stopped;

    LogInfo("Stopped.");
}

void PauseSimulation() {
    if (state == SimulationState::Paused) {
        return;
    }

    if (state != SimulationState::Running) {
        LogError("Could not pause in state " + ToString(state) + ".");
        return;
    }

    LogInfo("Pausing ...");

    StopSimulationThread();
    server->Pause(currentTime);
    state = SimulationState::Paused;

    LogInfo("Paused.");
}

void ContinueSimulation() {
    if (state == SimulationState::Running) {
        return;
    }

    if (state != SimulationState::Paused) {
        LogError("Could not start in state " + ToString(state) + ".");
        return;
    }

    LogInfo("Continuing ...");
    server->Continue(currentTime);

    StartSimulationThread();
    state = SimulationState::Running;

    LogInfo("Continued.");
}

void TerminateSimulation() {
    if (state == SimulationState::Terminated) {
        return;
    }

    if (state == SimulationState::Unloaded) {
        LogError("Could not terminate in state " + ToString(state) + ".");
        return;
    }

    LogInfo("Terminating ...");

    StopSimulationThread();

    server->Terminate(currentTime, DsVeosCoSim_TerminateReason_Error);
    state = SimulationState::Terminated;

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

[[nodiscard]] std::vector<CanController> CreateCanControllers() {
    std::vector<CanController> controllers;

    CanController controller{};
    controller.id = 1;
    controller.queueSize = 512;
    controller.bitsPerSecond = 1000;
    controller.flexibleDataRateBitsPerSecond = 1000;
    controller.name = "CanController1";
    controller.channelName = "CanChannel1";
    controller.clusterName = "CanCluster1";
    controllers.push_back(controller);

    return controllers;
}

[[nodiscard]] std::vector<EthController> CreateEthControllers() {
    std::vector<EthController> controllers;

    EthController controller{};
    controller.id = 1;
    controller.queueSize = 512;
    controller.bitsPerSecond = 1000;
    controller.macAddress = {0, 1, 2, 3, 4, 5};
    controller.name = "EthController1";
    controller.channelName = "EthChannel1";
    controller.clusterName = "EthCluster1";
    controllers.push_back(controller);

    return controllers;
}

[[nodiscard]] std::vector<LinController> CreateLinControllers() {
    std::vector<LinController> controllers;

    LinController controller{};
    controller.id = 1;
    controller.queueSize = 512;
    controller.bitsPerSecond = 1000;
    controller.type = DsVeosCoSim_LinControllerType_Responder;
    controller.name = "LinController1";
    controller.channelName = "LinChannel1";
    controller.clusterName = "LinCluster1";
    controllers.push_back(controller);

    return controllers;
}

[[nodiscard]] std::vector<IoSignal> CreateIncomingSignals() {
    std::vector<IoSignal> signals;

    IoSignal signal{};
    signal.id = 1;
    signal.length = 4;
    signal.dataType = DsVeosCoSim_DataType_UInt8;
    signal.sizeKind = DsVeosCoSim_SizeKind_Variable;
    signal.name = "IncomingIoSignal1";
    signals.push_back(signal);

    return signals;
}

[[nodiscard]] std::vector<IoSignal> CreateOutgoingSignals() {
    std::vector<IoSignal> signals;

    IoSignal signal{};
    signal.id = 1;
    signal.length = 4;
    signal.dataType = DsVeosCoSim_DataType_UInt8;
    signal.sizeKind = DsVeosCoSim_SizeKind_Variable;
    signal.name = "OutgoingIoSignal1";
    signals.push_back(signal);

    return signals;
}

void LoadSimulation(bool isClientOptional, std::string_view name) {
    LogInfo("Loading ...");

    if (state != SimulationState::Unloaded) {
        LogError("Could not load in state " + ToString(state) + ".");
        return;
    }

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
    config.canControllers = CreateCanControllers();
    config.ethControllers = CreateEthControllers();
    config.linControllers = CreateLinControllers();
    config.incomingSignals = CreateIncomingSignals();
    config.outgoingSignals = CreateOutgoingSignals();

    server = std::make_unique<CoSimServer>();
    server->Load(config);
    state = SimulationState::Stopped;

    StartBackgroundThread();

    LogInfo("Loaded.");
}

void UnloadSimulation() {
    LogInfo("Unloading ...");

    StopSimulationThread();
    StopBackgroundThread();
    server.reset();
    state = SimulationState::Unloaded;

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
