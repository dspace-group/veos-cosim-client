// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <cstdint>
#include <cstring>  // IWYU pragma: keep
#include <memory>
#include <string>
#include <utility>

#include "Channel.hpp"
#include "Result.hpp"
#include "Socket.hpp"

namespace DsVeosCoSim {

namespace {

constexpr int32_t DefaultReadPacketSize = 1024;

class SocketChannelWriter final : public ChannelWriter {
public:
    explicit SocketChannelWriter(SocketClient& client) : _client(client) {
    }

    ~SocketChannelWriter() noexcept override = default;

    SocketChannelWriter(const SocketChannelWriter&) = delete;
    SocketChannelWriter& operator=(const SocketChannelWriter&) = delete;

    SocketChannelWriter(SocketChannelWriter&&) = delete;
    SocketChannelWriter& operator=(SocketChannelWriter&&) = delete;

protected:
    [[nodiscard]] Result Send(const uint8_t* buffer, size_t size) override {
        return _client.Send(buffer, size);
    }

private:
    SocketClient& _client;
};

class SocketChannelReader final : public ChannelReader {
public:
    explicit SocketChannelReader(SocketClient& client) : _client(client) {
        _defaultSizeToRead = DefaultReadPacketSize;
    }

    ~SocketChannelReader() noexcept override = default;

    SocketChannelReader(const SocketChannelReader&) = delete;
    SocketChannelReader& operator=(const SocketChannelReader&) = delete;

    SocketChannelReader(SocketChannelReader&&) = delete;
    SocketChannelReader& operator=(SocketChannelReader&&) = delete;

protected:
    [[nodiscard]] Result Receive(void* destination, size_t size, size_t& receivedSize) override {
        return _client.Receive(destination, size, receivedSize);
    }

private:
    SocketClient& _client;
};

class SocketChannel final : public Channel {
public:
    explicit SocketChannel(SocketClient client) : _client(std::move(client)), _writer(_client), _reader(_client) {
    }

    ~SocketChannel() noexcept override = default;

    SocketChannel(const SocketChannel&) = delete;
    SocketChannel& operator=(const SocketChannel&) = delete;

    SocketChannel(SocketChannel&&) = delete;
    SocketChannel& operator=(SocketChannel&&) = delete;

    [[nodiscard]] Result GetRemoteAddress(std::string& remoteAddress) const override {
        return _client.GetRemoteAddress(remoteAddress);
    }

    void Disconnect() override {
        _client.Disconnect();
    }

    [[nodiscard]] ChannelWriter& GetWriter() override {
        return _writer;
    }

    [[nodiscard]] ChannelReader& GetReader() override {
        return _reader;
    }

private:
    SocketClient _client;

    SocketChannelWriter _writer;
    SocketChannelReader _reader;
};

class TcpChannelServer final : public ChannelServer {
public:
    TcpChannelServer(SocketListener listenerIpv4, SocketListener listenerIpv6, uint16_t port)
        : _listenerIpv4(std::move(listenerIpv4)), _listenerIpv6(std::move(listenerIpv6)), _port(port) {
    }

    ~TcpChannelServer() noexcept override = default;

    TcpChannelServer(const TcpChannelServer&) = delete;
    TcpChannelServer& operator=(const TcpChannelServer&) = delete;

    TcpChannelServer(TcpChannelServer&&) = delete;
    TcpChannelServer& operator=(TcpChannelServer&&) = delete;

    [[nodiscard]] uint16_t GetLocalPort() const override {
        return _port;
    }

    [[nodiscard]] Result TryAccept(std::unique_ptr<Channel>& channel) override {
        if (_listenerIpv4.IsRunning()) {
            SocketClient client{};
            Result result = _listenerIpv4.TryAccept(client);
            if (IsOk(result)) {
                channel = std::make_unique<SocketChannel>(std::move(client));
                return CreateOk();
            }

            if (!IsNotConnected(result)) {
                return result;
            }
        }

        if (_listenerIpv6.IsRunning()) {
            SocketClient client{};
            CheckResult(_listenerIpv6.TryAccept(client));
            channel = std::make_unique<SocketChannel>(std::move(client));
            return CreateOk();
        }

        return CreateNotConnected();
    }

private:
    SocketListener _listenerIpv4;
    SocketListener _listenerIpv6;
    uint16_t _port{};
};

#ifndef _WIN32

class LocalChannelServer final : public ChannelServer {
public:
    explicit LocalChannelServer(SocketListener listener) : _listener(std::move(listener)) {
    }

    ~LocalChannelServer() noexcept override = default;

    LocalChannelServer(const LocalChannelServer&) = delete;
    LocalChannelServer& operator=(const LocalChannelServer&) = delete;

    LocalChannelServer(LocalChannelServer&&) = delete;
    LocalChannelServer& operator=(LocalChannelServer&&) = delete;

    [[nodiscard]] uint16_t GetLocalPort() const override {
        return {};
    }

    [[nodiscard]] Result TryAccept(std::unique_ptr<Channel>& channel) override {
        SocketClient client;
        CheckResult(_listener.TryAccept(client));
        channel = std::make_unique<SocketChannel>(std::move(client));
        return CreateOk();
    }

private:
    SocketListener _listener;
};

#endif

}  // namespace

[[nodiscard]] Result TryConnectToTcpChannel(const std::string& remoteIpAddress,
                                            uint16_t remotePort,
                                            uint16_t localPort,
                                            uint32_t timeoutInMilliseconds,
                                            std::unique_ptr<Channel>& channel) {
    CheckResult(StartupNetwork());

    SocketClient client{};
    CheckResult(SocketClient::TryConnect(remoteIpAddress, remotePort, localPort, timeoutInMilliseconds, client));
    channel = std::make_unique<SocketChannel>(std::move(client));
    return CreateOk();
}

[[nodiscard]] Result CreateTcpChannelServer(uint16_t port, bool enableRemoteAccess, std::unique_ptr<ChannelServer>& server) {
    CheckResult(StartupNetwork());

    SocketListener listenerIpv4;
    if (IsIpv4SocketSupported()) {
        CheckResult(SocketListener::Create(AddressFamily::Ipv4, port, enableRemoteAccess, listenerIpv4));
        CheckResult(listenerIpv4.GetLocalPort(port));
    }

    SocketListener listenerIpv6;
    if (IsIpv6SocketSupported()) {
        CheckResult(SocketListener::Create(AddressFamily::Ipv6, port, enableRemoteAccess, listenerIpv6));
        CheckResult(listenerIpv6.GetLocalPort(port));
    }

    server = std::make_unique<TcpChannelServer>(std::move(listenerIpv4), std::move(listenerIpv6), port);
    return CreateOk();
}

#ifndef _WIN32

[[nodiscard]] Result TryConnectToLocalChannel(const std::string& name, std::unique_ptr<Channel>& channel) {
    CheckResult(StartupNetwork());

    if (!IsLocalSocketSupported()) {
        return CreateOk();
    }

    SocketClient client{};
    CheckResult(SocketClient::TryConnect(name, client));
    channel = std::make_unique<SocketChannel>(std::move(client));
    return CreateOk();
}

[[nodiscard]] Result CreateLocalChannelServer(const std::string& name, std::unique_ptr<ChannelServer>& server) {
    CheckResult(StartupNetwork());

    if (!IsLocalSocketSupported()) {
        return CreateOk();
    }

    SocketListener listener;
    CheckResult(SocketListener::Create(name, listener));
    server = std::make_unique<LocalChannelServer>(std::move(listener));
    return CreateOk();
}

#endif

}  // namespace DsVeosCoSim
