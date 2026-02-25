// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "Helper.hpp"

#include <cstdint>
#include <ostream>
#include <string>

#include <fmt/color.h>

#include "Error.hpp"
#include "Logger.hpp"
#include "Socket.hpp"

#ifdef _WIN32

#include <Windows.h>
#include <conio.h>

#include "OsUtilities.hpp"

#else

#include <termios.h>
#include <unistd.h>

#include <cstdlib>

#endif

namespace DsVeosCoSim {

namespace {

std::string LastMessage;

[[nodiscard]] Result GetNextFreeDynamicPort(uint16_t& localPort) {
    SocketListener listener;
    CheckResult(SocketListener::Create(AddressFamily::Ipv4, 0, false, listener));

    return listener.GetLocalPort(localPort);
}

[[nodiscard]] DataType GenerateDataType() {
    return GenerateRandom(DataType::Bool, DataType::Float64);
}

[[nodiscard]] SizeKind GenerateSizeKind() {
    return GenerateRandom(SizeKind::Fixed, SizeKind::Variable);
}

[[nodiscard]] BusMessageId GenerateBusMessageId() {
    return static_cast<BusMessageId>(GenerateU32());
}

}  // namespace

void LogError(const std::string& message) {
    print(fg(fmt::color::red), "{}\n", message);
}

void LogWarning(const std::string& message) {
    print(fg(fmt::color::yellow), "{}\n", message);
}

void LogInfo(const std::string& message) {
    print(fg(fmt::color::white), "{}\n", message);
}

void LogTrace(const std::string& message) {
    print(fg(fmt::color::light_gray), "{}\n", message);
}

void InitializeOutput() {
#ifdef _WIN32
    (void)SetConsoleOutputCP(CP_UTF8);
    (void)setvbuf(stdout, nullptr, _IONBF, 0);

    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

    DWORD dwMode = 0;
    BOOL result = GetConsoleMode(console, &dwMode);
    if (result != 0) {
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        (void)SetConsoleMode(console, dwMode);
    }
#endif

    Logger::Instance().SetLogCallback(OnLogCallback);
}

void MustBeOk(const Result& result) {
    if (!IsOk(result)) {
        exit(1);
    }
}

void OnLogCallback(Severity severity, std::string_view message) {
    LastMessage = message;
    switch (severity) {
        case Severity::Error:
            LogError(message);
            break;
        case Severity::Warning:
            LogWarning(message);
            break;
        case Severity::Info:
            LogInfo(message);
            break;
        case Severity::Trace:
            LogTrace(message);
            break;
    }
}

void ClearLastMessage() {
    LastMessage.clear();
}

[[nodiscard]] std::string GetLastMessage() {
    return LastMessage;
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

[[nodiscard]] Result StartUp() {
    InitializeOutput();

    CheckResult(StartupNetwork());

    uint16_t portMapperPort{};
    CheckResult(GetNextFreeDynamicPort(portMapperPort));

    std::string portMapperPortString = std::to_string(portMapperPort);
    SetEnvVariable("VEOS_COSIM_PORTMAPPER_PORT", portMapperPortString);
    return CreateOk();
}

void SetEnvVariable(const std::string& name, const std::string& value) {
#ifdef _WIN32
    std::string environmentString = fmt::format("{}={}", name, value);
    (void)_putenv(environmentString.c_str());
#else
    (void)setenv(name.data(), value.data(), 1);
#endif
}

[[nodiscard]] const char* GetLoopBackAddress(AddressFamily addressFamily) {
    if (addressFamily == AddressFamily::Ipv4) {
        return "127.0.0.1";
    }

    return "::1";
}

[[nodiscard]] Result ReceiveComplete(const SocketClient& client, void* buffer, size_t length) {
    auto* bufferPointer = static_cast<uint8_t*>(buffer);

    while (length > 0) {
        size_t receivedSize{};
        CheckResult(client.Receive(bufferPointer, length, receivedSize));

        length -= receivedSize;
        bufferPointer += receivedSize;
    }

    return CreateOk();
}

#ifdef _WIN32
[[nodiscard]] Result ReceiveComplete(ShmPipeClient& client, void* buffer, size_t length) {
    auto* bufferPointer = static_cast<uint8_t*>(buffer);

    while (length > 0) {
        size_t receivedSize{};
        CheckResult(client.Receive(bufferPointer, length, receivedSize));

        length -= receivedSize;
        bufferPointer += receivedSize;
    }

    return CreateOk();
}
#endif

[[nodiscard]] Result CreateBusBuffer(CoSimType coSimType,
                                     ConnectionKind connectionKind,
                                     const std::string& name,
                                     const std::vector<CanController>& controllers,
                                     IProtocol& protocol,
                                     std::unique_ptr<BusBuffer>& busBuffer) {
    return CreateBusBuffer(coSimType, connectionKind, name, controllers, {}, {}, {}, protocol, busBuffer);
}

[[nodiscard]] Result CreateBusBuffer(CoSimType coSimType,
                                     ConnectionKind connectionKind,
                                     const std::string& name,
                                     const std::vector<EthController>& controllers,
                                     IProtocol& protocol,
                                     std::unique_ptr<BusBuffer>& busBuffer) {
    return CreateBusBuffer(coSimType, connectionKind, name, {}, controllers, {}, {}, protocol, busBuffer);
}

[[nodiscard]] Result CreateBusBuffer(CoSimType coSimType,
                                     ConnectionKind connectionKind,
                                     const std::string& name,
                                     const std::vector<LinController>& controllers,
                                     IProtocol& protocol,
                                     std::unique_ptr<BusBuffer>& busBuffer) {
    return CreateBusBuffer(coSimType, connectionKind, name, {}, {}, controllers, {}, protocol, busBuffer);
}

[[nodiscard]] Result CreateBusBuffer(CoSimType coSimType,
                                     ConnectionKind connectionKind,
                                     const std::string& name,
                                     const std::vector<FrController>& controllers,
                                     IProtocol& protocol,
                                     std::unique_ptr<BusBuffer>& busBuffer) {
    return CreateBusBuffer(coSimType, connectionKind, name, {}, {}, {}, controllers, protocol, busBuffer);
}

void FillWithRandomData(uint8_t* data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        data[i] = GenerateU8();
    }
}

[[nodiscard]] uint8_t GenerateU8() {
    return static_cast<uint8_t>(GenerateU32() % 256);
}

[[nodiscard]] uint16_t GenerateU16() {
    return static_cast<uint16_t>(GenerateU32() % 65536);
}

[[nodiscard]] uint32_t GenerateU32() {
    static bool first = true;
    if (first) {
        srand(static_cast<uint32_t>(time({})));  // NOLINT
        first = false;
    }

    return static_cast<uint32_t>(rand());  // NOLINT
}

[[nodiscard]] uint64_t GenerateU64() {
    return (static_cast<uint64_t>(GenerateU32()) << sizeof(uint32_t)) + static_cast<uint64_t>(GenerateU32());
}

[[nodiscard]] int64_t GenerateI64() {
    return (static_cast<int64_t>(GenerateU32()) << sizeof(int32_t)) + static_cast<int64_t>(GenerateU32());
}

[[nodiscard]] size_t GenerateSizeT() {
    if constexpr (sizeof(size_t) == sizeof(uint32_t)) {
        return GenerateU32();
    }

    return GenerateU64();
}

[[nodiscard]] std::string GenerateString(const std::string& prefix) {
    return fmt::format("{}{}", prefix, GenerateU32());
}

[[nodiscard]] SimulationTime GenerateSimulationTime() {
    return SimulationTime{GenerateI64()};
}

[[nodiscard]] BusMessageId GenerateBusMessageId(uint32_t min, uint32_t max) {
    return static_cast<BusMessageId>(GenerateRandom(min, max));
}

[[nodiscard]] BusControllerId GenerateBusControllerId() {
    return static_cast<BusControllerId>(GenerateU32());
}

[[nodiscard]] IoSignalId GenerateIoSignalId() {
    return static_cast<IoSignalId>(GenerateU32());
}

[[nodiscard]] std::vector<uint8_t> GenerateBytes(size_t length) {
    std::vector<uint8_t> data;
    data.resize(length);
    for (size_t i = 0; i < length; i++) {
        data[i] = GenerateU8();
    }

    return data;
}

[[nodiscard]] IoSignalContainer CreateSignal() {
    return CreateSignal(GenerateDataType(), GenerateSizeKind());
}

[[nodiscard]] IoSignalContainer CreateSignal(DataType dataType) {
    return CreateSignal(dataType, GenerateSizeKind());
}

[[nodiscard]] IoSignalContainer CreateSignal(DataType dataType, SizeKind sizeKind) {
    IoSignalContainer signal{};
    signal.id = GenerateIoSignalId();
    signal.length = GenerateRandom(1U, 4U);
    signal.dataType = dataType;
    signal.sizeKind = sizeKind;
    signal.name = GenerateString("Signal名前\xF0\x9F\x98\x80");
    return signal;
}

[[nodiscard]] std::vector<uint8_t> GenerateIoData(const IoSignalContainer& signal) {
    std::vector<uint8_t> data = CreateZeroedIoData(signal);
    FillWithRandomData(data.data(), data.size());
    return data;
}

[[nodiscard]] std::vector<uint8_t> CreateZeroedIoData(const IoSignalContainer& signal) {
    std::vector<uint8_t> data;
    data.resize(GetDataTypeSize(signal.dataType) * signal.length);
    return data;
}

void FillWithRandom(CanControllerContainer& controller) {
    controller.id = GenerateBusControllerId();
    controller.queueSize = 1000;
    controller.bitsPerSecond = GenerateU64();
    controller.flexibleDataRateBitsPerSecond = GenerateU64();
    controller.name = GenerateString("CanController名前\xF0\x9F\x98\x80");
    controller.channelName = GenerateString("CanChannel名前\xF0\x9F\x98\x80");
    controller.clusterName = GenerateString("CanCluster名前\xF0\x9F\x98\x80");
}

void FillWithRandom(EthControllerContainer& controller) {
    controller.id = GenerateBusControllerId();
    controller.queueSize = 1000;
    controller.bitsPerSecond = GenerateU64();
    FillWithRandomData(controller.macAddress.data(), EthAddressLength);
    controller.name = GenerateString("EthController名前\xF0\x9F\x98\x80");
    controller.channelName = GenerateString("EthChannel名前\xF0\x9F\x98\x80");
    controller.clusterName = GenerateString("EthCluster名前\xF0\x9F\x98\x80");
}

void FillWithRandom(LinControllerContainer& controller) {
    controller.id = GenerateBusControllerId();
    controller.queueSize = 1000;
    controller.bitsPerSecond = GenerateU64();
    controller.type = GenerateRandom(LinControllerType::Responder, LinControllerType::Commander);
    controller.name = GenerateString("LinController名前\xF0\x9F\x98\x80");
    controller.channelName = GenerateString("LinChannel名前\xF0\x9F\x98\x80");
    controller.clusterName = GenerateString("LinCluster名前\xF0\x9F\x98\x80");
}

void FillWithRandom(FrControllerContainer& controller) {
    controller.id = GenerateBusControllerId();
    controller.queueSize = 1000;
    controller.bitsPerSecond = GenerateU64();
    controller.name = GenerateString("FrController名前\xF0\x9F\x98\x80");
    controller.channelName = GenerateString("FrChannel名前\xF0\x9F\x98\x80");
    controller.clusterName = GenerateString("FrCluster名前\xF0\x9F\x98\x80");
}

void FillWithRandom(CanMessageContainer& message, BusControllerId controllerId) {
    uint32_t length = GenerateRandom(1U, 8U);
    message.controllerId = controllerId;
    message.id = GenerateBusMessageId();
    message.timestamp = GenerateSimulationTime();
    message.length = length;
    FillWithRandomData(message.data.data(), length);
}

void FillWithRandom(EthMessageContainer& message, BusControllerId controllerId) {
    uint32_t length = GenerateRandom(1U, 8U);
    message.controllerId = controllerId;
    message.timestamp = GenerateSimulationTime();
    message.length = length;
    FillWithRandomData(message.data.data(), length);
}

void FillWithRandom(LinMessageContainer& message, BusControllerId controllerId) {
    uint32_t length = GenerateRandom(1U, 8U);
    message.controllerId = controllerId;
    message.id = GenerateBusMessageId();
    message.timestamp = GenerateSimulationTime();
    message.length = length;
    FillWithRandomData(message.data.data(), length);
}

void FillWithRandom(FrMessageContainer& message, BusControllerId controllerId) {
    uint32_t length = GenerateRandom(1U, 8U);
    message.controllerId = controllerId;
    message.id = GenerateBusMessageId();
    message.timestamp = GenerateSimulationTime();
    message.length = length;
    FillWithRandomData(message.data.data(), length);
}

[[nodiscard]] std::vector<IoSignalContainer> CreateSignals(size_t count) {
    std::vector<IoSignalContainer> signals;
    signals.reserve(count);
    for (size_t i = 0; i < count; i++) {
        signals.push_back(CreateSignal());
    }

    return signals;
}

[[nodiscard]] std::vector<CanControllerContainer> CreateCanControllers(size_t count) {
    std::vector<CanControllerContainer> controllers;
    for (size_t i = 0; i < count; i++) {
        CanControllerContainer controller{};
        FillWithRandom(controller);
        controllers.push_back(controller);
    }

    return controllers;
}

[[nodiscard]] std::vector<EthControllerContainer> CreateEthControllers(size_t count) {
    std::vector<EthControllerContainer> controllers;
    for (size_t i = 0; i < count; i++) {
        EthControllerContainer controller{};
        FillWithRandom(controller);
        controllers.push_back(controller);
    }

    return controllers;
}

[[nodiscard]] std::vector<LinControllerContainer> CreateLinControllers(size_t count) {
    std::vector<LinControllerContainer> controllers;
    for (size_t i = 0; i < count; i++) {
        LinControllerContainer controller{};
        FillWithRandom(controller);
        controllers.push_back(controller);
    }

    return controllers;
}

[[nodiscard]] std::vector<FrControllerContainer> CreateFrControllers(size_t count) {
    std::vector<FrControllerContainer> controllers;
    for (size_t i = 0; i < count; i++) {
        FrControllerContainer controller{};
        FillWithRandom(controller);
        controllers.push_back(controller);
    }

    return controllers;
}

std::ostream& operator<<(std::ostream& stream, SimulationTime simulationTime) {
    return stream << format_as(simulationTime);
}

std::ostream& operator<<(std::ostream& stream, Result result) {
    return stream << format_as(result);
}

std::ostream& operator<<(std::ostream& stream, CoSimType coSimType) {
    return stream << format_as(coSimType);
}

std::ostream& operator<<(std::ostream& stream, ConnectionKind connectionKind) {
    return stream << format_as(connectionKind);
}

std::ostream& operator<<(std::ostream& stream, Command command) {
    return stream << format_as(command);
}

std::ostream& operator<<(std::ostream& stream, Severity severity) {
    return stream << format_as(severity);
}

std::ostream& operator<<(std::ostream& stream, TerminateReason terminateReason) {
    return stream << format_as(terminateReason);
}

std::ostream& operator<<(std::ostream& stream, ConnectionState connectionState) {
    return stream << format_as(connectionState);
}

std::ostream& operator<<(std::ostream& stream, SimulationState simulationState) {
    return stream << format_as(simulationState);
}

std::ostream& operator<<(std::ostream& stream, Mode mode) {
    return stream << format_as(mode);
}

std::ostream& operator<<(std::ostream& stream, IoSignalId ioSignalId) {
    return stream << format_as(ioSignalId);
}

std::ostream& operator<<(std::ostream& stream, DataType dataType) {
    return stream << format_as(dataType);
}

std::ostream& operator<<(std::ostream& stream, SizeKind sizeKind) {
    return stream << format_as(sizeKind);
}

std::ostream& operator<<(std::ostream& stream, BusControllerId busControllerId) {
    return stream << format_as(busControllerId);
}

std::ostream& operator<<(std::ostream& stream, BusMessageId busMessageId) {
    return stream << format_as(busMessageId);
}

std::ostream& operator<<(std::ostream& stream, LinControllerType linControllerType) {
    return stream << format_as(linControllerType);
}

std::ostream& operator<<(std::ostream& stream, CanMessageFlags canMessageFlags) {
    return stream << format_as(canMessageFlags);
}

std::ostream& operator<<(std::ostream& stream, EthMessageFlags ethMessageFlags) {
    return stream << format_as(ethMessageFlags);
}

std::ostream& operator<<(std::ostream& stream, LinMessageFlags linMessageFlags) {
    return stream << format_as(linMessageFlags);
}

std::ostream& operator<<(std::ostream& stream, FrMessageFlags frMessageFlags) {
    return stream << format_as(frMessageFlags);
}

std::ostream& operator<<(std::ostream& stream, FrameKind frameKind) {
    return stream << format_as(frameKind);
}

std::ostream& operator<<(std::ostream& stream, const IoSignal& ioSignal) {
    return stream << format_as(ioSignal);
}

std::ostream& operator<<(std::ostream& stream, const IoSignalContainer& ioSignalContainer) {
    return stream << format_as(ioSignalContainer);
}

std::ostream& operator<<(std::ostream& stream, const CanController& canController) {
    return stream << format_as(canController);
}

std::ostream& operator<<(std::ostream& stream, const CanControllerContainer& canControllerContainer) {
    return stream << format_as(canControllerContainer);
}

std::ostream& operator<<(std::ostream& stream, const CanMessage& canMessage) {
    return stream << format_as(canMessage);
}

std::ostream& operator<<(std::ostream& stream, const CanMessageContainer& canMessageContainer) {
    return stream << format_as(canMessageContainer);
}

std::ostream& operator<<(std::ostream& stream, const EthController& ethController) {
    return stream << format_as(ethController);
}

std::ostream& operator<<(std::ostream& stream, const EthControllerContainer& ethControllerContainer) {
    return stream << format_as(ethControllerContainer);
}

std::ostream& operator<<(std::ostream& stream, const EthMessage& ethMessage) {
    return stream << format_as(ethMessage);
}

std::ostream& operator<<(std::ostream& stream, const EthMessageContainer& ethMessageContainer) {
    return stream << format_as(ethMessageContainer);
}

std::ostream& operator<<(std::ostream& stream, const LinController& linController) {
    return stream << format_as(linController);
}

std::ostream& operator<<(std::ostream& stream, const LinControllerContainer& frControllerContainer) {
    return stream << format_as(frControllerContainer);
}

std::ostream& operator<<(std::ostream& stream, const LinMessage& linMessage) {
    return stream << format_as(linMessage);
}

std::ostream& operator<<(std::ostream& stream, const LinMessageContainer& linMessageContainer) {
    return stream << format_as(linMessageContainer);
}

std::ostream& operator<<(std::ostream& stream, const FrController& frController) {
    return stream << format_as(frController);
}

std::ostream& operator<<(std::ostream& stream, const FrControllerContainer& frControllerContainer) {
    return stream << format_as(frControllerContainer);
}

std::ostream& operator<<(std::ostream& stream, const FrMessage& frMessage) {
    return stream << format_as(frMessage);
}

std::ostream& operator<<(std::ostream& stream, const FrMessageContainer& frMessageContainer) {
    return stream << format_as(frMessageContainer);
}

std::ostream& operator<<(std::ostream& stream, const std::vector<IoSignalContainer>& ioSignalContainers) {
    return stream << format_as(ioSignalContainers);
}

std::ostream& operator<<(std::ostream& stream, const std::vector<CanControllerContainer>& canControllerContainers) {
    return stream << format_as(canControllerContainers);
}

std::ostream& operator<<(std::ostream& stream, const std::vector<EthControllerContainer>& ethControllerContainers) {
    return stream << format_as(ethControllerContainers);
}

std::ostream& operator<<(std::ostream& stream, const std::vector<LinControllerContainer>& linControllerContainers) {
    return stream << format_as(linControllerContainers);
}

std::ostream& operator<<(std::ostream& stream, const std::vector<FrControllerContainer>& frControllerContainers) {
    return stream << format_as(frControllerContainers);
}

}  // namespace DsVeosCoSim
