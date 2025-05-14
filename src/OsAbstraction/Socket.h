// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace DsVeosCoSim {

void StartupNetwork();

enum class AddressFamily {
    Uds = 1,
    Ipv4 = 2,
    Ipv6 = 23
};

[[nodiscard]] std::string_view ToString(AddressFamily addressFamily) noexcept;

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
    Socket() noexcept = default;
    explicit Socket(AddressFamily addressFamily);

private:
    Socket(SocketHandle socket, AddressFamily addressFamily, const std::string& path);

public:
    ~Socket() noexcept;

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;

    [[nodiscard]] static bool IsIpv4Supported();
    [[nodiscard]] static bool IsIpv6Supported();
    [[nodiscard]] static bool IsUdsSupported();

    void Shutdown() const noexcept;
    void Close() noexcept;

    [[nodiscard]] bool IsValid() const;

    [[nodiscard]] static std::optional<Socket> TryConnect(std::string_view ipAddress,
                                                          uint16_t remotePort,
                                                          uint16_t localPort,
                                                          uint32_t timeoutInMilliseconds);

    [[nodiscard]] static std::optional<Socket> TryConnect(const std::string& name);
    void EnableIpv6Only() const;
    void Bind(uint16_t port, bool enableRemoteAccess) const;
    void Bind(const std::string& name);
    void EnableReuseAddress() const;
    void EnableNoDelay() const;
    void Listen() const;
    [[nodiscard]] std::optional<Socket> TryAccept(uint32_t timeoutInMilliseconds) const;
    [[nodiscard]] uint16_t GetLocalPort() const;
    [[nodiscard]] SocketAddress GetRemoteAddress() const;
    [[nodiscard]] bool Receive(void* destination, int32_t size, int32_t& receivedSize) const;
    [[nodiscard]] bool Send(const void* source, int32_t size, int32_t& sentSize) const;

private:
    void BindForIpv4(uint16_t port, bool enableRemoteAccess) const;
    void BindForIpv6(uint16_t port, bool enableRemoteAccess) const;

    [[nodiscard]] uint16_t GetLocalPortForIpv4() const;
    [[nodiscard]] uint16_t GetLocalPortForIpv6() const;

    [[nodiscard]] SocketAddress GetRemoteAddressForIpv4() const;
    [[nodiscard]] SocketAddress GetRemoteAddressForIpv6() const;

    SocketHandle _socket = InvalidSocket;
    AddressFamily _addressFamily{};
    std::string _path;
};

}  // namespace DsVeosCoSim
