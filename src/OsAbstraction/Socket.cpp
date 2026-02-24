// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "Socket.hpp"

#include <array>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <optional>
#include <string>
#include <utility>

#include "Error.hpp"

#ifdef _WIN32
#include <WS2tcpip.h>
#include <afunix.h>
#include <winsock2.h>
#undef min
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
constexpr int32_t ErrorCodeConnectionAborted = WSAECONNABORTED;
constexpr int32_t ErrorCodeConnectionReset = WSAECONNRESET;

inline int DoPoll(pollfd* fds, unsigned long nfds, int timeout) {
    return WSAPoll(fds, nfds, timeout);
}

inline int DoUnlink(const char* path) {
    return _unlink(path);
}

#else

using SocketLength = socklen_t;
constexpr int32_t ErrorCodeInterrupted = EINTR;
constexpr int32_t ErrorCodeInProgress = EINPROGRESS;
constexpr int32_t ErrorCodeBrokenPipe = EPIPE;
constexpr int32_t ErrorCodeConnectionAborted = ECONNABORTED;
constexpr int32_t ErrorCodeConnectionReset = ECONNRESET;

inline int DoPoll(pollfd* fds, nfds_t nfds, int timeout) {
    return poll(fds, nfds, timeout);
}

inline int DoUnlink(const char* path) {
    return unlink(path);
}

#endif

struct AddressInfoDeleter {
    void operator()(addrinfo* addressInfo) {
        freeaddrinfo(addressInfo);
    }
};

using UniqueAddressInfo = std::unique_ptr<addrinfo, AddressInfoDeleter>;

[[nodiscard]] std::string GetLocalPath(const std::string& name) {
    std::string fileName = "dSPACE.VEOS.CoSim." + name;
#ifdef _WIN32
    std::filesystem::path fileDir = std::filesystem::temp_directory_path() / fileName;
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

[[nodiscard]] Result ConvertToInternetAddress(const std::string& ipAddress, uint16_t port, UniqueAddressInfo& addressInfo) {
    // 5 characters for up to 65536 ports + terminating null
    char portString[6]{};
    std::snprintf(portString, sizeof(portString), "%u", static_cast<unsigned>(port));  // NOLINT(cppcoreguidelines-pro-type-vararg)

    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV;

    addrinfo* rawAddressInfo = nullptr;
    int32_t errorCode = getaddrinfo(ipAddress.c_str(), portString, &hints, &rawAddressInfo);
    if (errorCode != 0) {
        return CreateError("Could not get address information.", errorCode);
    }

    addressInfo = UniqueAddressInfo(rawAddressInfo);
    return CreateOk();
}

[[nodiscard]] Result ConvertFromInternetAddress(const sockaddr_in& ipv4Address, std::string& socketAddress) {
    uint16_t port = ntohs(ipv4Address.sin_port);

    std::array<char, INET_ADDRSTRLEN> ipAddress{};
    const char* ipAddressString = inet_ntop(AF_INET, &ipv4Address.sin_addr, ipAddress.data(), INET_ADDRSTRLEN);
    if (!ipAddressString) {
        return CreateError("Could not get address information.", GetLastNetworkError());
    }

    socketAddress = std::string(ipAddressString) + ':' + std::to_string(port);
    return CreateOk();
}

[[nodiscard]] Result ConvertFromInternetAddress(const sockaddr_in6& ipv6Address, std::string& socketAddress) {
    uint16_t port = ntohs(ipv6Address.sin6_port);

    std::array<char, INET6_ADDRSTRLEN> ipAddress{};
    const char* ipAddressString = inet_ntop(AF_INET6, &ipv6Address.sin6_addr, ipAddress.data(), INET6_ADDRSTRLEN);
    if (!ipAddressString) {
        return CreateError("Could not get address information.", GetLastNetworkError());
    }

    socketAddress = std::string(ipAddressString) + ':' + std::to_string(port);
    return CreateOk();
}

[[nodiscard]] Result SwitchToNonBlockingMode(const SocketHandle& socketHandle) {
#ifdef _WIN32
    u_long mode = 1;
    int32_t result = ioctlsocket(socketHandle.Get(), FIONBIO, &mode);
    if (result != 0) {
        return CreateError("Could not switch to non-blocking mode.", GetLastNetworkError());
    }
#else
    int32_t flags = fcntl(socketHandle.Get(), F_GETFL, 0);
    if (flags < 0) {
        return CreateError("Could not get socket flags.", GetLastNetworkError());
    }

    int32_t result = fcntl(socketHandle.Get(), F_SETFL, flags | O_NONBLOCK);
    if (result < 0) {
        return CreateError("Could not switch to non-blocking mode.", GetLastNetworkError());
    }
#endif

    return CreateOk();
}

[[nodiscard]] Result SwitchToBlockingMode(const SocketHandle& socketHandle) {
#ifdef _WIN32
    u_long mode = 0;
    int32_t result = ioctlsocket(socketHandle.Get(), FIONBIO, &mode);
    if (result != 0) {
        return CreateError("Could not switch to blocking mode.", GetLastNetworkError());
    }
#else
    int32_t flags = fcntl(socketHandle.Get(), F_GETFL, 0);
    if (flags < 0) {
        return CreateError("Could not get socket flags.", GetLastNetworkError());
    }

    int32_t result = fcntl(socketHandle.Get(), F_SETFL, flags & ~O_NONBLOCK);
    if (result < 0) {
        return CreateError("Could not switch to blocking mode.", GetLastNetworkError());
    }
#endif

    return CreateOk();
}

[[nodiscard]] Result CheckSocketError(const SocketHandle& socketHandle) {
    int32_t error = 0;
    auto len = static_cast<SocketLength>(sizeof(error));
    int32_t getSocketOptionResult = getsockopt(socketHandle.Get(), SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error), &len);
    if (getSocketOptionResult != 0) {
        int32_t errorCode = GetLastNetworkError();
        if (errorCode == ErrorCodeInterrupted) {
            return CreateNotConnected();
        }

        return CreateError("Could not get socket option SO_ERROR.", errorCode);
    }

    if (error != 0) {
        return CreateError("Socket error after connect.", error);
    }

    return CreateOk();
}

[[nodiscard]] Result ConnectWithTimeout(const SocketHandle& socketHandle,
                                        const sockaddr* socketAddress,
                                        SocketLength sizeOfSocketAddress,
                                        uint32_t timeoutInMilliseconds) {
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
        return CreateError("Could not connect to socket.", errorCode);
    }

    pollfd pfd{};
    pfd.fd = socketHandle.Get();
    pfd.events = POLLOUT;

    timeval timeout{};
    timeout.tv_sec = static_cast<decltype(timeout.tv_sec)>(timeoutInMilliseconds / 1000);
    timeout.tv_usec = static_cast<decltype(timeout.tv_usec)>(timeoutInMilliseconds % 1000) * 1000;

    int32_t pollResult = DoPoll(&pfd, 1, static_cast<int32_t>(timeoutInMilliseconds));
    if (pollResult == 0) {
        return CreateTimeout();
    }

    if (pollResult < 0) {
        return CreateError("Select failed.", GetLastNetworkError());
    }

    CheckResult(SwitchToBlockingMode(socketHandle));
    return CheckSocketError(socketHandle);
}

[[nodiscard]] Result ConvertAddressFamily(AddressFamily addressFamily, int32_t& convertedAddressFamily) {
    switch (addressFamily) {
        case AddressFamily::Local:
            convertedAddressFamily = AF_UNIX;
            return CreateOk();
        case AddressFamily::Ipv4:
            convertedAddressFamily = AF_INET;
            return CreateOk();
        case AddressFamily::Ipv6:
            convertedAddressFamily = AF_INET6;
            return CreateOk();
    }

    return CreateError("Invalid address family.");
}

[[nodiscard]] Result ConvertAddressFamily(int32_t addressFamily, AddressFamily& convertedAddressFamily) {
    switch (addressFamily) {
        case AF_UNIX:
            convertedAddressFamily = AddressFamily::Local;
            return CreateOk();
        case AF_INET:
            convertedAddressFamily = AddressFamily::Ipv4;
            return CreateOk();
        case AF_INET6:
            convertedAddressFamily = AddressFamily::Ipv6;
            return CreateOk();
    }

    return CreateError("Invalid address family.");
}

[[nodiscard]] Result EnableIpv6Only([[maybe_unused]] const SocketHandle& socketHandle) {
    // On windows, IPv6 only is enabled by default
#ifndef _WIN32
    int32_t flags = 1;
    int32_t result = setsockopt(socketHandle.Get(), IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<char*>(&flags), static_cast<SocketLength>(sizeof(flags)));
    if (result != 0) {
        return Error("Could not enable IPv6 only.", GetLastNetworkError());
    }
#endif

    return CreateOk();
}

[[nodiscard]] Result EnableReuseAddress(const SocketHandle& socketHandle) {
    int32_t flags = 1;
    int32_t result = setsockopt(socketHandle.Get(), SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&flags), static_cast<SocketLength>(sizeof(flags)));
    if (result != 0) {
        return CreateError("Could not enable socket option reuse address.", GetLastNetworkError());
    }

    return CreateOk();
}

[[nodiscard]] Result BindForIpv4(const SocketHandle& socketHandle, uint16_t port, bool enableRemoteAccess) {
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = enableRemoteAccess ? INADDR_ANY : htonl(INADDR_LOOPBACK);

    int32_t result = bind(socketHandle.Get(), reinterpret_cast<sockaddr*>(&address), static_cast<SocketLength>(sizeof(address)));
    if (result != 0) {
        return CreateError("Could not bind socket.", GetLastNetworkError());
    }

    return CreateOk();
}

[[nodiscard]] Result BindForIpv6(const SocketHandle& socketHandle, uint16_t port, bool enableRemoteAccess) {
    sockaddr_in6 address{};
    address.sin6_family = AF_INET6;
    address.sin6_port = htons(port);

    // We don't use in6addr_any, because that won't work, if lwip is linked. Both, lwip and ws2_32, define the same
    // symbol
    address.sin6_addr = enableRemoteAccess ? in6_addr{} : in6addr_loopback;

    int32_t result = bind(socketHandle.Get(), reinterpret_cast<sockaddr*>(&address), static_cast<SocketLength>(sizeof(address)));
    if (result != 0) {
        return CreateError("Could not bind socket.", GetLastNetworkError());
    }

    return CreateOk();
}

[[nodiscard]] sockaddr_un CreateUnixAddress(const std::string& path) {
    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    size_t maxLength = sizeof(address.sun_path) - 1;
    strncpy(address.sun_path, path.c_str(), maxLength);
    address.sun_path[maxLength] = '\0';

#ifndef _WIN32
    // Make it abstract for non-windows systems
    address.sun_path[0] = '\0';
#endif

    return address;
}

[[nodiscard]] Result BindForLocal(const SocketHandle& socketHandle, const std::string& path) {
    sockaddr_un address = CreateUnixAddress(path);

    int32_t result = bind(socketHandle.Get(), reinterpret_cast<sockaddr*>(&address), sizeof address);
    if (result != 0) {
        return CreateError("Could not bind socket.", GetLastNetworkError());
    }

    return CreateOk();
}

[[nodiscard]] Result Listen(const SocketHandle& socketHandle) {
    int32_t result = listen(socketHandle.Get(), SOMAXCONN);
    if (result != 0) {
        return CreateError("Could not listen on socket.", GetLastNetworkError());
    }

    return CreateOk();
}

[[nodiscard]] Result GetLocalPortForIpv4(const SocketHandle& socketHandle, uint16_t& localPort) {
    sockaddr_in address{};
    auto addressLength = static_cast<SocketLength>(sizeof(address));
    address.sin_family = AF_INET;

    int32_t result = getsockname(socketHandle.Get(), reinterpret_cast<sockaddr*>(&address), &addressLength);
    if (result != 0) {
        return CreateError("Could not get local socket address.", GetLastNetworkError());
    }

    localPort = ntohs(address.sin_port);
    return CreateOk();
}

[[nodiscard]] Result GetLocalPortForIpv6(const SocketHandle& socketHandle, uint16_t& localPort) {
    sockaddr_in6 address{};
    auto addressLength = static_cast<SocketLength>(sizeof(address));
    address.sin6_family = AF_INET6;

    int32_t result = getsockname(socketHandle.Get(), reinterpret_cast<sockaddr*>(&address), &addressLength);
    if (result != 0) {
        return CreateError("Could not get local socket address.", GetLastNetworkError());
    }

    localPort = ntohs(address.sin6_port);
    return CreateOk();
}

[[nodiscard]] Result GetRemoteAddressForIpv4(const SocketHandle& socketHandle, std::string& remoteAddress) {
    sockaddr_in address{};
    address.sin_family = AF_INET;
    auto addressLength = static_cast<SocketLength>(sizeof(address));

    int32_t result = getpeername(socketHandle.Get(), reinterpret_cast<sockaddr*>(&address), &addressLength);
    if (result != 0) {
        return CreateError("Could not get remote socket address.", GetLastNetworkError());
    }

    return ConvertFromInternetAddress(address, remoteAddress);
}

[[nodiscard]] Result GetRemoteAddressForIpv6(const SocketHandle& socketHandle, std::string& remoteAddress) {
    sockaddr_in6 address{};
    address.sin6_family = AF_INET6;
    auto addressLength = static_cast<SocketLength>(sizeof(address));

    int32_t result = getpeername(socketHandle.Get(), reinterpret_cast<sockaddr*>(&address), &addressLength);
    if (result != 0) {
        return CreateError("Could not get remote socket address.", GetLastNetworkError());
    }

    return ConvertFromInternetAddress(address, remoteAddress);
}

[[nodiscard]] Result EnableNoDelay(const SocketHandle& socketHandle) {
    int32_t flags = 1;
    int32_t result = setsockopt(socketHandle.Get(), IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&flags), static_cast<SocketLength>(sizeof(flags)));
    if (result != 0) {
        return CreateError("Could not enable TCP option no delay.", GetLastNetworkError());
    }

    return CreateOk();
}

[[nodiscard]] Result PollInternal(const SocketHandle& socketHandle) {
    pollfd fdArray{};
    fdArray.fd = socketHandle.Get();
    fdArray.events = POLLRDNORM;

    int32_t pollResult = DoPoll(&fdArray, 1, 0);
    if (pollResult < 0) {
        return CreateError("Could not poll on socket.", GetLastNetworkError());
    }

    if (pollResult == 0) {
        return CreateNotConnected();
    }

    return CheckSocketError(socketHandle);
}

[[nodiscard]] Result ConnectInternal(addrinfo& addressInfo, uint16_t localPort, uint32_t timeoutInMilliseconds, SocketHandle& socketHandle) {
    int32_t addressFamily = addressInfo.ai_family;

    socketHandle = SocketHandle(socket(addressFamily, addressInfo.ai_socktype, addressInfo.ai_protocol));
    if (!socketHandle.IsValid()) {
        return CreateError();
    }

    if (localPort != 0) {
        CheckResult(EnableReuseAddress(socketHandle));

        if (addressFamily == AF_INET) {
            CheckResult(BindForIpv4(socketHandle, localPort, false));
        } else {
            CheckResult(BindForIpv6(socketHandle, localPort, false));
        }
    }

    return ConnectWithTimeout(socketHandle, addressInfo.ai_addr, static_cast<SocketLength>(addressInfo.ai_addrlen), timeoutInMilliseconds);
}

void Shutdown(const SocketHandle& socketHandle) {
#ifdef _WIN32
    shutdown(socketHandle.Get(), SD_BOTH);
#else
    shutdown(socketHandle.Get(), SHUT_RDWR);
#endif
}

}  // namespace

[[nodiscard]] Result StartupNetwork() {
#ifdef _WIN32
    static std::once_flag initFlag;
    static std::optional<Result> initError;

    // call_once guarantees that initError is written once even in multi-threaded environments
    std::call_once(initFlag, []() {
        WSADATA wsaData;

        int32_t errorCode = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (errorCode != 0) {
            initError = CreateError("Could not initialize Windows sockets.", errorCode);
        }
    });

    if (initError) {
        return *initError;
    }
#endif

    return CreateOk();
}

[[nodiscard]] bool IsIpv4SocketSupported() {
    static std::once_flag initFlag;
    static bool isSupported = false;

    std::call_once(initFlag, []() {
        SocketHandle socketHandle(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
        isSupported = socketHandle.IsValid();
    });

    return isSupported;
}

[[nodiscard]] bool IsIpv6SocketSupported() {
    static std::once_flag initFlag;
    static bool isSupported = false;

    std::call_once(initFlag, []() {
        SocketHandle socketHandle(socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP));
        isSupported = socketHandle.IsValid();
    });

    return isSupported;
}

[[nodiscard]] bool IsLocalSocketSupported() {
    static std::once_flag initFlag;
    static bool isSupported = false;

    std::call_once(initFlag, []() {
        SocketHandle socketHandle(socket(AF_UNIX, SOCK_STREAM, 0));
        isSupported = socketHandle.IsValid();
    });

    return isSupported;
}

[[nodiscard]] const char* format_as(AddressFamily addressFamily) {
    switch (addressFamily) {
        case AddressFamily::Local:
            return "Local";
        case AddressFamily::Ipv4:
            return "Ipv4";
        case AddressFamily::Ipv6:
            return "Ipv6";
    }

    return "<Invalid AddressFamily>";
}

void SocketHandle::Reset(socket_t newSocket) {
    if (IsValid()) {
#ifdef _WIN32
        closesocket(_socket);
#else
        close(_socket);
#endif
    }

    _socket = newSocket;
}

SocketClient::SocketClient(SocketHandle socketHandle, AddressFamily addressFamily, std::string path)
    : _socketHandle(std::move(socketHandle)), _addressFamily(addressFamily), _path(std::move(path)), _isConnected(true) {
}

SocketClient::~SocketClient() noexcept {
    Disconnect();
}

[[nodiscard]] Result SocketClient::TryConnect(const std::string& ipAddress,
                                              uint16_t remotePort,
                                              uint16_t localPort,
                                              uint32_t timeoutInMilliseconds,
                                              SocketClient& client) {
    if (remotePort == 0) {
        return CreateError("Remote port 0 is not valid.");
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

        client = SocketClient(std::move(socketHandle), convertedAddressFamily, "");
        return CreateOk();
    }

    return CreateNotConnected();
}

[[nodiscard]] Result SocketClient::TryConnect(const std::string& name, SocketClient& client) {
    if (name.empty()) {
        return CreateError("Empty name is not valid.");
    }

    SocketHandle socketHandle(socket(AF_UNIX, SOCK_STREAM, 0));
    if (!socketHandle.IsValid()) {
        return CreateError("Could not create socket.", GetLastNetworkError());
    }

    std::string path = GetLocalPath(name);

    sockaddr_un address = CreateUnixAddress(path);

    int32_t result = connect(socketHandle.Get(), reinterpret_cast<sockaddr*>(&address), sizeof address);
    if (result != 0) {
        return CreateNotConnected();
    }

    client = SocketClient(std::move(socketHandle), AddressFamily::Local, std::move(path));
    return CreateOk();
}

void SocketClient::Disconnect() {
    if (_socketHandle.IsValid()) {
        Shutdown(_socketHandle);
    }

    _isConnected = false;
}

[[nodiscard]] Result SocketClient::GetRemoteAddress(std::string& remoteAddress) const {
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

[[nodiscard]] Result SocketClient::Receive(void* destination, size_t size, size_t& receivedSize) const {
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
        return CreateNotConnected();
    }

    int32_t errorCode = GetLastNetworkError();

    if ((errorCode == ErrorCodeConnectionAborted) || (errorCode == ErrorCodeConnectionReset)
#ifndef _WIN32
        || (errorCode == ErrorCodeBrokenPipe)
#endif
    ) {
        return CreateNotConnected();
    }

    return CreateError("Could not receive from remote endpoint.", errorCode);
}

[[nodiscard]] Result SocketClient::Send(const void* source, size_t size) const {
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
            return CreateNotConnected();
        }

        int32_t errorCode = GetLastNetworkError();

        if ((errorCode == ErrorCodeConnectionAborted) || (errorCode == ErrorCodeConnectionReset)
#ifndef _WIN32
            || (errorCode == ErrorCodeBrokenPipe)
#endif
        ) {
            return CreateNotConnected();
        }

        return CreateError("Could not send to remote endpoint.", errorCode);
    }

    return CreateOk();
}

[[nodiscard]] bool SocketClient::IsConnected() const {
    return _isConnected && _socketHandle.IsValid();
}

SocketListener::SocketListener(SocketHandle socketHandle, AddressFamily addressFamily, std::string path)
    : _socketHandle(std::move(socketHandle)), _addressFamily(addressFamily), _path(std::move(path)), _isRunning(true) {
}

SocketListener::~SocketListener() noexcept {
    Stop();
}

[[nodiscard]] Result SocketListener::Create(AddressFamily addressFamily, uint16_t port, bool enableRemoteAccess, SocketListener& listener) {
    if (addressFamily == AddressFamily::Local) {
        return CreateError("Not supported for local sockets.");
    }

    int32_t convertedAddressFamily{};
    CheckResult(ConvertAddressFamily(addressFamily, convertedAddressFamily));

    SocketHandle socketHandle(socket(convertedAddressFamily, SOCK_STREAM, IPPROTO_TCP));
    if (!socketHandle.IsValid()) {
        return CreateError("Could not create TCP socket.", GetLastNetworkError());
    }

    if (addressFamily == AddressFamily::Ipv6) {
        CheckResult(EnableIpv6Only(socketHandle));
    }

    CheckResult(EnableReuseAddress(socketHandle));

    if (addressFamily == AddressFamily::Ipv4) {
        CheckResult(BindForIpv4(socketHandle, port, enableRemoteAccess));
    } else {
        CheckResult(BindForIpv6(socketHandle, port, enableRemoteAccess));
    }

    CheckResult(Listen(socketHandle));

    listener = SocketListener(std::move(socketHandle), addressFamily, "");
    return CreateOk();
}

[[nodiscard]] Result SocketListener::Create(const std::string& name, SocketListener& listener) {
    if (name.empty()) {
        return CreateError("Empty name is not valid.");
    }

    SocketHandle socketHandle(socket(AF_UNIX, SOCK_STREAM, 0));
    if (!socketHandle.IsValid()) {
        return CreateError("Could not create local socket.", GetLastNetworkError());
    }

    std::string path = GetLocalPath(name);

#ifdef _WIN32
    DoUnlink(path.c_str());
#endif

    CheckResult(BindForLocal(socketHandle, path));
    CheckResult(Listen(socketHandle));

    listener = SocketListener(std::move(socketHandle), AddressFamily::Local, std::move(path));
    return CreateOk();
}

void SocketListener::Stop() {
    if (!_isRunning) {
        return;
    }

    if (_socketHandle.IsValid()) {
        Shutdown(_socketHandle);
    }

    if (!_path.empty()) {
        DoUnlink(_path.c_str());
        _path.clear();
    }

    _isRunning = false;
}

[[nodiscard]] Result SocketListener::TryAccept(SocketClient& client) const {
    if (!IsRunning()) {
        return CreateError("Server is not running.");
    }

    CheckResult(PollInternal(_socketHandle));

    SocketHandle acceptedSocketHandle(accept(_socketHandle.Get(), nullptr, nullptr));
    if (!acceptedSocketHandle.IsValid()) {
        return CreateError("Could not accept socket.", GetLastNetworkError());
    }

    if (_addressFamily != AddressFamily::Local) {
        CheckResult(EnableNoDelay(acceptedSocketHandle));
    }

    client = SocketClient(std::move(acceptedSocketHandle), _addressFamily, _path);
    return CreateOk();
}

[[nodiscard]] Result SocketListener::GetLocalPort(uint16_t& localPort) const {
    if (!IsRunning()) {
        return CreateError("Server is not running.");
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

[[nodiscard]] bool SocketListener::IsRunning() const {
    return _isRunning && _socketHandle.IsValid();
}

}  // namespace DsVeosCoSim
