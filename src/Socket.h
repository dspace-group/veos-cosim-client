// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include "CoSimTypes.h"

namespace DsVeosCoSim {

[[nodiscard]] Result StartupNetwork();

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

    [[nodiscard]] Result Create();
    void Close();

    [[nodiscard]] bool IsValid() const;

    [[nodiscard]] Result Connect(std::string_view ipAddress, uint16_t port) const;
    [[nodiscard]] Result Bind(std::string_view ipAddress, uint16_t port) const;
    [[nodiscard]] Result EnableReuseAddress() const;
    [[nodiscard]] Result EnableNoDelay() const;
    [[nodiscard]] Result Listen() const;
    [[nodiscard]] Result Accept(Socket& acceptedSocket) const;
    [[nodiscard]] Result GetLocalPort(uint16_t& port) const;
    [[nodiscard]] Result GetRemoteAddress(std::string& ipAddress, uint16_t& port) const;
    [[nodiscard]] Result Receive(void* destination, int size, int& receivedSize) const;
    [[nodiscard]] Result Send(const void* source, int size, int& sentSize) const;

private:
    socket_t _socket = InvalidSocket;
};

}  // namespace DsVeosCoSim
