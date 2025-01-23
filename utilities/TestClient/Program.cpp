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
#include <string_view>  // IWYU pragma: keep
#include <thread>
#include <vector>

#include "DsVeosCoSim/DsVeosCoSim.h"

namespace {

void InitializeOutput() {
#if _WIN32
    (void)SetConsoleOutputCP(CP_UTF8);
    (void)setvbuf(stdout, nullptr, _IONBF, 0);

    const HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);  // NOLINT

    DWORD dwMode = 0;
    if (GetConsoleMode(console, &dwMode) != 0) {
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

DsVeosCoSim_Handle Handle;

std::thread SimulationThread;

bool SendIoData;
bool SendCanMessages;
bool SendEthMessages;
bool SendLinMessages;

uint32_t CanControllersCount;
const DsVeosCoSim_CanController* CanControllers;

uint32_t EthControllersCount;
const DsVeosCoSim_EthController* EthControllers;

uint32_t LinControllersCount;
const DsVeosCoSim_LinController* LinControllers;

uint32_t IncomingSignalsCount;
const DsVeosCoSim_IoSignal* IncomingSignals;

uint32_t OutgoingSignalsCount;
const DsVeosCoSim_IoSignal* OutgoingSignals;

void OnLogCallback(const DsVeosCoSim_Severity severity, const char* message) {
    switch (severity) {
        case DsVeosCoSim_Severity_Error:
            print(fg(fmt::color::red), "{}\n", message);
            break;
        case DsVeosCoSim_Severity_Warning:
            print(fg(fmt::color::yellow), "{}\n", message);
            break;
        case DsVeosCoSim_Severity_Info:
            print(fg(fmt::color::white), "{}\n", message);
            break;
        case DsVeosCoSim_Severity_Trace:
            print(fg(fmt::color::light_gray), "{}\n", message);
            break;
        case DsVeosCoSim_Severity_INT_MAX_SENTINEL_DO_NOT_USE_:
            break;
    }
}

void LogError(const std::string_view message) {
    OnLogCallback(DsVeosCoSim_Severity_Error, message.data());
}

void LogInfo(const std::string_view message) {
    OnLogCallback(DsVeosCoSim_Severity_Info, message.data());
}

void LogTrace(const std::string_view message) {
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

[[nodiscard]] int32_t Random(const int32_t min, const int32_t max) {
    static bool first = true;
    if (first) {
        srand(21);  // NOLINT
        first = false;
    }

    const int32_t diff = max + 1 - min;

    return min + (rand() % diff);  // NOLINT
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

[[nodiscard]] int64_t GenerateI64() {
    return (static_cast<int64_t>(GenerateU32()) << sizeof(uint32_t)) + static_cast<int64_t>(GenerateU32());
}

[[nodiscard]] std::vector<uint8_t> GenerateBytes(const size_t length) {
    std::vector<uint8_t> data;
    data.resize(length);
    for (size_t i = 0; i < length; i++) {
        data[i] = GenerateU8();
    }

    return data;
}

[[nodiscard]] DsVeosCoSim_Result WriteOutGoingSignal(const DsVeosCoSim_IoSignal& ioSignal) {
    const size_t length = DsVeosCoSim_GetDataTypeSize(ioSignal.dataType) * ioSignal.length;
    const std::vector<uint8_t> data = GenerateBytes(length);

    CheckResultWithMessage(DsVeosCoSim_WriteOutgoingSignal(Handle, ioSignal.id, ioSignal.length, data.data()),
                           "Could not write outgoing signal.");

    return DsVeosCoSim_Result_Ok;
}

[[nodiscard]] DsVeosCoSim_Result TransmitCanMessage(const DsVeosCoSim_CanController& controller) {
    const uint32_t length = GenerateU32(1, 8);
    const std::vector<uint8_t> data = GenerateBytes(length);

    DsVeosCoSim_CanMessage message{};
    message.controllerId = controller.id;
    message.id = GenerateU32();
    message.timestamp = GenerateI64();
    message.length = length;
    message.data = data.data();

    CheckResultWithMessage(DsVeosCoSim_TransmitCanMessage(Handle, &message), "Could not transmit CAN message.");

    return DsVeosCoSim_Result_Ok;
}

[[nodiscard]] DsVeosCoSim_Result TransmitEthMessage(const DsVeosCoSim_EthController& controller) {
    const uint32_t length = GenerateU32(15, 28);
    const std::vector<uint8_t> data = GenerateBytes(length);

    DsVeosCoSim_EthMessage message{};
    message.controllerId = controller.id;
    message.timestamp = GenerateI64();
    message.length = length;
    message.data = data.data();

    CheckResultWithMessage(DsVeosCoSim_TransmitEthMessage(Handle, &message), "Could not transmit ETH message.");

    return DsVeosCoSim_Result_Ok;
}

[[nodiscard]] DsVeosCoSim_Result TransmitLinMessage(const DsVeosCoSim_LinController& controller) {
    const uint32_t length = GenerateU32(1, DSVEOSCOSIM_LIN_MESSAGE_MAX_LENGTH);
    const std::vector<uint8_t> data = GenerateBytes(length);

    DsVeosCoSim_LinMessage message{};
    message.controllerId = controller.id;
    message.id = GenerateU32(0, 63);
    message.timestamp = GenerateI64();
    message.length = length;
    message.data = data.data();

    CheckResultWithMessage(DsVeosCoSim_TransmitLinMessage(Handle, &message), "Could not transmit LIN message.");

    return DsVeosCoSim_Result_Ok;
}

[[nodiscard]] DsVeosCoSim_Result SendSomeData(const DsVeosCoSim_SimulationTime simulationTime) {
    static int64_t lastHalfSecond = -1;
    static int64_t counter = 0;
    const int64_t currentHalfSecond = simulationTime / 500000000;
    if (currentHalfSecond == lastHalfSecond) {
        return DsVeosCoSim_Result_Ok;
    }

    lastHalfSecond = currentHalfSecond;
    counter++;

    if (SendIoData && ((counter % 4) == 0)) {
        for (uint32_t i = 0; i < OutgoingSignalsCount; i++) {
            const DsVeosCoSim_IoSignal& ioSignal = OutgoingSignals[i];
            CheckResult(WriteOutGoingSignal(ioSignal));
        }
    }

    if (SendCanMessages && ((counter % 4) == 1)) {
        for (uint32_t i = 0; i < CanControllersCount; i++) {
            const DsVeosCoSim_CanController& controller = CanControllers[i];
            CheckResult(TransmitCanMessage(controller));
        }
    }

    if (SendEthMessages && ((counter % 4) == 2)) {
        for (uint32_t i = 0; i < EthControllersCount; i++) {
            const DsVeosCoSim_EthController& controller = EthControllers[i];
            CheckResult(TransmitEthMessage(controller));
        }
    }

    if (SendLinMessages && ((counter % 4) == 3)) {
        for (uint32_t i = 0; i < LinControllersCount; i++) {
            const DsVeosCoSim_LinController& controller = LinControllers[i];
            CheckResult(TransmitLinMessage(controller));
        }
    }

    return DsVeosCoSim_Result_Ok;
}

void LogIoData(const DsVeosCoSim_SimulationTime simulationTime,
               const DsVeosCoSim_IoSignal* ioSignal,
               const uint32_t length,
               const void* value,
               [[maybe_unused]] void* userData) {
    print(fg(fmt::color::fuchsia), "{}\n", DsVeosCoSim_IoDataToString(simulationTime, *ioSignal, length, value));
}

void LogCanMessage(const DsVeosCoSim_SimulationTime simulationTime,
                   const DsVeosCoSim_CanController* controller,
                   const DsVeosCoSim_CanMessage* message,
                   [[maybe_unused]] void* userData) {
    print(fg(fmt::color::dodger_blue), "{}\n", DsVeosCoSim_CanMessageToString(simulationTime, *controller, *message));
}

void LogEthMessage(const DsVeosCoSim_SimulationTime simulationTime,
                   const DsVeosCoSim_EthController* controller,
                   const DsVeosCoSim_EthMessage* message,
                   [[maybe_unused]] void* userData) {
    print(fg(fmt::color::cyan), "{}\n", DsVeosCoSim_EthMessageToString(simulationTime, *controller, *message));
}

void LogLinMessage(const DsVeosCoSim_SimulationTime simulationTime,
                   const DsVeosCoSim_LinController* controller,
                   const DsVeosCoSim_LinMessage* message,
                   [[maybe_unused]] void* userData) {
    print(fg(fmt::color::lime), "{}\n", DsVeosCoSim_LinMessageToString(simulationTime, *controller, *message));
}

void OnSimulationPostStepCallback(const DsVeosCoSim_SimulationTime simulationTime, [[maybe_unused]] void* userData) {
    (void)SendSomeData(simulationTime);
}

void StartSimulationThread(const std::function<void()>& function) {
    SimulationThread = std::thread(function);
    SimulationThread.detach();
}

void OnSimulationStartedCallback(const DsVeosCoSim_SimulationTime simulationTime, [[maybe_unused]] void* userData) {
    LogInfo("Simulation started at " + DsVeosCoSim_SimulationTimeToString(simulationTime) + " s.");
}

void OnSimulationStoppedCallback(const DsVeosCoSim_SimulationTime simulationTime, [[maybe_unused]] void* userData) {
    LogInfo("Simulation stopped at " + DsVeosCoSim_SimulationTimeToString(simulationTime) + " s.");
}

void OnSimulationTerminatedCallback(const DsVeosCoSim_SimulationTime simulationTime,
                                    const DsVeosCoSim_TerminateReason reason,
                                    [[maybe_unused]] void* userData) {
    LogInfo("Simulation terminated with reason " + DsVeosCoSim_TerminateReasonToString(reason) + " at " +
            DsVeosCoSim_SimulationTimeToString(simulationTime) + " s.");
}

void OnSimulationPausedCallback(const DsVeosCoSim_SimulationTime simulationTime, [[maybe_unused]] void* userData) {
    LogInfo("Simulation paused at " + DsVeosCoSim_SimulationTimeToString(simulationTime) + " s.");
}

void OnSimulationContinuedCallback(const DsVeosCoSim_SimulationTime simulationTime, [[maybe_unused]] void* userData) {
    LogInfo("Simulation continued at " + DsVeosCoSim_SimulationTimeToString(simulationTime) + " s.");
}

[[nodiscard]] DsVeosCoSim_Result Connect(const std::string_view host, const std::string_view serverName) {
    LogInfo("Connecting ...");

    DsVeosCoSim_ConnectionState connectionState{};
    CheckResultWithMessage(DsVeosCoSim_GetConnectionState(Handle, &connectionState), "Could not get connection state.");

    if (connectionState == DsVeosCoSim_ConnectionState_Connected) {
        LogInfo("Already connected.");
        return DsVeosCoSim_Result_Ok;
    }

    DsVeosCoSim_ConnectConfig connectConfig{};
    connectConfig.clientName = "Example Test Client";
    connectConfig.serverName = serverName.data();
    connectConfig.remoteIpAddress = host.data();

    CheckResultWithMessage(DsVeosCoSim_Connect(Handle, connectConfig), "Could not connect.");

    LogTrace("");

    DsVeosCoSim_SimulationTime stepSize{};
    CheckResultWithMessage(DsVeosCoSim_GetStepSize(Handle, &stepSize), "Could not get step size.");
    LogTrace("Step size: " + DsVeosCoSim_SimulationTimeToString(stepSize) + " s");
    LogTrace("");

    CheckResultWithMessage(DsVeosCoSim_GetCanControllers(Handle, &CanControllersCount, &CanControllers),
                           "Could not get CAN controllers.");
    if (CanControllersCount > 0) {
        LogTrace("Found the following CAN controllers:");
        for (uint32_t i = 0; i < CanControllersCount; i++) {
            LogTrace("  " + DsVeosCoSim_CanControllerToString(CanControllers[i]));
        }

        LogTrace("");
    }

    CheckResultWithMessage(DsVeosCoSim_GetEthControllers(Handle, &EthControllersCount, &EthControllers),
                           "Could not get ETH controllers.");
    if (EthControllersCount > 0) {
        LogTrace("Found the following ETH controllers:");
        for (uint32_t i = 0; i < EthControllersCount; i++) {
            LogTrace("  " + DsVeosCoSim_EthControllerToString(EthControllers[i]));
        }

        LogTrace("");
    }

    CheckResultWithMessage(DsVeosCoSim_GetLinControllers(Handle, &LinControllersCount, &LinControllers),
                           "Could not get LIN controllers.");
    if (LinControllersCount > 0) {
        LogTrace("Found the following LIN controllers:");
        for (uint32_t i = 0; i < LinControllersCount; i++) {
            LogTrace("  " + DsVeosCoSim_LinControllerToString(LinControllers[i]));
        }

        LogTrace("");
    }

    CheckResultWithMessage(DsVeosCoSim_GetIncomingSignals(Handle, &IncomingSignalsCount, &IncomingSignals),
                           "Could not get incoming signals.");
    if (IncomingSignalsCount > 0) {
        LogTrace("Found the following incoming signals:");
        for (uint32_t i = 0; i < IncomingSignalsCount; i++) {
            LogTrace("  " + DsVeosCoSim_IoSignalToString(IncomingSignals[i]));
        }

        LogTrace("");
    }

    CheckResultWithMessage(DsVeosCoSim_GetOutgoingSignals(Handle, &OutgoingSignalsCount, &OutgoingSignals),
                           "Could not get outgoing signals.");
    if (OutgoingSignalsCount > 0) {
        LogTrace("Found the following outgoing signals:");
        for (uint32_t i = 0; i < OutgoingSignalsCount; i++) {
            LogTrace("  " + DsVeosCoSim_IoSignalToString(OutgoingSignals[i]));
        }

        LogTrace("");
    }

    LogInfo("Connected.");
    return DsVeosCoSim_Result_Ok;
}

[[nodiscard]] DsVeosCoSim_Result Disconnect() {
    LogInfo("Disconnecting ...");
    CheckResultWithMessage(DsVeosCoSim_Disconnect(Handle), "Could not disconnect.");
    LogInfo("Disconnected.");

    return DsVeosCoSim_Result_Ok;
}

[[noreturn]] void RunCallbackBasedCoSimulation() {
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
    const DsVeosCoSim_Result result = DsVeosCoSim_RunCallbackBasedCoSimulation(Handle, callbacks);
    if ((result == DsVeosCoSim_Result_Disconnected) || (result == DsVeosCoSim_Result_Ok)) {
        exit(0);  // NOLINT
    }

    LogError("DsVeosCoSim_RunCallbackBasedCoSimulation finished with the following error code: " +
             DsVeosCoSim_ResultToString(result) + ".");
    exit(1);  // NOLINT
}

[[nodiscard]] DsVeosCoSim_Result HostClient(const std::string_view host, const std::string_view name) {
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
                CheckResultWithMessage(DsVeosCoSim_StartSimulation(Handle), "Could not start simulation");
                break;
            case 'o':
                CheckResultWithMessage(DsVeosCoSim_StopSimulation(Handle), "Could not stop simulation");
                break;
            case 'p':
                CheckResultWithMessage(DsVeosCoSim_PauseSimulation(Handle), "Could not pause simulation");
                break;
            case 't':
                CheckResultWithMessage(DsVeosCoSim_TerminateSimulation(Handle, DsVeosCoSim_TerminateReason_Error),
                                       "Could not terminate simulation");
                break;
            case 'n':
                CheckResultWithMessage(DsVeosCoSim_ContinueSimulation(Handle), "Could not pause simulation");
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

    DsVeosCoSim_SetLogCallback(OnLogCallback);

    Handle = DsVeosCoSim_Create();
    if (!Handle) {
        LogError("Could not create handle.");
        return 1;
    }

    const DsVeosCoSim_Result result = HostClient(host, name);

    DsVeosCoSim_Destroy(Handle);

    return result == DsVeosCoSim_Result_Ok ? 0 : 1;
}
