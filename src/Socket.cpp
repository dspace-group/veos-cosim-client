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
#include <netdb.h>
#include <netinet/in.h>
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
constexpr int ErrorCodeNotSupported = WSAEAFNOSUPPORT;
#define poll WSAPoll
#else
constexpr int ErrorCodeInterrupted = EINTR;
constexpr int ErrorCodeNotSupported = EAFNOSUPPORT;
#endif

[[nodiscard]] int GetLastNetworkError() {
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}

[[nodiscard]] int GetInt(AddressFamily addressFamily) {
    if (addressFamily == AddressFamily::Ipv4) {
        return AF_INET;
    }

    return AF_INET6;
}

[[nodiscard]] Result ConvertToInternetAddress(std::string_view ipAddress, uint16_t port, addrinfo*& addressInfo) {
    const std::string portString = std::to_string(port);

    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    const int errorCode = getaddrinfo(ipAddress.data(), portString.c_str(), &hints, &addressInfo);
    if (errorCode != 0) {
        LogSystemError("Could not get address information.", errorCode);
        return Result::Error;
    }

    return Result::Ok;
}

[[nodiscard]] Result ConvertFromInternetAddress(const sockaddr_in& ipv4Address, std::string& ipAddress, uint16_t& port) {
    port = ntohs(ipv4Address.sin_port);

    if (ipv4Address.sin_addr.s_addr != 0) {
        char ipAddressArray[INET_ADDRSTRLEN]{};
        const char* result = inet_ntop(AF_INET, &ipv4Address.sin_addr.s_addr, ipAddressArray, INET_ADDRSTRLEN);
        if (!result) {
            LogSystemError("Could not convert IPv4 address.", GetLastNetworkError());
            return Result::Error;
        }

        ipAddress = std::string(ipAddressArray);
    } else {
        ipAddress = std::string("127.0.0.1");
    }

    return Result::Ok;
}

[[nodiscard]] Result ConvertFromInternetAddress(const sockaddr_in6& ipv6Address, std::string& ipAddress, uint16_t& port) {
    port = ntohs(ipv6Address.sin6_port);

    char ipAddressArray[INET6_ADDRSTRLEN]{};
    const char* result = inet_ntop(AF_INET6, &ipv6Address.sin6_addr, ipAddressArray, INET6_ADDRSTRLEN);
    if (!result) {
        LogSystemError("Could not convert IPv6 address.", GetLastNetworkError());
        return Result::Error;
    }

    ipAddress = std::string(ipAddressArray);

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
    _addressFamily = other._addressFamily;
    other._socket = InvalidSocket;
    other._addressFamily = {};
}

Socket& Socket::operator=(Socket&& other) noexcept {
    Close();

    _socket = other._socket;
    _addressFamily = other._addressFamily;
    other._socket = InvalidSocket;
    other._addressFamily = {};
    return *this;
}

bool Socket::IsIpv4Supported() {
    static bool hasValue = false;
    static bool isSupported = false;

    if (!hasValue) {
        socket_t sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        isSupported = (sock != InvalidSocket) || (GetLastNetworkError() != ErrorCodeNotSupported);

#ifdef _WIN32
        (void)closesocket(sock);
#else
        (void)close(sock);
#endif
        hasValue = true;
    }

    return isSupported;
}

bool Socket::IsIpv6Supported() {
    static bool hasValue = false;
    static bool isSupported = false;

    if (!hasValue) {
        socket_t sock = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);

        isSupported = (sock != InvalidSocket) || (GetLastNetworkError() != ErrorCodeNotSupported);

#ifdef _WIN32
        (void)closesocket(sock);
#else
        (void)close(sock);
#endif
        hasValue = true;
    }

    return isSupported;
}

void Socket::Close() {
    const socket_t sock = _socket;
    if (sock == InvalidSocket) {
        return;
    }

    _socket = InvalidSocket;
    _addressFamily = {};

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

Result Socket::Create(AddressFamily addressFamily) {
    if (_socket != InvalidSocket) {
        return Result::Ok;
    }

    const int af = GetInt(addressFamily);

    _socket = socket(af, SOCK_STREAM, IPPROTO_TCP);
    if (_socket == InvalidSocket) {
        LogSystemError("Could not create socket.", GetLastNetworkError());
        return Result::Error;
    }

    _addressFamily = af;
    return Result::Ok;
}

Result Socket::EnableIpv6Only() const {
    // On windows, IPv6 only is enabled by default
#ifndef _WIN32
    int flags = 1;
    if (setsockopt(_socket, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<char*>(&flags), static_cast<socklen_t>(sizeof(flags))) < 0) {
        LogSystemError("Could not enable IPv6 only.", GetLastNetworkError());

        return Result::Error;
    }
#endif

    return Result::Ok;
}

Result Socket::Connect(std::string_view ipAddress, uint16_t remotePort, uint16_t localPort) {
    if (remotePort == 0) {
        LogError("Remote port 0 is not valid.");
        return Result::Error;
    }

    addrinfo* addressInfo{};
    CheckResult(ConvertToInternetAddress(ipAddress, remotePort, addressInfo));

    addrinfo* currentAddressInfo = addressInfo;

    while (currentAddressInfo) {
        Close();

        _socket = socket(currentAddressInfo->ai_family, currentAddressInfo->ai_socktype, currentAddressInfo->ai_protocol);
        if (_socket == InvalidSocket) {
            LogSystemError("Could not create socket.", GetLastNetworkError());
            currentAddressInfo = currentAddressInfo->ai_next;
            continue;
        }

        if (localPort != 0) {
            if (EnableReuseAddress() != Result::Ok) {
                currentAddressInfo = currentAddressInfo->ai_next;
                continue;
            }

            if (Bind(localPort, false) != Result::Ok) {
                currentAddressInfo = currentAddressInfo->ai_next;
                continue;
            }
        }

        if (connect(_socket, currentAddressInfo->ai_addr, static_cast<socklen_t>(currentAddressInfo->ai_addrlen)) < 0) {
            LogSystemError("Could not connect to server.", GetLastNetworkError());
            currentAddressInfo = currentAddressInfo->ai_next;
            continue;
        }

        freeaddrinfo(addressInfo);
        return Result::Ok;
    }

    freeaddrinfo(addressInfo);
    return Result::Error;
}

Result Socket::Bind(uint16_t port, bool enableRemoteAccess) const {
    if (_addressFamily == AF_INET) {
        return BindForIpv4(port, enableRemoteAccess);
    }

    return BindForIpv6(port, enableRemoteAccess);
}

Result Socket::BindForIpv4(uint16_t port, bool enableRemoteAccess) const {
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = enableRemoteAccess ? INADDR_ANY : htonl(INADDR_LOOPBACK);

    if (bind(_socket, reinterpret_cast<sockaddr*>(&address), static_cast<socklen_t>(sizeof(address))) < 0) {
        LogSystemError("Could not bind socket.", GetLastNetworkError());
        return Result::Error;
    }

    return Result::Ok;
}

Result Socket::BindForIpv6(uint16_t port, bool enableRemoteAccess) const {
    sockaddr_in6 address{};
    address.sin6_family = AF_INET6;
    address.sin6_port = htons(port);
    address.sin6_addr = enableRemoteAccess ? in6addr_any : in6addr_loopback;

    if (bind(_socket, reinterpret_cast<sockaddr*>(&address), static_cast<socklen_t>(sizeof(address))) < 0) {
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

    acceptedSocket._addressFamily = _addressFamily;
    return Result::Ok;
}

Result Socket::GetLocalPort(uint16_t& localPort) const {
    if (_addressFamily == AF_INET) {
        return GetLocalPortForIpv4(localPort);
    }

    return GetLocalPortForIpv6(localPort);
}

Result Socket::GetLocalPortForIpv4(uint16_t& localPort) const {
    sockaddr_in address{};
    auto addressLength = static_cast<socklen_t>(sizeof(address));
    address.sin_family = AF_INET;

    if (getsockname(_socket, reinterpret_cast<sockaddr*>(&address), &addressLength) != 0) {
        LogSystemError("Could not get local socket address.", GetLastNetworkError());
        return Result::Error;
    }

    std::string ipAddress;
    return ConvertFromInternetAddress(address, ipAddress, localPort);
}

Result Socket::GetLocalPortForIpv6(uint16_t& localPort) const {
    sockaddr_in6 address{};
    auto addressLength = static_cast<socklen_t>(sizeof(address));
    address.sin6_family = AF_INET6;

    if (getsockname(_socket, reinterpret_cast<sockaddr*>(&address), &addressLength) != 0) {
        LogSystemError("Could not get local socket address.", GetLastNetworkError());
        return Result::Error;
    }

    std::string ipAddress;
    return ConvertFromInternetAddress(address, ipAddress, localPort);
}

Result Socket::GetRemoteAddress(std::string& remoteIpAddress, uint16_t& remotePort) const {
    if (_addressFamily == AF_INET) {
        return GetRemoteAddressForIpv4(remoteIpAddress, remotePort);
    }

    return GetRemoteAddressForIpv6(remoteIpAddress, remotePort);
}

Result Socket::GetRemoteAddressForIpv4(std::string& remoteIpAddress, uint16_t& remotePort) const {
    sockaddr_in address{};
    auto addressLength = static_cast<socklen_t>(sizeof(address));
    address.sin_family = AF_INET;

    if (getpeername(_socket, reinterpret_cast<sockaddr*>(&address), &addressLength) != 0) {
        LogSystemError("Could not get remote socket address.", GetLastNetworkError());
        return Result::Error;
    }

    return ConvertFromInternetAddress(address, remoteIpAddress, remotePort);
}

Result Socket::GetRemoteAddressForIpv6(std::string& remoteIpAddress, uint16_t& remotePort) const {
    sockaddr_in6 address{};
    auto addressLength = static_cast<socklen_t>(sizeof(address));
    address.sin6_family = AF_INET6;

    if (getpeername(_socket, reinterpret_cast<sockaddr*>(&address), &addressLength) != 0) {
        LogSystemError("Could not get remote socket address.", GetLastNetworkError());
        return Result::Error;
    }

    if (address.sin6_family == AF_INET) {
        return GetRemoteAddressForIpv4(remoteIpAddress, remotePort);
    }

    return ConvertFromInternetAddress(address, remoteIpAddress, remotePort);
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
