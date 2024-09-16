// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <vector>

#include "Channel.h"
#include "Socket.h"

namespace DsVeosCoSim {

class SocketChannelWriter final : public ChannelWriter {
public:
    explicit SocketChannelWriter(Socket* socket);
    ~SocketChannelWriter() noexcept override = default;

    SocketChannelWriter(const SocketChannelWriter&) = delete;
    SocketChannelWriter& operator=(const SocketChannelWriter&) = delete;

    SocketChannelWriter(SocketChannelWriter&&) noexcept;
    SocketChannelWriter& operator=(SocketChannelWriter&&) noexcept;

    [[nodiscard]] bool Write(const void* source, size_t size) override;

    [[nodiscard]] bool EndWrite() override;

private:
    Socket* _socket{};

    int32_t _writeIndex{};
    std::vector<uint8_t> _writeBuffer;
};

class SocketChannelReader final : public ChannelReader {
public:
    explicit SocketChannelReader(Socket* socket);
    ~SocketChannelReader() noexcept override = default;

    SocketChannelReader(const SocketChannelReader&) = delete;
    SocketChannelReader& operator=(const SocketChannelReader&) = delete;

    SocketChannelReader(SocketChannelReader&&) noexcept;
    SocketChannelReader& operator=(SocketChannelReader&&) noexcept;

    [[nodiscard]] bool Read(void* destination, size_t size) override;

private:
    [[nodiscard]] bool BeginRead();

    Socket* _socket{};

    int32_t _readIndex{};
    int32_t _writeIndex{};
    int32_t _endFrameIndex{};
    std::vector<uint8_t> _readBuffer;
};

class SocketChannel final : public Channel {
public:
    explicit SocketChannel(Socket socket);
    ~SocketChannel() noexcept override = default;

    SocketChannel(const SocketChannel&) = delete;
    SocketChannel& operator=(const SocketChannel&) = delete;

    SocketChannel(SocketChannel&&) noexcept;
    SocketChannel& operator=(SocketChannel&&) noexcept;

    [[nodiscard]] SocketAddress GetRemoteAddress() const;

    void Disconnect() override;

    [[nodiscard]] ChannelWriter& GetWriter() override;
    [[nodiscard]] ChannelReader& GetReader() override;

private:
    Socket _socket;

    SocketChannelWriter _writer;
    SocketChannelReader _reader;
};

[[nodiscard]] std::optional<SocketChannel> TryConnectToTcpChannel(std::string_view remoteIpAddress,
                                                                  uint16_t remotePort,
                                                                  uint16_t localPort,
                                                                  uint32_t timeoutInMilliseconds);

class TcpChannelServer final {
public:
    TcpChannelServer(uint16_t port, bool enableRemoteAccess);
    ~TcpChannelServer() noexcept = default;

    TcpChannelServer(const TcpChannelServer&) = delete;
    TcpChannelServer& operator=(TcpChannelServer const&) = delete;

    TcpChannelServer(TcpChannelServer&&) = delete;
    TcpChannelServer& operator=(TcpChannelServer&&) = delete;

    [[nodiscard]] uint16_t GetLocalPort() const;

    [[nodiscard]] std::optional<SocketChannel> TryAccept(uint32_t timeoutInMilliseconds = 0) const;

private:
    uint16_t _port{};
    Socket _listenSocketIpv4;
    Socket _listenSocketIpv6;
};

[[nodiscard]] std::optional<SocketChannel> TryConnectToUdsChannel(const std::string& name);

class UdsChannelServer final {
public:
    explicit UdsChannelServer(const std::string& name);
    ~UdsChannelServer() noexcept = default;

    UdsChannelServer(const UdsChannelServer&) = delete;
    UdsChannelServer& operator=(UdsChannelServer const&) = delete;

    UdsChannelServer(UdsChannelServer&&) = delete;
    UdsChannelServer& operator=(UdsChannelServer&&) = delete;

    [[nodiscard]] std::optional<SocketChannel> TryAccept(uint32_t timeoutInMilliseconds = 0) const;

private:
    Socket _listenSocket;
};

}  // namespace DsVeosCoSim
