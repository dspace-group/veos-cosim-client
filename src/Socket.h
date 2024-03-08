// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include "CoSimTypes.h"

namespace DsVeosCoSim {

[[nodiscard]] Result StartupNetwork();

enum class AddressFamily {
    Ipv4 = 1,
    Ipv6
};

class Socket {
#ifdef _WIN32
    using socket_t = uintptr_t;
    static constexpr socket_t InvalidSocket = UINTPTR_MAX;
#else
    using socket_t = int;
    static constexpr socket_t InvalidSocket = -1;
#endif

public:
    Socket() = default;
    ~Socket() noexcept;

    Socket(const Socket&) = delete;
    Socket& operator=(Socket const&) = delete;

    Socket(Socket&&) noexcept;
    Socket& operator=(Socket&&) noexcept;

    static bool IsIpv4Supported();
    static bool IsIpv6Supported();

    void Close();

    [[nodiscard]] bool IsValid() const;

    [[nodiscard]] Result Create(AddressFamily addressFamily);
    [[nodiscard]] Result EnableIpv6Only() const;
    [[nodiscard]] Result Connect(std::string_view ipAddress, uint16_t remotePort, uint16_t localPort);
    [[nodiscard]] Result Bind(uint16_t port, bool enableRemoteAccess) const;
    [[nodiscard]] Result EnableReuseAddress() const;
    [[nodiscard]] Result EnableNoDelay() const;
    [[nodiscard]] Result Listen() const;
    [[nodiscard]] Result Accept(Socket& acceptedSocket) const;
    [[nodiscard]] Result GetLocalPort(uint16_t& localPort) const;
    [[nodiscard]] Result GetRemoteAddress(std::string& remoteIpAddress, uint16_t& remotePort) const;
    [[nodiscard]] Result Receive(void* destination, int size, int& receivedSize) const;
    [[nodiscard]] Result Send(const void* source, int size, int& sentSize) const;

private:
    [[nodiscard]] Result BindForIpv4(uint16_t port, bool enableRemoteAccess) const;
    [[nodiscard]] Result BindForIpv6(uint16_t port, bool enableRemoteAccess) const;

    [[nodiscard]] Result GetLocalPortForIpv4(uint16_t& localPort) const;
    [[nodiscard]] Result GetLocalPortForIpv6(uint16_t& localPort) const;

    [[nodiscard]] Result GetRemoteAddressForIpv4(std::string& remoteIpAddress, uint16_t& remotePort) const;
    [[nodiscard]] Result GetRemoteAddressForIpv6(std::string& remoteIpAddress, uint16_t& remotePort) const;

    socket_t _socket = InvalidSocket;
    int _addressFamily{};
};

}  // namespace DsVeosCoSim
