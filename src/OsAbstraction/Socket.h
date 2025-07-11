// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include "DsVeosCoSim/CoSimTypes.h"

namespace DsVeosCoSim {

[[nodiscard]] Result StartupNetwork();

enum class AddressFamily {
    Uds = 1,
    Ipv4 = 2,
    Ipv6 = 23
};

[[nodiscard]] std::string_view ToString(AddressFamily addressFamily);

#ifdef _WIN32
using SocketHandle = uintptr_t;
constexpr SocketHandle InvalidSocket = UINTPTR_MAX;
#else
using SocketHandle = int32_t;
constexpr SocketHandle InvalidSocket = -1;
#endif

struct SocketAddress {
    std::string ipAddress;
    uint16_t port{};
};

class Socket {
public:
    Socket() = default;

private:
    Socket(SocketHandle socket, AddressFamily addressFamily, std::string_view path);

public:
    ~Socket() noexcept;

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;

    [[nodiscard]] static bool IsIpv4Supported();
    [[nodiscard]] static bool IsIpv6Supported();
    [[nodiscard]] static bool IsUdsSupported();

    void Shutdown() const;
    void Close();

    [[nodiscard]] bool IsValid() const;

    [[nodiscard]] static Result Create(AddressFamily addressFamily, Socket& socket);

    [[nodiscard]] static Result TryConnect(std::string_view ipAddress,
                                           uint16_t remotePort,
                                           uint16_t localPort,
                                           uint32_t timeoutInMilliseconds,
                                           std::optional<Socket>& connectedSocket);

    [[nodiscard]] static Result TryConnect(std::string_view name, std::optional<Socket>& connectedSocket);
    [[nodiscard]] Result EnableIpv6Only() const;
    [[nodiscard]] Result Bind(uint16_t port, bool enableRemoteAccess) const;
    [[nodiscard]] Result Bind(std::string_view name);
    [[nodiscard]] Result EnableReuseAddress() const;
    [[nodiscard]] Result EnableNoDelay() const;
    [[nodiscard]] Result Listen() const;
    [[nodiscard]] Result TryAccept(std::optional<Socket>& acceptedSocket) const;
    [[nodiscard]] Result GetLocalPort(uint16_t& localPort) const;
    [[nodiscard]] Result GetRemoteAddress(SocketAddress& remoteAddress) const;
    [[nodiscard]] Result Receive(void* destination, int32_t size, int32_t& receivedSize) const;
    [[nodiscard]] Result Send(const void* source, int32_t size, int32_t& sentSize) const;

private:
    [[nodiscard]] Result BindForIpv4(uint16_t port, bool enableRemoteAccess) const;
    [[nodiscard]] Result BindForIpv6(uint16_t port, bool enableRemoteAccess) const;

    [[nodiscard]] Result GetLocalPortForIpv4(uint16_t& localPort) const;
    [[nodiscard]] Result GetLocalPortForIpv6(uint16_t& localPort) const;

    [[nodiscard]] Result GetRemoteAddressForIpv4(SocketAddress& remoteAddress) const;
    [[nodiscard]] Result GetRemoteAddressForIpv6(SocketAddress& remoteAddress) const;

    SocketHandle _socket = InvalidSocket;
    AddressFamily _addressFamily{};
    std::string _path;
};

}  // namespace DsVeosCoSim
