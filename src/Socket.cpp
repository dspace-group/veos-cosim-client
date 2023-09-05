// Copyright dSPACE GmbH. All rights reserved.

#include "Socket.h"

#ifdef _WIN32
#include <WS2tcpip.h>
#include <WinSock2.h>
#undef min
#undef max
#ifdef _MSC_VER
#pragma comment(lib, "WS2_32.Lib")
#endif
#else
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <pthread.h>
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <cerrno>
#endif

#include "Logger.h"

namespace DsVeosCoSim {

namespace {

#ifdef _WIN32
using socklen_t = int;
#endif

constexpr int SocketAddressLength = 65;

[[nodiscard]] int GetLastNetworkError() {
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}

[[nodiscard]] Result ConvertToInternetAddress(std::string_view ipAddress, uint16_t port, sockaddr_in& address) {
    in_addr ipAddressInt{};
    const int result = inet_pton(AF_INET, ipAddress.data(), &ipAddressInt);
    switch (result) {
        case 0:
            LogError("Could not interpret IPv4 address. " + std::string(ipAddress) + " has an invalid format.");
            return Result::Error;
        case -1:
            LogSystemError("Could not parse IP address.", GetLastNetworkError());
            return Result::Error;
        default:
            address.sin_family = AF_INET;
            address.sin_addr = ipAddressInt;
            address.sin_port = htons(port);
            return Result::Ok;
    }
}

[[nodiscard]] Result ConvertFromInternetAddress(const sockaddr_in& address, std::string& ipAddress, uint16_t& port) {
    port = ntohs(address.sin_port);

    if (address.sin_addr.s_addr != 0) {
        char ipAddressArray[SocketAddressLength]{};
        const char* result = inet_ntop(AF_INET, &address.sin_addr.s_addr, ipAddressArray, INET_ADDRSTRLEN);
        if (!result) {
            LogSystemError("Could not convert IP address.", GetLastNetworkError());
            return Result::Error;
        }

        ipAddress = std::string(ipAddressArray);
    } else {
        ipAddress = std::string("0.0.0.0");
    }

    return Result::Ok;
}

}  // namespace

Result StartupNetwork() {
#ifdef _WIN32
    static bool g_networkStarted = false;
    if (!g_networkStarted) {
        WSADATA wsaData;

        const int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            LogSystemError("Could not initialize Windows sockets.", result);
            return Result::Error;
        }

        g_networkStarted = true;
    }
#endif

    return Result::Ok;
}

Socket::~Socket() noexcept {
    Close();
}

Socket::Socket(Socket&& other) noexcept {
    Close();

    _socket = other._socket;
    other._socket = InvalidSocket;
}

Socket& Socket::operator=(Socket&& other) noexcept {
    Close();

    _socket = other._socket;
    other._socket = InvalidSocket;
    return *this;
}

Result Socket::Create() {
    _socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_socket == InvalidSocket) {
        LogSystemError("Could not create socket.", GetLastNetworkError());
        return Result::Error;
    }

    return Result::Ok;
}

void Socket::Close() {
    const socket_t sock = _socket;
    if (sock == InvalidSocket) {
        return;
    }

    _socket = InvalidSocket;

#ifdef _WIN32
    (void)shutdown(sock, SD_BOTH);
    (void)closesocket(sock);
#else
    (void)shutdown(sock, SHUT_RDWR);
    (void)close(sock);
#endif
}

bool Socket::IsValid() const {
    return _socket != InvalidSocket;
}

Result Socket::Connect(std::string_view ipAddress, uint16_t port) const {
    if (port == 0) {
        LogError("Port 0 is not valid.");
        return Result::Error;
    }

    sockaddr_in internetAddress{};
    CheckResult(ConvertToInternetAddress(ipAddress, port, internetAddress));

    if (connect(_socket, reinterpret_cast<sockaddr*>(&internetAddress), static_cast<socklen_t>(sizeof(internetAddress))) < 0) {
        LogSystemError("Could not connect to server.", GetLastNetworkError());
        return Result::Error;
    }

    return Result::Ok;
}

Result Socket::Bind(std::string_view ipAddress, uint16_t port) const {
    sockaddr_in internetAddress{};
    CheckResult(ConvertToInternetAddress(ipAddress, port, internetAddress));

    if (bind(_socket, reinterpret_cast<sockaddr*>(&internetAddress), static_cast<socklen_t>(sizeof(internetAddress))) < 0) {
        LogSystemError("Could not bind socket.", GetLastNetworkError());
        return Result::Error;
    }

    return Result::Ok;
}

Result Socket::EnableReuseAddress() const {
    int flags = 1;
    if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&flags), static_cast<socklen_t>(sizeof(flags))) < 0) {
        LogSystemError("Could not set socket option reuse address.", GetLastNetworkError());
        return Result::Error;
    }

    return Result::Ok;
}

Result Socket::EnableNoDelay() const {
    int flags = 1;
    if (setsockopt(_socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&flags), static_cast<socklen_t>(sizeof(flags))) < 0) {
        LogSystemError("Could not set socket option no delay.", GetLastNetworkError());
        return Result::Error;
    }

    return Result::Ok;
}

Result Socket::Listen() const {
    if (listen(_socket, SOMAXCONN) < 0) {
        LogSystemError("Could not listen.", GetLastNetworkError());
        return Result::Error;
    }

    return Result::Ok;
}

Result Socket::Accept(Socket& acceptedSocket) const {
    acceptedSocket._socket = accept(_socket, nullptr, nullptr);
    if (acceptedSocket._socket == InvalidSocket) {
        const int errorCode = GetLastNetworkError();
        if (_socket != InvalidSocket) {
            LogSystemError("Could not accept.", errorCode);
        }

        return Result::Error;
    }

    return Result::Ok;
}

Result Socket::GetLocalPort(uint16_t& port) const {
    sockaddr_in internetAddress{};
    auto serverAddressLength = static_cast<socklen_t>(sizeof(internetAddress));
    internetAddress.sin_family = AF_INET;

    if (getsockname(_socket, reinterpret_cast<sockaddr*>(&internetAddress), &serverAddressLength) != 0) {
        LogSystemError("Could not get local socket address.", GetLastNetworkError());
        return Result::Error;
    }

    std::string ipAddress;
    return ConvertFromInternetAddress(internetAddress, ipAddress, port);
}

Result Socket::GetRemoteAddress(std::string& ipAddress, uint16_t& port) const {
    sockaddr_in internetAddress{};
    auto serverAddressLength = static_cast<socklen_t>(sizeof(internetAddress));
    internetAddress.sin_family = AF_INET;

    if (getpeername(_socket, reinterpret_cast<sockaddr*>(&internetAddress), &serverAddressLength) != 0) {
        LogSystemError("Could not get remote socket address.", GetLastNetworkError());
        return Result::Error;
    }

    return ConvertFromInternetAddress(internetAddress, ipAddress, port);
}

Result Socket::Receive(void* destination, int size, int& receivedSize) const {
#ifdef _WIN32
    receivedSize = recv(_socket, static_cast<char*>(destination), size, 0);
#else
    receivedSize = (int)recv(_socket, destination, size, MSG_NOSIGNAL);
#endif

    if (receivedSize > 0) {
        return Result::Ok;
    }

    if ((receivedSize < 0) && (_socket != InvalidSocket)) {
        LogSystemError("Could not receive data.", GetLastNetworkError());
    }

    return Result::Disconnected;
}

Result Socket::Send(const void* source, int size, int& sentSize) const {
#ifdef _WIN32
    sentSize = send(_socket, static_cast<const char*>(source), size, 0);
#else
    sentSize = (int)send(_socket, source, size, MSG_NOSIGNAL);
#endif

    if (sentSize > 0) {
        return Result::Ok;
    }

    if ((sentSize < 0) && (_socket != InvalidSocket)) {
        LogSystemError("Could not send data.", GetLastNetworkError());
    }

    return Result::Disconnected;
}

}  // namespace DsVeosCoSim
