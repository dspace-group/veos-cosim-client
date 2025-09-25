// Copyright dSPACE GmbH. All rights reserved.

#include "Helper.h"

#include <string>

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

void OnLogCallback(Severity severity, const std::string& message) {
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

void ClearLastMessage() {
    LastMessage = "";
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
    return Result::Ok;
}

void SetEnvVariable(const std::string& name, const std::string& value) {
#ifdef _WIN32
    std::string environmentString(name);
    environmentString.append("=");
    environmentString.append(value);
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

[[nodiscard]] Result ReceiveComplete(const Socket& socket, void* buffer, size_t length) {
    auto* bufferPointer = static_cast<uint8_t*>(buffer);
    while (length > 0) {
        size_t receivedSize{};
        CheckResult(socket.Receive(bufferPointer, length, receivedSize));

        length -= receivedSize;
        bufferPointer += receivedSize;
    }

    return Result::Ok;
}

[[nodiscard]] Result CreateBusBuffer(CoSimType coSimType,
                                     ConnectionKind connectionKind,
                                     const std::string& name,
                                     const std::vector<CanController>& controllers,
                                     std::unique_ptr<BusBuffer>& busBuffer) {
    return CreateBusBuffer(coSimType, connectionKind, name, controllers, {}, {}, busBuffer);
}

[[nodiscard]] Result CreateBusBuffer(CoSimType coSimType,
                                     ConnectionKind connectionKind,
                                     const std::string& name,
                                     const std::vector<EthController>& controllers,
                                     std::unique_ptr<BusBuffer>& busBuffer) {
    return CreateBusBuffer(coSimType, connectionKind, name, {}, controllers, {}, busBuffer);
}

[[nodiscard]] Result CreateBusBuffer(CoSimType coSimType,
                                     ConnectionKind connectionKind,
                                     const std::string& name,
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

[[nodiscard]] uint64_t GenerateU64() {
    return (static_cast<uint64_t>(GenerateU32()) << sizeof(uint32_t)) + static_cast<uint64_t>(GenerateU32());
}

[[nodiscard]] int64_t GenerateI64() {
    return (static_cast<int64_t>(GenerateU32()) << sizeof(int32_t)) + static_cast<int64_t>(GenerateU32());
}

[[nodiscard]] std::string GenerateString(const std::string& prefix) {
    return fmt::format("{}{}", prefix, GenerateU32());
}

[[nodiscard]] SimulationTime GenerateSimulationTime() {
    return SimulationTime(GenerateU64());
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
    FillWithRandomData(controller.macAddress.data(), EthAddressLength);
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
