// Copyright dSPACE GmbH. All rights reserved.

#include "Helper.h"

#include <string>
#include <string_view>

#include <fmt/color.h>

#include "Socket.h"

#ifdef _WIN32
#include <Windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>

#include <cstdlib>
#endif

namespace DsVeosCoSim {

namespace {

std::string LastMessage;

[[nodiscard]] Result GetNextFreeDynamicPort(uint16_t& port) {
    Socket socket;
    CheckResult(Socket::Create(AddressFamily::Ipv4, socket));
    CheckResult(socket.Bind(0, false));
    return socket.GetLocalPort(port);
}

[[nodiscard]] bool Equals(std::string_view first, std::string_view second) {
    if (first.length() != second.length()) {
        return false;
    }

    return strcmp(first.data(), second.data()) == 0;
}

[[nodiscard]] bool Equals(const void* expected, const void* actual, size_t size) {
    const auto* expectedBytes = static_cast<const uint8_t*>(expected);
    const auto* actualBytes = static_cast<const uint8_t*>(actual);
    for (size_t i = 0; i < size; i++) {
        if (expectedBytes[i] != actualBytes[i]) {
            return false;
        }
    }

    return true;
}

template <typename T, size_t TSize>
[[nodiscard]] bool Equals(const std::array<T, TSize>& expected, const std::array<T, TSize>& actual) {
    for (size_t i = 0; i < TSize; i++) {
        if (expected[i] != actual[i]) {
            return false;
        }
    }

    return true;
}

[[nodiscard]] DataType GenerateDataType() {
    return GenerateRandom(DataType::Bool, DataType::Float64);
}

[[nodiscard]] SizeKind GenerateSizeKind() {
    return GenerateRandom(SizeKind::Fixed, SizeKind::Variable);
}

[[nodiscard]] BusControllerId GenerateBusControllerId() {
    return static_cast<BusControllerId>(GenerateU32());
}

[[nodiscard]] BusMessageId GenerateBusMessageId() {
    return static_cast<BusMessageId>(GenerateU32());
}

}  // namespace

void InitializeOutput() {
#if _WIN32
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

    SetLogCallback(OnLogCallback);
}

void OnLogCallback(Severity severity, std::string_view message) {
    LastMessage = message;
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

void LogCanMessageContainer(const CanMessageContainer& messageContainer) {
    print(fg(fmt::color::dodger_blue), "{}\n", messageContainer.ToString());
}

void LogEthMessageContainer(const EthMessageContainer& messageContainer) {
    print(fg(fmt::color::cyan), "{}\n", messageContainer.ToString());
}

void LogLinMessageContainer(const LinMessageContainer& messageContainer) {
    print(fg(fmt::color::lime), "{}\n", messageContainer.ToString());
}

void LogIoData(const IoSignal& ioSignal, uint32_t length, const void* value) {
    print(fg(fmt::color::fuchsia), "{}\n", IoDataToString(ioSignal, length, value));
}

void ClearLastMessage() {
    LastMessage = "";
}

[[nodiscard]] std::string GetLastMessage() {
    return LastMessage;
}

[[nodiscard]] bool operator==(const IoSignal& first, const IoSignal& second) {
    if (first.id != second.id) {
        return false;
    }

    if (first.length != second.length) {
        return false;
    }

    if (first.dataType != second.dataType) {
        return false;
    }

    if (first.sizeKind != second.sizeKind) {
        return false;
    }

    if (!Equals(first.name, second.name)) {
        return false;
    }

    return true;
}

[[nodiscard]] std::ostream& operator<<(std::ostream& stream, const IoSignal& signal) {
    return stream << signal.ToString();
}

[[nodiscard]] bool operator==(const CanController& first, const CanController& second) {
    if (first.id != second.id) {
        return false;
    }

    if (first.queueSize != second.queueSize) {
        return false;
    }

    if (first.bitsPerSecond != second.bitsPerSecond) {
        return false;
    }

    if (first.flexibleDataRateBitsPerSecond != second.flexibleDataRateBitsPerSecond) {
        return false;
    }

    if (!Equals(first.name, second.name)) {
        return false;
    }

    if (!Equals(first.channelName, second.channelName)) {
        return false;
    }

    if (!Equals(first.clusterName, second.clusterName)) {
        return false;
    }

    return true;
}

[[nodiscard]] std::ostream& operator<<(std::ostream& stream, const CanController& controller) {
    return stream << controller.ToString();
}

[[nodiscard]] bool operator==(const CanMessageContainer& first, const CanMessageContainer& second) {
    if (first.timestamp != second.timestamp) {
        return false;
    }

    if (first.controllerId != second.controllerId) {
        return false;
    }

    if (first.id != second.id) {
        return false;
    }

    if (first.flags != second.flags) {
        return false;
    }

    if (first.length != second.length) {
        return false;
    }

    if (!Equals(first.data.data(), second.data.data(), first.length)) {
        return false;
    }

    return true;
}

[[nodiscard]] bool operator==(const CanMessage& first, const CanMessage& second) {
    if (first.timestamp != second.timestamp) {
        return false;
    }

    if (first.controllerId != second.controllerId) {
        return false;
    }

    if (first.id != second.id) {
        return false;
    }

    if (first.flags != second.flags) {
        return false;
    }

    if (first.length != second.length) {
        return false;
    }

    if (!Equals(first.data, second.data, first.length)) {
        return false;
    }

    return true;
}

[[nodiscard]] std::ostream& operator<<(std::ostream& stream, const CanMessage& message) {
    return stream << message.ToString();
}

[[nodiscard]] bool operator==(const EthController& first, const EthController& second) {
    if (first.id != second.id) {
        return false;
    }

    if (first.queueSize != second.queueSize) {
        return false;
    }

    if (first.bitsPerSecond != second.bitsPerSecond) {
        return false;
    }

    if (!Equals(first.macAddress, second.macAddress)) {
        return false;
    }

    if (!Equals(first.name, second.name)) {
        return false;
    }

    if (!Equals(first.channelName, second.channelName)) {
        return false;
    }

    if (!Equals(first.clusterName, second.clusterName)) {
        return false;
    }

    return true;
}

[[nodiscard]] std::ostream& operator<<(std::ostream& stream, const EthController& controller) {
    return stream << controller.ToString();
}

[[nodiscard]] bool operator==(const EthMessageContainer& first, const EthMessageContainer& second) {
    if (first.timestamp != second.timestamp) {
        return false;
    }

    if (first.controllerId != second.controllerId) {
        return false;
    }

    if (first.flags != second.flags) {
        return false;
    }

    if (first.length != second.length) {
        return false;
    }

    if (!Equals(first.data.data(), second.data.data(), first.length)) {
        return false;
    }

    return true;
}

[[nodiscard]] bool operator==(const EthMessage& first, const EthMessage& second) {
    if (first.timestamp != second.timestamp) {
        return false;
    }

    if (first.controllerId != second.controllerId) {
        return false;
    }

    if (first.flags != second.flags) {
        return false;
    }

    if (first.length != second.length) {
        return false;
    }

    if (!Equals(first.data, second.data, first.length)) {
        return false;
    }

    return true;
}

[[nodiscard]] std::ostream& operator<<(std::ostream& stream, const EthMessage& message) {
    return stream << message.ToString();
}

[[nodiscard]] bool operator==(const LinController& first, const LinController& second) {
    if (first.id != second.id) {
        return false;
    }

    if (first.queueSize != second.queueSize) {
        return false;
    }

    if (first.bitsPerSecond != second.bitsPerSecond) {
        return false;
    }

    if (first.type != second.type) {
        return false;
    }

    if (!Equals(first.name, second.name)) {
        return false;
    }

    if (!Equals(first.channelName, second.channelName)) {
        return false;
    }

    if (!Equals(first.clusterName, second.clusterName)) {
        return false;
    }

    return true;
}

[[nodiscard]] std::ostream& operator<<(std::ostream& stream, const LinController& controller) {
    return stream << controller.ToString();
}

[[nodiscard]] bool operator==(const LinMessageContainer& first, const LinMessageContainer& second) {
    if (first.timestamp != second.timestamp) {
        return false;
    }

    if (first.controllerId != second.controllerId) {
        return false;
    }

    if (first.id != second.id) {
        return false;
    }

    if (first.flags != second.flags) {
        return false;
    }

    if (first.length != second.length) {
        return false;
    }

    if (!Equals(first.data.data(), second.data.data(), first.length)) {
        return false;
    }

    return true;
}

[[nodiscard]] bool operator==(const LinMessage& first, const LinMessage& second) {
    if (first.timestamp != second.timestamp) {
        return false;
    }

    if (first.controllerId != second.controllerId) {
        return false;
    }

    if (first.id != second.id) {
        return false;
    }

    if (first.flags != second.flags) {
        return false;
    }

    if (first.length != second.length) {
        return false;
    }

    if (!Equals(first.data, second.data, first.length)) {
        return false;
    }

    return true;
}

[[nodiscard]] std::ostream& operator<<(std::ostream& stream, const LinMessage& message) {
    return stream << message.ToString();
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
    return Result::Ok;
}

void SetEnvVariable(std::string_view name, std::string_view value) {
#ifdef _WIN32
    std::string environmentString(name);
    environmentString.append("=");
    environmentString.append(value);
    (void)_putenv(environmentString.c_str());
#else
    (void)setenv(name.data(), value.data(), 1);
#endif
}

[[nodiscard]] std::string_view GetLoopBackAddress(AddressFamily addressFamily) {
    if (addressFamily == AddressFamily::Ipv4) {
        return "127.0.0.1";
    }

    return "::1";
}

[[nodiscard]] Result SendComplete(const Socket& socket, const void* buffer, size_t length) {
    const auto* bufferPointer = static_cast<const uint8_t*>(buffer);
    while (length > 0) {
        int32_t sentSize{};
        CheckResult(socket.Send(bufferPointer, static_cast<int32_t>(length), sentSize));

        length -= sentSize;
        bufferPointer += sentSize;
    }

    return Result::Ok;
}

[[nodiscard]] Result ReceiveComplete(const Socket& socket, void* buffer, size_t length) {
    auto* bufferPointer = static_cast<uint8_t*>(buffer);
    while (length > 0) {
        int32_t receivedSize{};
        CheckResult(socket.Receive(bufferPointer, static_cast<int32_t>(length), receivedSize));

        length -= receivedSize;
        bufferPointer += receivedSize;
    }

    return Result::Ok;
}

[[nodiscard]] Result CreateBusBuffer(CoSimType coSimType,
                                     ConnectionKind connectionKind,
                                     std::string_view name,
                                     const std::vector<CanController>& controllers,
                                     std::unique_ptr<BusBuffer>& busBuffer) {
    return CreateBusBuffer(coSimType, connectionKind, name, controllers, {}, {}, busBuffer);
}

[[nodiscard]] Result CreateBusBuffer(CoSimType coSimType,
                                     ConnectionKind connectionKind,
                                     std::string_view name,
                                     const std::vector<EthController>& controllers,
                                     std::unique_ptr<BusBuffer>& busBuffer) {
    return CreateBusBuffer(coSimType, connectionKind, name, {}, controllers, {}, busBuffer);
}

[[nodiscard]] Result CreateBusBuffer(CoSimType coSimType,
                                     ConnectionKind connectionKind,
                                     std::string_view name,
                                     const std::vector<LinController>& controllers,
                                     std::unique_ptr<BusBuffer>& busBuffer) {
    return CreateBusBuffer(coSimType, connectionKind, name, {}, {}, controllers, busBuffer);
}

[[nodiscard]] uint32_t GenerateU32() {
    static bool first = true;
    if (first) {
        srand(static_cast<uint32_t>(std::time({})));  // NOLINT
        first = false;
    }

    return static_cast<uint32_t>(rand());  // NOLINT
}

void FillWithRandom(uint8_t* data, size_t length) {
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

[[nodiscard]] uint64_t GenerateU64() {
    return (static_cast<uint64_t>(GenerateU32()) << sizeof(uint32_t)) + static_cast<uint64_t>(GenerateU32());
}

[[nodiscard]] std::string GenerateString(std::string_view prefix) {
    return fmt::format("{}{}", prefix, GenerateU32());
}

[[nodiscard]] SimulationTime GenerateSimulationTime() {
    return SimulationTime(GenerateU64());
}

[[nodiscard]] BusMessageId GenerateBusMessageId(uint32_t min, uint32_t max) {
    return static_cast<BusMessageId>(GenerateRandom(min, max));
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
    signal.id = static_cast<IoSignalId>(GenerateU32());
    signal.length = GenerateRandom(1U, 4U);
    signal.dataType = dataType;
    signal.sizeKind = sizeKind;
    signal.name = GenerateString("Signal名前\xF0\x9F\x98\x80");
    return signal;
}

[[nodiscard]] std::vector<uint8_t> GenerateIoData(const IoSignalContainer& signal) {
    std::vector<uint8_t> data = CreateZeroedIoData(signal);
    FillWithRandom(data.data(), data.size());
    return data;
}

[[nodiscard]] std::vector<uint8_t> CreateZeroedIoData(const IoSignalContainer& signal) {
    std::vector<uint8_t> data;
    data.resize(GetDataTypeSize(signal.dataType) * signal.length);
    return data;
}

void FillWithRandom(CanControllerContainer& controller) {
    controller.id = GenerateBusControllerId();
    controller.queueSize = 100;
    controller.bitsPerSecond = GenerateU64();
    controller.flexibleDataRateBitsPerSecond = GenerateU64();
    controller.name = GenerateString("CanController名前\xF0\x9F\x98\x80");
    controller.channelName = GenerateString("CanChannel名前\xF0\x9F\x98\x80");
    controller.clusterName = GenerateString("CanCluster名前\xF0\x9F\x98\x80");
}

void FillWithRandom(EthControllerContainer& controller) {
    controller.id = GenerateBusControllerId();
    controller.queueSize = 100;
    controller.bitsPerSecond = GenerateU64();
    FillWithRandom(controller.macAddress.data(), EthAddressLength);
    controller.name = GenerateString("EthController名前\xF0\x9F\x98\x80");
    controller.channelName = GenerateString("EthChannel名前\xF0\x9F\x98\x80");
    controller.clusterName = GenerateString("EthCluster名前\xF0\x9F\x98\x80");
}

void FillWithRandom(LinControllerContainer& controller) {
    controller.id = GenerateBusControllerId();
    controller.queueSize = 100;
    controller.bitsPerSecond = GenerateU64();
    controller.type = GenerateRandom(LinControllerType::Responder, LinControllerType::Commander);
    controller.name = GenerateString("LinController名前\xF0\x9F\x98\x80");
    controller.channelName = GenerateString("LinChannel名前\xF0\x9F\x98\x80");
    controller.clusterName = GenerateString("LinCluster名前\xF0\x9F\x98\x80");
}

void FillWithRandom(CanMessageContainer& message, BusControllerId controllerId) {
    uint32_t length = GenerateRandom(1U, 8U);
    message.controllerId = controllerId;
    message.id = GenerateBusMessageId();
    message.timestamp = GenerateSimulationTime();
    message.length = length;
    FillWithRandom(message.data.data(), length);
}

void FillWithRandom(EthMessageContainer& message, BusControllerId controllerId) {
    uint32_t length = GenerateRandom(1U, 8U);
    message.controllerId = controllerId;
    message.timestamp = GenerateSimulationTime();
    message.length = length;
    FillWithRandom(message.data.data(), length);
}

void FillWithRandom(LinMessageContainer& message, BusControllerId controllerId) {
    uint32_t length = GenerateRandom(1U, 8U);
    message.controllerId = controllerId;
    message.id = GenerateBusMessageId();
    message.timestamp = GenerateSimulationTime();
    message.length = length;
    FillWithRandom(message.data.data(), length);
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

}  // namespace DsVeosCoSim
