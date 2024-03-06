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
    const char* result = inet_ntop(AF_INET, &ipv6Address.sin6_addr, ipAddressArray, INET6_ADDRSTRLEN);
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
    other._socket = InvalidSocket;
}

Socket& Socket::operator=(Socket&& other) noexcept {
    Close();

    _socket = other._socket;
    other._socket = InvalidSocket;
    return *this;
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

        if (currentAddressInfo->ai_family == AF_INET) {
            CheckResult(CreateForIpv4());
            if (localPort != 0) {
                CheckResult(BindForIpv4(localPort, false));
            }
        // } else if (currentAddressInfo->ai_family == AF_INET6) {
        //     CheckResult(CreateForIpv6());
        //     if (localPort != 0) {
        //         CheckResult(BindForIpv6(localPort, false));
        //     }
        } else {
            LogError("Address family is not supported.");
            currentAddressInfo = currentAddressInfo->ai_next;
            continue;
        }

        if (connect(_socket, currentAddressInfo->ai_addr, static_cast<socklen_t>(sizeof(*currentAddressInfo->ai_addr))) < 0) {
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

Result Socket::Bind(uint16_t port, bool enableRemoteAccess) {
    // Result result = BindForIpv6(port, enableRemoteAccess);
    // if (result == Result::Ok) {
    //     _usingIpv6 = true;
    //     return Result::Ok;
    // }

    // Close();
    // LogInfo("Using IPv4 socket instead.");
    // _usingIpv6 = false;
    return BindForIpv4(port, enableRemoteAccess);
}

// Result Socket::BindForIpv6(uint16_t port, bool enableRemoteAccess) {
//     CheckResult(CreateForIpv6());

//     if (EnableReuseAddress() != Result::Ok) {
//         LogSystemError("Could not enable socket option reuse address for IPv6 socket.", GetLastNetworkError());
//         return Result::Error;
//     }

//     sockaddr_in6 ipv6Address{};
//     ipv6Address.sin6_family = AF_INET6;
//     ipv6Address.sin6_port = htons(port);
//     ipv6Address.sin6_addr = enableRemoteAccess ? in6addr_any : in6addr_loopback;

//     if (bind(_socket, reinterpret_cast<sockaddr*>(&ipv6Address), static_cast<socklen_t>(sizeof(ipv6Address))) < 0) {
//         LogSystemError("Could not bind IPv6 socket.", GetLastNetworkError());
//         return Result::Error;
//     }

//     return DisableIpv6Only();
// }

Result Socket::BindForIpv4(uint16_t port, bool enableRemoteAccess) {
    CheckResult(CreateForIpv4());

    if (EnableReuseAddress() != Result::Ok) {
        LogSystemError("Could not enable socket option reuse address for IPv4 socket.", GetLastNetworkError());
        return Result::Error;
    }

    sockaddr_in ipv4Address{};
    ipv4Address.sin_family = AF_INET;
    ipv4Address.sin_port = htons(port);
    ipv4Address.sin_addr = enableRemoteAccess ? in4addr_any : in4addr_loopback;

    if (bind(_socket, reinterpret_cast<sockaddr*>(&ipv4Address), static_cast<socklen_t>(sizeof(ipv4Address))) < 0) {
        LogSystemError("Could not bind IPv4 socket.", GetLastNetworkError());
        return Result::Error;
    }

    return Result::Ok;
}

// Result Socket::CreateForIpv6() {
//     if (_socket != InvalidSocket) {
//         return Result::Ok;
//     }

//     _socket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
//     if (_socket == InvalidSocket) {
//         LogSystemError("Could not create IPv6 socket.", GetLastNetworkError());
//         return Result::Error;
//     }

//     return Result::Ok;
// }

Result Socket::CreateForIpv4() {
    if (_socket != InvalidSocket) {
        return Result::Ok;
    }

    _socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_socket == InvalidSocket) {
        LogSystemError("Could not create IPv4 socket.", GetLastNetworkError());
        return Result::Error;
    }

    return Result::Ok;
}

Result Socket::EnableReuseAddress() const {
    int flags = 1;
    if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&flags), static_cast<socklen_t>(sizeof(flags))) < 0) {
        return Result::Error;
    }

    return Result::Ok;
}

// Result Socket::DisableIpv6Only() const {
//     int flags = 0;
//     if (setsockopt(_socket, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<char*>(&flags), static_cast<socklen_t>(sizeof(flags))) < 0) {
//         LogSystemError("Could not disable IPv6 option IPv6 only.", GetLastNetworkError());
//         return Result::Error;
//     }

//     return Result::Ok;
// }

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

Result Socket::GetLocalPort(uint16_t& localPort) const {
    // if (_usingIpv6) {
    //     return GetLocalPortForIpv6(localPort);
    // }

    return GetLocalPortForIpv4(localPort);
}

// Result Socket::GetLocalPortForIpv6(uint16_t& localPort) const {
//     sockaddr_in6 ipv6Address{};
//     auto ipv6AddressLength = static_cast<socklen_t>(sizeof(ipv6Address));
//     ipv6Address.sin6_family = AF_INET6;

//     if (getsockname(_socket, reinterpret_cast<sockaddr*>(&ipv6Address), &ipv6AddressLength) != 0) {
//         LogSystemError("Could not get local IPv6 socket address.", GetLastNetworkError());
//         return Result::Error;
//     }

//     std::string ipAddress;
//     return ConvertFromInternetAddress(ipv6Address, ipAddress, localPort);
// }

Result Socket::GetLocalPortForIpv4(uint16_t& localPort) const {
    sockaddr_in ipv4Address{};
    auto ipv4AddressLength = static_cast<socklen_t>(sizeof(ipv4Address));
    ipv4Address.sin_family = AF_INET;

    if (getsockname(_socket, reinterpret_cast<sockaddr*>(&ipv4Address), &ipv4AddressLength) != 0) {
        LogSystemError("Could not get local IPv4 socket address.", GetLastNetworkError());
        return Result::Error;
    }

    std::string ipAddress;
    return ConvertFromInternetAddress(ipv4Address, ipAddress, localPort);
}

Result Socket::GetRemoteAddress(std::string& remoteIpAddress, uint16_t& remotePort) const {
    // if (_usingIpv6) {
    //     return GetRemoteIpv6Address(remoteIpAddress, remotePort);
    // }

    return GetRemoteIpv4Address(remoteIpAddress, remotePort);
}

// Result Socket::GetRemoteIpv6Address(std::string& remoteIpAddress, uint16_t& remotePort) const {
//     sockaddr_in6 ipv6Address{};
//     auto ipv6AddressLength = static_cast<socklen_t>(sizeof(ipv6Address));
//     ipv6Address.sin6_family = AF_INET6;

//     if (getpeername(_socket, reinterpret_cast<sockaddr*>(&ipv6Address), &ipv6AddressLength) != 0) {
//         LogSystemError("Could not get remote IPv6 socket address.", GetLastNetworkError());
//         return Result::Error;
//     }

//     return ConvertFromInternetAddress(ipv6Address, remoteIpAddress, remotePort);
// }

Result Socket::GetRemoteIpv4Address(std::string& remoteIpAddress, uint16_t& remotePort) const {
    sockaddr_in ipv4Address{};
    auto ipv4AddressLength = static_cast<socklen_t>(sizeof(ipv4Address));
    ipv4Address.sin_family = AF_INET;

    if (getpeername(_socket, reinterpret_cast<sockaddr*>(&ipv4Address), &ipv4AddressLength) != 0) {
        LogSystemError("Could not get remote IPv4 socket address.", GetLastNetworkError());
        return Result::Error;
    }

    return ConvertFromInternetAddress(ipv4Address, remoteIpAddress, remotePort);
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
