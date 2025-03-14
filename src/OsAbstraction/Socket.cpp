// Copyright dSPACE GmbH. All rights reserved.

#include "Socket.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

#include "CoSimHelper.h"
#include "DsVeosCoSim/CoSimTypes.h"

#ifdef _WIN32
#include <WS2tcpip.h>
#include <Windows.h>  // NOLINT
#include <afunix.h>
#include <stdio.h>  // NOLINT
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/un.h>

#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <pthread.h>
#include <unistd.h>

#include <cerrno>
#endif

namespace DsVeosCoSim {

namespace {

#ifdef _WIN32
using SocketLength = int32_t;
constexpr int32_t ErrorCodeInterrupted = WSAEINTR;
constexpr int32_t ErrorCodeWouldBlock = WSAEWOULDBLOCK;
constexpr int32_t ErrorCodeNotSupported = WSAEAFNOSUPPORT;
constexpr int32_t ErrorCodeConnectionAborted = WSAECONNABORTED;
constexpr int32_t ErrorCodeConnectionReset = WSAECONNRESET;
#define Poll WSAPoll
#define Unlink _unlink
#else
using SocketLength = socklen_t;
constexpr int32_t ErrorCodeInterrupted = EINTR;
constexpr int32_t ErrorCodeWouldBlock = EINPROGRESS;
constexpr int32_t ErrorCodeBrokenPipe = EPIPE;
constexpr int32_t ErrorCodeNotSupported = EAFNOSUPPORT;
constexpr int32_t ErrorCodeConnectionAborted = ECONNABORTED;
constexpr int32_t ErrorCodeConnectionReset = ECONNRESET;
#define Poll poll
#define Unlink unlink
#endif

[[nodiscard]] std::string GetUdsPath(const std::string& name) {
#ifdef _WIN32
    const std::filesystem::path tempDir = std::filesystem::temp_directory_path();
    const std::filesystem::path fileDir = tempDir / ("dSPACE.VEOS.CoSim." + name);
    return fileDir.string();
#else
    return "dSPACE.VEOS.CoSim." + name;
#endif
}

[[nodiscard]] int64_t GetCurrentTimeInMilliseconds() {
#ifdef _WIN32
    FILETIME fileTime{};                 // NOLINT
    GetSystemTimeAsFileTime(&fileTime);  // NOLINT

    ULARGE_INTEGER largeInteger{};  // NOLINT
    largeInteger.LowPart = fileTime.dwLowDateTime;
    largeInteger.HighPart = fileTime.dwHighDateTime;

    return static_cast<int64_t>(largeInteger.QuadPart / 10000);
#else
    timeval currentTime{};
    (void)gettimeofday(&currentTime, nullptr);

    return (currentTime.tv_sec * 1000) + (currentTime.tv_usec / 1000);
#endif
}

[[nodiscard]] int32_t GetLastNetworkError() {
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}

[[nodiscard]] addrinfo* ConvertToInternetAddress(const std::string_view ipAddress, const uint16_t port) {
    const std::string portString = std::to_string(port);

    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    addrinfo* addressInfo{};

    const int32_t errorCode = getaddrinfo(ipAddress.data(), portString.c_str(), &hints, &addressInfo);
    if (errorCode != 0) {
        throw CoSimException("Could not get address information. " + GetSystemErrorMessage(errorCode));
    }

    return addressInfo;
}

[[nodiscard]] SocketAddress ConvertFromInternetAddress(const sockaddr_in& ipv4Address) {
    SocketAddress socketAddress;
    socketAddress.port = ntohs(ipv4Address.sin_port);

    std::array<char, INET_ADDRSTRLEN> ipAddress{};
    const char* result = inet_ntop(AF_INET, &ipv4Address.sin_addr, ipAddress.data(), INET_ADDRSTRLEN);
    if (!result) {
        throw CoSimException("Could not get address information. " + GetSystemErrorMessage(GetLastNetworkError()));
    }

    socketAddress.ipAddress = result;

    return socketAddress;
}

[[nodiscard]] SocketAddress ConvertFromInternetAddress(const sockaddr_in6& ipv6Address) {  // NOLINT
    SocketAddress socketAddress;
    socketAddress.port = ntohs(ipv6Address.sin6_port);

    std::array<char, INET6_ADDRSTRLEN> ipAddress{};
    const char* result = inet_ntop(AF_INET6, &ipv6Address.sin6_addr, ipAddress.data(), INET6_ADDRSTRLEN);
    if (!result) {
        throw CoSimException("Could not get address information. " + GetSystemErrorMessage(GetLastNetworkError()));
    }

    socketAddress.ipAddress = result;

    return socketAddress;
}

void CloseSocket(SocketHandle socket) {
    if (socket == InvalidSocket) {
        return;
    }

#ifdef _WIN32
    (void)closesocket(socket);
#else
    (void)close(socket);
#endif
}

[[nodiscard]] bool PollInternal(const SocketHandle socket, const int16_t events, const uint32_t timeoutInMilliseconds) {
    const int64_t deadline = GetCurrentTimeInMilliseconds() + timeoutInMilliseconds;
    int64_t millisecondsUntilDeadline = timeoutInMilliseconds;
    while (true) {
        if (millisecondsUntilDeadline < 0) {
            return false;
        }

        pollfd fdArray{};
        fdArray.fd = socket;
        fdArray.events = events;

        const int32_t pollResult = Poll(&fdArray, 1, static_cast<int32_t>(millisecondsUntilDeadline));
        if (pollResult < 0) {
            throw CoSimException("Could not poll on socket. " + GetSystemErrorMessage(GetLastNetworkError()));
        }

        if (pollResult == 0) {
            return {};
        }

        // Make sure it really succeeded
        int32_t error = 0;
        auto len = static_cast<SocketLength>(sizeof(error));
        const int32_t getSocketOptionResult =
            getsockopt(socket, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error), &len);
        if (getSocketOptionResult != 0) {
            const int32_t errorCode = GetLastNetworkError();
            if (errorCode == ErrorCodeInterrupted) {
                millisecondsUntilDeadline = deadline - GetCurrentTimeInMilliseconds();
                continue;
            }

            throw CoSimException("Could not poll on socket. " + GetSystemErrorMessage(errorCode));
        }

        return true;
    }
}

void SwitchToNonBlockingMode(const SocketHandle& socket) {
#ifdef _WIN32
    u_long mode = 1;
    const int32_t result = ioctlsocket(socket, FIONBIO, &mode);
    if (result != 0) {
        throw CoSimException("Could not switch to non-blocking mode. " + GetSystemErrorMessage(GetLastNetworkError()));
    }
#else
    const int32_t result = fcntl(socket, F_SETFL, O_NONBLOCK);
    if (result < 0) {
        throw CoSimException("Could not switch to non-blocking mode. " + GetSystemErrorMessage(GetLastNetworkError()));
    }
#endif
}

void SwitchToBlockingMode(const SocketHandle& socket) {
#ifdef _WIN32
    u_long mode = 0;
    const int32_t result = ioctlsocket(socket, FIONBIO, &mode);
    if (result != 0) {
        throw CoSimException("Could not switch to blocking mode. " + GetSystemErrorMessage(GetLastNetworkError()));
    }
#else
    const int32_t result = fcntl(socket, F_SETFL, 0);
    if (result < 0) {
        throw CoSimException("Could not switch to non-blocking mode. " + GetSystemErrorMessage(GetLastNetworkError()));
    }
#endif
}

[[nodiscard]] bool ConnectWithTimeout(SocketHandle& socket,
                                      const sockaddr* socketAddress,
                                      const SocketLength sizeOfSocketAddress,
                                      const uint32_t timeoutInMilliseconds) {
    SwitchToNonBlockingMode(socket);

    const int32_t connectResult = connect(socket, socketAddress, sizeOfSocketAddress);
    if (connectResult >= 0) {
        throw CoSimException("Invalid connect result.");
    }

    const int32_t errorCode = GetLastNetworkError();
    if (errorCode != ErrorCodeWouldBlock) {
        throw CoSimException("Could not connect. " + GetSystemErrorMessage(errorCode));
    }

    fd_set set{};
    FD_ZERO(&set);
    FD_SET(socket, &set);

    timeval timeout{};
    timeout.tv_sec = static_cast<long>(timeoutInMilliseconds / 1000);
    timeout.tv_usec = static_cast<long>(timeoutInMilliseconds % 1000) * 1000;

#ifdef _WIN32
    const int32_t selectResult = select(0, nullptr, &set, nullptr, &timeout);
#else
    const int32_t selectResult = select(socket + 1, nullptr, &set, nullptr, &timeout);
#endif
    if ((selectResult > 0) && FD_ISSET(socket, &set)) {
        SwitchToBlockingMode(socket);
        return true;
    }

    return false;
}

}  // namespace

void StartupNetwork() {
#ifdef _WIN32
    static bool networkStarted = false;
    if (!networkStarted) {
        WSADATA wsaData;

        const int32_t errorCode = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (errorCode != 0) {
            throw CoSimException("Could not initialize Windows sockets. " + GetSystemErrorMessage(errorCode));
        }

        networkStarted = true;
    }
#endif
}

Socket::Socket(const AddressFamily addressFamily) : _addressFamily(addressFamily) {
    int32_t protocol{};
    int32_t domain{};
    switch (addressFamily) {
        case AddressFamily::Ipv4:
            protocol = IPPROTO_TCP;
            domain = AF_INET;
            break;
        case AddressFamily::Ipv6:
            protocol = IPPROTO_TCP;
            domain = AF_INET6;
            break;
        case AddressFamily::Uds:
            protocol = 0;
            domain = AF_UNIX;
            break;
    }

    _socket = socket(domain, SOCK_STREAM, protocol);

    if (_socket == InvalidSocket) {
        throw CoSimException("Could not create socket. " + GetSystemErrorMessage(GetLastNetworkError()));
    }
}

Socket::Socket(const SocketHandle socket, const AddressFamily addressFamily)
    : _socket(socket), _addressFamily(addressFamily) {
}

Socket::~Socket() noexcept {
    Close();
}

Socket::Socket(Socket&& other) noexcept {
    Close();

    _socket = other._socket;                // NOLINT
    _addressFamily = other._addressFamily;  // NOLINT
    _path = other._path;                    // NOLINT

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

[[nodiscard]] bool Socket::IsIpv4Supported() {
    static bool hasValue = false;
    static bool isSupported = false;

    if (!hasValue) {
        const SocketHandle socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        isSupported = (socket != InvalidSocket) || (GetLastNetworkError() != ErrorCodeNotSupported);

        CloseSocket(socket);
        hasValue = true;
    }

    return isSupported;
}

[[nodiscard]] bool Socket::IsIpv6Supported() {
    static bool hasValue = false;
    static bool isSupported = false;

    if (!hasValue) {
        const SocketHandle socket = ::socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);

        isSupported = (socket != InvalidSocket) || (GetLastNetworkError() != ErrorCodeNotSupported);

        CloseSocket(socket);
        hasValue = true;
    }

    return isSupported;
}

[[nodiscard]] bool Socket::IsUdsSupported() {
    static bool hasValue = false;
    static bool isSupported = false;

    if (!hasValue) {
        const SocketHandle socket = ::socket(AF_UNIX, SOCK_STREAM, 0);

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
    (void)shutdown(_socket, SD_BOTH);
#else
    (void)shutdown(_socket, SHUT_RDWR);
#endif
}

void Socket::Close() {
    const SocketHandle socket = _socket;
    if (socket == InvalidSocket) {
        return;
    }

    Shutdown();

    _socket = InvalidSocket;
    _addressFamily = {};

    if (!_path.empty()) {
        (void)Unlink(_path.c_str());
        _path = {};
    }

    CloseSocket(socket);
}

[[nodiscard]] bool Socket::IsValid() const {
    return _socket != InvalidSocket;
}

void Socket::EnableIpv6Only() const {  // NOLINT
    // On windows, IPv6 only is enabled by default
#ifndef _WIN32
    int32_t flags = 1;
    const int32_t result = setsockopt(_socket,
                                      IPPROTO_IPV6,
                                      IPV6_V6ONLY,
                                      reinterpret_cast<char*>(&flags),
                                      static_cast<SocketLength>(sizeof(flags)));
    if (result != 0) {
        throw CoSimException("Could not enable IPv6 only. " + GetSystemErrorMessage(GetLastNetworkError()));
    }
#endif
}

[[nodiscard]] std::optional<Socket> Socket::TryConnect(const std::string_view ipAddress,
                                                       const uint16_t remotePort,
                                                       const uint16_t localPort,
                                                       const uint32_t timeoutInMilliseconds) {
    if (remotePort == 0) {
        throw CoSimException("Remote port 0 is not valid.");
    }

    addrinfo* addressInfo = ConvertToInternetAddress(ipAddress, remotePort);

    const addrinfo* currentAddressInfo = addressInfo;

    while (currentAddressInfo) {
        int32_t addressFamily = currentAddressInfo->ai_family;

        const SocketHandle socket =
            ::socket(addressFamily, currentAddressInfo->ai_socktype, currentAddressInfo->ai_protocol);
        if (socket == InvalidSocket) {
            currentAddressInfo = currentAddressInfo->ai_next;
            continue;
        }

        Socket connectedSocket(socket, static_cast<AddressFamily>(addressFamily));

        if (localPort != 0) {
            try {
                connectedSocket.EnableReuseAddress();
                connectedSocket.Bind(localPort, false);
            } catch (const CoSimException&) {
                currentAddressInfo = currentAddressInfo->ai_next;
                continue;
            }
        }

        if (!ConnectWithTimeout(connectedSocket._socket,
                                currentAddressInfo->ai_addr,
                                static_cast<SocketLength>(currentAddressInfo->ai_addrlen),
                                timeoutInMilliseconds)) {
            currentAddressInfo = currentAddressInfo->ai_next;
            continue;
        }

        freeaddrinfo(addressInfo);
        return connectedSocket;
    }

    freeaddrinfo(addressInfo);
    return {};
}

[[nodiscard]] bool Socket::TryConnect(const std::string& name) const {
    EnsureIsValid();

    if (_addressFamily != AddressFamily::Uds) {
        throw CoSimException("Not supported for address family.");
    }

    if (name.empty()) {
        throw CoSimException("Empty name is not valid.");
    }

    const std::string path = GetUdsPath(name);

    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    (void)strncpy(address.sun_path, path.c_str(), sizeof(address.sun_path) - 1);

#ifndef _WIN32
    address.sun_path[0] = '\0';
#endif

    return connect(_socket, reinterpret_cast<const sockaddr*>(&address), sizeof address) >= 0;
}

void Socket::Bind(const uint16_t port, const bool enableRemoteAccess) const {
    EnsureIsValid();

    if (_addressFamily == AddressFamily::Uds) {
        throw CoSimException("Not supported for address family.");
    }

    if (_addressFamily == AddressFamily::Ipv4) {
        BindForIpv4(port, enableRemoteAccess);
    } else {
        BindForIpv6(port, enableRemoteAccess);
    }
}

void Socket::BindForIpv4(const uint16_t port, const bool enableRemoteAccess) const {
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = enableRemoteAccess ? INADDR_ANY : htonl(INADDR_LOOPBACK);

    const int32_t result = bind(_socket, reinterpret_cast<sockaddr*>(&address), sizeof(address));
    if (result != 0) {
        throw CoSimException("Could not bind socket. " + GetSystemErrorMessage(GetLastNetworkError()));
    }
}

void Socket::BindForIpv6(const uint16_t port, const bool enableRemoteAccess) const {
    sockaddr_in6 address{};
    address.sin6_family = AF_INET6;
    address.sin6_port = htons(port);

    // We don't use in6addr_any, because that won't work, if lwip is linked. Both, lwip and ws2_32, define the same
    // symbol
    address.sin6_addr = enableRemoteAccess ? in6_addr{} : in6addr_loopback;  // NOLINT

    const int32_t result = bind(_socket, reinterpret_cast<sockaddr*>(&address), sizeof(address));
    if (result != 0) {
        throw CoSimException("Could not bind socket. " + GetSystemErrorMessage(GetLastNetworkError()));
    }
}

void Socket::Bind(const std::string& name) {
    EnsureIsValid();

    if (_addressFamily != AddressFamily::Uds) {
        throw CoSimException("Not supported for address family.");
    }

    if (name.empty()) {
        throw CoSimException("Empty name is not valid.");
    }

    _path = GetUdsPath(name);
#ifdef _WIN32
    (void)Unlink(_path.c_str());
#endif

    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    (void)strncpy(address.sun_path, _path.c_str(), sizeof(address.sun_path) - 1);

#ifndef _WIN32
    address.sun_path[0] = '\0';
#endif

    const int32_t result = bind(_socket, reinterpret_cast<const sockaddr*>(&address), sizeof address);
    if (result != 0) {
        throw CoSimException("Could not bind socket. " + GetSystemErrorMessage(GetLastNetworkError()));
    }
}

void Socket::EnableReuseAddress() const {
    EnsureIsValid();

    if (_addressFamily == AddressFamily::Uds) {
        throw CoSimException("Not supported for address family.");
    }

    int32_t flags = 1;
    const int32_t result =
        setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&flags), sizeof(flags));
    if (result != 0) {
        throw CoSimException("Could not enable socket option reuse address. " +
                             GetSystemErrorMessage(GetLastNetworkError()));
    }
}

void Socket::EnableNoDelay() const {
    EnsureIsValid();

    int32_t flags = 1;
    const int32_t result =
        setsockopt(_socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&flags), sizeof(flags));
    if (result != 0) {
        throw CoSimException("Could not enable TCP option no delay. " + GetSystemErrorMessage(GetLastNetworkError()));
    }
}

void Socket::Listen() const {
    EnsureIsValid();

    const int32_t result = listen(_socket, SOMAXCONN);
    if (result != 0) {
        throw CoSimException("Could not listen. " + GetSystemErrorMessage(GetLastNetworkError()));
    }
}

[[nodiscard]] std::optional<Socket> Socket::TryAccept(const uint32_t timeoutInMilliseconds) const {
    EnsureIsValid();

    if (!PollInternal(_socket, POLLRDNORM, timeoutInMilliseconds)) {
        return {};
    }

    const SocketHandle socket = accept(_socket, nullptr, nullptr);
    if (socket == InvalidSocket) {
        throw CoSimException("Could not accept. " + GetSystemErrorMessage(GetLastNetworkError()));
    }

    return Socket(socket, _addressFamily);
}

[[nodiscard]] uint16_t Socket::GetLocalPort() const {
    EnsureIsValid();

    if (_addressFamily == AddressFamily::Ipv4) {
        return GetLocalPortForIpv4();
    }

    if (_addressFamily == AddressFamily::Ipv6) {
        return GetLocalPortForIpv6();
    }

    return 0;
}

[[nodiscard]] uint16_t Socket::GetLocalPortForIpv4() const {
    sockaddr_in address{};
    auto addressLength = static_cast<SocketLength>(sizeof(address));
    address.sin_family = AF_INET;

    const int32_t result = getsockname(_socket, reinterpret_cast<sockaddr*>(&address), &addressLength);
    if (result != 0) {
        throw CoSimException("Could not get local socket address. " + GetSystemErrorMessage(GetLastNetworkError()));
    }

    const auto [ipAddress, port] = ConvertFromInternetAddress(address);
    return port;
}

[[nodiscard]] uint16_t Socket::GetLocalPortForIpv6() const {
    sockaddr_in6 address{};
    auto addressLength = static_cast<SocketLength>(sizeof(address));
    address.sin6_family = AF_INET6;

    const int32_t result = getsockname(_socket, reinterpret_cast<sockaddr*>(&address), &addressLength);
    if (result != 0) {
        throw CoSimException("Could not get local socket address. " + GetSystemErrorMessage(GetLastNetworkError()));
    }

    const auto [ipAddress, port] = ConvertFromInternetAddress(address);
    return port;
}

[[nodiscard]] SocketAddress Socket::GetRemoteAddress() const {
    EnsureIsValid();

    if (_addressFamily == AddressFamily::Ipv4) {
        return GetRemoteAddressForIpv4();
    }

    if (_addressFamily == AddressFamily::Ipv6) {
        return GetRemoteAddressForIpv6();
    }

    return {"127.0.0.1", 0};  // NOLINT
}

[[nodiscard]] SocketAddress Socket::GetRemoteAddressForIpv4() const {
    sockaddr_in address{};
    auto addressLength = static_cast<SocketLength>(sizeof(address));
    address.sin_family = AF_INET;

    const int32_t result = getpeername(_socket, reinterpret_cast<sockaddr*>(&address), &addressLength);
    if (result != 0) {
        throw CoSimException("Could not get remote socket address. " + GetSystemErrorMessage(GetLastNetworkError()));
    }

    return ConvertFromInternetAddress(address);
}

[[nodiscard]] SocketAddress Socket::GetRemoteAddressForIpv6() const {
    sockaddr_in6 address{};
    auto addressLength = static_cast<SocketLength>(sizeof(address));
    address.sin6_family = AF_INET6;

    const int32_t result = getpeername(_socket, reinterpret_cast<sockaddr*>(&address), &addressLength);
    if (result != 0) {
        throw CoSimException("Could not get remote socket address. " + GetSystemErrorMessage(GetLastNetworkError()));
    }

    if (address.sin6_family == AF_INET) {
        return GetRemoteAddressForIpv4();
    }

    return ConvertFromInternetAddress(address);
}

[[nodiscard]] bool Socket::Receive(void* destination, int32_t size, int32_t& receivedSize) const {
#ifdef _WIN32
    receivedSize = recv(_socket, static_cast<char*>(destination), size, 0);
#else
    receivedSize = static_cast<int32_t>(recv(_socket, destination, size, MSG_NOSIGNAL));
#endif

    if (receivedSize > 0) {
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

    LogError("Could not receive from remote endpoint. " + GetSystemErrorMessage(errorCode));
    return false;
}

[[nodiscard]] bool Socket::Send(const void* source, int32_t size, int32_t& sentSize) const {
#ifdef _WIN32
    sentSize = send(_socket, static_cast<const char*>(source), size, 0);
#else
    sentSize = static_cast<int32_t>(send(_socket, source, size, MSG_NOSIGNAL));
#endif

    if (sentSize > 0) {
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

    LogError("Could not send to remote endpoint. " + GetSystemErrorMessage(errorCode));
    return false;
}

void Socket::EnsureIsValid() const {
    if (!IsValid()) {
        throw CoSimException("Socket is not valid.");
    }
}

}  // namespace DsVeosCoSim
