// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "DsVeosCoSim/CoSimTypes.h"

class InternetAddress final {
public:
    InternetAddress() = default;
    ~InternetAddress() = default;

    InternetAddress(const InternetAddress&) = delete;
    InternetAddress& operator=(InternetAddress const&) = delete;

    InternetAddress(InternetAddress&&) = delete;
    InternetAddress& operator=(InternetAddress&&) = delete;

    [[nodiscard]] DsVeosCoSim::Result Initialize(const std::string& ipAddress, uint16_t port);

private:
    std::array<uint8_t, 16> _address{};
};

class UdpSocket final {
public:
    UdpSocket() = default;
    ~UdpSocket();

    UdpSocket(const UdpSocket&) = delete;
    UdpSocket& operator=(UdpSocket const&) = delete;

    UdpSocket(UdpSocket&&) = delete;
    UdpSocket& operator=(UdpSocket&&) = delete;

    [[nodiscard]] DsVeosCoSim::Result Initialize();

    [[nodiscard]] DsVeosCoSim::Result Bind(const std::string& ipAddress, uint16_t port) const;

    [[nodiscard]] DsVeosCoSim::Result SendTo(const void* source, uint32_t size, const InternetAddress& address) const;
    [[nodiscard]] DsVeosCoSim::Result ReceiveFrom(void* destination, uint32_t size, InternetAddress& address) const;

private:
#ifdef _WIN32
    using socket_t = uintptr_t;
    static constexpr socket_t InvalidSocket = UINTPTR_MAX;
#else
    using socket_t = int32_t;
    static constexpr socket_t InvalidSocket = -1;
#endif

    socket_t _socket = InvalidSocket;
    std::string _path;
};

class Pipe final {
public:
    Pipe() = default;
    ~Pipe();

    Pipe(const Pipe&) = delete;
    Pipe& operator=(Pipe const&) = delete;

    Pipe(Pipe&&) = delete;
    Pipe& operator=(Pipe&&) = delete;

    [[nodiscard]] DsVeosCoSim::Result Initialize(const std::string& name);

    [[nodiscard]] DsVeosCoSim::Result Accept();
    [[nodiscard]] DsVeosCoSim::Result Connect();

    [[nodiscard]] DsVeosCoSim::Result Write(const void* source, uint32_t size) const;
    [[nodiscard]] DsVeosCoSim::Result Read(void* destination, uint32_t size) const;

private:
#ifdef _WIN32
    using pipe_t = void*;
#else
    using pipe_t = int32_t;
#endif

#ifndef _WIN32
    [[nodiscard]] DsVeosCoSim::Result CreatePipe(const std::string& name, pipe_t& pipe);
#endif

#ifdef _WIN32
    std::string _name;
    pipe_t _pipe = nullptr;
#else
    pipe_t _pipe1 = 0;
    pipe_t _pipe2 = 0;

    pipe_t _writePipe = 0;
    pipe_t _readPipe = 0;
#endif
};
