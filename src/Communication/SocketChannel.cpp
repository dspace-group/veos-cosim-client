// Copyright dSPACE GmbH. All rights reserved.

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include "Channel.h"
#include "CoSimHelper.h"
#include "Socket.h"

namespace DsVeosCoSim {

namespace {

constexpr int32_t HeaderSize = 4;
constexpr int32_t BufferSize = 64 * 1024;
constexpr int32_t ReadPacketSize = 1024;

class SocketChannelWriter final : public ChannelWriter {
public:
    explicit SocketChannelWriter(Socket* socket) : _socket(socket), _writeIndex(HeaderSize) {
        _writeBuffer.resize(BufferSize);
    }

    ~SocketChannelWriter() noexcept override = default;

    SocketChannelWriter(const SocketChannelWriter&) = delete;
    SocketChannelWriter& operator=(const SocketChannelWriter&) = delete;

    SocketChannelWriter(SocketChannelWriter&&) = delete;
    SocketChannelWriter& operator=(SocketChannelWriter&&) = delete;

    [[nodiscard]] bool Write(const void* source, size_t size) override {
        const auto* bufferPointer = static_cast<const uint8_t*>(source);

        while (size > 0) {
            const int32_t sizeToCopy = std::min(static_cast<int32_t>(size), BufferSize - _writeIndex);
            if (sizeToCopy == 0) {
                CheckResult(EndWrite());
                continue;
            }

            (void)memcpy(&_writeBuffer[_writeIndex], bufferPointer, sizeToCopy);
            _writeIndex += sizeToCopy;
            bufferPointer += sizeToCopy;
            size -= sizeToCopy;
        }

        return true;
    }

    [[nodiscard]] bool EndWrite() override {
        uint8_t* sourcePtr = _writeBuffer.data();

        // Write header
        (void)memcpy(sourcePtr, &_writeIndex, sizeof _writeIndex);

        while (_writeIndex > 0) {
            int32_t sentSize{};
            CheckResult(_socket->Send(sourcePtr, _writeIndex, sentSize));

            sourcePtr += sentSize;
            _writeIndex -= sentSize;
        }

        _writeIndex = HeaderSize;
        return true;
    }

private:
    Socket* _socket{};

    int32_t _writeIndex{};
    std::vector<uint8_t> _writeBuffer;
};

class SocketChannelReader final : public ChannelReader {
public:
    explicit SocketChannelReader(Socket* socket) : _socket(socket), _readIndex(HeaderSize) {
        _readBuffer.resize(BufferSize);
    }

    ~SocketChannelReader() noexcept override = default;

    SocketChannelReader(const SocketChannelReader&) = delete;
    SocketChannelReader& operator=(const SocketChannelReader&) = delete;

    SocketChannelReader(SocketChannelReader&&) = delete;
    SocketChannelReader& operator=(SocketChannelReader&&) = delete;

    [[nodiscard]] bool Read(void* destination, size_t size) override {
        auto* bufferPointer = static_cast<uint8_t*>(destination);

        while (size > 0) {
            const int32_t sizeToCopy = std::min(static_cast<int32_t>(size), _endFrameIndex - _readIndex);
            if (sizeToCopy <= 0) {
                CheckResult(BeginRead());
                continue;
            }

            (void)memcpy(bufferPointer, &_readBuffer[_readIndex], sizeToCopy);
            _readIndex += sizeToCopy;
            bufferPointer += sizeToCopy;
            size -= sizeToCopy;
        }

        return true;
    }

private:
    [[nodiscard]] bool BeginRead() {
        _readIndex = HeaderSize;
        int32_t sizeToRead = ReadPacketSize;
        bool readHeader = true;

        // Did we read more than one frame the last time?
        if (_writeIndex > _endFrameIndex) {
            const int32_t bytesToMove = _writeIndex - _endFrameIndex;
            (void)memcpy(_readBuffer.data(), &_readBuffer[_endFrameIndex], bytesToMove);

            _writeIndex -= _endFrameIndex;

            // Did we read at least HeaderSize bytes more?
            if (bytesToMove >= HeaderSize) {
                readHeader = false;
                (void)memcpy(&_endFrameIndex, _readBuffer.data(), HeaderSize);

                // Did we read at least an entire second frame?
                if (_writeIndex >= _endFrameIndex) {
                    return true;
                }

                sizeToRead = _endFrameIndex - _writeIndex;
            }
        } else {
            _writeIndex = 0;
        }

        while (sizeToRead > 0) {
            int32_t receivedSize{};
            CheckResult(_socket->Receive(&_readBuffer[_writeIndex], sizeToRead, receivedSize));

            sizeToRead -= receivedSize;
            _writeIndex += receivedSize;

            if (readHeader && (_writeIndex >= HeaderSize)) {
                readHeader = false;
                (void)memcpy(&_endFrameIndex, _readBuffer.data(), HeaderSize);

                if (_endFrameIndex > BufferSize) {
                    throw std::runtime_error("Protocol error. The buffer size is too small.");
                }

                sizeToRead = _endFrameIndex - _writeIndex;
            }
        }

        return true;
    }

    Socket* _socket{};

    int32_t _readIndex{};
    int32_t _writeIndex{};
    int32_t _endFrameIndex{};
    std::vector<uint8_t> _readBuffer;
};

class SocketChannel final : public Channel {
public:
    explicit SocketChannel(Socket socket) : _socket(std::move(socket)), _writer(&_socket), _reader(&_socket) {
    }

    ~SocketChannel() noexcept override = default;

    SocketChannel(const SocketChannel&) = delete;
    SocketChannel& operator=(const SocketChannel&) = delete;

    SocketChannel(SocketChannel&&) = delete;
    SocketChannel& operator=(SocketChannel&&) = delete;

    [[nodiscard]] std::string GetRemoteAddress() const override {
        const SocketAddress socketAddress = _socket.GetRemoteAddress();
        std::string remoteAddress = socketAddress.ipAddress;
        remoteAddress.append(":");
        remoteAddress.append(std::to_string(socketAddress.port));
        return remoteAddress;
    }

    void Disconnect() override {
        _socket.Shutdown();
    }

    [[nodiscard]] ChannelWriter& GetWriter() override {
        return _writer;
    }

    [[nodiscard]] ChannelReader& GetReader() override {
        return _reader;
    }

private:
    Socket _socket;

    SocketChannelWriter _writer;
    SocketChannelReader _reader;
};

class TcpChannelServer final : public ChannelServer {
public:
    TcpChannelServer(const uint16_t port, const bool enableRemoteAccess) : _port(port) {
        if (Socket::IsIpv4Supported()) {
            _listenSocketIpv4 = Socket(AddressFamily::Ipv4);
            _listenSocketIpv4.EnableReuseAddress();
            _listenSocketIpv4.Bind(_port, enableRemoteAccess);
            _port = _listenSocketIpv4.GetLocalPort();
            _listenSocketIpv4.Listen();
        }

        if (Socket::IsIpv6Supported()) {
            _listenSocketIpv6 = Socket(AddressFamily::Ipv6);
            _listenSocketIpv6.EnableIpv6Only();
            _listenSocketIpv6.EnableReuseAddress();
            _listenSocketIpv6.Bind(_port, enableRemoteAccess);
            _port = _listenSocketIpv6.GetLocalPort();
            _listenSocketIpv6.Listen();
        }
    }

    ~TcpChannelServer() noexcept override = default;

    TcpChannelServer(const TcpChannelServer&) = delete;
    TcpChannelServer& operator=(const TcpChannelServer&) = delete;

    TcpChannelServer(TcpChannelServer&&) = delete;
    TcpChannelServer& operator=(TcpChannelServer&&) = delete;

    [[nodiscard]] uint16_t GetLocalPort() const override {
        return _port;
    }

    [[nodiscard]] std::unique_ptr<Channel> TryAccept() override {
        return TryAccept(0);
    }

    [[nodiscard]] std::unique_ptr<Channel> TryAccept(uint32_t timeoutInMilliseconds) override {
        do {
            if (_listenSocketIpv4.IsValid()) {
                std::optional<Socket> socket = _listenSocketIpv4.TryAccept();
                if (socket) {
                    socket->EnableNoDelay();
                    return std::make_unique<SocketChannel>(std::move(*socket));
                }
            }

            if (_listenSocketIpv6.IsValid()) {
                std::optional<Socket> socket = _listenSocketIpv6.TryAccept();
                if (socket) {
                    socket->EnableNoDelay();
                    return std::make_unique<SocketChannel>(std::move(*socket));
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));

            if (timeoutInMilliseconds > 0) {
                timeoutInMilliseconds--;
            }
        } while (timeoutInMilliseconds > 0);

        return {};
    }

private:
    uint16_t _port{};
    Socket _listenSocketIpv4;
    Socket _listenSocketIpv6;
};

class UdsChannelServer final : public ChannelServer {
public:
    explicit UdsChannelServer(const std::string& name) {
        _listenSocket = Socket(AddressFamily::Uds);
        _listenSocket.Bind(name);
        _listenSocket.Listen();
    }

    ~UdsChannelServer() noexcept override = default;

    UdsChannelServer(const UdsChannelServer&) = delete;
    UdsChannelServer& operator=(const UdsChannelServer&) = delete;

    UdsChannelServer(UdsChannelServer&&) = delete;
    UdsChannelServer& operator=(UdsChannelServer&&) = delete;

    [[nodiscard]] uint16_t GetLocalPort() const override {
        return {};
    }

    [[nodiscard]] std::unique_ptr<Channel> TryAccept() override {
        return TryAccept(0);
    }

    [[nodiscard]] std::unique_ptr<Channel> TryAccept(const uint32_t timeoutInMilliseconds) override {
        std::optional<Socket> socket = _listenSocket.TryAccept(timeoutInMilliseconds);
        if (socket) {
            return std::make_unique<SocketChannel>(std::move(*socket));
        }

        return {};
    }

private:
    Socket _listenSocket;
};

}  // namespace

[[nodiscard]] std::unique_ptr<Channel> TryConnectToTcpChannel(const std::string_view remoteIpAddress,
                                                              const uint16_t remotePort,
                                                              const uint16_t localPort,
                                                              const uint32_t timeoutInMilliseconds) {
    StartupNetwork();

    std::optional<Socket> connectedSocket =
        Socket::TryConnect(remoteIpAddress, remotePort, localPort, timeoutInMilliseconds);
    if (connectedSocket) {
        connectedSocket->EnableNoDelay();
        return std::make_unique<SocketChannel>(std::move(*connectedSocket));
    }

    return {};
}

[[nodiscard]] std::unique_ptr<Channel> TryConnectToUdsChannel(const std::string& name) {
    StartupNetwork();

    if (!Socket::IsUdsSupported()) {
        return {};
    }

    std::optional<Socket> connectedSocket = Socket::TryConnect(name);
    if (connectedSocket) {
        return std::make_unique<SocketChannel>(std::move(*connectedSocket));
    }

    return {};
}

[[nodiscard]] std::unique_ptr<ChannelServer> CreateTcpChannelServer(const uint16_t port,
                                                                    const bool enableRemoteAccess) {
    StartupNetwork();

    return std::make_unique<TcpChannelServer>(port, enableRemoteAccess);
}

[[nodiscard]] std::unique_ptr<ChannelServer> CreateUdsChannelServer(const std::string& name) {
    StartupNetwork();

    if (!Socket::IsUdsSupported()) {
        return {};
    }

    return std::make_unique<UdsChannelServer>(name);
}

}  // namespace DsVeosCoSim
