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

[[nodiscard]] inline std::string ToString(AddressFamily addressFamily) {
    switch (addressFamily) {
        case DsVeosCoSim::AddressFamily::Ipv4:
            return "Ipv4";
        case DsVeosCoSim::AddressFamily::Ipv6:
            return "Ipv6";
        case DsVeosCoSim::AddressFamily::Uds:
            return "Uds";
    }

    return "<Invalid AddressFamily>";
}

#ifdef _WIN32
using socket_t = uintptr_t;
constexpr socket_t InvalidSocket = UINTPTR_MAX;
#else
using socket_t = int32_t;
constexpr socket_t InvalidSocket = -1;
#endif

struct SocketAddress {
    std::string ipAddress;
    uint16_t port{};
};

class Socket {
public:
    Socket() = default;
    explicit Socket(AddressFamily addressFamily);

private:
    Socket(socket_t socket, AddressFamily addressFamily);

public:
    ~Socket() noexcept;

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    Socket(Socket&&) noexcept;
    Socket& operator=(Socket&&) noexcept;

    [[nodiscard]] static bool IsIpv4Supported();
    [[nodiscard]] static bool IsIpv6Supported();
    [[nodiscard]] static bool IsUdsSupported();

    void Shutdown() const;
    void Close();

    [[nodiscard]] bool IsValid() const;

    [[nodiscard]] static std::optional<Socket> TryConnect(std::string_view ipAddress,
                                                          uint16_t remotePort,
                                                          uint16_t localPort,
                                                          uint32_t timeoutInMilliseconds);

    [[nodiscard]] bool TryConnect(const std::string& name) const;
    void EnableIpv6Only() const;
    void Bind(uint16_t port, bool enableRemoteAccess) const;
    void Bind(const std::string& name);
    void EnableReuseAddress() const;
    void EnableNoDelay() const;
    void Listen() const;
    [[nodiscard]] std::optional<Socket> TryAccept(uint32_t timeoutInMilliseconds = 0) const;
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

    void EnsureIsValid() const;

    socket_t _socket = InvalidSocket;
    AddressFamily _addressFamily{};
    std::string _path;
};

}  // namespace DsVeosCoSim
