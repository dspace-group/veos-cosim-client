// Copyright dSPACE GmbH. All rights reserved.

#include "Communication.h"

#include <algorithm>
#include <cstring>

#include "Logger.h"

namespace DsVeosCoSim {

namespace {

constexpr int HeaderSize = 4;
constexpr int BufferSize = 64 * 1024;
constexpr int ReadPacketSize = 1024;

}  // namespace

Channel::Channel() {
    Reset();
}

Channel::Channel(Socket socket) : _socket(std::move(socket)) {
    Reset();
}

Result Channel::GetRemoteAddress(std::string& ipAddress, uint16_t& port) const {
    return _socket.GetRemoteAddress(ipAddress, port);
}

void Channel::Disconnect() {
    Stop();
    Reset();
}

void Channel::Stop() {
    _socket.Close();
}

Result Channel::Write(const void* source, size_t size) {
    const auto* bufferPointer = static_cast<const uint8_t*>(source);

    while (size > 0) {
        const int sizeToCopy = std::min(static_cast<int>(size), BufferSize - _writeBufferWriteIndex);
        if (sizeToCopy == 0) {
            CheckResult(FlushWriteBuffer());
            continue;
        }

        memcpy(&_writeBuffer[_writeBufferWriteIndex], bufferPointer, sizeToCopy);
        _writeBufferWriteIndex += sizeToCopy;
        bufferPointer += sizeToCopy;
        size -= sizeToCopy;
    }

    return Result::Ok;
}

Result Channel::EndWrite() {
    return FlushWriteBuffer();
}

Result Channel::Read(void* destination, size_t size) {
    auto* bufferPointer = static_cast<uint8_t*>(destination);

    while (size > 0) {
        const int sizeToCopy = std::min(static_cast<int>(size), _readBufferEndFrameIndex - _readBufferReadIndex);
        if (sizeToCopy <= 0) {
            CheckResult(FillReadBuffer());
            continue;
        }

        memcpy(bufferPointer, &_readBuffer[_readBufferReadIndex], sizeToCopy);
        _readBufferReadIndex += sizeToCopy;
        bufferPointer += sizeToCopy;
        size -= sizeToCopy;
    }

    return Result::Ok;
}

void Channel::Reset() {
    _writeBufferWriteIndex = HeaderSize;
    _readBufferReadIndex = HeaderSize;
    _readBufferWriteIndex = 0;
    _readBufferEndFrameIndex = 0;

    _writeBuffer.resize(BufferSize);
    _readBuffer.resize(BufferSize);
}

Result Channel::FlushWriteBuffer() {
    uint8_t* sourcePtr = _writeBuffer.data();

    // Write header
    memcpy(sourcePtr, &_writeBufferWriteIndex, sizeof _writeBufferWriteIndex);

    while (_writeBufferWriteIndex > 0) {
        int size = 0;
        CheckResult(_socket.Send(sourcePtr, _writeBufferWriteIndex, size));

        sourcePtr += size;
        _writeBufferWriteIndex -= size;
    }

    _writeBufferWriteIndex = HeaderSize;
    return Result::Ok;
}

Result Channel::FillReadBuffer() {
    _readBufferReadIndex = HeaderSize;
    int sizeToRead = ReadPacketSize;
    bool readHeader = true;

    // Did we read more than one frame the last time?
    if (_readBufferWriteIndex > _readBufferEndFrameIndex) {
        const int bytesToMove = _readBufferWriteIndex - _readBufferEndFrameIndex;
        memmove(_readBuffer.data(), &_readBuffer[_readBufferEndFrameIndex], bytesToMove);

        _readBufferWriteIndex -= _readBufferEndFrameIndex;

        // Did we read at least HeaderSize bytes more?
        if (bytesToMove >= HeaderSize) {
            readHeader = false;
            memcpy(&_readBufferEndFrameIndex, _readBuffer.data(), HeaderSize);

            // Did we read at least an entire second frame?
            if (_readBufferWriteIndex >= _readBufferEndFrameIndex) {
                return Result::Ok;
            }

            sizeToRead = _readBufferEndFrameIndex - _readBufferWriteIndex;
        }
    } else {
        _readBufferWriteIndex = 0;
    }

    while (sizeToRead > 0) {
        int receivedSize = 0;
        CheckResult(_socket.Receive(&_readBuffer[_readBufferWriteIndex], sizeToRead, receivedSize));

        sizeToRead -= receivedSize;
        _readBufferWriteIndex += receivedSize;

        if (readHeader && _readBufferWriteIndex >= HeaderSize) {
            readHeader = false;
            memcpy(&_readBufferEndFrameIndex, _readBuffer.data(), HeaderSize);

            if (_readBufferEndFrameIndex > BufferSize) {
                LogError("Protocol error. The buffer size is too small.");
                return Result::Error;
            }

            sizeToRead = _readBufferEndFrameIndex - _readBufferWriteIndex;
        }
    }

    return Result::Ok;
}

Result ConnectToServer(std::string_view remoteIpAddress, uint16_t remotePort, uint16_t localPort, Channel& channel) {
    CheckResult(StartupNetwork());

    Socket socket;
    CheckResult(socket.Connect(remoteIpAddress, remotePort, localPort));
    CheckResult(socket.EnableNoDelay());

    channel = Channel(std::move(socket));
    return Result::Ok;
}

Result Server::Start(uint16_t& port, bool enableRemoteAccess) {
    if (_isRunning) {
        return Result::Ok;
    }

    CheckResult(StartupNetwork());

    Socket socket;
    CheckResult(socket.Bind(port, enableRemoteAccess));
    CheckResult(socket.Listen());
    CheckResult(socket.GetLocalPort(port));

    _listenSocket = std::move(socket);
    _isRunning = true;
    return Result::Ok;
}

void Server::Stop() {
    _listenSocket.Close();
    _isRunning = false;
}

Result Server::Accept(Channel& channel) const {
    Socket socket;
    CheckResult(_listenSocket.Accept(socket));
    CheckResult(socket.EnableNoDelay());

    channel = Channel(std::move(socket));
    return Result::Ok;
}

}  // namespace DsVeosCoSim
