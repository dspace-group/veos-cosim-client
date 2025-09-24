// Copyright dSPACE GmbH. All rights reserved.

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>  // IWYU pragma: keep
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include "Channel.h"
#include "CoSimHelper.h"
#include "DsVeosCoSim/CoSimTypes.h"
#include "Socket.h"

namespace DsVeosCoSim {

namespace {

constexpr int32_t HeaderSize = 4;
constexpr int32_t BufferSize = 65536;
constexpr int32_t ReadPacketSize = 1024;

class SocketChannelWriter final : public ChannelWriter {
public:
    explicit SocketChannelWriter(Socket& socket) : _socket(socket) {
    }

    ~SocketChannelWriter() override = default;

    SocketChannelWriter(const SocketChannelWriter&) = delete;
    SocketChannelWriter& operator=(const SocketChannelWriter&) = delete;

    SocketChannelWriter(SocketChannelWriter&&) = delete;
    SocketChannelWriter& operator=(SocketChannelWriter&&) = delete;

    [[nodiscard]] Result Reserve(size_t size, BlockWriter& blockWriter) override {
        auto sizeToReserve = static_cast<int32_t>(size);
        if (BufferSize - _writeIndex < sizeToReserve) {
            CheckResult(EndWrite());

            if (BufferSize - _writeIndex < sizeToReserve) {
                throw std::runtime_error("No more space available.");
            }
        }

        blockWriter = BlockWriter(&_writeBuffer[static_cast<size_t>(_writeIndex)], size);
        _writeIndex += sizeToReserve;

        return Result::Ok;
    }

    [[nodiscard]] Result Write(uint16_t value) override {
        auto size = static_cast<int32_t>(sizeof(value));
        if (BufferSize - _writeIndex < size) {
            CheckResult(EndWrite());

            if (BufferSize - _writeIndex < size) {
                throw std::runtime_error("No more space available.");
            }
        }

        *(reinterpret_cast<decltype(value)*>(&_writeBuffer[static_cast<size_t>(_writeIndex)])) = value;
        _writeIndex += size;

        return Result::Ok;
    }

    [[nodiscard]] Result Write(uint32_t value) override {
        auto size = static_cast<int32_t>(sizeof(value));
        if (BufferSize - _writeIndex < size) {
            CheckResult(EndWrite());

            if (BufferSize - _writeIndex < size) {
                throw std::runtime_error("No more space available.");
            }
        }

        *(reinterpret_cast<decltype(value)*>(&_writeBuffer[static_cast<size_t>(_writeIndex)])) = value;
        _writeIndex += size;

        return Result::Ok;
    }

    [[nodiscard]] Result Write(uint64_t value) override {
        auto size = static_cast<int32_t>(sizeof(value));
        if (BufferSize - _writeIndex < size) {
            CheckResult(EndWrite());

            if (BufferSize - _writeIndex < size) {
                throw std::runtime_error("No more space available.");
            }
        }

        *(reinterpret_cast<decltype(value)*>(&_writeBuffer[static_cast<size_t>(_writeIndex)])) = value;
        _writeIndex += size;

        return Result::Ok;
    }

    [[nodiscard]] Result Write(const void* source, size_t size) override {
        const auto* bufferPointer = static_cast<const uint8_t*>(source);
        auto sizeToCopy = static_cast<int32_t>(size);

        while (sizeToCopy > 0) {
            if (BufferSize == _writeIndex) {
                CheckResult(EndWrite());
                continue;
            }

            int32_t sizeOfChunkToCopy = std::min(sizeToCopy, BufferSize - _writeIndex);
            (void)memcpy(&_writeBuffer[static_cast<size_t>(_writeIndex)],
                         bufferPointer,
                         static_cast<size_t>(sizeOfChunkToCopy));
            _writeIndex += sizeOfChunkToCopy;
            bufferPointer += sizeOfChunkToCopy;
            sizeToCopy -= sizeOfChunkToCopy;
        }

        return Result::Ok;
    }

    [[nodiscard]] Result EndWrite() override {
        uint8_t* sourcePtr = _writeBuffer.data();

        // Write header
        *reinterpret_cast<int32_t*>(sourcePtr) = _writeIndex;

        CheckResult(_socket.Send(sourcePtr, static_cast<size_t>(_writeIndex)));

        _writeIndex = HeaderSize;
        return Result::Ok;
    }

private:
    Socket& _socket;

    int32_t _writeIndex = HeaderSize;
    std::array<uint8_t, BufferSize> _writeBuffer{};
};

class SocketChannelReader final : public ChannelReader {
public:
    explicit SocketChannelReader(Socket& socket) : _socket(socket) {
    }

    ~SocketChannelReader() override = default;

    SocketChannelReader(const SocketChannelReader&) = delete;
    SocketChannelReader& operator=(const SocketChannelReader&) = delete;

    SocketChannelReader(SocketChannelReader&&) = delete;
    SocketChannelReader& operator=(SocketChannelReader&&) = delete;

    [[nodiscard]] Result ReadBlock(size_t size, BlockReader& blockReader) override {
        auto blockSize = static_cast<int32_t>(size);
        while (_endFrameIndex - _readIndex < blockSize) {
            CheckResult(BeginRead());
        }

        blockReader = BlockReader(&_readBuffer[static_cast<size_t>(_readIndex)], size);
        _readIndex += blockSize;
        return Result::Ok;
    }

    [[nodiscard]] Result Read(uint16_t& value) override {
        auto size = static_cast<int32_t>(sizeof(value));
        while (_endFrameIndex - _readIndex < size) {
            CheckResult(BeginRead());
        }

        value =
            *reinterpret_cast<std::remove_reference_t<decltype(value)>*>(&_readBuffer[static_cast<size_t>(_readIndex)]);
        _readIndex += size;
        return Result::Ok;
    }

    [[nodiscard]] Result Read(uint32_t& value) override {
        auto size = static_cast<int32_t>(sizeof(value));
        while (_endFrameIndex - _readIndex < size) {
            CheckResult(BeginRead());
        }

        value =
            *reinterpret_cast<std::remove_reference_t<decltype(value)>*>(&_readBuffer[static_cast<size_t>(_readIndex)]);
        _readIndex += size;
        return Result::Ok;
    }

    [[nodiscard]] Result Read(uint64_t& value) override {
        auto size = static_cast<int32_t>(sizeof(value));
        while (_endFrameIndex - _readIndex < size) {
            CheckResult(BeginRead());
        }

        value =
            *reinterpret_cast<std::remove_reference_t<decltype(value)>*>(&_readBuffer[static_cast<size_t>(_readIndex)]);
        _readIndex += size;
        return Result::Ok;
    }

    [[nodiscard]] Result Read(void* destination, size_t size) override {
        auto* bufferPointer = static_cast<uint8_t*>(destination);
        auto sizeToCopy = static_cast<int32_t>(size);

        while (sizeToCopy > 0) {
            if (_endFrameIndex <= _readIndex) {
                CheckResult(BeginRead());
                continue;
            }

            int32_t sizeOfChunkToCopy = std::min(sizeToCopy, _endFrameIndex - _readIndex);
            (void)memcpy(bufferPointer,
                         &_readBuffer[static_cast<size_t>(_readIndex)],
                         static_cast<size_t>(sizeOfChunkToCopy));
            _readIndex += sizeOfChunkToCopy;
            bufferPointer += sizeOfChunkToCopy;
            sizeToCopy -= sizeOfChunkToCopy;
        }

        return Result::Ok;
    }

private:
    [[nodiscard]] Result BeginRead() {
        _readIndex = HeaderSize;
        int32_t sizeToRead = ReadPacketSize;
        bool readHeader = true;

        // Did we read more than one frame the last time?
        if (_writeIndex > _endFrameIndex) {
            int32_t bytesToMove = _writeIndex - _endFrameIndex;
            (void)memcpy(_readBuffer.data(),
                         &_readBuffer[static_cast<size_t>(_endFrameIndex)],
                         static_cast<size_t>(bytesToMove));

            _writeIndex = bytesToMove;

            // Did we read at least HeaderSize bytes more?
            if (bytesToMove >= HeaderSize) {
                readHeader = false;
                _endFrameIndex = *reinterpret_cast<int32_t*>(_readBuffer.data());

                // Did we read at least an entire second frame?
                if (_writeIndex >= _endFrameIndex) {
                    return Result::Ok;
                }

                sizeToRead = _endFrameIndex - _writeIndex;
            }
        } else {
            _writeIndex = 0;
        }

        while (sizeToRead > 0) {
            size_t receivedSize{};
            CheckResult(_socket.Receive(&_readBuffer[static_cast<size_t>(_writeIndex)],
                                        static_cast<size_t>(sizeToRead),
                                        receivedSize));

            sizeToRead -= static_cast<int32_t>(receivedSize);
            _writeIndex += static_cast<int32_t>(receivedSize);

            if (readHeader && (_writeIndex >= HeaderSize)) {
                readHeader = false;
                _endFrameIndex = *reinterpret_cast<int32_t*>(_readBuffer.data());

                if (_endFrameIndex > BufferSize) {
                    LogError("Protocol error. The buffer size is too small.");
                    return Result::Error;
                }

                if (_writeIndex >= _endFrameIndex) {
                    return Result::Ok;
                }

                sizeToRead = _endFrameIndex - _writeIndex;
            }
        }

        return Result::Ok;
    }

    Socket& _socket;

    int32_t _readIndex = HeaderSize;
    int32_t _writeIndex{};
    int32_t _endFrameIndex{};
    std::array<uint8_t, BufferSize> _readBuffer{};
};

class SocketChannel final : public Channel {
public:
    explicit SocketChannel(Socket socket) : _socket(std::move(socket)), _writer(_socket), _reader(_socket) {
    }

    ~SocketChannel() override = default;

    SocketChannel(const SocketChannel&) = delete;
    SocketChannel& operator=(const SocketChannel&) = delete;

    SocketChannel(SocketChannel&&) = delete;
    SocketChannel& operator=(SocketChannel&&) = delete;

    [[nodiscard]] Result GetRemoteAddress(std::string& remoteAddress) const override {
        SocketAddress socketAddress{};
        CheckResult(_socket.GetRemoteAddress(socketAddress));
        remoteAddress = socketAddress.ipAddress;
        remoteAddress.append(":");
        remoteAddress.append(std::to_string(socketAddress.port));
        return Result::Ok;
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
    TcpChannelServer(Socket listenSocketIpv4, Socket listenSocketIpv6, uint16_t port)
        : _listenSocketIpv4(std::move(listenSocketIpv4)), _listenSocketIpv6(std::move(listenSocketIpv6)), _port(port) {
    }

    ~TcpChannelServer() override = default;

    TcpChannelServer(const TcpChannelServer&) = delete;
    TcpChannelServer& operator=(const TcpChannelServer&) = delete;

    TcpChannelServer(TcpChannelServer&&) = delete;
    TcpChannelServer& operator=(TcpChannelServer&&) = delete;

    [[nodiscard]] uint16_t GetLocalPort() const override {
        return _port;
    }

    [[nodiscard]] Result TryAccept(std::unique_ptr<Channel>& acceptedChannel) override {
        if (_listenSocketIpv4.IsValid()) {
            std::optional<Socket> acceptedSocket{};
            CheckResult(_listenSocketIpv4.TryAccept(acceptedSocket));
            if (acceptedSocket) {
                CheckResult(acceptedSocket->EnableNoDelay());
                acceptedChannel = std::make_unique<SocketChannel>(std::move(*acceptedSocket));
                return Result::Ok;
            }
        }

        if (_listenSocketIpv6.IsValid()) {
            std::optional<Socket> acceptedSocket{};
            CheckResult(_listenSocketIpv6.TryAccept(acceptedSocket));
            if (acceptedSocket) {
                CheckResult(acceptedSocket->EnableNoDelay());
                acceptedChannel = std::make_unique<SocketChannel>(std::move(*acceptedSocket));
                return Result::Ok;
            }
        }

        return Result::Ok;
    }

private:
    Socket _listenSocketIpv4;
    Socket _listenSocketIpv6;
    uint16_t _port{};
};

class UdsChannelServer final : public ChannelServer {
public:
    explicit UdsChannelServer(Socket socket) : _listenSocket(std::move(socket)) {
    }

    ~UdsChannelServer() override = default;

    UdsChannelServer(const UdsChannelServer&) = delete;
    UdsChannelServer& operator=(const UdsChannelServer&) = delete;

    UdsChannelServer(UdsChannelServer&&) = delete;
    UdsChannelServer& operator=(UdsChannelServer&&) = delete;

    [[nodiscard]] uint16_t GetLocalPort() const override {
        return {};
    }

    [[nodiscard]] Result TryAccept(std::unique_ptr<Channel>& acceptedChannel) override {
        std::optional<Socket> acceptedSocket{};
        CheckResult(_listenSocket.TryAccept(acceptedSocket));

        if (acceptedSocket) {
            acceptedChannel = std::make_unique<SocketChannel>(std::move(*acceptedSocket));
        }

        return Result::Ok;
    }

private:
    Socket _listenSocket;
};

}  // namespace

[[nodiscard]] Result TryConnectToTcpChannel(const std::string& remoteIpAddress,
                                            uint16_t remotePort,
                                            uint16_t localPort,
                                            uint32_t timeoutInMilliseconds,
                                            std::unique_ptr<Channel>& connectedChannel) {
    CheckResult(StartupNetwork());

    std::optional<Socket> connectedSocket{};
    CheckResult(Socket::TryConnect(remoteIpAddress, remotePort, localPort, timeoutInMilliseconds, connectedSocket));
    if (connectedSocket) {
        CheckResult(connectedSocket->EnableNoDelay());
        connectedChannel = std::make_unique<SocketChannel>(std::move(*connectedSocket));
    }

    return Result::Ok;
}

[[nodiscard]] Result TryConnectToUdsChannel(const std::string& name, std::unique_ptr<Channel>& connectedChannel) {
    CheckResult(StartupNetwork());

    if (!Socket::IsUdsSupported()) {
        return Result::Ok;
    }

    std::optional<Socket> connectedSocket{};
    CheckResult(Socket::TryConnect(name, connectedSocket));
    if (connectedSocket) {
        connectedChannel = std::make_unique<SocketChannel>(std::move(*connectedSocket));
    }

    return Result::Ok;
}

[[nodiscard]] Result CreateTcpChannelServer(uint16_t port,
                                            bool enableRemoteAccess,
                                            std::unique_ptr<ChannelServer>& channelServer) {
    CheckResult(StartupNetwork());

    Socket listenSocketIpv4;
    if (Socket::IsIpv4Supported()) {
        CheckResult(Socket::Create(AddressFamily::Ipv4, listenSocketIpv4));
        CheckResult(listenSocketIpv4.EnableReuseAddress());
        CheckResult(listenSocketIpv4.Bind(port, enableRemoteAccess));
        CheckResult(listenSocketIpv4.GetLocalPort(port));
        CheckResult(listenSocketIpv4.Listen());
    }

    Socket listenSocketIpv6;
    if (Socket::IsIpv6Supported()) {
        CheckResult(Socket::Create(AddressFamily::Ipv6, listenSocketIpv6));
        CheckResult(listenSocketIpv6.EnableIpv6Only());
        CheckResult(listenSocketIpv6.EnableReuseAddress());
        CheckResult(listenSocketIpv6.Bind(port, enableRemoteAccess));
        CheckResult(listenSocketIpv6.GetLocalPort(port));
        CheckResult(listenSocketIpv6.Listen());
    }

    channelServer = std::make_unique<TcpChannelServer>(std::move(listenSocketIpv4), std::move(listenSocketIpv6), port);
    return Result::Ok;
}

[[nodiscard]] Result CreateUdsChannelServer(const std::string& name, std::unique_ptr<ChannelServer>& channelServer) {
    CheckResult(StartupNetwork());

    if (!Socket::IsUdsSupported()) {
        return Result::Ok;
    }

    Socket listenSocket;
    CheckResult(Socket::Create(AddressFamily::Uds, listenSocket));
    CheckResult(listenSocket.Bind(name));
    CheckResult(listenSocket.Listen());

    channelServer = std::make_unique<UdsChannelServer>(std::move(listenSocket));
    return Result::Ok;
}

}  // namespace DsVeosCoSim
