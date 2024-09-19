// Copyright dSPACE GmbH. All rights reserved.

#include "SocketChannel.h"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <thread>

#include "CoSimHelper.h"

namespace DsVeosCoSim {

namespace {

constexpr int32_t HeaderSize = 4;
constexpr int32_t BufferSize = 64 * 1024;
constexpr int32_t ReadPacketSize = 1024;

}  // namespace

SocketChannelWriter::SocketChannelWriter(Socket* socket) : _socket(socket) {
    _writeIndex = HeaderSize;

    _writeBuffer.resize(BufferSize);
}

SocketChannelWriter::SocketChannelWriter(SocketChannelWriter&& other) noexcept {
    _socket = other._socket;
    _writeIndex = other._writeIndex;
    _writeBuffer = other._writeBuffer;

    other._socket = nullptr;
    other._writeIndex = 0;
    other._writeBuffer.clear();
}

SocketChannelWriter& SocketChannelWriter::operator=(SocketChannelWriter&& other) noexcept {
    _socket = other._socket;
    _writeIndex = other._writeIndex;
    _writeBuffer = other._writeBuffer;

    other._socket = nullptr;
    other._writeIndex = 0;
    other._writeBuffer.clear();

    return *this;
}

bool SocketChannelWriter::Write(const void* source, size_t size) {
    const auto* bufferPointer = static_cast<const uint8_t*>(source);

    while (size > 0) {
        const int32_t sizeToCopy = std::min(static_cast<int32_t>(size), BufferSize - _writeIndex);
        if (sizeToCopy == 0) {
            CheckResult(EndWrite());
            continue;
        }

        (void)::memcpy(&_writeBuffer[_writeIndex], bufferPointer, sizeToCopy);
        _writeIndex += sizeToCopy;
        bufferPointer += sizeToCopy;
        size -= sizeToCopy;
    }

    return true;
}

bool SocketChannelWriter::EndWrite() {
    uint8_t* sourcePtr = _writeBuffer.data();

    // Write header
    (void)::memcpy(sourcePtr, &_writeIndex, sizeof _writeIndex);

    while (_writeIndex > 0) {
        int32_t sentSize{};
        CheckResult(_socket->Send(sourcePtr, _writeIndex, sentSize));

        sourcePtr += sentSize;
        _writeIndex -= sentSize;
    }

    _writeIndex = HeaderSize;
    return true;
}

SocketChannelReader::SocketChannelReader(Socket* socket) : _socket(socket) {
    _readIndex = HeaderSize;
    _writeIndex = 0;
    _endFrameIndex = 0;

    _readBuffer.resize(BufferSize);
}

SocketChannelReader::SocketChannelReader(SocketChannelReader&& other) noexcept {
    _socket = other._socket;
    _readIndex = other._readIndex;
    _writeIndex = other._writeIndex;
    _endFrameIndex = other._endFrameIndex;
    _readBuffer = other._readBuffer;

    other._socket = nullptr;
    other._readIndex = 0;
    other._writeIndex = 0;
    other._endFrameIndex = 0;
    other._readBuffer.clear();
}

SocketChannelReader& SocketChannelReader::operator=(SocketChannelReader&& other) noexcept {
    _socket = other._socket;
    _readIndex = other._readIndex;
    _writeIndex = other._writeIndex;
    _endFrameIndex = other._endFrameIndex;
    _readBuffer = other._readBuffer;

    other._socket = nullptr;
    other._readIndex = 0;
    other._writeIndex = 0;
    other._endFrameIndex = 0;
    other._readBuffer.clear();

    return *this;
}

bool SocketChannelReader::BeginRead() {
    _readIndex = HeaderSize;
    int32_t sizeToRead = ReadPacketSize;
    bool readHeader = true;

    // Did we read more than one frame the last time?
    if (_writeIndex > _endFrameIndex) {
        const int32_t bytesToMove = _writeIndex - _endFrameIndex;
        (void)::memcpy(_readBuffer.data(), &_readBuffer[_endFrameIndex], bytesToMove);

        _writeIndex -= _endFrameIndex;

        // Did we read at least HeaderSize bytes more?
        if (bytesToMove >= HeaderSize) {
            readHeader = false;
            (void)::memcpy(&_endFrameIndex, _readBuffer.data(), HeaderSize);

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

        if (readHeader && _writeIndex >= HeaderSize) {
            readHeader = false;
            (void)::memcpy(&_endFrameIndex, _readBuffer.data(), HeaderSize);

            if (_endFrameIndex > BufferSize) {
                throw CoSimException("Protocol error. The buffer size is too small.");
            }

            sizeToRead = _endFrameIndex - _writeIndex;
        }
    }

    return true;
}

bool SocketChannelReader::Read(void* destination, size_t size) {
    auto* bufferPointer = static_cast<uint8_t*>(destination);

    while (size > 0) {
        const int32_t sizeToCopy = std::min(static_cast<int32_t>(size), _endFrameIndex - _readIndex);
        if (sizeToCopy <= 0) {
            CheckResult(BeginRead());
            continue;
        }

        (void)::memcpy(bufferPointer, &_readBuffer[_readIndex], sizeToCopy);
        _readIndex += sizeToCopy;
        bufferPointer += sizeToCopy;
        size -= sizeToCopy;
    }

    return true;
}

SocketChannel::SocketChannel(Socket socket) : _socket(std::move(socket)), _writer(&_socket), _reader(&_socket) {
}

SocketChannel::SocketChannel(SocketChannel&& other) noexcept
    : _socket(std::move(other._socket)), _writer(&_socket), _reader(&_socket) {
}

SocketChannel& SocketChannel::operator=(SocketChannel&& other) noexcept {
    _socket = std::move(other._socket);
    _writer = SocketChannelWriter(&_socket);
    _reader = SocketChannelReader(&_socket);
    return *this;
}

SocketAddress SocketChannel::GetRemoteAddress() const {
    return _socket.GetRemoteAddress();
}

void SocketChannel::Disconnect() {
    _socket.Shutdown();
}

ChannelWriter& SocketChannel::GetWriter() {
    return _writer;
}

ChannelReader& SocketChannel::GetReader() {
    return _reader;
}

std::optional<SocketChannel> TryConnectToTcpChannel(std::string_view remoteIpAddress,
                                                    uint16_t remotePort,
                                                    uint16_t localPort,
                                                    uint32_t timeoutInMilliseconds) {
    StartupNetwork();

    std::optional<Socket> connectedSocket =
        Socket::TryConnect(remoteIpAddress, remotePort, localPort, timeoutInMilliseconds);
    if (connectedSocket) {
        connectedSocket->EnableNoDelay();
        return SocketChannel(std::move(*connectedSocket));
    }

    return {};
}

TcpChannelServer::TcpChannelServer(uint16_t port, bool enableRemoteAccess) {
    StartupNetwork();

    _port = port;

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

uint16_t TcpChannelServer::GetLocalPort() const {
    return _port;
}

std::optional<SocketChannel> TcpChannelServer::TryAccept(uint32_t timeoutInMilliseconds) const {
    do {
        std::optional<Socket> socket = _listenSocketIpv4.TryAccept();
        if (socket) {
            socket->EnableNoDelay();
            return SocketChannel(std::move(*socket));
        }

        socket = _listenSocketIpv6.TryAccept();
        if (socket) {
            socket->EnableNoDelay();
            return SocketChannel(std::move(*socket));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (timeoutInMilliseconds > 0) {
            timeoutInMilliseconds--;
        }
    } while (timeoutInMilliseconds > 0);

    return {};
}

std::optional<SocketChannel> TryConnectToUdsChannel(std::string_view name) {
    StartupNetwork();

    Socket socket(AddressFamily::Uds);
    if (socket.TryConnect(name)) {
        return SocketChannel(std::move(socket));
    }

    return {};
}

UdsChannelServer::UdsChannelServer(std::string_view name) {
    StartupNetwork();

    _listenSocket = Socket(AddressFamily::Uds);
    _listenSocket.Bind(name);
    _listenSocket.Listen();
}

std::optional<SocketChannel> UdsChannelServer::TryAccept(uint32_t timeoutInMilliseconds) const {
    std::optional<Socket> socket = _listenSocket.TryAccept(timeoutInMilliseconds);
    if (socket) {
        return SocketChannel(std::move(*socket));
    }

    return {};
}

}  // namespace DsVeosCoSim
