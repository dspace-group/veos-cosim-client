// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

#include "Result.hpp"

namespace DsVeosCoSim {

[[nodiscard]] Result StartupNetwork();

[[nodiscard]] bool IsIpv4SocketSupported();
[[nodiscard]] bool IsIpv6SocketSupported();

#ifndef _WIN32

[[nodiscard]] bool IsLocalSocketSupported();

#endif

enum class AddressFamily {
#ifndef _WIN32
    Local,
#endif
    Ipv4,
    Ipv6,
};

[[nodiscard]] constexpr std::string_view format_as(AddressFamily addressFamily) noexcept
{
    switch (addressFamily) {
#ifndef _WIN32
    case AddressFamily::Local:
        return "Local";
#endif
    case AddressFamily::Ipv4:
        return "Ipv4";
    case AddressFamily::Ipv6:
        return "Ipv6";
    }

    return "<Invalid AddressFamily>";
}

class SocketHandle final {
  public:
#ifdef _WIN32
    using socket_t = uintptr_t;
    static constexpr socket_t InvalidSocket = UINTPTR_MAX;
#else
    using socket_t = int32_t;
    static constexpr socket_t InvalidSocket = -1;
#endif

    SocketHandle() = default;

    explicit SocketHandle(socket_t sock)
        : _socket(sock)
    {
    }

    ~SocketHandle() noexcept
    {
        Reset();
    }

    SocketHandle(const SocketHandle&) = delete;
    SocketHandle& operator=(const SocketHandle&) = delete;

    SocketHandle(SocketHandle&& other) noexcept
        : _socket(other.Release())
    {
    }

    SocketHandle& operator=(SocketHandle&& other) noexcept
    {
        if (this != &other) {
            Reset(other.Release());
        }

        return *this;
    }

    [[nodiscard]] socket_t Get() const noexcept
    {
        return _socket;
    }

    [[nodiscard]] socket_t Release() noexcept
    {
        return std::exchange(_socket, InvalidSocket);
    }

    void Reset(socket_t newSocket = InvalidSocket);

    [[nodiscard]] bool IsValid() const noexcept
    {
        return _socket != InvalidSocket;
    }

    explicit operator bool() const noexcept
    {
        return IsValid();
    }

  private:
    socket_t _socket = InvalidSocket;
};

class SocketListener;

class SocketClient final {
#ifdef _WIN32
    SocketClient(SocketHandle socketHandle, AddressFamily addressFamily);
#else
    SocketClient(SocketHandle socketHandle, AddressFamily addressFamily, std::string path);
#endif

    friend class SocketListener;

  public:
    SocketClient() = default;
    ~SocketClient() noexcept;

    SocketClient(const SocketClient&) = delete;
    SocketClient& operator=(const SocketClient&) = delete;

    SocketClient(SocketClient&& other) noexcept = default;
    SocketClient& operator=(SocketClient&& other) noexcept = default;

    [[nodiscard]] static Result TryConnect(const std::string& ipAddress,
                                           uint16_t remotePort,
                                           uint16_t localPort,
                                           uint32_t timeoutInMilliseconds,
                                           SocketClient& client);
#ifndef _WIN32
    [[nodiscard]] static Result TryConnect(std::string_view name, SocketClient& client);
#endif

    void Disconnect();

    [[nodiscard]] Result GetRemoteAddress(std::string& remoteAddress) const;

    [[nodiscard]] Result Receive(void* destination, size_t size, size_t& receivedSize) const;
    [[nodiscard]] Result Send(const void* source, size_t size) const;
    [[nodiscard]] Result WaitForData(uint32_t timeoutInMilliseconds) const;

    [[nodiscard]] bool IsConnected() const;

  private:
    SocketHandle _socketHandle;
    AddressFamily _addressFamily{};
#ifndef _WIN32
    std::string _path;
#endif
    bool _isConnected{};
};

class SocketListener final {
#ifdef _WIN32
    SocketListener(SocketHandle socketHandle, AddressFamily addressFamily);
#else
    SocketListener(SocketHandle socketHandle, AddressFamily addressFamily, std::string path);
#endif

  public:
    SocketListener() = default;
    ~SocketListener() noexcept;

    SocketListener(const SocketListener&) = delete;
    SocketListener& operator=(const SocketListener&) = delete;

    SocketListener(SocketListener&& other) noexcept = default;
    SocketListener& operator=(SocketListener&& other) noexcept = default;

    [[nodiscard]] static Result Create(const std::string& ipAddress, uint16_t port, SocketListener& listener);

#ifndef _WIN32
    [[nodiscard]] static Result Create(std::string_view name, SocketListener& listener);
#endif

    void Stop();

    [[nodiscard]] Result TryAccept(SocketClient& client) const;
    [[nodiscard]] Result GetLocalPort(uint16_t& localPort) const;

    [[nodiscard]] bool IsRunning() const;

  private:
    SocketHandle _socketHandle;
    AddressFamily _addressFamily{};
#ifndef _WIN32
    std::string _path;
#endif
    bool _isRunning{};
};

}  // namespace DsVeosCoSim
