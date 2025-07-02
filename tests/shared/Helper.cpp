// Copyright dSPACE GmbH. All rights reserved.

#include "Helper.h"

#include <string>
#include <string_view>

#include "LogHelper.h"
#include "Socket.h"

#ifdef _WIN32
#include <Windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>

#include <cstdlib>
#endif

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] Result GetNextFreeDynamicPort(uint16_t& port) {
    Socket socket;
    CheckResult(Socket::Create(AddressFamily::Ipv4, socket));
    CheckResult(socket.Bind(0, false));
    return socket.GetLocalPort(port);
}

}  // namespace

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
