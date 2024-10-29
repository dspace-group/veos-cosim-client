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

#include <cstdint>
#include <cstring>
#include <functional>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "DsVeosCoSim/DsVeosCoSim.h"

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

DsVeosCoSim_Handle handle;

std::thread simulationThread;

bool sendIoData;
bool sendCanMessages;
bool sendEthMessages;
bool sendLinMessages;

uint32_t canControllersCount;
const DsVeosCoSim_CanController* canControllers;

uint32_t ethControllersCount;
const DsVeosCoSim_EthController* ethControllers;

uint32_t linControllersCount;
const DsVeosCoSim_LinController* linControllers;

uint32_t incomingSignalsCount;
const DsVeosCoSim_IoSignal* incomingSignals;

uint32_t outgoingSignalsCount;
const DsVeosCoSim_IoSignal* outgoingSignals;

void OnLogCallback(DsVeosCoSim_Severity severity, const char* message) {
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

void LogError(std::string_view message) {
    OnLogCallback(DsVeosCoSim_Severity_Error, message.data());
}

void LogInfo(std::string_view message) {
    OnLogCallback(DsVeosCoSim_Severity_Info, message.data());
}

void LogTrace(std::string_view message) {
    OnLogCallback(DsVeosCoSim_Severity_Trace, message.data());
}

#define CheckResult(expression)                  \
    do {                                         \
        DsVeosCoSim_Result _result = expression; \
        if (_result != DsVeosCoSim_Result_Ok) {  \
            return _result;                      \
        }                                        \
    } while (0)

#define CheckResultWithMessage(expression, message) \
    do {                                            \
        DsVeosCoSim_Result _result = expression;    \
        if (_result != DsVeosCoSim_Result_Ok) {     \
            LogTrace(message);                      \
            return _result;                         \
        }                                           \
    } while (0)

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

[[nodiscard]] int32_t Random(int32_t min, int32_t max) {
    static bool first = true;
    if (first) {
        srand(21);
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

[[nodiscard]] DsVeosCoSim_Result WriteOutGoingSignal(const DsVeosCoSim_IoSignal& ioSignal) {
    size_t length = DsVeosCoSim_GetDataTypeSize(ioSignal.dataType) * ioSignal.length;
    std::vector<uint8_t> data = GenerateBytes(length);

    CheckResultWithMessage(DsVeosCoSim_WriteOutgoingSignal(handle, ioSignal.id, ioSignal.length, data.data()),
                           "Could not write outgoing signal.");

    return DsVeosCoSim_Result_Ok;
}

[[nodiscard]] DsVeosCoSim_Result TransmitCanMessage(const DsVeosCoSim_CanController& controller) {
    const uint32_t length = GenerateU32(1, 8);
    std::vector<uint8_t> data = GenerateBytes(length);

    DsVeosCoSim_CanMessage message{};
    message.controllerId = controller.id;
    message.id = GenerateU32();
    message.timestamp = GenerateI64();
    message.length = length;
    message.data = data.data();

    CheckResultWithMessage(DsVeosCoSim_TransmitCanMessage(handle, &message), "Could not transmit CAN message.");

    return DsVeosCoSim_Result_Ok;
}

[[nodiscard]] DsVeosCoSim_Result TransmitEthMessage(const DsVeosCoSim_EthController& controller) {
    const uint32_t length = GenerateU32(15, 28);
    std::vector<uint8_t> data = GenerateBytes(length);

    DsVeosCoSim_EthMessage message{};
    message.controllerId = controller.id;
    message.timestamp = GenerateI64();
    message.length = length;
    message.data = data.data();

    CheckResultWithMessage(DsVeosCoSim_TransmitEthMessage(handle, &message), "Could not transmit ETH message.");

    return DsVeosCoSim_Result_Ok;
}

[[nodiscard]] DsVeosCoSim_Result TransmitLinMessage(const DsVeosCoSim_LinController& controller) {
    const uint32_t length = GenerateU32(1, DSVEOSCOSIM_LIN_MESSAGE_MAX_LENGTH);
    std::vector<uint8_t> data = GenerateBytes(length);

    DsVeosCoSim_LinMessage message{};
    message.controllerId = controller.id;
    message.id = GenerateU32(0, 63);
    message.timestamp = GenerateI64();
    message.length = length;
    message.data = data.data();

    CheckResultWithMessage(DsVeosCoSim_TransmitLinMessage(handle, &message), "Could not transmit LIN message.");

    return DsVeosCoSim_Result_Ok;
}

[[nodiscard]] DsVeosCoSim_Result SendSomeData(DsVeosCoSim_SimulationTime simulationTime) {
    static int64_t lastHalfSecond = -1;
    static int64_t counter = 0;
    const int64_t currentHalfSecond = simulationTime / 500000000;
    if (currentHalfSecond == lastHalfSecond) {
        return DsVeosCoSim_Result_Ok;
    }

    lastHalfSecond = currentHalfSecond;
    counter++;

    if (sendIoData && ((counter % 4) == 0)) {
        for (uint32_t i = 0; i < outgoingSignalsCount; i++) {
            const DsVeosCoSim_IoSignal& ioSignal = outgoingSignals[i];
            CheckResult(WriteOutGoingSignal(ioSignal));
        }
    }

    if (sendCanMessages && ((counter % 4) == 1)) {
        for (uint32_t i = 0; i < canControllersCount; i++) {
            const DsVeosCoSim_CanController& controller = canControllers[i];
            CheckResult(TransmitCanMessage(controller));
        }
    }

    if (sendEthMessages && ((counter % 4) == 2)) {
        for (uint32_t i = 0; i < ethControllersCount; i++) {
            const DsVeosCoSim_EthController& controller = ethControllers[i];
            CheckResult(TransmitEthMessage(controller));
        }
    }

    if (sendLinMessages && ((counter % 4) == 3)) {
        for (uint32_t i = 0; i < linControllersCount; i++) {
            const DsVeosCoSim_LinController& controller = linControllers[i];
            CheckResult(TransmitLinMessage(controller));
        }
    }

    return DsVeosCoSim_Result_Ok;
}

void LogIoData(DsVeosCoSim_SimulationTime simulationTime,
               const DsVeosCoSim_IoSignal* ioSignal,
               uint32_t length,
               const void* value,
               [[maybe_unused]] void* userData) {
    fmt::print(fmt::fg(fmt::color::fuchsia), "{}\n", DsVeosCoSim_IoDataToString(simulationTime, *ioSignal, length, value));
}

void LogCanMessage(DsVeosCoSim_SimulationTime simulationTime,
                   const DsVeosCoSim_CanController* controller,
                   const DsVeosCoSim_CanMessage* message,
                   [[maybe_unused]] void* userData) {
    fmt::print(fmt::fg(fmt::color::dodger_blue), "{}\n", DsVeosCoSim_CanMessageToString(simulationTime, *controller, *message));
}

void LogEthMessage(DsVeosCoSim_SimulationTime simulationTime,
                   const DsVeosCoSim_EthController* controller,
                   const DsVeosCoSim_EthMessage* message,
                   [[maybe_unused]] void* userData) {
    fmt::print(fmt::fg(fmt::color::cyan), "{}\n", DsVeosCoSim_EthMessageToString(simulationTime, *controller, *message));
}

void LogLinMessage(DsVeosCoSim_SimulationTime simulationTime,
                   const DsVeosCoSim_LinController* controller,
                   const DsVeosCoSim_LinMessage* message,
                   [[maybe_unused]] void* userData) {
    fmt::print(fmt::fg(fmt::color::lime), "{}\n", DsVeosCoSim_LinMessageToString(simulationTime, *controller, *message));
}

void OnSimulationPostStepCallback(DsVeosCoSim_SimulationTime simulationTime, [[maybe_unused]] void* userData) {
    (void)SendSomeData(simulationTime);
}

void StartSimulationThread(const std::function<void()>& function) {
    simulationThread = std::thread(function);
    simulationThread.detach();
}

void OnSimulationStartedCallback(DsVeosCoSim_SimulationTime simulationTime, [[maybe_unused]] void* userData) {
    LogInfo("Simulation started at " + DsVeosCoSim_SimulationTimeToString(simulationTime) + " s.");
}

void OnSimulationStoppedCallback(DsVeosCoSim_SimulationTime simulationTime, [[maybe_unused]] void* userData) {
    LogInfo("Simulation stopped at " + DsVeosCoSim_SimulationTimeToString(simulationTime) + " s.");
}

void OnSimulationTerminatedCallback(DsVeosCoSim_SimulationTime simulationTime,
                                    DsVeosCoSim_TerminateReason reason,
                                    [[maybe_unused]] void* userData) {
    LogInfo("Simulation terminated with reason " + DsVeosCoSim_TerminateReasonToString(reason) + " at " +
            DsVeosCoSim_SimulationTimeToString(simulationTime) + " s.");
}

void OnSimulationPausedCallback(DsVeosCoSim_SimulationTime simulationTime, [[maybe_unused]] void* userData) {
    LogInfo("Simulation paused at " + DsVeosCoSim_SimulationTimeToString(simulationTime) + " s.");
}

void OnSimulationContinuedCallback(DsVeosCoSim_SimulationTime simulationTime, [[maybe_unused]] void* userData) {
    LogInfo("Simulation continued at " + DsVeosCoSim_SimulationTimeToString(simulationTime) + " s.");
}

[[nodiscard]] DsVeosCoSim_Result Connect(std::string_view host, std::string_view serverName) {
    LogInfo("Connecting ...");

    DsVeosCoSim_ConnectionState connectionState{};
    CheckResultWithMessage(DsVeosCoSim_GetConnectionState(handle, &connectionState), "Could not get connection state.");

    if (connectionState == DsVeosCoSim_ConnectionState_Connected) {
        LogInfo("Already connected.");
        return DsVeosCoSim_Result_Ok;
    }

    DsVeosCoSim_ConnectConfig connectConfig{};
    connectConfig.clientName = "Example Test Client";
    connectConfig.serverName = serverName.data();
    connectConfig.remoteIpAddress = host.data();

    CheckResultWithMessage(DsVeosCoSim_Connect(handle, connectConfig), "Could not connect.");

    LogTrace("");

    DsVeosCoSim_SimulationTime stepSize{};
    CheckResultWithMessage(DsVeosCoSim_GetStepSize(handle, &stepSize), "Could not get step size.");
    LogTrace("Step size: " + DsVeosCoSim_SimulationTimeToString(stepSize) + " s");
    LogTrace("");

    CheckResultWithMessage(DsVeosCoSim_GetCanControllers(handle, &canControllersCount, &canControllers),
                           "Could not get CAN controllers.");
    if (canControllersCount > 0) {
        LogTrace("Found the following CAN controllers:");
        for (uint32_t i = 0; i < canControllersCount; i++) {
            LogTrace("  " + DsVeosCoSim_CanControllerToString(canControllers[i]));
        }

        LogTrace("");
    }

    CheckResultWithMessage(DsVeosCoSim_GetEthControllers(handle, &ethControllersCount, &ethControllers),
                           "Could not get ETH controllers.");
    if (ethControllersCount > 0) {
        LogTrace("Found the following ETH controllers:");
        for (uint32_t i = 0; i < ethControllersCount; i++) {
            LogTrace("  " + DsVeosCoSim_EthControllerToString(ethControllers[i]));
        }

        LogTrace("");
    }

    CheckResultWithMessage(DsVeosCoSim_GetLinControllers(handle, &linControllersCount, &linControllers),
                           "Could not get LIN controllers.");
    if (linControllersCount > 0) {
        LogTrace("Found the following LIN controllers:");
        for (uint32_t i = 0; i < linControllersCount; i++) {
            LogTrace("  " + DsVeosCoSim_LinControllerToString(linControllers[i]));
        }

        LogTrace("");
    }

    CheckResultWithMessage(DsVeosCoSim_GetIncomingSignals(handle, &incomingSignalsCount, &incomingSignals),
                           "Could not get incoming signals.");
    if (incomingSignalsCount > 0) {
        LogTrace("Found the following incoming signals:");
        for (uint32_t i = 0; i < incomingSignalsCount; i++) {
            LogTrace("  " + DsVeosCoSim_IoSignalToString(incomingSignals[i]));
        }

        LogTrace("");
    }

    CheckResultWithMessage(DsVeosCoSim_GetOutgoingSignals(handle, &outgoingSignalsCount, &outgoingSignals),
                           "Could not get outgoing signals.");
    if (outgoingSignalsCount > 0) {
        LogTrace("Found the following outgoing signals:");
        for (uint32_t i = 0; i < outgoingSignalsCount; i++) {
            LogTrace("  " + DsVeosCoSim_IoSignalToString(outgoingSignals[i]));
        }

        LogTrace("");
    }

    LogInfo("Connected.");
    return DsVeosCoSim_Result_Ok;
}

[[nodiscard]] DsVeosCoSim_Result Disconnect() {
    LogInfo("Disconnecting ...");
    CheckResultWithMessage(DsVeosCoSim_Disconnect(handle), "Could not disconnect.");
    LogInfo("Disconnected.");

    return DsVeosCoSim_Result_Ok;
}

void RunCallbackBasedCoSimulation() {
    DsVeosCoSim_Callbacks callbacks{};
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
    DsVeosCoSim_Result result = DsVeosCoSim_RunCallbackBasedCoSimulation(handle, callbacks);
    if ((result == DsVeosCoSim_Result_Disconnected) || (result == DsVeosCoSim_Result_Ok)) {
        exit(0);
    }

    LogError("DsVeosCoSim_RunCallbackBasedCoSimulation finished with the following error code: " +
             DsVeosCoSim_ResultToString(result) + ".");
    exit(1);
}

[[nodiscard]] DsVeosCoSim_Result HostClient(std::string_view host, std::string_view name) {
    CheckResult(Connect(host, name));

    StartSimulationThread(RunCallbackBasedCoSimulation);

    while (true) {
        switch (GetChar()) {
            case CTRL('c'):
                return Disconnect();
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
                CheckResultWithMessage(DsVeosCoSim_StartSimulation(handle), "Could not start simulation");
                break;
            case 'o':
                CheckResultWithMessage(DsVeosCoSim_StopSimulation(handle), "Could not stop simulation");
                break;
            case 'p':
                CheckResultWithMessage(DsVeosCoSim_PauseSimulation(handle), "Could not pause simulation");
                break;
            case 't':
                CheckResultWithMessage(DsVeosCoSim_TerminateSimulation(handle, DsVeosCoSim_TerminateReason_Error),
                                       "Could not terminate simulation");
                break;
            case 'n':
                CheckResultWithMessage(DsVeosCoSim_ContinueSimulation(handle), "Could not pause simulation");
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

    DsVeosCoSim_SetLogCallback(OnLogCallback);

    handle = DsVeosCoSim_Create();
    if (!handle) {
        LogError("Could not create handle.");
        return 1;
    }

    DsVeosCoSim_Result result = HostClient(host, name);

    DsVeosCoSim_Destroy(handle);

    return result == DsVeosCoSim_Result_Ok ? 0 : 1;
}
