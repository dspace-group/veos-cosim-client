// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "Result.hpp"
#include "Socket.hpp"

#ifdef _WIN32

#include "OsUtilities.hpp"

#endif

// #include "CoSimTypes.hpp"

namespace DsVeosCoSim {

class InternetAddress final {
    explicit InternetAddress(std::array<uint8_t, 16> address) : _address(address) {
    }

public:
    InternetAddress() = default;
    ~InternetAddress() noexcept = default;

    InternetAddress(const InternetAddress&) = delete;
    InternetAddress& operator=(InternetAddress const&) = delete;

    InternetAddress(InternetAddress&&) noexcept = default;
    InternetAddress& operator=(InternetAddress&&) noexcept = default;

    [[nodiscard]] static Result Create(const std::string& ipAddress, uint16_t port, InternetAddress& internetAddress);

private:
    std::array<uint8_t, 16> _address{};
};

class UdpSocket final {
    explicit UdpSocket(SocketHandle socketHandle) : _socketHandle(std::move(socketHandle)) {
    }

public:
    UdpSocket() = default;
    ~UdpSocket() noexcept;

    UdpSocket(const UdpSocket&) = delete;
    UdpSocket& operator=(UdpSocket const&) = delete;

    UdpSocket(UdpSocket&&) noexcept = default;
    UdpSocket& operator=(UdpSocket&&) noexcept = default;

    [[nodiscard]] static Result CreateClient(UdpSocket& udpSocket);
    [[nodiscard]] static Result CreateServer(const std::string& ipAddress, uint16_t port, UdpSocket& udpSocket);

    [[nodiscard]] Result SendTo(const void* source, uint32_t size, const InternetAddress& address) const;
    [[nodiscard]] Result ReceiveFrom(void* destination, uint32_t size, InternetAddress& address) const;

private:
    SocketHandle _socketHandle;
};

class PipeClient final {
public:
#ifdef _WIN32
    using pipe_t = Handle;
#else
    using pipe_t = int32_t;
#endif

    PipeClient() = default;
    ~PipeClient() noexcept;

    PipeClient(const PipeClient&) = delete;
    PipeClient& operator=(PipeClient const&) = delete;

    PipeClient(PipeClient&&) noexcept = default;
    PipeClient& operator=(PipeClient&&) noexcept = default;

    [[nodiscard]] static Result Connect(const std::string& name, PipeClient& client);
    [[nodiscard]] static Result Accept(const std::string& name, PipeClient& client);

    [[nodiscard]] Result Write(const void* source, uint32_t size) const;
    [[nodiscard]] Result Read(void* destination, uint32_t size) const;

private:
#ifdef _WIN32
    pipe_t _pipe;
#else
    pipe_t _writePipe{};
    pipe_t _readPipe{};
#endif
};

}  // namespace DsVeosCoSim
