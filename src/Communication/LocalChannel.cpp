// Copyright dSPACE SE & Co. KG. All rights reserved.

#ifdef _WIN32

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <utility>

#include "Channel.hpp"
#include "OsUtilities.hpp"

namespace DsVeosCoSim {

namespace {

class LocalChannelWriter final : public ChannelWriter {
public:
    LocalChannelWriter(ShmPipeClient& client) : _client(client) {
    }

    ~LocalChannelWriter() noexcept override = default;

    LocalChannelWriter(const LocalChannelWriter&) = delete;
    LocalChannelWriter& operator=(const LocalChannelWriter&) = delete;

    LocalChannelWriter(LocalChannelWriter&&) = delete;
    LocalChannelWriter& operator=(LocalChannelWriter&&) = delete;

protected:
    [[nodiscard]] Result Send(const uint8_t* buffer, size_t size) override {
        return _client.Send(buffer, size);
    }

private:
    ShmPipeClient& _client;
};

class LocalChannelReader final : public ChannelReader {
public:
    LocalChannelReader(ShmPipeClient& client) : _client(client) {
        _defaultSizeToRead = ShmPipePart::PipeBufferSize;
    }

    ~LocalChannelReader() noexcept override = default;

    LocalChannelReader(const LocalChannelReader&) = delete;
    LocalChannelReader& operator=(const LocalChannelReader&) = delete;

    LocalChannelReader(LocalChannelReader&&) = delete;
    LocalChannelReader& operator=(LocalChannelReader&&) = delete;

protected:
    [[nodiscard]] Result Receive(void* destination, size_t size, size_t& receivedSize) override {
        return _client.Receive(destination, size, receivedSize);
    }

private:
    ShmPipeClient& _client;
};

class LocalChannel final : public Channel {
public:
    explicit LocalChannel(ShmPipeClient client) : _client(std::move(client)), _writer(_client), _reader(_client) {
    }

    ~LocalChannel() noexcept override = default;

    LocalChannel(const LocalChannel&) = delete;
    LocalChannel& operator=(const LocalChannel&) = delete;

    LocalChannel(LocalChannel&&) = delete;
    LocalChannel& operator=(LocalChannel&&) = delete;

    [[nodiscard]] Result GetRemoteAddress(std::string& remoteAddress) const override {
        remoteAddress.clear();
        return CreateOk();
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
    ShmPipeClient _client;

    LocalChannelWriter _writer;
    LocalChannelReader _reader;
};

class LocalChannelServer final : public ChannelServer {
public:
    explicit LocalChannelServer(ShmPipeListener listener) : _listener(std::move(listener)) {
    }

    ~LocalChannelServer() noexcept override = default;

    LocalChannelServer(const LocalChannelServer&) = delete;
    LocalChannelServer& operator=(const LocalChannelServer&) = delete;

    LocalChannelServer(LocalChannelServer&&) = delete;
    LocalChannelServer& operator=(LocalChannelServer&&) = delete;

    [[nodiscard]] uint16_t GetLocalPort() const override {
        return 0;
    }

    [[nodiscard]] Result TryAccept(std::unique_ptr<Channel>& channel) override {
        ShmPipeClient client;
        CheckResult(_listener.TryAccept(client));
        channel = std::make_unique<LocalChannel>(std::move(client));
        return CreateOk();
    }

private:
    ShmPipeListener _listener;
};

}  // namespace

[[nodiscard]] Result TryConnectToLocalChannel(const std::string& name, std::unique_ptr<Channel>& channel) {
    ShmPipeClient client{};
    CheckResult(ShmPipeClient::TryConnect(name, client));
    channel = std::make_unique<LocalChannel>(std::move(client));
    return CreateOk();
}

[[nodiscard]] Result CreateLocalChannelServer(const std::string& name, std::unique_ptr<ChannelServer>& server) {
    ShmPipeListener listener;
    CheckResult(ShmPipeListener::Create(name, listener));
    server = std::make_unique<LocalChannelServer>(std::move(listener));
    return CreateOk();
}

}  // namespace DsVeosCoSim

#endif
