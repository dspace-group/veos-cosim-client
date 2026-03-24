// Copyright dSPACE SE & Co. KG. All rights reserved.

#ifdef _WIN32

#include <cstddef>  // IWYU pragma: keep
#include <cstdint>
#include <cstring>  // IWYU pragma: keep
#include <memory>
#include <string>
#include <utility>

#include "Channel.hpp"
#include "OsUtilities.hpp"
#include "Result.hpp"

namespace DsVeosCoSim {

namespace {

class LocalChannelWriter final : public ChannelWriter {
public:
    explicit LocalChannelWriter(ShmPipeClient& client) : _client(client) {
        // The local channel does not include the length of the frame
        _writeIndex = 0;
    }

    ~LocalChannelWriter() noexcept override = default;

    LocalChannelWriter(const LocalChannelWriter&) = delete;
    LocalChannelWriter& operator=(const LocalChannelWriter&) = delete;

    LocalChannelWriter(LocalChannelWriter&&) = delete;
    LocalChannelWriter& operator=(LocalChannelWriter&&) = delete;

    [[nodiscard]] Result EndWrite() override {
        CheckResult(Send(_writeBuffer.data(), static_cast<size_t>(_writeIndex)));

        _writeIndex = 0;
        return CreateOk();
    }

protected:
    [[nodiscard]] Result Send(const uint8_t* buffer, size_t size) override {
        return _client.Send(buffer, size);
    }

private:
    ShmPipeClient& _client;
};

class LocalChannelReader final : public ChannelReader {
public:
    explicit LocalChannelReader(ShmPipeClient& client) : _client(client) {
        _defaultSizeToRead = ShmPipePart::PipeBufferSize;
    }

    ~LocalChannelReader() noexcept override = default;

    LocalChannelReader(const LocalChannelReader&) = delete;
    LocalChannelReader& operator=(const LocalChannelReader&) = delete;

    LocalChannelReader(LocalChannelReader&&) = delete;
    LocalChannelReader& operator=(LocalChannelReader&&) = delete;

    void EndRead() const override {
        // The local channel does not include the length of the frame
    }

protected:
    [[nodiscard]] Result Receive(void* destination, size_t size, size_t& receivedSize) override {
        return _client.Receive(destination, size, receivedSize);
    }

    [[nodiscard]] Result BeginRead() override {
        // The new local channel has a write index and an end of frame index.
        // The legacy local channel only has a write index. But since the read functions of
        // the channel only operate on the end of frame index, we handle the end of frame index
        // like the write index
        int32_t& writeIndex = _endFrameIndex;

        int32_t unreadSize = writeIndex - _readIndex;
        if (unreadSize > 0) {
            // Move remaining bytes to the beginning
            memmove(_readBuffer.data(), &_readBuffer[static_cast<size_t>(_readIndex)], static_cast<size_t>(unreadSize));
        }

        writeIndex -= _readIndex;
        _readIndex = 0;

        auto maxSizeToRead = static_cast<uint32_t>(BufferSize - unreadSize);

        size_t receivedSize{};
        CheckResult(Receive(&_readBuffer[static_cast<size_t>(writeIndex)], maxSizeToRead, receivedSize));

        writeIndex += static_cast<int32_t>(receivedSize);
        return CreateOk();
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
