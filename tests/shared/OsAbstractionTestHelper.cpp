// Copyright dSPACE GmbH. All rights reserved.

#include "OsAbstractionTestHelper.h"

#include <string>

#include <fmt/format.h>

#ifdef _WIN32
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <Windows.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/un.h>

#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#endif

#include "CoSimHelper.h"
#include "DsVeosCoSim/CoSimTypes.h"

#ifdef _WIN32
using socklen_t = int32_t;
#define CAST(expression) (expression)
#else
#define CAST(expression) static_cast<int32_t>(expression)
#endif

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] Result Socket_InitializeAddress(sockaddr_in& address, const std::string& ipAddress, uint16_t port) {
    in_addr ipAddressInt{};
    int32_t result = inet_pton(AF_INET, ipAddress.c_str(), &ipAddressInt);
    if (result <= 0) {
        LogError("Could not convert IP address string to integer.");
        return Result::Error;
    }

    address.sin_family = AF_INET;
    address.sin_addr = ipAddressInt;
    address.sin_port = htons(port);
    return Result::Ok;
}

}  // namespace

[[nodiscard]] Result InternetAddress::Initialize(const std::string& ipAddress, uint16_t port) {
    auto* address = reinterpret_cast<sockaddr_in*>(_address.data());
    return Socket_InitializeAddress(*address, ipAddress, port);
}

UdpSocket::~UdpSocket() {
#ifdef _WIN32
    closesocket(_socket);
#else
    shutdown(_socket, SHUT_RDWR);
    close(_socket);
#endif
}

[[nodiscard]] Result UdpSocket::Initialize() {
    _socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (_socket == InvalidSocket) {
        LogError("Could not create socket.");
        return Result::Error;
    }

    return Result::Ok;
}

[[nodiscard]] Result UdpSocket::Bind(const std::string& ipAddress, uint16_t port) const {
    sockaddr_in address{};
    CheckResult(Socket_InitializeAddress(address, ipAddress, port));
    int32_t result = bind(_socket, reinterpret_cast<const sockaddr*>(&address), sizeof address);
    if (result < 0) {
        LogError("Could not bind.");
        return Result::Error;
    }

    return Result::Ok;
}

[[nodiscard]] Result UdpSocket::SendTo(const void* source, uint32_t size, const InternetAddress& address) const {
    const auto* sourcePointer = static_cast<const char*>(source);
    static auto addressLength = static_cast<socklen_t>(sizeof(sockaddr_in));
    auto length = CAST(sendto(_socket,
                              sourcePointer,
                              static_cast<int32_t>(size),
                              0,
                              reinterpret_cast<const sockaddr*>(&address),
                              addressLength));
    return length == static_cast<int32_t>(size) ? Result::Ok : Result::Error;
}

[[nodiscard]] Result UdpSocket::ReceiveFrom(void* destination, uint32_t size, InternetAddress& address) const {
    auto* destinationPointer = static_cast<char*>(destination);
    static auto addressLength = static_cast<socklen_t>(sizeof(sockaddr_in));
    auto length = CAST(recvfrom(_socket,
                                destinationPointer,
                                static_cast<int32_t>(size),
                                0,
                                reinterpret_cast<sockaddr*>(&address),
                                &addressLength));
    return length == static_cast<int32_t>(size) ? Result::Ok : Result::Error;
}

#ifdef _WIN32
constexpr uint32_t PipeBufferSize = 1024 * 16;
#endif

#ifndef _WIN32
[[nodiscard]] Result Pipe::CreatePipe(const std::string& name, pipe_t& pipe) {
    mkfifo(name.data(), 0666);

    pipe = open(name.data(), O_RDWR | O_CLOEXEC);
    if (pipe < 0) {
        LogError("Could not open pipe.");
        return Result::Error;
    }

    return Result::Ok;
}
#endif

Pipe::~Pipe() {
#ifdef _WIN32
    CloseHandle(_pipe);
#else
    close(_pipe1);
    close(_pipe2);
#endif
}

[[nodiscard]] Result Pipe::Initialize(const std::string& name) {
#ifdef _WIN32
    _name = fmt::format(R"(\\.\pipe\{})", name);
#else
    CheckResult(CreatePipe(fmt::format("/tmp/Pipe1{}", name), _pipe1));
    CheckResult(CreatePipe(fmt::format("/tmp/Pipe2{}", name), _pipe2));
#endif
    return Result::Ok;
}

[[nodiscard]] Result Pipe::Accept() {
#ifdef _WIN32
    _pipe = CreateNamedPipeA(_name.c_str(),
                             PIPE_ACCESS_DUPLEX,
                             PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                             1,
                             PipeBufferSize,
                             PipeBufferSize,
                             0,
                             nullptr);

    if (_pipe == INVALID_HANDLE_VALUE) {
        LogError("Could not create pipe.");
        return Result::Error;
    }

    bool connected = ConnectNamedPipe(_pipe, nullptr) != 0 ? true : GetLastError() == ERROR_PIPE_CONNECTED;
    if (!connected) {
        LogError("Could not connect.");
        return Result::Error;
    }
#else
    _writePipe = _pipe1;
    _readPipe = _pipe2;
#endif
    return Result::Ok;
}

[[nodiscard]] Result Pipe::Connect() {
#ifdef _WIN32
    while (true) {
        _pipe = CreateFileA(_name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
        if (_pipe != INVALID_HANDLE_VALUE) {
            break;
        }

        DWORD lastError = GetLastError();
        if (lastError != ERROR_PIPE_BUSY) {
            LogError("Could not create pipe.");
            return Result::Error;
        }

        BOOL result = WaitNamedPipeA(_name.c_str(), 10);
        if (result == 0) {
            LogError("Could not create pipe.");
            return Result::Error;
        }
    }

    DWORD dwMode = PIPE_READMODE_MESSAGE;
    BOOL success = SetNamedPipeHandleState(_pipe, &dwMode, nullptr, nullptr);
    if (success == 0) {
        LogError("Could not set pipe to message mode.");
        return Result::Error;
    }
#else
    _writePipe = _pipe2;
    _readPipe = _pipe1;
#endif
    return Result::Ok;
}

[[nodiscard]] Result Pipe::Write(const void* source, uint32_t size) const {
#ifdef _WIN32
    DWORD processedSize = 0;
    BOOL success = WriteFile(_pipe, source, size, &processedSize, nullptr);
    return (success != 0) && (size == processedSize) ? Result::Ok : Result::Error;
#else
    ssize_t length = write(_writePipe, source, size);
    return length == static_cast<ssize_t>(size) ? Result::Ok : Result::Error;
#endif
}

[[nodiscard]] Result Pipe::Read(void* destination, uint32_t size) const {
#ifdef _WIN32
    DWORD processedSize = 0;
    BOOL success = ReadFile(_pipe, destination, size, &processedSize, nullptr);
    return (success != 0) && (size == processedSize) ? Result::Ok : Result::Error;
#else
    ssize_t length = read(_readPipe, destination, size);
    return length == static_cast<ssize_t>(size) ? Result::Ok : Result::Error;
#endif
}
