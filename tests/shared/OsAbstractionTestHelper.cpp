// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "OsAbstractionTestHelper.hpp"

#include <string>

#include <fmt/format.h>

#include "Logger.hpp"
#include "Result.hpp"
#include "Socket.hpp"

#ifdef _WIN32

#include <WS2tcpip.h>
#include <WinSock2.h>
#include <Windows.h>
#undef min
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

#ifdef _WIN32

using SocketLength = int32_t;
#define CAST(expression) (expression)

#else

using SocketLength = socklen_t;
#define CAST(expression) static_cast<int32_t>(expression)

#endif

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] int32_t GetLastNetworkError() {
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}

[[nodiscard]] Result CreateAddress(const std::string& ipAddress, uint16_t port, sockaddr_in& address) {
    in_addr ipAddressInt{};
    int32_t result = inet_pton(AF_INET, ipAddress.c_str(), &ipAddressInt);
    if (result <= 0) {
        LogError(GetLastNetworkError(), "Could not convert IP address string to integer.");
        return CreateError();
    }

    address.sin_family = AF_INET;
    address.sin_addr = ipAddressInt;
    address.sin_port = htons(port);
    return CreateOk();
}

void Shutdown(const SocketHandle& socketHandle) {
#ifdef _WIN32
    shutdown(socketHandle.Get(), SD_BOTH);
#else
    shutdown(socketHandle.Get(), SHUT_RDWR);
#endif
}

#ifndef _WIN32

[[nodiscard]] Result CreatePipe(const std::string& name, PipeClient::pipe_t& pipe) {
    mkfifo(name.data(), 0666);

    pipe = open(name.data(), O_RDWR | O_CLOEXEC);  // NOLINT(cppcoreguidelines-pro-type-vararg)
    if (pipe < 0) {
        LogError("Could not open pipe.");
        return CreateError();
    }

    return CreateOk();
}

#endif

#ifdef _WIN32

[[nodiscard]] std::string GetFullPipeName(const std::string& name) {
    return fmt::format(R"(\\.\pipe\{})", name);
}

#else

[[nodiscard]] std::string GetFirstPipePath(const std::string& name) {
    return fmt::format("/tmp/Pipe1{}", name);
}

[[nodiscard]] std::string GetSecondPipePath(const std::string& name) {
    return fmt::format("/tmp/Pipe2{}", name);
}

#endif

}  // namespace

[[nodiscard]] Result InternetAddress::Create(const std::string& ipAddress, uint16_t port, InternetAddress& internetAddress) {
    sockaddr_in socketAddress{};
    CheckResult(CreateAddress(ipAddress, port, socketAddress));

    std::array<uint8_t, 16> address{};
    memcpy(address.data(), &socketAddress, sizeof(address));

    internetAddress = InternetAddress(address);
    return CreateOk();
}

UdpSocket::~UdpSocket() noexcept {
    Shutdown(_socketHandle);
}

[[nodiscard]] Result UdpSocket::CreateClient(UdpSocket& udpSocket) {
    SocketHandle socketHandle(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));
    if (!socketHandle.IsValid()) {
        LogError(GetLastNetworkError(), "Could not create UDP socket.");
        return CreateError();
    }

    udpSocket = UdpSocket(std::move(socketHandle));
    return CreateOk();
}

[[nodiscard]] Result UdpSocket::CreateServer(const std::string& ipAddress, uint16_t port, UdpSocket& udpSocket) {
    SocketHandle socketHandle(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));
    if (!socketHandle.IsValid()) {
        LogError(GetLastNetworkError(), "Could not create UDP socket.");
        return CreateError();
    }

    sockaddr_in socketAddress{};
    CheckResult(CreateAddress(ipAddress, port, socketAddress));

    int32_t result = bind(socketHandle.Get(), reinterpret_cast<const sockaddr*>(&socketAddress), static_cast<SocketLength>(sizeof socketAddress));
    if (result < 0) {
        LogError(GetLastNetworkError(), "Could not bind.");
        return CreateError();
    }

    udpSocket = UdpSocket(std::move(socketHandle));
    return CreateOk();
}

[[nodiscard]] Result UdpSocket::SendTo(const void* source, uint32_t size, const InternetAddress& address) const {
    const auto* sourcePointer = static_cast<const char*>(source);
    static auto addressLength = static_cast<socklen_t>(sizeof(sockaddr_in));
    auto length = CAST(sendto(_socketHandle.Get(), sourcePointer, static_cast<int32_t>(size), 0, reinterpret_cast<const sockaddr*>(&address), addressLength));
    if (length != static_cast<int32_t>(size)) {
        LogError(GetLastNetworkError(), "Could not send.");
        return CreateError();
    }

    return CreateOk();
}

[[nodiscard]] Result UdpSocket::ReceiveFrom(void* destination, uint32_t size, InternetAddress& address) const {
    auto* destinationPointer = static_cast<char*>(destination);
    static auto addressLength = static_cast<socklen_t>(sizeof(sockaddr_in));
    auto length = CAST(recvfrom(_socketHandle.Get(), destinationPointer, static_cast<int32_t>(size), 0, reinterpret_cast<sockaddr*>(&address), &addressLength));
    if (length != static_cast<int32_t>(size)) {
        LogError(GetLastNetworkError(), "Could not receive.");
        return CreateError();
    }

    return CreateOk();
}

#ifdef _WIN32

constexpr uint32_t PipeBufferSize = 65536;

#endif

PipeClient::~PipeClient() noexcept {
#ifndef _WIN32
    close(_writePipe);
    close(_readPipe);
#endif
}

[[nodiscard]] Result PipeClient::Connect(const std::string& name, PipeClient& client) {
#ifdef _WIN32
    std::string fullName = GetFullPipeName(name);

    Handle pipe;
    while (true) {
        pipe = Handle(CreateFileA(fullName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr));
        if (pipe.IsValid()) {
            break;
        }

        DWORD lastError = GetLastError();
        if (lastError != ERROR_PIPE_BUSY) {
            LogError("Could not create pipe.");
            return CreateError();
        }

        BOOL result = WaitNamedPipeA(fullName.c_str(), 10);
        if (result == 0) {
            LogError("Could not create pipe.");
            return CreateError();
        }
    }

    DWORD dwMode = PIPE_READMODE_MESSAGE;
    BOOL success = SetNamedPipeHandleState(pipe.Get(), &dwMode, nullptr, nullptr);
    if (success == 0) {
        LogError("Could not set pipe to message mode.");
        return CreateError();
    }

    client = PipeClient();
    client._pipe = std::move(pipe);
    return CreateOk();
#else
    pipe_t writePipe{};
    CheckResult(CreatePipe(GetFirstPipePath(name), writePipe));

    pipe_t readPipe{};
    CheckResult(CreatePipe(GetSecondPipePath(name), readPipe));

    client = PipeClient();
    client._writePipe = writePipe;
    client._readPipe = readPipe;
    return CreateOk();
#endif
}

[[nodiscard]] Result PipeClient::Accept(const std::string& name, PipeClient& client) {
#ifdef _WIN32
    std::string fullName = GetFullPipeName(name);

    Handle pipe(CreateNamedPipeA(fullName.c_str(),
                                 PIPE_ACCESS_DUPLEX,
                                 PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                                 1,
                                 PipeBufferSize,
                                 PipeBufferSize,
                                 0,
                                 nullptr));
    if (!pipe.IsValid()) {
        LogError("Could not create pipe.");
        return CreateError();
    }

    bool connected = ConnectNamedPipe(pipe.Get(), nullptr) != 0 ? true : GetLastError() == ERROR_PIPE_CONNECTED;
    if (!connected) {
        LogError("Could not connect.");
        return CreateError();
    }

    client = PipeClient();
    client._pipe = std::move(pipe);
    return CreateOk();
#else
    pipe_t readPipe{};
    CheckResult(CreatePipe(GetFirstPipePath(name), readPipe));

    pipe_t writePipe{};
    CheckResult(CreatePipe(GetSecondPipePath(name), writePipe));

    client = PipeClient();
    client._writePipe = writePipe;
    client._readPipe = readPipe;
    return CreateOk();
#endif
}

[[nodiscard]] Result PipeClient::Write(const void* source, uint32_t size) const {
#ifdef _WIN32
    DWORD processedSize = 0;
    BOOL success = WriteFile(_pipe.Get(), source, size, &processedSize, nullptr);
    if ((success == 0) || (size != processedSize)) {
        return CreateError();
    }

    return CreateOk();
#else
    ssize_t length = write(_writePipe, source, size);
    if (length != static_cast<ssize_t>(size)) {
        return CreateError();
    }

    return CreateOk();
#endif
}

[[nodiscard]] Result PipeClient::Read(void* destination, uint32_t size) const {
#ifdef _WIN32
    DWORD processedSize = 0;
    BOOL success = ReadFile(_pipe.Get(), destination, size, &processedSize, nullptr);
    if ((success == 0) || (size != processedSize)) {
        return CreateError();
    }

    return CreateOk();
#else
    ssize_t length = read(_readPipe, destination, size);
    if (length != static_cast<ssize_t>(size)) {
        return CreateError();
    }

    return CreateOk();
#endif
}
