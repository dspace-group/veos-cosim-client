// Copyright dSPACE GmbH. All rights reserved.

#include "Socket.h"

#include "Logger.h"
#include "OsUtilities.h"

#ifdef _WIN32
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <Windows.h>
#include <afunix.h>
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <pthread.h>
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <unistd.h>
#include <cerrno>
#endif

#ifdef _MSC_VER
#pragma comment(lib, "WS2_32.Lib")
#endif

namespace DsVeosCoSim {

namespace {

#ifdef _WIN32
using socklen_t = int32_t;
constexpr int32_t ErrorCodeInterrupted = WSAEINTR;
constexpr int32_t ErrorCodeWouldBlock = WSAEWOULDBLOCK;
constexpr int32_t ErrorCodeNotSupported = WSAEAFNOSUPPORT;
constexpr int32_t ErrorCodeConnectionAborted = WSAECONNABORTED;
constexpr int32_t ErrorCodeConnectionReset = WSAECONNRESET;
#define poll WSAPoll
#define unlink _unlink
#else
constexpr int32_t ErrorCodeInterrupted = EINTR;
constexpr int32_t ErrorCodeWouldBlock = EINPROGRESS;
constexpr int32_t ErrorCodeBrokenPipe = EPIPE;
constexpr int32_t ErrorCodeNotSupported = EAFNOSUPPORT;
constexpr int32_t ErrorCodeConnectionAborted = ECONNABORTED;
constexpr int32_t ErrorCodeConnectionReset = ECONNRESET;
#endif

[[nodiscard]] int32_t GetLastNetworkError() {
#ifdef _WIN32
    return ::WSAGetLastError();
#else
    return errno;
#endif
}

[[nodiscard]] addrinfo* ConvertToInternetAddress(std::string_view ipAddress, uint16_t port) {
    const std::string portString = std::to_string(port);

    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    addrinfo* addressInfo{};

    const int32_t errorCode = ::getaddrinfo(ipAddress.data(), portString.c_str(), &hints, &addressInfo);
    if (errorCode != 0) {
        throw OsAbstractionException("Could not get address information. ", errorCode);
    }

    return addressInfo;
}

[[nodiscard]] SocketAddress ConvertFromInternetAddress(const sockaddr_in& ipv4Address) {
    SocketAddress socketAddress;
    socketAddress.port = ::ntohs(ipv4Address.sin_port);

    if (ipv4Address.sin_addr.s_addr != 0) {
        char ipAddressArray[INET_ADDRSTRLEN]{};
        const char* result = ::inet_ntop(AF_INET, &ipv4Address.sin_addr.s_addr, ipAddressArray, INET_ADDRSTRLEN);
        if (!result) {
            throw OsAbstractionException("Could not get address information.", GetLastNetworkError());
        }

        socketAddress.ipAddress = ipAddressArray;
    } else {
        socketAddress.ipAddress = "127.0.0.1";
    }

    return socketAddress;
}

[[nodiscard]] SocketAddress ConvertFromInternetAddress(const sockaddr_in6& ipv6Address) {
    SocketAddress socketAddress;
    socketAddress.port = ::ntohs(ipv6Address.sin6_port);

    char ipAddressArray[INET6_ADDRSTRLEN]{};
    const char* result = ::inet_ntop(AF_INET6, &ipv6Address.sin6_addr, ipAddressArray, INET6_ADDRSTRLEN);
    if (!result) {
        throw OsAbstractionException("Could not get address information.", GetLastNetworkError());
    }

    socketAddress.ipAddress = ipAddressArray;

    return socketAddress;
}

void CloseSocket(socket_t socket) {
    if (socket == InvalidSocket) {
        return;
    }

#ifdef _WIN32
    (void)::closesocket(socket);
#else
    (void)::close(socket);
#endif
}

[[nodiscard]] bool Poll(socket_t socket, int16_t events, uint32_t timeoutInMilliseconds) {
    int64_t deadline = GetCurrentTimeInMilliseconds() + timeoutInMilliseconds;
    int64_t millisecondsUntilDeadline = timeoutInMilliseconds;
    while (true) {
        if (millisecondsUntilDeadline < 0) {
            return false;
        }

        pollfd fdArray{};
        fdArray.fd = socket;
        fdArray.events = events;

        int32_t result = ::poll(&fdArray, 1, static_cast<int32_t>(millisecondsUntilDeadline));
        if (result < 0) {
            throw OsAbstractionException("Could not poll on socket.", GetLastNetworkError());
        }

        if (result == 0) {
            return {};
        }

        // Make sure it really succeeded
        int32_t error = 0;
        socklen_t len = sizeof(error);
        if (::getsockopt(socket, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error), &len) != 0) {
            int32_t errorCode = GetLastNetworkError();
            if (errorCode == ErrorCodeInterrupted) {
                millisecondsUntilDeadline = deadline - GetCurrentTimeInMilliseconds();
                continue;
            }

            throw OsAbstractionException("Could not poll on socket.", errorCode);
        }

        return true;
    }
}

void SwitchToNonBlockingMode(const socket_t& socket) {
#ifdef _WIN32
    u_long mode = 1;
    if (::ioctlsocket(socket, FIONBIO, &mode) < 0) {
        throw OsAbstractionException("Could not switch to non-blocking mode.", GetLastNetworkError());
    }
#else
    if (::fcntl(socket, F_SETFL, O_NONBLOCK) < 0) {
        throw OsAbstractionException("Could not switch to non-blocking mode.", GetLastNetworkError());
    }
#endif
}

void SwitchToBlockingMode(const socket_t& socket) {
#ifdef _WIN32
    u_long mode = 0;
    if (::ioctlsocket(socket, FIONBIO, &mode) < 0) {
        throw OsAbstractionException("Could not switch to blocking mode.", GetLastNetworkError());
    }
#else
    if (::fcntl(socket, F_SETFL, 0) < 0) {
        throw OsAbstractionException("Could not switch to non-blocking mode.", GetLastNetworkError());
    }
#endif
}

bool ConnectWithTimeout(socket_t& socket,
                        const sockaddr* socketAddress,
                        socklen_t sizeOfSocketAddress,
                        uint32_t timeoutInMilliseconds) {
    SwitchToNonBlockingMode(socket);

    if (::connect(socket, socketAddress, sizeOfSocketAddress) >= 0) {
        throw OsAbstractionException("Invalid connect result.");
    }

    int32_t errorCode = GetLastNetworkError();
    if (errorCode != ErrorCodeWouldBlock) {
        throw OsAbstractionException("Could not connect.", errorCode);
    }

    fd_set set{};
    FD_ZERO(&set);
    FD_SET(socket, &set);

    timeval timeout{};
    timeout.tv_sec = static_cast<long>(timeoutInMilliseconds / 1000);
    timeout.tv_usec = static_cast<long>(timeoutInMilliseconds % 1000) * 1000;

#ifdef _WIN32
    int32_t result = ::select(0, nullptr, &set, nullptr, &timeout);
#else
    int32_t result = ::select(socket + 1, nullptr, &set, nullptr, &timeout);
#endif
    if ((result > 0) && FD_ISSET(socket, &set)) {
        SwitchToBlockingMode(socket);
        return true;
    }

    return false;
}

}  // namespace

void StartupNetwork() {
#ifdef _WIN32
    static bool g_networkStarted = false;
    if (!g_networkStarted) {
        WSADATA wsaData;

        const int32_t errorCode = ::WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (errorCode != 0) {
            throw OsAbstractionException("Could not initialize Windows sockets.", errorCode);
        }

        g_networkStarted = true;
    }
#endif
}

Socket::Socket(AddressFamily addressFamily) {
    _addressFamily = addressFamily;
    int32_t protocol{};
    int32_t af{};
    switch (addressFamily) {
        case AddressFamily::Ipv4:
            protocol = IPPROTO_TCP;
            af = AF_INET;
            break;
        case AddressFamily::Ipv6:
            protocol = IPPROTO_TCP;
            af = AF_INET6;
            break;
        case AddressFamily::Uds:
            protocol = 0;
            af = AF_UNIX;
            break;
    }

    _socket = ::socket(af, SOCK_STREAM, protocol);

    if (_socket == InvalidSocket) {
        throw OsAbstractionException("Could not create socket.", GetLastNetworkError());
    }
}

Socket::Socket(socket_t socket, AddressFamily addressFamily) : _socket(socket), _addressFamily(addressFamily) {
}

Socket::~Socket() noexcept {
    Close();
}

Socket::Socket(Socket&& other) noexcept {
    Close();

    _socket = other._socket;
    _addressFamily = other._addressFamily;
    _path = other._path;

    other._socket = InvalidSocket;
    other._addressFamily = {};
    other._path = {};
}

Socket& Socket::operator=(Socket&& other) noexcept {
    Close();

    _socket = other._socket;
    _addressFamily = other._addressFamily;
    _path = other._path;

    other._socket = InvalidSocket;
    other._addressFamily = {};
    other._path = {};

    return *this;
}

bool Socket::IsIpv4Supported() {
    static bool hasValue = false;
    static bool isSupported = false;

    if (!hasValue) {
        socket_t socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        isSupported = (socket != InvalidSocket) || (GetLastNetworkError() != ErrorCodeNotSupported);

        CloseSocket(socket);
        hasValue = true;
    }

    return isSupported;
}

bool Socket::IsIpv6Supported() {
    static bool hasValue = false;
    static bool isSupported = false;

    if (!hasValue) {
        socket_t socket = ::socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);

        isSupported = (socket != InvalidSocket) || (GetLastNetworkError() != ErrorCodeNotSupported);

        CloseSocket(socket);
        hasValue = true;
    }

    return isSupported;
}

bool Socket::IsUdsSupported() {
    static bool hasValue = false;
    static bool isSupported = false;

    if (!hasValue) {
        socket_t socket = ::socket(AF_UNIX, SOCK_STREAM, 0);

        isSupported = (socket != InvalidSocket) || (GetLastNetworkError() != ErrorCodeNotSupported);

        CloseSocket(socket);
        hasValue = true;
    }

    return isSupported;
}

void Socket::Shutdown() const {
    if (_socket == InvalidSocket) {
        return;
    }

#ifdef _WIN32
    (void)::shutdown(_socket, SD_BOTH);
#else
    (void)::shutdown(_socket, SHUT_RDWR);
#endif
}

void Socket::Close() {
    socket_t socket = _socket;
    if (socket == InvalidSocket) {
        return;
    }

    Shutdown();

    _socket = InvalidSocket;
    _addressFamily = {};

    if (!_path.empty()) {
        (void)::unlink(_path.c_str());
        _path = {};
    }

    CloseSocket(socket);
}

bool Socket::IsValid() const {
    return _socket != InvalidSocket;
}

void Socket::EnableIpv6Only() const {  // NOLINT
    // On windows, IPv6 only is enabled by default
#ifndef _WIN32
    int32_t flags = 1;
    if (::setsockopt(_socket,
                     IPPROTO_IPV6,
                     IPV6_V6ONLY,
                     reinterpret_cast<char*>(&flags),
                     static_cast<socklen_t>(sizeof(flags))) < 0) {
        throw OsAbstractionException("Could not enable IPv6 only.", GetLastNetworkError());
    }
#endif
}

std::optional<Socket> Socket::TryConnect(std::string_view ipAddress,
                                         uint16_t remotePort,
                                         uint16_t localPort,
                                         uint32_t timeoutInMilliseconds) {
    if (remotePort == 0) {
        throw OsAbstractionException("Remote port 0 is not valid.");
    }

    addrinfo* addressInfo = ConvertToInternetAddress(ipAddress, remotePort);

    addrinfo* currentAddressInfo = addressInfo;

    while (currentAddressInfo) {
        int32_t addressFamily = currentAddressInfo->ai_family;

        socket_t socket = ::socket(addressFamily, currentAddressInfo->ai_socktype, currentAddressInfo->ai_protocol);
        if (socket == InvalidSocket) {
            currentAddressInfo = currentAddressInfo->ai_next;
            continue;
        }

        Socket connectedSocket(socket, static_cast<AddressFamily>(addressFamily));

        if (localPort != 0) {
            try {
                connectedSocket.EnableReuseAddress();
                connectedSocket.Bind(localPort, false);
            } catch (const OsAbstractionException&) {
                currentAddressInfo = currentAddressInfo->ai_next;
                continue;
            }
        }

        if (!ConnectWithTimeout(connectedSocket._socket,
                                currentAddressInfo->ai_addr,
                                static_cast<socklen_t>(currentAddressInfo->ai_addrlen),
                                timeoutInMilliseconds)) {
            currentAddressInfo = currentAddressInfo->ai_next;
            continue;
        }

        ::freeaddrinfo(addressInfo);
        return connectedSocket;
    }

    freeaddrinfo(addressInfo);
    return {};
}

bool Socket::TryConnect(const std::string& name) const {
    EnsureIsValid();

    if (_addressFamily != AddressFamily::Uds) {
        throw OsAbstractionException("Not supported for address family.");
    }

    if (name.empty()) {
        throw OsAbstractionException("Empty name is not valid.");
    }

    std::string path = GetUdsPath(name);

    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    (void)::strncpy(address.sun_path, path.c_str(), sizeof(address.sun_path) - 1);

#ifndef _WIN32
    address.sun_path[0] = '\0';
#endif

    return ::connect(_socket, reinterpret_cast<const sockaddr*>(&address), sizeof address) >= 0;
}

void Socket::Bind(uint16_t port, bool enableRemoteAccess) const {
    EnsureIsValid();

    if (_addressFamily == AddressFamily::Uds) {
        throw OsAbstractionException("Not supported for address family.");
    }

    if (_addressFamily == AddressFamily::Ipv4) {
        BindForIpv4(port, enableRemoteAccess);
    } else {
        BindForIpv6(port, enableRemoteAccess);
    }
}

void Socket::BindForIpv4(uint16_t port, bool enableRemoteAccess) const {
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = ::htons(port);
    address.sin_addr.s_addr = enableRemoteAccess ? INADDR_ANY : ::htonl(INADDR_LOOPBACK);

    if (::bind(_socket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        throw OsAbstractionException("Could not bind socket.", GetLastNetworkError());
    }
}

void Socket::BindForIpv6(uint16_t port, bool enableRemoteAccess) const {
    sockaddr_in6 address{};
    address.sin6_family = AF_INET6;
    address.sin6_port = ::htons(port);
    address.sin6_addr = enableRemoteAccess ? in6addr_any : in6addr_loopback;

    if (::bind(_socket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        throw OsAbstractionException("Could not bind socket.", GetLastNetworkError());
    }
}

void Socket::Bind(const std::string& name) {
    EnsureIsValid();

    if (_addressFamily != AddressFamily::Uds) {
        throw OsAbstractionException("Not supported for address family.");
    }

    if (name.empty()) {
        throw OsAbstractionException("Empty name is not valid.");
    }

    _path = GetUdsPath(name);
#ifdef _WIN32
    (void)::unlink(_path.c_str());
#endif

    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    (void)::strncpy(address.sun_path, _path.c_str(), sizeof(address.sun_path) - 1);

#ifndef _WIN32
    address.sun_path[0] = '\0';
#endif

    if (::bind(_socket, reinterpret_cast<const sockaddr*>(&address), sizeof address) < 0) {
        throw OsAbstractionException("Could not bind socket.", GetLastNetworkError());
    }
}

void Socket::EnableReuseAddress() const {
    EnsureIsValid();

    if (_addressFamily == AddressFamily::Uds) {
        throw OsAbstractionException("Not supported for address family.");
    }

    int32_t flags = 1;
    if (::setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&flags), sizeof(flags)) < 0) {
        throw OsAbstractionException("Could not enable socket option reuse address.", GetLastNetworkError());
    }
}

void Socket::EnableNoDelay() const {
    EnsureIsValid();

    int32_t flags = 1;
    if (::setsockopt(_socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&flags), sizeof(flags)) < 0) {
        throw OsAbstractionException("Could not enable TCP option no delay.", GetLastNetworkError());
    }
}

void Socket::Listen() const {
    EnsureIsValid();

    if (::listen(_socket, SOMAXCONN) < 0) {
        throw OsAbstractionException("Could not listen.", GetLastNetworkError());
    }
}

std::optional<Socket> Socket::TryAccept(uint32_t timeoutInMilliseconds) const {
    EnsureIsValid();

    if (!Poll(_socket, POLLRDNORM, timeoutInMilliseconds)) {
        return {};
    }

    socket_t socket = ::accept(_socket, nullptr, nullptr);
    if (socket == InvalidSocket) {
        throw OsAbstractionException("Could not accept.", GetLastNetworkError());
    }

    return Socket(socket, _addressFamily);
}

uint16_t Socket::GetLocalPort() const {
    EnsureIsValid();

    if (_addressFamily == AddressFamily::Ipv4) {
        return GetLocalPortForIpv4();
    }

    if (_addressFamily == AddressFamily::Ipv6) {
        return GetLocalPortForIpv6();
    }

    return 0;
}

uint16_t Socket::GetLocalPortForIpv4() const {
    sockaddr_in address{};
    auto addressLength = static_cast<socklen_t>(sizeof(address));
    address.sin_family = AF_INET;

    if (::getsockname(_socket, reinterpret_cast<sockaddr*>(&address), &addressLength) != 0) {
        throw OsAbstractionException("Could not get local socket address.", GetLastNetworkError());
    }

    SocketAddress socketAddress = ConvertFromInternetAddress(address);
    return socketAddress.port;
}

uint16_t Socket::GetLocalPortForIpv6() const {
    sockaddr_in6 address{};
    auto addressLength = static_cast<socklen_t>(sizeof(address));
    address.sin6_family = AF_INET6;

    if (::getsockname(_socket, reinterpret_cast<sockaddr*>(&address), &addressLength) != 0) {
        throw OsAbstractionException("Could not get local socket address.", GetLastNetworkError());
    }

    SocketAddress socketAddress = ConvertFromInternetAddress(address);
    return socketAddress.port;
}

SocketAddress Socket::GetRemoteAddress() const {
    EnsureIsValid();

    if (_addressFamily == AddressFamily::Ipv4) {
        return GetRemoteAddressForIpv4();
    }

    if (_addressFamily == AddressFamily::Ipv6) {
        return GetRemoteAddressForIpv6();
    }

    return {"127.0.0.1", 0};
}

SocketAddress Socket::GetRemoteAddressForIpv4() const {
    sockaddr_in address{};
    auto addressLength = static_cast<socklen_t>(sizeof(address));
    address.sin_family = AF_INET;

    if (::getpeername(_socket, reinterpret_cast<sockaddr*>(&address), &addressLength) != 0) {
        throw OsAbstractionException("Could not get remote socket address.", GetLastNetworkError());
    }

    return ConvertFromInternetAddress(address);
}

SocketAddress Socket::GetRemoteAddressForIpv6() const {
    sockaddr_in6 address{};
    auto addressLength = static_cast<socklen_t>(sizeof(address));
    address.sin6_family = AF_INET6;

    if (::getpeername(_socket, reinterpret_cast<sockaddr*>(&address), &addressLength) != 0) {
        throw OsAbstractionException("Could not get remote socket address.", GetLastNetworkError());
    }

    if (address.sin6_family == AF_INET) {
        return GetRemoteAddressForIpv4();
    }

    return ConvertFromInternetAddress(address);
}

bool Socket::Receive(void* destination, int32_t size, int32_t& receivedSize) const {
#ifdef _WIN32
    receivedSize = ::recv(_socket, static_cast<char*>(destination), size, 0);
#else
    receivedSize = static_cast<int32_t>(::recv(_socket, destination, size, MSG_NOSIGNAL));
#endif

    if (receivedSize > 0) [[likely]] {
        return true;
    }

    if (receivedSize == 0) {
        LogTrace("Remote endpoint disconnected.");
        return false;
    }

    int32_t errorCode = GetLastNetworkError();

    if ((errorCode == ErrorCodeConnectionAborted) || (errorCode == ErrorCodeConnectionReset)
#ifndef _WIN32
        || (errorCode == ErrorCodeBrokenPipe)
#endif
    ) {
        LogTrace("Remote endpoint disconnected.");
        return false;
    }

    LogError("Could not receive from remote endpoint. {}", GetSystemErrorMessage(errorCode));
    return false;
}

bool Socket::Send(const void* source, int32_t size, int32_t& sentSize) const {
#ifdef _WIN32
    sentSize = ::send(_socket, static_cast<const char*>(source), size, 0);
#else
    sentSize = static_cast<int32_t>(::send(_socket, source, size, MSG_NOSIGNAL));
#endif

    if (sentSize > 0) [[likely]] {
        return true;
    }

    if (sentSize == 0) {
        LogTrace("Remote endpoint disconnected.");
        return false;
    }

    int32_t errorCode = GetLastNetworkError();

    if ((errorCode == ErrorCodeConnectionAborted) || (errorCode == ErrorCodeConnectionReset)
#ifndef _WIN32
        || (errorCode == ErrorCodeBrokenPipe)
#endif
    ) {
        LogTrace("Remote endpoint disconnected.");
        return false;
    }

    LogError("Could not send to remote endpoint. {}", GetSystemErrorMessage(errorCode));
    return false;
}

void Socket::EnsureIsValid() const {
    if (!IsValid()) [[unlikely]] {
        throw OsAbstractionException("Socket is not valid.");
    }
}

}  // namespace DsVeosCoSim