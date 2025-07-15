// Copyright dSPACE GmbH. All rights reserved.

#include "Socket.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>

#include "CoSimHelper.h"
#include "DsVeosCoSim/CoSimTypes.h"

#ifdef _WIN32
#include <WS2tcpip.h>
#include <afunix.h>
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

using AddressInfoPtr = addrinfo*;

[[nodiscard]] std::string GetUdsPath(const std::string& name) {
    std::string fileName = "dSPACE.VEOS.CoSim.";
    fileName.append(name);
#ifdef _WIN32
    std::filesystem::path tempDir = std::filesystem::temp_directory_path();
    std::filesystem::path fileDir = tempDir / fileName;
    return fileDir.string();
#else
    return fileName;
#endif
}

[[nodiscard]] int32_t GetLastNetworkError() {
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}

[[nodiscard]] Result ConvertToInternetAddress(const std::string& ipAddress,
                                              uint16_t port,
                                              AddressInfoPtr& addressInfo) {
    std::string portString = std::to_string(port);

    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int32_t errorCode = getaddrinfo(ipAddress.c_str(), portString.c_str(), &hints, &addressInfo);
    if (errorCode != 0) {
        LogSystemError("Could not get address information.", errorCode);
        return Result::Error;
    }

    return Result::Ok;
}

[[nodiscard]] Result ConvertFromInternetAddress(const sockaddr_in& ipv4Address, SocketAddress& socketAddress) {
    socketAddress.port = ntohs(ipv4Address.sin_port);

    std::array<char, INET_ADDRSTRLEN> ipAddress{};
    const char* result = inet_ntop(AF_INET, &ipv4Address.sin_addr, ipAddress.data(), INET_ADDRSTRLEN);
    if (!result) {
        LogSystemError("Could not get address information.", GetLastNetworkError());
        return Result::Error;
    }

    socketAddress.ipAddress = result;
    return Result::Ok;
}

[[nodiscard]] Result ConvertFromInternetAddress(const sockaddr_in6& ipv6Address, SocketAddress& socketAddress) {
    socketAddress.port = ntohs(ipv6Address.sin6_port);

    std::array<char, INET6_ADDRSTRLEN> ipAddress{};
    const char* result = inet_ntop(AF_INET6, &ipv6Address.sin6_addr, ipAddress.data(), INET6_ADDRSTRLEN);
    if (!result) {
        LogSystemError("Could not get address information.", GetLastNetworkError());
        return Result::Error;
    }

    socketAddress.ipAddress = result;
    return Result::Ok;
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

[[nodiscard]] Result PollInternal(SocketHandle socket, int16_t events, bool& eventAvailable) {
    pollfd fdArray{};
    fdArray.fd = socket;
    fdArray.events = events;

    int32_t pollResult = Poll(&fdArray, 1, 0);
    if (pollResult < 0) {
        LogSystemError("Could not poll on socket.", GetLastNetworkError());
        return Result::Error;
    }

    if (pollResult == 0) {
        eventAvailable = false;
        return Result::Ok;
    }

    // Make sure it really succeeded
    int32_t error = 0;
    auto len = static_cast<SocketLength>(sizeof(error));
    int32_t getSocketOptionResult = getsockopt(socket, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error), &len);
    if (getSocketOptionResult != 0) {
        int32_t errorCode = GetLastNetworkError();
        if (errorCode == ErrorCodeInterrupted) {
            eventAvailable = false;
            return Result::Ok;
        }

        LogSystemError("Could not get socket option SO_ERROR.", errorCode);
        return Result::Error;
    }

    eventAvailable = true;
    return Result::Ok;
}

[[nodiscard]] Result SwitchToNonBlockingMode(const SocketHandle& socket) {
#ifdef _WIN32
    u_long mode = 1;
    int32_t result = ioctlsocket(socket, FIONBIO, &mode);
    if (result != 0) {
        LogSystemError("Could not switch to non-blocking mode.", GetLastNetworkError());
        return Result::Error;
    }
#else
    int32_t result = fcntl(socket, F_SETFL, O_NONBLOCK);
    if (result < 0) {
        LogSystemError("Could not switch to non-blocking mode.", GetLastNetworkError());
        return Result::Error;
    }
#endif

    return Result::Ok;
}

[[nodiscard]] Result SwitchToBlockingMode(const SocketHandle& socket) {
#ifdef _WIN32
    u_long mode = 0;
    int32_t result = ioctlsocket(socket, FIONBIO, &mode);
    if (result != 0) {
        LogSystemError("Could not switch to blocking mode.", GetLastNetworkError());
        return Result::Error;
    }
#else
    int32_t result = fcntl(socket, F_SETFL, 0);
    if (result < 0) {
        LogSystemError("Could not switch to blocking mode.", GetLastNetworkError());
        return Result::Error;
    }
#endif

    return Result::Ok;
}

[[nodiscard]] Result ConnectWithTimeout(SocketHandle& socket,
                                        const sockaddr* socketAddress,
                                        SocketLength sizeOfSocketAddress,
                                        uint32_t timeoutInMilliseconds,
                                        bool& connectSuccess) {
    connectSuccess = false;
    CheckResult(SwitchToNonBlockingMode(socket));

    int32_t connectResult = connect(socket, socketAddress, sizeOfSocketAddress);
    if (connectResult >= 0) {
        LogError("Invalid connect result.");
        return Result::Error;
    }

    int32_t errorCode = GetLastNetworkError();
    if (errorCode != ErrorCodeWouldBlock) {
        LogSystemError("Could not connect to socket.", errorCode);
        return Result::Error;
    }

    fd_set set{};
    FD_ZERO(&set);
    FD_SET(socket, &set);

    timeval timeout{};
    timeout.tv_sec = static_cast<decltype(timeout.tv_sec)>(timeoutInMilliseconds / 1000);
    timeout.tv_usec = static_cast<decltype(timeout.tv_usec)>(timeoutInMilliseconds % 1000) * 1000;

#ifdef _WIN32
    int32_t selectResult = select(0, nullptr, &set, nullptr, &timeout);
#else
    int32_t selectResult = select(socket + 1, nullptr, &set, nullptr, &timeout);
#endif
    if ((selectResult > 0) && FD_ISSET(socket, &set)) {
        CheckResult(SwitchToBlockingMode(socket));
        connectSuccess = true;
        return Result::Ok;
    }

    return Result::Ok;
}

}  // namespace

[[nodiscard]] const char* ToString(AddressFamily addressFamily) {
    switch (addressFamily) {
        case AddressFamily::Ipv4:
            return "Ipv4";
        case AddressFamily::Ipv6:
            return "Ipv6";
        case AddressFamily::Uds:
            return "Uds";
    }

    return "<Invalid AddressFamily>";
}

[[nodiscard]] Result StartupNetwork() {
#ifdef _WIN32
    static bool networkStarted = false;
    if (!networkStarted) {
        WSADATA wsaData;

        int32_t errorCode = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (errorCode != 0) {
            LogSystemError("Could not initialize Windows sockets.", errorCode);
            return Result::Error;
        }

        networkStarted = true;
    }
#endif

    return Result::Ok;
}

Socket::Socket(SocketHandle socket, AddressFamily addressFamily, const std::string& path)
    : _socket(socket), _addressFamily(addressFamily), _path(path) {
}

Socket::~Socket() noexcept {
    Close();
}

Socket::Socket(Socket&& other) noexcept {
    Close();

    _socket = other._socket;                // NOLINT(cppcoreguidelines-prefer-member-initializer)
    _addressFamily = other._addressFamily;  // NOLINT(cppcoreguidelines-prefer-member-initializer)
    _path = std::move(other._path);         // NOLINT(cppcoreguidelines-prefer-member-initializer)

    other._socket = InvalidSocket;
    other._addressFamily = {};
}

Socket& Socket::operator=(Socket&& other) noexcept {
    Close();

    _socket = other._socket;
    _addressFamily = other._addressFamily;
    _path = std::move(other._path);

    other._socket = InvalidSocket;
    other._addressFamily = {};

    return *this;
}

[[nodiscard]] bool Socket::IsIpv4Supported() {
    static bool hasValue = false;
    static bool isSupported = false;

    if (!hasValue) {
        SocketHandle socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

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
        SocketHandle socket = ::socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);

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
        SocketHandle socket = ::socket(AF_UNIX, SOCK_STREAM, 0);

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
    SocketHandle socket = _socket;
    if (socket == InvalidSocket) {
        return;
    }

    Shutdown();

    _socket = InvalidSocket;
    _addressFamily = {};

    if (!_path.empty()) {
        (void)Unlink(_path.c_str());
        _path.clear();
    }

    CloseSocket(socket);
}

[[nodiscard]] bool Socket::IsValid() const {
    return _socket != InvalidSocket;
}

[[nodiscard]] Result Socket::EnableIpv6Only() const {  // NOLINT(readability-convert-member-functions-to-static)
    // On windows, IPv6 only is enabled by default
#ifndef _WIN32
    int32_t flags = 1;
    int32_t result = setsockopt(_socket,
                                IPPROTO_IPV6,
                                IPV6_V6ONLY,
                                reinterpret_cast<char*>(&flags),
                                static_cast<SocketLength>(sizeof(flags)));
    if (result != 0) {
        LogSystemError("Could not enable IPv6 only.", GetLastNetworkError());
        return Result::Error;
    }
#endif

    return Result::Ok;
}

[[nodiscard]] Result Socket::Create(AddressFamily addressFamily, Socket& socket) {
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

    SocketHandle socketHandle = ::socket(domain, SOCK_STREAM, protocol);
    if (socketHandle == InvalidSocket) {
        LogSystemError("Could not create socket.", GetLastNetworkError());
        return Result::Error;
    }

    socket = Socket(socketHandle, addressFamily, {});
    return Result::Ok;
}

[[nodiscard]] Result Socket::TryConnect(const std::string& ipAddress,
                                        uint16_t remotePort,
                                        uint16_t localPort,
                                        uint32_t timeoutInMilliseconds,
                                        std::optional<Socket>& connectedSocket) {
    if (remotePort == 0) {
        LogError("Remote port 0 is not valid.");
        return Result::Error;
    }

    AddressInfoPtr addressInfo{};
    CheckResult(ConvertToInternetAddress(ipAddress, remotePort, addressInfo));

    AddressInfoPtr currentAddressInfo = addressInfo;

    while (currentAddressInfo) {
        int32_t addressFamily = currentAddressInfo->ai_family;

        SocketHandle socketHandle =
            ::socket(addressFamily, currentAddressInfo->ai_socktype, currentAddressInfo->ai_protocol);
        if (socketHandle == InvalidSocket) {
            currentAddressInfo = currentAddressInfo->ai_next;
            continue;
        }

        Socket socket(socketHandle, static_cast<AddressFamily>(addressFamily), {});

        if (localPort != 0) {
            if (!IsOk(socket.EnableReuseAddress()) || !IsOk(socket.Bind(localPort, false))) {
                currentAddressInfo = currentAddressInfo->ai_next;
                continue;
            }
        }

        bool connectSuccess{};
        CheckResult(ConnectWithTimeout(socket._socket,
                                       currentAddressInfo->ai_addr,
                                       static_cast<SocketLength>(currentAddressInfo->ai_addrlen),
                                       timeoutInMilliseconds,
                                       connectSuccess));
        if (!connectSuccess) {
            currentAddressInfo = currentAddressInfo->ai_next;
            continue;
        }

        freeaddrinfo(addressInfo);
        connectedSocket = std::move(socket);
        return Result::Ok;
    }

    freeaddrinfo(addressInfo);
    return Result::Ok;
}

[[nodiscard]] Result Socket::TryConnect(const std::string& name, std::optional<Socket>& connectedSocket) {
    if (name.empty()) {
        LogError("Empty name is not valid.");
        return Result::Error;
    }

    SocketHandle socket = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket == InvalidSocket) {
        LogSystemError("Could not create socket.", GetLastNetworkError());
        return Result::Error;
    }

    std::string path = GetUdsPath(name);

    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    (void)strncpy(address.sun_path, path.c_str(), sizeof(address.sun_path) - 1);

#ifndef _WIN32
    address.sun_path[0] = '\0';
#endif

    int32_t result = connect(socket, reinterpret_cast<sockaddr*>(&address), sizeof address);
    if (result != 0) {
        CloseSocket(socket);
        return Result::Ok;
    }

    connectedSocket = Socket{socket, AddressFamily::Uds, path};
    return Result::Ok;
}

[[nodiscard]] Result Socket::Bind(uint16_t port, bool enableRemoteAccess) const {
    if (_addressFamily == AddressFamily::Uds) {
        LogError("Not supported for address family.");
        return Result::Error;
    }

    if (_addressFamily == AddressFamily::Ipv4) {
        return BindForIpv4(port, enableRemoteAccess);
    }

    return BindForIpv6(port, enableRemoteAccess);
}

[[nodiscard]] Result Socket::BindForIpv4(uint16_t port, bool enableRemoteAccess) const {
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = enableRemoteAccess ? INADDR_ANY : htonl(INADDR_LOOPBACK);

    int32_t result = bind(_socket, reinterpret_cast<sockaddr*>(&address), sizeof(address));
    if (result != 0) {
        LogSystemError("Could not bind socket.", GetLastNetworkError());
        return Result::Error;
    }

    return Result::Ok;
}

[[nodiscard]] Result Socket::BindForIpv6(uint16_t port, bool enableRemoteAccess) const {
    sockaddr_in6 address{};
    address.sin6_family = AF_INET6;
    address.sin6_port = htons(port);

    // We don't use in6addr_any, because that won't work, if lwip is linked. Both, lwip and ws2_32, define the same
    // symbol
    address.sin6_addr = enableRemoteAccess ? in6_addr{} : in6addr_loopback;

    int32_t result = bind(_socket, reinterpret_cast<sockaddr*>(&address), sizeof(address));
    if (result != 0) {
        LogSystemError("Could not bind socket.", GetLastNetworkError());
        return Result::Error;
    }

    return Result::Ok;
}

[[nodiscard]] Result Socket::Bind(const std::string& name) {
    if (_addressFamily != AddressFamily::Uds) {
        LogError("Not supported for address family.");
        return Result::Error;
    }

    if (name.empty()) {
        LogError("Empty name is not valid.");
        return Result::Error;
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

    int32_t result = bind(_socket, reinterpret_cast<sockaddr*>(&address), sizeof address);
    if (result != 0) {
        LogSystemError("Could not bind socket.", GetLastNetworkError());
        return Result::Error;
    }

    return Result::Ok;
}

[[nodiscard]] Result Socket::EnableReuseAddress() const {
    if (_addressFamily == AddressFamily::Uds) {
        LogError("Not supported for address family.");
        return Result::Error;
    }

    int32_t flags = 1;
    int32_t result = setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&flags), sizeof(flags));
    if (result != 0) {
        LogSystemError("Could not enable socket option reuse address.", GetLastNetworkError());
        return Result::Error;
    }

    return Result::Ok;
}

[[nodiscard]] Result Socket::EnableNoDelay() const {
    int32_t flags = 1;
    int32_t result = setsockopt(_socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&flags), sizeof(flags));
    if (result != 0) {
        LogSystemError("Could not enable TCP option no delay.", GetLastNetworkError());
        return Result::Error;
    }

    return Result::Ok;
}

[[nodiscard]] Result Socket::Listen() const {
    int32_t result = listen(_socket, SOMAXCONN);
    if (result != 0) {
        LogSystemError("Could not listen on socket.", GetLastNetworkError());
        return Result::Error;
    }

    return Result::Ok;
}

[[nodiscard]] Result Socket::TryAccept(std::optional<Socket>& acceptedSocket) const {
    bool eventAvailable{};
    CheckResult(PollInternal(_socket, POLLRDNORM, eventAvailable));
    if (!eventAvailable) {
        return Result::Ok;
    }

    SocketHandle socket = accept(_socket, nullptr, nullptr);
    if (socket == InvalidSocket) {
        LogSystemError("Could not accept socket.", GetLastNetworkError());
        return Result::Error;
    }

    acceptedSocket = Socket{socket, _addressFamily, _path};
    return Result::Ok;
}

[[nodiscard]] Result Socket::GetLocalPort(uint16_t& localPort) const {
    if (_addressFamily == AddressFamily::Ipv4) {
        return GetLocalPortForIpv4(localPort);
    }

    if (_addressFamily == AddressFamily::Ipv6) {
        return GetLocalPortForIpv6(localPort);
    }

    localPort = 0;
    return Result::Ok;
}

[[nodiscard]] Result Socket::GetLocalPortForIpv4(uint16_t& localPort) const {
    sockaddr_in address{};
    auto addressLength = static_cast<SocketLength>(sizeof(address));
    address.sin_family = AF_INET;

    int32_t result = getsockname(_socket, reinterpret_cast<sockaddr*>(&address), &addressLength);
    if (result != 0) {
        LogSystemError("Could not get local socket address.", GetLastNetworkError());
        return Result::Error;
    }

    SocketAddress localAddress{};
    CheckResult(ConvertFromInternetAddress(address, localAddress));
    localPort = localAddress.port;
    return Result::Ok;
}

[[nodiscard]] Result Socket::GetLocalPortForIpv6(uint16_t& localPort) const {
    sockaddr_in6 address{};
    auto addressLength = static_cast<SocketLength>(sizeof(address));
    address.sin6_family = AF_INET6;

    int32_t result = getsockname(_socket, reinterpret_cast<sockaddr*>(&address), &addressLength);
    if (result != 0) {
        LogSystemError("Could not get local socket address.", GetLastNetworkError());
        return Result::Error;
    }

    SocketAddress localAddress{};
    CheckResult(ConvertFromInternetAddress(address, localAddress));
    localPort = localAddress.port;
    return Result::Ok;
}

[[nodiscard]] Result Socket::GetRemoteAddress(SocketAddress& remoteAddress) const {
    if (_addressFamily == AddressFamily::Ipv4) {
        return GetRemoteAddressForIpv4(remoteAddress);
    }

    if (_addressFamily == AddressFamily::Ipv6) {
        return GetRemoteAddressForIpv6(remoteAddress);
    }

    remoteAddress = SocketAddress{"127.0.0.1", 0};
    return Result::Ok;
}

[[nodiscard]] Result Socket::GetRemoteAddressForIpv4(SocketAddress& remoteAddress) const {
    sockaddr_in address{};
    address.sin_family = AF_INET;
    auto addressLength = static_cast<SocketLength>(sizeof(address));

    int32_t result = getpeername(_socket, reinterpret_cast<sockaddr*>(&address), &addressLength);
    if (result != 0) {
        LogSystemError("Could not get remote socket address.", GetLastNetworkError());
        return Result::Error;
    }

    return ConvertFromInternetAddress(address, remoteAddress);
}

[[nodiscard]] Result Socket::GetRemoteAddressForIpv6(SocketAddress& remoteAddress) const {
    sockaddr_in6 address{};
    address.sin6_family = AF_INET6;
    auto addressLength = static_cast<SocketLength>(sizeof(address));

    int32_t result = getpeername(_socket, reinterpret_cast<sockaddr*>(&address), &addressLength);
    if (result != 0) {
        LogSystemError("Could not get remote socket address.", GetLastNetworkError());
        return Result::Error;
    }

    if (address.sin6_family == AF_INET) {
        return GetRemoteAddressForIpv4(remoteAddress);
    }

    return ConvertFromInternetAddress(address, remoteAddress);
}

[[nodiscard]] Result Socket::Receive(void* destination, int32_t size, int32_t& receivedSize) const {
#ifdef _WIN32
    receivedSize = recv(_socket, static_cast<char*>(destination), size, 0);
#else
    receivedSize = static_cast<int32_t>(recv(_socket, destination, size, MSG_NOSIGNAL));
#endif

    if (receivedSize > 0) {
        return Result::Ok;
    }

    if (receivedSize == 0) {
        LogTrace("Remote endpoint disconnected.");
        return Result::Disconnected;
    }

    int32_t errorCode = GetLastNetworkError();

    if ((errorCode == ErrorCodeConnectionAborted) || (errorCode == ErrorCodeConnectionReset)
#ifndef _WIN32
        || (errorCode == ErrorCodeBrokenPipe)
#endif
    ) {
        LogTrace("Remote endpoint disconnected.");
        return Result::Disconnected;
    }

    LogSystemError("Could not receive from remote endpoint.", errorCode);
    return Result::Error;
}

[[nodiscard]] Result Socket::Send(const void* source, int32_t size, int32_t& sentSize) const {
#ifdef _WIN32
    sentSize = send(_socket, static_cast<const char*>(source), size, 0);
#else
    sentSize = static_cast<int32_t>(send(_socket, source, size, MSG_NOSIGNAL));
#endif

    if (sentSize > 0) {
        return Result::Ok;
    }

    if (sentSize == 0) {
        LogTrace("Remote endpoint disconnected.");
        return Result::Disconnected;
    }

    int32_t errorCode = GetLastNetworkError();

    if ((errorCode == ErrorCodeConnectionAborted) || (errorCode == ErrorCodeConnectionReset)
#ifndef _WIN32
        || (errorCode == ErrorCodeBrokenPipe)
#endif
    ) {
        LogTrace("Remote endpoint disconnected.");
        return Result::Disconnected;
    }

    LogSystemError("Could not send to remote endpoint.", errorCode);
    return Result::Error;
}

}  // namespace DsVeosCoSim
