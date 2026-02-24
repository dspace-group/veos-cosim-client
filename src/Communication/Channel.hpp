// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <algorithm>
#include <array>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "Error.hpp"

namespace DsVeosCoSim {

constexpr int32_t HeaderSize = 4;
constexpr int32_t BufferSize = 65536;

class BlockWriter final {
public:
    BlockWriter() = default;
    BlockWriter(uint8_t* data, size_t size) : _data(data), _size(size) {
    }

    ~BlockWriter() noexcept = default;

    BlockWriter(const BlockWriter&) = delete;
    BlockWriter& operator=(const BlockWriter&) = delete;

    BlockWriter(BlockWriter&&) noexcept = default;
    BlockWriter& operator=(BlockWriter&&) noexcept = default;

    void Write(uint16_t value) {
        size_t size = sizeof(value);
        if (size > _size) {
            throw std::runtime_error("No more space available.");
        }

        *reinterpret_cast<decltype(value)*>(_data) = value;
        _data += size;
        _size -= size;
    }

    void Write(uint32_t value) {
        size_t size = sizeof(value);
        if (size > _size) {
            throw std::runtime_error("No more space available.");
        }

        *reinterpret_cast<decltype(value)*>(_data) = value;
        _data += size;
        _size -= size;
    }

    void Write(uint64_t value) {
        size_t size = sizeof(value);
        if (size > _size) {
            throw std::runtime_error("No more space available.");
        }

        *reinterpret_cast<decltype(value)*>(_data) = value;
        _data += size;
        _size -= size;
    }

    void Write(const void* source, size_t size) {
        if (size > _size) {
            throw std::runtime_error("No more space available.");
        }

        (void)memcpy(_data, source, size);
        _data += size;
        _size -= size;
    }

    void Write(int64_t value) {
        Write(static_cast<uint64_t>(value));
    }

    template <typename TEnum, std::enable_if_t<std::is_enum_v<TEnum>, int> = 0>
    void Write(TEnum value) {
        using TUnderlying = std::underlying_type_t<TEnum>;
        Write(static_cast<TUnderlying>(value));
    }

    void EndWrite() const {
        if (_size != 0) {
            throw std::runtime_error("Not all space has been used.");
        }
    }

private:
    uint8_t* _data{};
    size_t _size{};
};

class ChannelWriter {
protected:
    ChannelWriter() = default;

public:
    virtual ~ChannelWriter() noexcept = default;

    ChannelWriter(const ChannelWriter&) = delete;
    ChannelWriter& operator=(const ChannelWriter&) = delete;

    ChannelWriter(ChannelWriter&&) = delete;
    ChannelWriter& operator=(ChannelWriter&&) = delete;

    [[nodiscard]] Result Reserve(size_t size, BlockWriter& blockWriter) {
        auto sizeToReserve = static_cast<int32_t>(size);
        if (BufferSize - _writeIndex < sizeToReserve) {
            CheckResult(EndWrite());

            if (BufferSize - _writeIndex < sizeToReserve) {
                return CreateError("No more space available.");
            }
        }

        blockWriter = BlockWriter(&_writeBuffer[static_cast<size_t>(_writeIndex)], size);
        _writeIndex += sizeToReserve;

        return CreateOk();
    }

    [[nodiscard]] Result Write(uint16_t value) {
        auto size = static_cast<int32_t>(sizeof(value));
        if (BufferSize - _writeIndex < size) {
            CheckResult(EndWrite());

            if (BufferSize - _writeIndex < size) {
                return CreateError("No more space available.");
            }
        }

        *(reinterpret_cast<decltype(value)*>(&_writeBuffer[static_cast<size_t>(_writeIndex)])) = value;
        _writeIndex += size;

        return CreateOk();
    }

    [[nodiscard]] Result Write(uint32_t value) {
        auto size = static_cast<int32_t>(sizeof(value));
        if (BufferSize - _writeIndex < size) {
            CheckResult(EndWrite());

            if (BufferSize - _writeIndex < size) {
                return CreateError("No more space available.");
            }
        }

        *(reinterpret_cast<decltype(value)*>(&_writeBuffer[static_cast<size_t>(_writeIndex)])) = value;
        _writeIndex += size;

        return CreateOk();
    }

    [[nodiscard]] Result Write(uint64_t value) {
        auto size = static_cast<int32_t>(sizeof(value));
        if (BufferSize - _writeIndex < size) {
            CheckResult(EndWrite());

            if (BufferSize - _writeIndex < size) {
                return CreateError("No more space available.");
            }
        }

        *(reinterpret_cast<decltype(value)*>(&_writeBuffer[static_cast<size_t>(_writeIndex)])) = value;
        _writeIndex += size;

        return CreateOk();
    }

    [[nodiscard]] Result Write(const void* source, size_t size) {
        const auto* bufferPointer = static_cast<const uint8_t*>(source);
        auto sizeToCopy = static_cast<int32_t>(size);

        while (sizeToCopy > 0) {
            if (BufferSize == _writeIndex) {
                CheckResult(EndWrite());
                continue;
            }

            int32_t sizeOfChunkToCopy = std::min(sizeToCopy, BufferSize - _writeIndex);
            (void)memcpy(&_writeBuffer[static_cast<size_t>(_writeIndex)], bufferPointer, static_cast<size_t>(sizeOfChunkToCopy));
            _writeIndex += sizeOfChunkToCopy;
            bufferPointer += sizeOfChunkToCopy;
            sizeToCopy -= sizeOfChunkToCopy;
        }

        return CreateOk();
    }

    [[nodiscard]] Result Write(int64_t value) {
        return Write(static_cast<uint64_t>(value));
    }

    template <typename TEnum, std::enable_if_t<std::is_enum_v<TEnum>, int> = 0>
    [[nodiscard]] Result Write(TEnum value) {
        using TUnderlying = std::underlying_type_t<TEnum>;
        return Write(static_cast<TUnderlying>(value));
    }

    [[nodiscard]] Result EndWrite() {
        uint8_t* buffer = _writeBuffer.data();

        // Write header
        *reinterpret_cast<int32_t*>(buffer) = _writeIndex;

        CheckResult(Send(buffer, static_cast<size_t>(_writeIndex)));

        _writeIndex = HeaderSize;
        return CreateOk();
    }

protected:
    [[nodiscard]] virtual Result Send(const uint8_t* buffer, size_t size) = 0;

private:
    int32_t _writeIndex = HeaderSize;
    std::array<uint8_t, BufferSize> _writeBuffer{};
};

class BlockReader final {
public:
    BlockReader() = default;
    BlockReader(uint8_t* data, size_t size) : _data(data), _size(size) {
    }

    ~BlockReader() noexcept = default;

    BlockReader(const BlockReader&) = delete;
    BlockReader& operator=(const BlockReader&) = delete;

    BlockReader(BlockReader&&) noexcept = default;
    BlockReader& operator=(BlockReader&&) noexcept = default;

    void Read(uint16_t& value) {
        size_t size = sizeof(value);
        if (size > _size) {
            throw std::runtime_error("No more data available.");
        }

        value = *reinterpret_cast<std::remove_reference_t<decltype(value)>*>(_data);
        _data += size;
        _size -= size;
    }

    void Read(uint32_t& value) {
        size_t size = sizeof(value);
        if (size > _size) {
            throw std::runtime_error("No more data available.");
        }

        value = *reinterpret_cast<std::remove_reference_t<decltype(value)>*>(_data);
        _data += size;
        _size -= size;
    }

    void Read(uint64_t& value) {
        size_t size = sizeof(value);
        if (size > _size) {
            throw std::runtime_error("No more data available.");
        }

        value = *reinterpret_cast<std::remove_reference_t<decltype(value)>*>(_data);
        _data += size;
        _size -= size;
    }

    void Read(void* destination, size_t size) {
        if (size > _size) {
            throw std::runtime_error("No more data available.");
        }

        memcpy(destination, _data, size);
        _data += size;
        _size -= size;
    }

    void Read(int64_t& value) {
        Read(reinterpret_cast<uint64_t&>(value));
    }

    template <typename TEnum, std::enable_if_t<std::is_enum_v<TEnum>, int> = 0>
    void Read(TEnum& value) {
        using TUnderlying = std::underlying_type_t<TEnum>;
        Read(reinterpret_cast<TUnderlying&>(value));
    }

    void EndRead() const {
        if (_size != 0) {
            throw std::runtime_error("Not all data has been read.");
        }
    }

private:
    uint8_t* _data{};
    size_t _size{};
};

class ChannelReader {
protected:
    ChannelReader() = default;

public:
    virtual ~ChannelReader() noexcept = default;

    ChannelReader(const ChannelReader&) = delete;
    ChannelReader& operator=(const ChannelReader&) = delete;

    ChannelReader(ChannelReader&&) = delete;
    ChannelReader& operator=(ChannelReader&&) = delete;

    [[nodiscard]] Result ReadBlock(size_t size, BlockReader& blockReader) {
        auto blockSize = static_cast<int32_t>(size);
        while (_endFrameIndex - _readIndex < blockSize) {
            CheckResult(BeginRead());
        }

        blockReader = BlockReader(&_readBuffer[static_cast<size_t>(_readIndex)], size);
        _readIndex += blockSize;
        return CreateOk();
    }

    [[nodiscard]] Result Read(uint16_t& value) {
        auto size = static_cast<int32_t>(sizeof(value));
        while (_endFrameIndex - _readIndex < size) {
            CheckResult(BeginRead());
        }

        value = *reinterpret_cast<std::remove_reference_t<decltype(value)>*>(&_readBuffer[static_cast<size_t>(_readIndex)]);
        _readIndex += size;
        return CreateOk();
    }

    [[nodiscard]] Result Read(uint32_t& value) {
        auto size = static_cast<int32_t>(sizeof(value));
        while (_endFrameIndex - _readIndex < size) {
            CheckResult(BeginRead());
        }

        value = *reinterpret_cast<std::remove_reference_t<decltype(value)>*>(&_readBuffer[static_cast<size_t>(_readIndex)]);
        _readIndex += size;
        return CreateOk();
    }

    [[nodiscard]] Result Read(uint64_t& value) {
        auto size = static_cast<int32_t>(sizeof(value));
        while (_endFrameIndex - _readIndex < size) {
            CheckResult(BeginRead());
        }

        value = *reinterpret_cast<std::remove_reference_t<decltype(value)>*>(&_readBuffer[static_cast<size_t>(_readIndex)]);
        _readIndex += size;
        return CreateOk();
    }

    [[nodiscard]] Result Read(void* destination, size_t size) {
        auto* bufferPointer = static_cast<uint8_t*>(destination);
        auto sizeToCopy = static_cast<int32_t>(size);

        while (sizeToCopy > 0) {
            if (_endFrameIndex <= _readIndex) {
                CheckResult(BeginRead());
                continue;
            }

            int32_t sizeOfChunkToCopy = std::min(sizeToCopy, _endFrameIndex - _readIndex);
            (void)memcpy(bufferPointer, &_readBuffer[static_cast<size_t>(_readIndex)], static_cast<size_t>(sizeOfChunkToCopy));
            _readIndex += sizeOfChunkToCopy;
            bufferPointer += sizeOfChunkToCopy;
            sizeToCopy -= sizeOfChunkToCopy;
        }

        return CreateOk();
    }

    [[nodiscard]] Result Read(int64_t& value) {
        return Read(reinterpret_cast<uint64_t&>(value));
    }

    template <typename TEnum, std::enable_if_t<std::is_enum_v<TEnum>, int> = 0>
    [[nodiscard]] Result Read(TEnum& value) {
        using TUnderlying = std::underlying_type_t<TEnum>;
        return Read(reinterpret_cast<TUnderlying&>(value));
    }

    [[nodiscard]] Result EndRead() const {
        // This should not happen. It's just an assert, that works for release as well
        if (_readIndex != _endFrameIndex) {
            return CreateError("Not all data has been read.");
        }

        return CreateOk();
    }

protected:
    [[nodiscard]] virtual Result Receive(void* destination, size_t size, size_t& receivedSize) = 0;

    int32_t _defaultSizeToRead{};

private:
    [[nodiscard]] Result BeginRead() {
        uint8_t* buffer = _readBuffer.data();

        _readIndex = HeaderSize;
        int32_t sizeToRead = _defaultSizeToRead;
        bool readHeader = true;

        // Did we read more than one frame the last time?
        if (_writeIndex > _endFrameIndex) {
            int32_t bytesToMove = _writeIndex - _endFrameIndex;
            (void)memcpy(buffer, &buffer[static_cast<size_t>(_endFrameIndex)], static_cast<size_t>(bytesToMove));

            _writeIndex = bytesToMove;

            // Did we read at least HeaderSize bytes more?
            if (bytesToMove >= HeaderSize) {
                readHeader = false;
                _endFrameIndex = *reinterpret_cast<int32_t*>(buffer);

                // Did we read at least an entire second frame?
                if (_writeIndex >= _endFrameIndex) {
                    return CreateOk();
                }

                sizeToRead = _endFrameIndex - _writeIndex;
            }
        } else {
            _writeIndex = 0;
        }

        while (sizeToRead > 0) {
            size_t receivedSize{};
            CheckResult(Receive(&buffer[static_cast<size_t>(_writeIndex)], static_cast<size_t>(sizeToRead), receivedSize));

            sizeToRead -= static_cast<int32_t>(receivedSize);
            _writeIndex += static_cast<int32_t>(receivedSize);

            if (readHeader && (_writeIndex >= HeaderSize)) {
                readHeader = false;
                _endFrameIndex = *reinterpret_cast<int32_t*>(buffer);

                if (_endFrameIndex > BufferSize) {
                    return CreateError("Protocol error. The buffer size is too small.");
                }

                if (_writeIndex >= _endFrameIndex) {
                    return CreateOk();
                }

                sizeToRead = _endFrameIndex - _writeIndex;
            }
        }

        return CreateOk();
    }

    int32_t _readIndex{};
    int32_t _endFrameIndex{};
    int32_t _writeIndex{};
    std::array<uint8_t, BufferSize> _readBuffer{};
};

class Channel {
protected:
    Channel() = default;

public:
    virtual ~Channel() noexcept = default;

    Channel(const Channel&) = delete;
    Channel& operator=(const Channel&) = delete;

    Channel(Channel&&) = delete;
    Channel& operator=(Channel&&) = delete;

    [[nodiscard]] virtual Result GetRemoteAddress(std::string& remoteAddress) const = 0;

    virtual void Disconnect() = 0;

    [[nodiscard]] virtual ChannelWriter& GetWriter() = 0;
    [[nodiscard]] virtual ChannelReader& GetReader() = 0;
};

class ChannelServer {
protected:
    ChannelServer() = default;

public:
    virtual ~ChannelServer() noexcept = default;

    ChannelServer(const ChannelServer&) = delete;
    ChannelServer& operator=(const ChannelServer&) = delete;

    ChannelServer(ChannelServer&&) = delete;
    ChannelServer& operator=(ChannelServer&&) = delete;

    [[nodiscard]] virtual uint16_t GetLocalPort() const = 0;

    [[nodiscard]] virtual Result TryAccept(std::unique_ptr<Channel>& channel) = 0;
};

[[nodiscard]] Result TryConnectToTcpChannel(const std::string& remoteIpAddress,
                                            uint16_t remotePort,
                                            uint16_t localPort,
                                            uint32_t timeoutInMilliseconds,
                                            std::unique_ptr<Channel>& channel);

[[nodiscard]] Result CreateTcpChannelServer(uint16_t port, bool enableRemoteAccess, std::unique_ptr<ChannelServer>& server);

[[nodiscard]] Result TryConnectToLocalChannel(const std::string& name, std::unique_ptr<Channel>& channel);

[[nodiscard]] Result CreateLocalChannelServer(const std::string& name, std::unique_ptr<ChannelServer>& server);

}  // namespace DsVeosCoSim
