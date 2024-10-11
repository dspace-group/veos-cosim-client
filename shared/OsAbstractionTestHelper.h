// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <string>
#include <string_view>

class InternetAddress final {
public:
    InternetAddress(std::string_view ipAddress, uint16_t port);
    ~InternetAddress() = default;

    InternetAddress(const InternetAddress&) = delete;
    InternetAddress& operator=(InternetAddress const&) = delete;

    InternetAddress(InternetAddress&&) = delete;
    InternetAddress& operator=(InternetAddress&&) = delete;

private:
    uint8_t _address[16]{};
};

class UdpSocket final {
public:
    UdpSocket();
    ~UdpSocket();

    UdpSocket(const UdpSocket&) = delete;
    UdpSocket& operator=(UdpSocket const&) = delete;

    UdpSocket(UdpSocket&&) = delete;
    UdpSocket& operator=(UdpSocket&&) = delete;

    void Bind(std::string_view ipAddress, uint16_t port) const;
    void Connect(std::string_view ipAddress, uint16_t port) const;
    void SetNoDelay(bool value) const;
    void SetReuseAddress(bool value) const;
    void Listen() const;

    [[nodiscard]] bool SendTo(const void* source, uint32_t size, const InternetAddress& address) const;
    [[nodiscard]] bool ReceiveFrom(void* destination, uint32_t size, InternetAddress& address) const;

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
    explicit Pipe(const std::string& name);
    ~Pipe();

    Pipe(const Pipe&) = delete;
    Pipe& operator=(Pipe const&) = delete;

    Pipe(Pipe&&) = delete;
    Pipe& operator=(Pipe&&) = delete;

    void Accept();
    void Connect();

    [[nodiscard]] bool Write(const void* source, uint32_t size) const;
    [[nodiscard]] bool Read(void* destination, uint32_t size) const;

private:
#ifdef _WIN32
    using pipe_t = void*;
#else
    using pipe_t = int32_t;
#endif

#ifndef _WIN32
    [[nodiscard]] static pipe_t CreatePipe(std::string_view name);
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
