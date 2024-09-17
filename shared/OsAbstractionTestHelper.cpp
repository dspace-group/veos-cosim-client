// Copyright dSPACE GmbH. All rights reserved.

#include "OsAbstractionTestHelper.h"

#include <stdexcept>

#ifdef _WIN32
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <Windows.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <unistd.h>
#endif

#ifdef _WIN32
using socklen_t = int32_t;
#endif

namespace {

void Socket_InitializeAddress(sockaddr_in& address, std::string_view ipAddress, uint16_t port) {
    in_addr ipAddressInt{};
    if (inet_pton(AF_INET, ipAddress.data(), &ipAddressInt) <= 0) {
        throw std::runtime_error("Could not convert IP address string to integer.");
    }

    address.sin_family = AF_INET;
    address.sin_addr = ipAddressInt;
    address.sin_port = htons(port);
}

}  // namespace

InternetAddress::InternetAddress(std::string_view ipAddress, uint16_t port) {
    auto* address = reinterpret_cast<sockaddr_in*>(_address);
    Socket_InitializeAddress(*address, ipAddress, port);
}

UdpSocket::UdpSocket() : _socket(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) {
    if (_socket == InvalidSocket) {
        throw std::runtime_error("Could not create socket.");
    }
}

UdpSocket::~UdpSocket() {
#ifdef _WIN32
    closesocket(_socket);
#else
    shutdown(_socket, SHUT_RDWR);
    close(_socket);
#endif
}

void UdpSocket::Bind(std::string_view ipAddress, uint16_t port) const {
    sockaddr_in address{};
    Socket_InitializeAddress(address, ipAddress, port);
    if (bind(_socket, reinterpret_cast<const sockaddr*>(&address), sizeof address) < 0) {
        throw std::runtime_error("Could not bind.");
    }
}

void UdpSocket::Connect(std::string_view ipAddress, uint16_t port) const {
    sockaddr_in address{};
    Socket_InitializeAddress(address, ipAddress, port);
    if (connect(_socket, reinterpret_cast<const sockaddr*>(&address), sizeof address) < 0) {
        throw std::runtime_error("Could not connect.");
    }
}

void UdpSocket::SetNoDelay(bool value) const {
    const int32_t flags = value ? 1 : 0;
    constexpr auto flagsLength = static_cast<socklen_t>(sizeof flags);
    if (setsockopt(_socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&flags), flagsLength) < 0) {
        throw std::runtime_error("Could not set no delay.");
    }
}

void UdpSocket::SetReuseAddress(bool value) const {
    const int32_t flags = value ? 1 : 0;
    constexpr auto flagsLength = static_cast<socklen_t>(sizeof(flags));
    if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&flags), flagsLength) < 0) {
        throw std::runtime_error("Could not set reuse address.");
    }
}

void UdpSocket::Listen() const {
    if (listen(_socket, SOMAXCONN) < 0) {
        throw std::runtime_error("Could not listen.");
    }
}

bool UdpSocket::SendTo(const void* source, uint32_t size, const InternetAddress& address) const {
    const auto* const sourcePointer = static_cast<const char*>(source);
    static auto addressLength = static_cast<socklen_t>(sizeof(sockaddr_in));
    const auto length = static_cast<int32_t>(sendto(_socket,
                                                    sourcePointer,
                                                    static_cast<int32_t>(size),
                                                    0,
                                                    reinterpret_cast<const sockaddr*>(&address),
                                                    addressLength));
    return length == static_cast<int32_t>(size);
}

bool UdpSocket::ReceiveFrom(void* destination, uint32_t size, InternetAddress& address) const {
    auto* const destinationPointer = static_cast<char*>(destination);
    static auto addressLength = static_cast<socklen_t>(sizeof(sockaddr_in));
    const auto length = static_cast<int32_t>(recvfrom(_socket,
                                                      destinationPointer,
                                                      static_cast<int32_t>(size),
                                                      0,
                                                      reinterpret_cast<sockaddr*>(&address),
                                                      &addressLength));
    return length == static_cast<int32_t>(size);
}

#ifdef _WIN32
constexpr uint32_t PipeBufferSize = 1024 * 16;
#endif

#ifndef _WIN32
Pipe::pipe_t Pipe::CreatePipe(std::string_view name) {
    mkfifo(name.data(), 0666);

    pipe_t pipe = open(name.data(), O_RDWR);
    if (pipe < 0) {
        throw std::runtime_error("Could not open pipe.");
    }

    return pipe;
}
#endif

Pipe::Pipe(std::string_view name) {
#ifdef _WIN32
    _name = std::string(R"(\\.\pipe\)") + std::string(name);
#else
    _pipe1 = CreatePipe(std::string("/tmp/Pipe1") + std::string(name));
    _pipe2 = CreatePipe(std::string("/tmp/Pipe2") + std::string(name));
#endif
}

Pipe::~Pipe() {
#ifdef _WIN32
    CloseHandle(_pipe);
#else
    close(_pipe1);
    close(_pipe2);
#endif
}

void Pipe::Accept() {
#ifdef _WIN32
    _pipe = ::CreateNamedPipeA(_name.c_str(),
                               PIPE_ACCESS_DUPLEX,
                               PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                               1,
                               PipeBufferSize,
                               PipeBufferSize,
                               0,
                               nullptr);

    if (_pipe == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Could not create pipe.");
    }

    bool connected = ::ConnectNamedPipe(_pipe, nullptr) != 0 ? true : GetLastError() == ERROR_PIPE_CONNECTED;
    if (!connected) {
        throw std::runtime_error("Could not connect.");
    }
#else
    _writePipe = _pipe1;
    _readPipe = _pipe2;
#endif
}

void Pipe::Connect() {
#ifdef _WIN32
    while (true) {
        _pipe = ::CreateFileA(_name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
        if (_pipe != INVALID_HANDLE_VALUE) {
            break;
        }

        if (::GetLastError() != ERROR_PIPE_BUSY) {
            throw std::runtime_error("Could not create pipe.");
        }

        if (::WaitNamedPipeA(_name.c_str(), 10) == 0) {
            throw std::runtime_error("Could not create pipe.");
        }
    }

    DWORD dwMode = PIPE_READMODE_MESSAGE;
    BOOL success = ::SetNamedPipeHandleState(_pipe, &dwMode, nullptr, nullptr);
    if (success == 0) {
        throw std::runtime_error("Could not set pipe to message mode.");
    }
#else
    _writePipe = _pipe2;
    _readPipe = _pipe1;
#endif
}

bool Pipe::Write(const void* source, uint32_t size) const {
#ifdef _WIN32
    DWORD processedSize = 0;
    BOOL success = ::WriteFile(_pipe, source, size, &processedSize, nullptr);
    return (success != 0) && (size == processedSize);
#else
    ssize_t length = write(_writePipe, source, size);
    return length == (ssize_t)size;
#endif
}

bool Pipe::Read(void* destination, uint32_t size) const {
#ifdef _WIN32
    DWORD processedSize = 0;
    BOOL success = ReadFile(_pipe, destination, size, &processedSize, nullptr);
    return (success != 0) && (size == processedSize);
#else
    ssize_t length = read(_readPipe, destination, size);
    return length == (ssize_t)size;
#endif
}