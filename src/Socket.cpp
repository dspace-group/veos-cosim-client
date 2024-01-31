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
constexpr int ErrorCodeInterrupted = WSAEINTR;
#define poll WSAPoll
#else
constexpr int ErrorCodeInterrupted = EINTR;
#endif

constexpr int SocketAddressLength = 65;

[[nodiscard]] int GetLastNetworkError() {
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}

[[nodiscard]] Result ConvertToInternetAddress(std::string_view ipAddress, uint16_t port, addrinfo*& addressInfo) {
    const std::string portString = std::to_string(port);

    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    const int errorCode = getaddrinfo(ipAddress.data(), portString.c_str(), &hints, &addressInfo);
    if (errorCode != 0) {
        LogSystemError("Could not get address information.", errorCode);
        return Result::Error;
    }

    return Result::Ok;
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

    addrinfo* addressInfo{};
    CheckResult(ConvertToInternetAddress(ipAddress, port, addressInfo));

    while (addressInfo) {
        if (connect(_socket, addressInfo->ai_addr, static_cast<socklen_t>(sizeof(*addressInfo->ai_addr))) < 0) {
            LogSystemError("Could not connect to server.", GetLastNetworkError());
            addressInfo = addressInfo->ai_next;
            continue;
        }

        return Result::Ok;
    }

    return Result::Error;
}

Result Socket::Bind(uint16_t port, bool enableRemoteAccess) const {
    sockaddr_in internetAddress{};
    internetAddress.sin_family = AF_INET;
    internetAddress.sin_port = htons(port);
    internetAddress.sin_addr = enableRemoteAccess ? in4addr_any : in4addr_loopback;

    if (bind(_socket, reinterpret_cast<sockaddr*>(&internetAddress), static_cast<socklen_t>(sizeof(internetAddress))) < 0) {
        LogSystemError("Could not bind socket.", GetLastNetworkError());
        return Result::Error;
    }

    return Result::Ok;
}

Result Socket::EnableReuseAddress() const {
    int flags = 1;
    if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&flags), static_cast<socklen_t>(sizeof(flags))) < 0) {
        LogSystemError("Could not enable socket option reuse address.", GetLastNetworkError());
        return Result::Error;
    }

    return Result::Ok;
}

Result Socket::DisableIpv6Only() const {
    int flags = 0;
    if (setsockopt(_socket, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<char*>(&flags), static_cast<socklen_t>(sizeof(flags))) < 0) {
        LogSystemError("Could not disable IPv6 option IPv6 only.", GetLastNetworkError());
        return Result::Error;
    }

    return Result::Ok;
}

Result Socket::EnableNoDelay() const {
    int flags = 1;
    if (setsockopt(_socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&flags), static_cast<socklen_t>(sizeof(flags))) < 0) {
        LogSystemError("Could not enable TCP option no delay.", GetLastNetworkError());
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
    pollfd fdArray{};
    fdArray.fd = _socket;
    fdArray.events = POLLRDNORM;

    const int result = poll(&fdArray, 1, 100);
    if (result < 0) {
        LogSystemError("Could not poll on socket.", GetLastNetworkError());
        return Result::Error;
    }

    if (result == 0) {
        acceptedSocket._socket = InvalidSocket;
        return Result::TryAgain;
    }

    acceptedSocket._socket = accept(_socket, nullptr, nullptr);
    if (acceptedSocket._socket == InvalidSocket) {
        const int errorCode = GetLastNetworkError();
        if (errorCode != ErrorCodeInterrupted) {
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
