// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "Socket.hpp"

#include <fmt/format.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

#include "Logger.hpp"
#include "Result.hpp"

#ifdef _WIN32

#include <WS2tcpip.h>
#include <winsock2.h>

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
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <unistd.h>

#include <cerrno>
#include <string_view>

#endif

namespace DsVeosCoSim {

#ifdef _WIN32

using SocketLength = int32_t;
constexpr int32_t ErrorCodeInterrupted = WSAEINTR;
constexpr int32_t ErrorCodeWouldBlock = WSAEWOULDBLOCK;
constexpr int32_t ErrorCodeConnectionAborted = WSAECONNABORTED;
constexpr int32_t ErrorCodeConnectionReset = WSAECONNRESET;

#else

using SocketLength = socklen_t;
constexpr int32_t ErrorCodeInterrupted = EINTR;
constexpr int32_t ErrorCodeInProgress = EINPROGRESS;
constexpr int32_t ErrorCodeBrokenPipe = EPIPE;
constexpr int32_t ErrorCodeConnectionAborted = ECONNABORTED;
constexpr int32_t ErrorCodeConnectionReset = ECONNRESET;

#endif

struct AddressInfoDeleter {  // NOLINT(misc-use-internal-linkage)

    void operator()(addrinfo* addressInfo) const
    {
        freeaddrinfo(addressInfo);
    }
};

using UniqueAddressInfo = std::unique_ptr<addrinfo, AddressInfoDeleter>;

namespace {

#ifdef _WIN32

int32_t DoPoll(pollfd* fds, uint32_t countFds, int32_t timeoutInMilliseconds)
{
    return WSAPoll(fds, countFds, timeoutInMilliseconds);
}

#else

int32_t DoPoll(pollfd* fds, nfds_t countFds, int32_t timeoutInMilliseconds)
{
    return poll(fds, countFds, timeoutInMilliseconds);
}

int32_t DoUnlink(const std::string& path)
{
    return unlink(path.c_str());
}

[[nodiscard]] std::string GetLocalPath(std::string_view name)
{
    return fmt::format("dSPACE.VEOS.CoSim.{}", name);
}

#endif

[[nodiscard]] int32_t GetLastNetworkError()
{
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}

[[nodiscard]] Result ConvertToInternetAddress(const std::string& ipAddress, uint16_t port, UniqueAddressInfo& addressInfo)
{
    // 5 characters for up to 65536 ports + terminating null
    char portString[6]{};
    (void)std::snprintf(portString, sizeof(portString), "%u", static_cast<unsigned>(port));  // NOLINT(cppcoreguidelines-pro-type-vararg)

    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV;

    addrinfo* rawAddressInfo = nullptr;
    int32_t errorCode = getaddrinfo(ipAddress.c_str(), portString, &hints, &rawAddressInfo);
    if (errorCode != 0) {
        LogError(errorCode, "Could not get address information.");
        return CreateError();
    }

    addressInfo = UniqueAddressInfo(rawAddressInfo);
    return CreateOk();
}

[[nodiscard]] Result ConvertFromInternetAddress(const sockaddr_in& ipv4Address, std::string& socketAddress)
{
    uint16_t port = ntohs(ipv4Address.sin_port);

    std::array<char, INET_ADDRSTRLEN> ipAddress{};
    const char* ipAddressString = inet_ntop(AF_INET, &ipv4Address.sin_addr, ipAddress.data(), INET_ADDRSTRLEN);
    if (!ipAddressString) {
        LogError(GetLastNetworkError(), "Could not get address information.");
        return CreateError();
    }

    socketAddress = fmt::format("{}:{}", ipAddressString, port);
    return CreateOk();
}

[[nodiscard]] Result ConvertFromInternetAddress(const sockaddr_in6& ipv6Address, std::string& socketAddress)
{
    uint16_t port = ntohs(ipv6Address.sin6_port);

    std::array<char, INET6_ADDRSTRLEN> ipAddress{};
    const char* ipAddressString = inet_ntop(AF_INET6, &ipv6Address.sin6_addr, ipAddress.data(), INET6_ADDRSTRLEN);
    if (!ipAddressString) {
        LogError(GetLastNetworkError(), "Could not get address information.");
        return CreateError();
    }

    socketAddress = fmt::format("{}:{}", ipAddressString, port);
    return CreateOk();
}

[[nodiscard]] Result SwitchToNonBlockingMode(const SocketHandle& socketHandle)
{
#ifdef _WIN32
    u_long mode = 1;
    int32_t result = ioctlsocket(socketHandle.Get(), FIONBIO, &mode);
    if (result != 0) {
        LogError(GetLastNetworkError(), "Could not switch to non-blocking mode.");
        return CreateError();
    }
#else
    int32_t flags = fcntl(socketHandle.Get(), F_GETFL, 0);
    if (flags < 0) {
        LogError(GetLastNetworkError(), "Could not get socket flags.");
        return CreateError();
    }

    int32_t result = fcntl(socketHandle.Get(), F_SETFL, flags | O_NONBLOCK);  // NOLINT(cppcoreguidelines-pro-type-vararg)
    if (result < 0) {
        LogError(GetLastNetworkError(), "Could not switch to non-blocking mode.");
        return CreateError();
    }
#endif

    return CreateOk();
}

[[nodiscard]] Result SwitchToBlockingMode(const SocketHandle& socketHandle)
{
#ifdef _WIN32
    u_long mode = 0;
    int32_t result = ioctlsocket(socketHandle.Get(), FIONBIO, &mode);
    if (result != 0) {
        LogError(GetLastNetworkError(), "Could not switch to blocking mode.");
        return CreateError();
    }
#else
    int32_t flags = fcntl(socketHandle.Get(), F_GETFL, 0);
    if (flags < 0) {
        LogError(GetLastNetworkError(), "Could not get socket flags.");
        return CreateError();
    }

    int32_t result = fcntl(socketHandle.Get(), F_SETFL, flags & ~O_NONBLOCK);  // NOLINT(cppcoreguidelines-pro-type-vararg)
    if (result < 0) {
        LogError(GetLastNetworkError(), "Could not switch to blocking mode.");
        return CreateError();
    }
#endif

    return CreateOk();
}

[[nodiscard]] Result CheckSocketError(const SocketHandle& socketHandle)
{
    int32_t error = 0;
    auto len = static_cast<SocketLength>(sizeof(error));
    int32_t getSocketOptionResult = getsockopt(socketHandle.Get(), SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error), &len);
    if (getSocketOptionResult != 0) {
        int32_t errorCode = GetLastNetworkError();
        if (errorCode == ErrorCodeInterrupted) {
            return CreateNotConnected();
        }

        LogError(errorCode, "Could not get socket option SO_ERROR.");
        return CreateError();
    }

    if (error != 0) {
        LogError(error, "Socket error after connect.");
        return CreateError();
    }

    return CreateOk();
}

[[nodiscard]] Result ConnectWithTimeout(const SocketHandle& socketHandle,
                                        const sockaddr* socketAddress,
                                        SocketLength sizeOfSocketAddress,
                                        uint32_t timeoutInMilliseconds)
{
    CheckResult(SwitchToNonBlockingMode(socketHandle));

    int32_t connectResult = connect(socketHandle.Get(), socketAddress, sizeOfSocketAddress);
    if (connectResult == 0) {
        return SwitchToBlockingMode(socketHandle);
    }

    int32_t errorCode = GetLastNetworkError();
#ifdef _WIN32
    constexpr int32_t allowedErrorCode = ErrorCodeWouldBlock;
#else
    constexpr int32_t allowedErrorCode = ErrorCodeInProgress;
#endif
    if (errorCode != allowedErrorCode) {
        LogError(errorCode, "Could not connect to socket.");
        return CreateError();
    }

    pollfd pfd{};
    pfd.fd = socketHandle.Get();
    pfd.events = POLLOUT;

    int32_t pollResult = DoPoll(&pfd, 1, static_cast<int32_t>(timeoutInMilliseconds));
    if (pollResult == 0) {
        return CreateTimeout();
    }

    if (pollResult < 0) {
        LogError(GetLastNetworkError(), "Select failed.");
        return CreateError();
    }

    CheckResult(SwitchToBlockingMode(socketHandle));
    return CheckSocketError(socketHandle);
}

[[nodiscard]] Result ConvertAddressFamily(int32_t addressFamily, AddressFamily& convertedAddressFamily)
{
    switch (addressFamily) {
#ifndef _WIN32
    case AF_UNIX:
        convertedAddressFamily = AddressFamily::Local;
        return CreateOk();
#endif
    case AF_INET:
        convertedAddressFamily = AddressFamily::Ipv4;
        return CreateOk();
    case AF_INET6:
        convertedAddressFamily = AddressFamily::Ipv6;
        return CreateOk();
    default:
        LogError("Invalid address family.");
        return CreateError();
    }
}

[[nodiscard]] Result EnableIpv6Only(const SocketHandle& socketHandle)
{
    // Always set explicitly to disable dual mode, independent of the platform default
    int32_t flags = 1;
    int32_t result = setsockopt(socketHandle.Get(), IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<char*>(&flags), static_cast<SocketLength>(sizeof(flags)));
    if (result != 0) {
        LogError(GetLastNetworkError(), "Could not enable IPv6 only.");
        return CreateError();
    }

    return CreateOk();
}

[[nodiscard]] Result ConvertToNumericAddress(const std::string& ipAddress, uint16_t port, UniqueAddressInfo& addressInfo)
{
    // 5 characters for up to 65536 ports + terminating null
    char portString[6]{};
    (void)std::snprintf(portString, sizeof(portString), "%u", static_cast<unsigned>(port));  // NOLINT(cppcoreguidelines-pro-type-vararg)

    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_NUMERICHOST | AI_NUMERICSERV | AI_PASSIVE;

    addrinfo* rawAddressInfo = nullptr;
    int32_t errorCode = getaddrinfo(ipAddress.c_str(), portString, &hints, &rawAddressInfo);
    if ((errorCode != 0) || !rawAddressInfo) {
        LogError("'{}' is not a valid IP address.", ipAddress);
        return CreateError();
    }

    addressInfo = UniqueAddressInfo(rawAddressInfo);
    return CreateOk();
}

[[nodiscard]] Result EnableReuseAddress(const SocketHandle& socketHandle)
{
    int32_t flags = 1;
    int32_t result = setsockopt(socketHandle.Get(), SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&flags), static_cast<SocketLength>(sizeof(flags)));
    if (result != 0) {
        LogError(GetLastNetworkError(), "Could not enable socket option reuse address.");
        return CreateError();
    }

    return CreateOk();
}

[[nodiscard]] Result EnableExclusiveListenAddress(const SocketHandle& socketHandle)
{
    // Overlapping binds (same address twice or wildcard plus specific address on the same port) must fail on all platforms.
    // On Linux, SO_REUSEADDR still rejects them for listening sockets and only allows rebinding while connections are in TIME_WAIT.
    // On Windows, SO_REUSEADDR would allow them, so SO_EXCLUSIVEADDRUSE is used instead.
#ifdef _WIN32
    int32_t flags = 1;
    int32_t result = setsockopt(socketHandle.Get(), SOL_SOCKET, SO_EXCLUSIVEADDRUSE, reinterpret_cast<char*>(&flags), static_cast<SocketLength>(sizeof(flags)));
    if (result != 0) {
        LogError(GetLastNetworkError(), "Could not enable socket option exclusive address use.");
        return CreateError();
    }

    return CreateOk();
#else
    return EnableReuseAddress(socketHandle);
#endif
}

[[nodiscard]] Result Bind(const SocketHandle& socketHandle, const std::string& ipAddress, uint16_t port)
{
    UniqueAddressInfo addressInfo{};
    CheckResult(ConvertToNumericAddress(ipAddress, port, addressInfo));

    int32_t result = bind(socketHandle.Get(), addressInfo->ai_addr, static_cast<SocketLength>(addressInfo->ai_addrlen));
    if (result != 0) {
        LogError(GetLastNetworkError(), "Could not bind socket to {}:{}.", ipAddress, port);
        return CreateError();
    }

    return CreateOk();
}

#ifndef _WIN32

[[nodiscard]] sockaddr_un CreateUnixAddress(std::string_view path)
{
    sockaddr_un address{};
    address.sun_family = AF_UNIX;

    size_t maxLength = sizeof(address.sun_path) - 1;
    size_t length = std::min(path.size(), maxLength);
    memcpy(address.sun_path, path.data(), length);
    address.sun_path[length] = '\0';

    // Make it abstract
    address.sun_path[0] = '\0';

    return address;
}

[[nodiscard]] Result BindForLocal(const SocketHandle& socketHandle, std::string_view path)
{
    sockaddr_un address = CreateUnixAddress(path);

    int32_t result = bind(socketHandle.Get(), reinterpret_cast<sockaddr*>(&address), sizeof address);
    if (result != 0) {
        LogError(GetLastNetworkError(), "Could not bind socket.");
        return CreateError();
    }

    return CreateOk();
}

#endif

[[nodiscard]] Result Listen(const SocketHandle& socketHandle)
{
    int32_t result = listen(socketHandle.Get(), SOMAXCONN);
    if (result != 0) {
        LogError(GetLastNetworkError(), "Could not listen on socket.");
        return CreateError();
    }

    return CreateOk();
}

[[nodiscard]] Result GetLocalPortForIpv4(const SocketHandle& socketHandle, uint16_t& localPort)
{
    sockaddr_in address{};
    auto addressLength = static_cast<SocketLength>(sizeof(address));
    address.sin_family = AF_INET;

    int32_t result = getsockname(socketHandle.Get(), reinterpret_cast<sockaddr*>(&address), &addressLength);
    if (result != 0) {
        LogError(GetLastNetworkError(), "Could not get local socket address.");
        return CreateError();
    }

    localPort = ntohs(address.sin_port);
    return CreateOk();
}

[[nodiscard]] Result GetLocalPortForIpv6(const SocketHandle& socketHandle, uint16_t& localPort)
{
    sockaddr_in6 address{};
    auto addressLength = static_cast<SocketLength>(sizeof(address));
    address.sin6_family = AF_INET6;

    int32_t result = getsockname(socketHandle.Get(), reinterpret_cast<sockaddr*>(&address), &addressLength);
    if (result != 0) {
        LogError(GetLastNetworkError(), "Could not get local socket address.");
        return CreateError();
    }

    localPort = ntohs(address.sin6_port);
    return CreateOk();
}

[[nodiscard]] Result GetRemoteAddressForIpv4(const SocketHandle& socketHandle, std::string& remoteAddress)
{
    sockaddr_in address{};
    address.sin_family = AF_INET;
    auto addressLength = static_cast<SocketLength>(sizeof(address));

    int32_t result = getpeername(socketHandle.Get(), reinterpret_cast<sockaddr*>(&address), &addressLength);
    if (result != 0) {
        LogError(GetLastNetworkError(), "Could not get remote socket address.");
        return CreateError();
    }

    return ConvertFromInternetAddress(address, remoteAddress);
}

[[nodiscard]] Result GetRemoteAddressForIpv6(const SocketHandle& socketHandle, std::string& remoteAddress)
{
    sockaddr_in6 address{};
    address.sin6_family = AF_INET6;
    auto addressLength = static_cast<SocketLength>(sizeof(address));

    int32_t result = getpeername(socketHandle.Get(), reinterpret_cast<sockaddr*>(&address), &addressLength);
    if (result != 0) {
        LogError(GetLastNetworkError(), "Could not get remote socket address.");
        return CreateError();
    }

    return ConvertFromInternetAddress(address, remoteAddress);
}

[[nodiscard]] Result EnableNoDelay(const SocketHandle& socketHandle)
{
    int32_t flags = 1;
    int32_t result = setsockopt(socketHandle.Get(), IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&flags), static_cast<SocketLength>(sizeof(flags)));
    if (result != 0) {
        LogError(GetLastNetworkError(), "Could not enable TCP option no delay.");
        return CreateError();
    }

    return CreateOk();
}

[[nodiscard]] Result PollInternal(const SocketHandle& socketHandle)
{
    pollfd fdArray{};
    fdArray.fd = socketHandle.Get();
    fdArray.events = POLLRDNORM;

    int32_t pollResult = DoPoll(&fdArray, 1, 0);
    if (pollResult < 0) {
        LogError(GetLastNetworkError(), "Could not poll on socket.");
        return CreateError();
    }

    if (pollResult == 0) {
        return CreateNotConnected();
    }

    return CheckSocketError(socketHandle);
}

[[nodiscard]] Result ConnectInternal(const addrinfo& addressInfo, uint16_t localPort, uint32_t timeoutInMilliseconds, SocketHandle& socketHandle)
{
    int32_t addressFamily = addressInfo.ai_family;

    socketHandle = SocketHandle(socket(addressFamily, addressInfo.ai_socktype, addressInfo.ai_protocol));
    if (!socketHandle.IsValid()) {
        return CreateError();
    }

    if (localPort != 0) {
        CheckResult(EnableReuseAddress(socketHandle));

        // Bind to the wildcard address, so that the operating system can choose the interface for reaching the remote endpoint
        const char* anyAddress = (addressFamily == AF_INET) ? "0.0.0.0" : "::";
        CheckResult(Bind(socketHandle, anyAddress, localPort));
    }

    return ConnectWithTimeout(socketHandle, addressInfo.ai_addr, static_cast<SocketLength>(addressInfo.ai_addrlen), timeoutInMilliseconds);
}

void Shutdown(const SocketHandle& socketHandle)
{
#ifdef _WIN32
    shutdown(socketHandle.Get(), SD_BOTH);
#else
    shutdown(socketHandle.Get(), SHUT_RDWR);
#endif
}

}  // namespace

[[nodiscard]] Result StartupNetwork()
{
#ifdef _WIN32
    static std::once_flag initFlag;
    static bool isInitialized = false;

    std::call_once(initFlag, [] {
        WSADATA wsaData;

        int32_t errorCode = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (errorCode != 0) {
            LogError(errorCode, "Could not initialize Windows sockets.");
        } else {
            isInitialized = true;
        }
    });

    if (!isInitialized) {
        return CreateError();
    }
#endif

    return CreateOk();
}

[[nodiscard]] bool IsIpv4SocketSupported()
{
    static std::once_flag initFlag;
    static bool isSupported = false;

    std::call_once(initFlag, [] {
        SocketHandle socketHandle(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
        isSupported = socketHandle.IsValid();
    });

    return isSupported;
}

[[nodiscard]] bool IsIpv6SocketSupported()
{
    static std::once_flag initFlag;
    static bool isSupported = false;

    std::call_once(initFlag, [] {
        SocketHandle socketHandle(socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP));
        isSupported = socketHandle.IsValid();
    });

    return isSupported;
}

#ifndef _WIN32

[[nodiscard]] bool IsLocalSocketSupported()
{
    static std::once_flag initFlag;
    static bool isSupported = false;

    std::call_once(initFlag, [] {
        SocketHandle socketHandle(socket(AF_UNIX, SOCK_STREAM, 0));
        isSupported = socketHandle.IsValid();
    });

    return isSupported;
}

#endif

void SocketHandle::Reset(socket_t newSocket)
{
    if (IsValid()) {
#ifdef _WIN32
        closesocket(_socket);
#else
        close(_socket);
#endif
    }

    _socket = newSocket;
}

#ifdef _WIN32

SocketClient::SocketClient(SocketHandle socketHandle, AddressFamily addressFamily)
    : _socketHandle(std::move(socketHandle))
    , _addressFamily(addressFamily)
    , _isConnected(true)
{
}

#else

SocketClient::SocketClient(SocketHandle socketHandle, AddressFamily addressFamily, std::string path)
    : _socketHandle(std::move(socketHandle))
    , _addressFamily(addressFamily)
    , _path(std::move(path))
    , _isConnected(true)
{
}

#endif

SocketClient::~SocketClient() noexcept
{
    Disconnect();
}

[[nodiscard]] Result SocketClient::TryConnect(const std::string& ipAddress,
                                              uint16_t remotePort,
                                              uint16_t localPort,
                                              uint32_t timeoutInMilliseconds,
                                              SocketClient& client)
{
    if (remotePort == 0) {
        LogError("Remote port 0 is not valid.");
        return CreateError();
    }

    if (client._isConnected || client._socketHandle.IsValid()) {
        LogError("Client is already in use.");
        return CreateError();
    }

    UniqueAddressInfo addressInfo{};
    CheckResult(ConvertToInternetAddress(ipAddress, remotePort, addressInfo));

    addrinfo* currentAddressInfo = addressInfo.get();

    while (currentAddressInfo) {
        SocketHandle socketHandle;
        if (!IsOk(ConnectInternal(*currentAddressInfo, localPort, timeoutInMilliseconds, socketHandle))) {
            currentAddressInfo = currentAddressInfo->ai_next;
            continue;
        }

        CheckResult(EnableNoDelay(socketHandle));

        AddressFamily convertedAddressFamily{};
        CheckResult(ConvertAddressFamily(currentAddressInfo->ai_family, convertedAddressFamily));

        client._addressFamily = convertedAddressFamily;
        client._socketHandle = std::move(socketHandle);
#ifndef _WIN32
        client._path = "";
#endif
        client._isConnected = true;
        return CreateOk();
    }

    return CreateNotConnected();
}

#ifndef _WIN32

[[nodiscard]] Result SocketClient::TryConnect(std::string_view name, SocketClient& client)
{
    if (name.empty()) {
        LogError("Empty name is not valid.");
        return CreateError();
    }

    if (client._isConnected || client._socketHandle.IsValid()) {
        LogError("Client is already in use.");
        return CreateError();
    }

    SocketHandle socketHandle(socket(AF_UNIX, SOCK_STREAM, 0));
    if (!socketHandle.IsValid()) {
        LogError(GetLastNetworkError(), "Could not create socket.");
        return CreateError();
    }

    std::string path = GetLocalPath(name);

    sockaddr_un address = CreateUnixAddress(path);

    int32_t result = connect(socketHandle.Get(), reinterpret_cast<sockaddr*>(&address), sizeof address);
    if (result != 0) {
        return CreateNotConnected();
    }

    client._addressFamily = AddressFamily::Local;
    client._socketHandle = std::move(socketHandle);
    client._path = path;
    client._isConnected = true;
    return CreateOk();
}

#endif

void SocketClient::Disconnect()
{
    if (!_isConnected) {
        return;
    }

    _isConnected = false;

    if (_socketHandle.IsValid()) {
        Shutdown(_socketHandle);
        _socketHandle.Reset();
    }
}

[[nodiscard]] Result SocketClient::GetRemoteAddress(std::string& remoteAddress) const
{
    if (!IsConnected()) {
        return CreateNotConnected();
    }

    if (_addressFamily == AddressFamily::Ipv4) {
        return GetRemoteAddressForIpv4(_socketHandle, remoteAddress);
    }

    if (_addressFamily == AddressFamily::Ipv6) {
        return GetRemoteAddressForIpv6(_socketHandle, remoteAddress);
    }

    remoteAddress = "local";
    return CreateOk();
}

[[nodiscard]] Result SocketClient::Receive(void* destination, size_t size, size_t& receivedSize) const
{
    if (!IsConnected()) {
        return CreateNotConnected();
    }

    if (size == 0) {
        receivedSize = 0;
        return CreateOk();
    }

#ifdef _WIN32
    int32_t chunkSize = static_cast<int32_t>(std::min<size_t>(size, INT32_MAX));
    int32_t receivedSizeTmp = recv(_socketHandle.Get(), static_cast<char*>(destination), chunkSize, 0);
#else
    ssize_t receivedSizeTmp = recv(_socketHandle.Get(), destination, size, MSG_NOSIGNAL);
#endif

    if (receivedSizeTmp > 0) {
        receivedSize = static_cast<size_t>(receivedSizeTmp);
        return CreateOk();
    }

    if (receivedSizeTmp == 0) {
        LogTrace("Remote endpoint disconnected.");
        return CreateNotConnected();
    }

    if (!_isConnected) {
        LogTrace("Local endpoint disconnected.");
        return CreateNotConnected();
    }

    int32_t errorCode = GetLastNetworkError();

    if ((errorCode == ErrorCodeConnectionAborted) || (errorCode == ErrorCodeConnectionReset)
#ifndef _WIN32
        || (errorCode == ErrorCodeBrokenPipe)
#endif
    ) {
        LogTrace("Local endpoint disconnected.");
        return CreateNotConnected();
    }

    LogError(GetLastNetworkError(), "Could not receive from remote endpoint.");
    return CreateError();
}

[[nodiscard]] Result SocketClient::WaitForData(uint32_t timeoutInMilliseconds) const
{
    if (!IsConnected()) {
        return CreateNotConnected();
    }

    pollfd pfd{};
    pfd.fd = _socketHandle.Get();
    pfd.events = POLLIN;

    int32_t pollResult = DoPoll(&pfd, 1, static_cast<int32_t>(timeoutInMilliseconds));
    if (pollResult == 0) {
        return CreateTimeout();
    }

    if (pollResult < 0) {
        LogError(GetLastNetworkError(), "Poll failed.");
        return CreateError();
    }

    return CreateOk();
}

[[nodiscard]] Result SocketClient::Send(const void* source, size_t size) const
{
    if (!IsConnected()) {
        return CreateNotConnected();
    }

    if (size == 0) {
        return CreateOk();
    }

    const auto* buffer = static_cast<const char*>(source);

    while (size > 0) {
#ifdef _WIN32
        int32_t chunkSize = static_cast<int32_t>(std::min<size_t>(size, INT32_MAX));
        int32_t sentSize = send(_socketHandle.Get(), buffer, chunkSize, 0);
#else
        ssize_t sentSize = send(_socketHandle.Get(), buffer, size, MSG_NOSIGNAL);
#endif

        if (sentSize > 0) {
            size -= static_cast<size_t>(sentSize);
            buffer += static_cast<size_t>(sentSize);
            continue;
        }

        if (sentSize == 0) {
            LogTrace("Remote endpoint disconnected.");
            return CreateNotConnected();
        }

        if (!_isConnected) {
            LogTrace("Local endpoint disconnected.");
            return CreateNotConnected();
        }

        int32_t errorCode = GetLastNetworkError();

        if ((errorCode == ErrorCodeConnectionAborted) || (errorCode == ErrorCodeConnectionReset)
#ifndef _WIN32
            || (errorCode == ErrorCodeBrokenPipe)
#endif
        ) {
            LogTrace("Local endpoint disconnected.");
            return CreateNotConnected();
        }

        LogError(GetLastNetworkError(), "Could not send to remote endpoint.");
        return CreateError();
    }

    return CreateOk();
}

[[nodiscard]] bool SocketClient::IsConnected() const
{
    return _isConnected && _socketHandle.IsValid();
}

#ifdef _WIN32

SocketListener::SocketListener(SocketHandle socketHandle, AddressFamily addressFamily)
    : _socketHandle(std::move(socketHandle))
    , _addressFamily(addressFamily)
    , _isRunning(true)
{
}

#else

SocketListener::SocketListener(SocketHandle socketHandle, AddressFamily addressFamily, std::string path)
    : _socketHandle(std::move(socketHandle))
    , _addressFamily(addressFamily)
    , _path(std::move(path))
    , _isRunning(true)
{
}

#endif

SocketListener::~SocketListener() noexcept
{
    Stop();
}

[[nodiscard]] Result SocketListener::Create(const std::string& ipAddress, uint16_t port, SocketListener& listener)
{
    UniqueAddressInfo addressInfo{};
    CheckResult(ConvertToNumericAddress(ipAddress, port, addressInfo));

    AddressFamily addressFamily{};
    CheckResult(ConvertAddressFamily(addressInfo->ai_family, addressFamily));

    SocketHandle socketHandle(socket(addressInfo->ai_family, addressInfo->ai_socktype, addressInfo->ai_protocol));
    if (!socketHandle.IsValid()) {
        LogError(GetLastNetworkError(), "Could not create TCP socket.");
        return CreateError();
    }

    if (addressFamily == AddressFamily::Ipv6) {
        CheckResult(EnableIpv6Only(socketHandle));
    }

    CheckResult(EnableExclusiveListenAddress(socketHandle));
    CheckResult(Bind(socketHandle, ipAddress, port));
    CheckResult(Listen(socketHandle));

#ifdef _WIN32
    listener = SocketListener(std::move(socketHandle), addressFamily);
#else
    listener = SocketListener(std::move(socketHandle), addressFamily, "");
#endif
    return CreateOk();
}

#ifndef _WIN32

[[nodiscard]] Result SocketListener::Create(std::string_view name, SocketListener& listener)
{
    if (name.empty()) {
        LogError("Empty name is not valid.");
        return CreateError();
    }

    SocketHandle socketHandle(socket(AF_UNIX, SOCK_STREAM, 0));
    if (!socketHandle.IsValid()) {
        LogError(GetLastNetworkError(), "Could not create local socket.");
        return CreateError();
    }

    std::string path = GetLocalPath(name);

    CheckResult(BindForLocal(socketHandle, path));
    CheckResult(Listen(socketHandle));

    listener = SocketListener(std::move(socketHandle), AddressFamily::Local, std::move(path));
    return CreateOk();
}

#endif

void SocketListener::Stop()
{
    if (!_isRunning) {
        return;
    }

    if (_socketHandle.IsValid()) {
        Shutdown(_socketHandle);
    }

#ifndef _WIN32
    if (!_path.empty()) {
        DoUnlink(_path);
        _path.clear();
    }
#endif

    _isRunning = false;
}

[[nodiscard]] Result SocketListener::TryAccept(SocketClient& client) const
{
    if (!IsRunning()) {
        LogError("Server is not running.");
        return CreateError();
    }

    CheckResult(PollInternal(_socketHandle));

    SocketHandle acceptedSocketHandle(accept(_socketHandle.Get(), nullptr, nullptr));
    if (!acceptedSocketHandle.IsValid()) {
        LogError(GetLastNetworkError(), "Could not accept socket.");
        return CreateError();
    }

#ifndef _WIN32
    if (_addressFamily != AddressFamily::Local) {
        CheckResult(EnableNoDelay(acceptedSocketHandle));
    }
#endif

#ifdef _WIN32
    client = SocketClient(std::move(acceptedSocketHandle), _addressFamily);
#else
    client = SocketClient(std::move(acceptedSocketHandle), _addressFamily, _path);
#endif
    return CreateOk();
}

[[nodiscard]] Result SocketListener::GetLocalPort(uint16_t& localPort) const
{
    if (!IsRunning()) {
        LogError("Server is not running.");
        return CreateError();
    }

    if (_addressFamily == AddressFamily::Ipv4) {
        return GetLocalPortForIpv4(_socketHandle, localPort);
    }

    if (_addressFamily == AddressFamily::Ipv6) {
        return GetLocalPortForIpv6(_socketHandle, localPort);
    }

    localPort = 0U;
    return CreateOk();
}

[[nodiscard]] bool SocketListener::IsRunning() const
{
    return _isRunning && _socketHandle.IsValid();
}

}  // namespace DsVeosCoSim
