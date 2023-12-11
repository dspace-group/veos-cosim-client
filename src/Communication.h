// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <vector>

#include "CoSimTypes.h"
#include "Socket.h"

namespace DsVeosCoSim {

class Channel {
public:
    Channel();
    explicit Channel(Socket socket);
    ~Channel() = default;

    Channel(const Channel&) = delete;
    Channel& operator=(Channel const&) = delete;

    Channel(Channel&&) = default;
    Channel& operator=(Channel&&) = default;

    [[nodiscard]] Result GetRemoteAddress(std::string& ipAddress, uint16_t& port) const;

    void Disconnect();
    void Stop();

    template <typename T>
    [[nodiscard]] Result Write(const T& value) {
        static_assert(std::is_trivially_copyable_v<T>);

        return Write(&value, sizeof(T));
    }

    [[nodiscard]] Result Write(const void* source, size_t size);
    [[nodiscard]] Result EndWrite();

    template <typename T>
    [[nodiscard]] Result Read(T& value) {
        static_assert(std::is_trivially_copyable_v<T>);

        return Read(&value, sizeof(T));
    }

    [[nodiscard]] Result Read(void* destination, size_t size);

private:
    void Reset();
    [[nodiscard]] Result FlushWriteBuffer();
    [[nodiscard]] Result FillReadBuffer();

    Socket _socket;

    int _readBufferReadIndex = 0;
    int _readBufferWriteIndex = 0;
    int _readBufferEndFrameIndex = 0;

    int _writeBufferWriteIndex = 0;

    std::vector<uint8_t> _readBuffer;
    std::vector<uint8_t> _writeBuffer;
};

Result ConnectToServer(std::string_view remoteIpAddress, uint16_t remotePort, uint16_t localPort, Channel& channel);

class Server {
public:
    Server() = default;
    ~Server() = default;

    Server(const Server&) = delete;
    Server& operator=(Server const&) = delete;

    Server(Server&&) = default;
    Server& operator=(Server&&) = default;

    [[nodiscard]] Result Start(uint16_t& port, bool enableRemoteAccess);
    void Stop();

    [[nodiscard]] Result Accept(Channel& channel) const;

private:
    Socket _listenSocket;
    bool _isRunning = false;
};

}  // namespace DsVeosCoSim
