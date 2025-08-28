// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "DsVeosCoSim/CoSimTypes.h"

namespace DsVeosCoSim {

class BlockWriter {
public:
    BlockWriter() = default;
    BlockWriter(uint8_t* data, size_t size) : _data(data), _size(size) {
    }

    virtual ~BlockWriter() = default;

    BlockWriter(const BlockWriter&) = delete;
    BlockWriter& operator=(const BlockWriter&) = delete;

    BlockWriter(BlockWriter&&) = default;
    BlockWriter& operator=(BlockWriter&&) = default;

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

    void Write(std::chrono::nanoseconds nanoseconds) {
        Write(static_cast<uint64_t>(nanoseconds.count()));
    }

    template <typename TEnum, std::enable_if_t<std::is_enum_v<TEnum>, int> = 0>
    void Write(TEnum value) {
        using TUnderlying = std::underlying_type_t<TEnum>;
        Write(static_cast<TUnderlying>(value));
    }

private:
    uint8_t* _data{};
    size_t _size{};
};

class ChannelWriter {
protected:
    ChannelWriter() = default;

public:
    virtual ~ChannelWriter() = default;

    ChannelWriter(const ChannelWriter&) = delete;
    ChannelWriter& operator=(const ChannelWriter&) = delete;

    ChannelWriter(ChannelWriter&&) = delete;
    ChannelWriter& operator=(ChannelWriter&&) = delete;

    [[nodiscard]] virtual Result Reserve(size_t size, BlockWriter& blockWriter) = 0;

    [[nodiscard]] virtual Result Write(uint16_t value) = 0;
    [[nodiscard]] virtual Result Write(uint32_t value) = 0;
    [[nodiscard]] virtual Result Write(uint64_t value) = 0;
    [[nodiscard]] virtual Result Write(const void* source, size_t size) = 0;

    [[nodiscard]] Result Write(std::chrono::nanoseconds nanoseconds) {
        return Write(static_cast<uint64_t>(nanoseconds.count()));
    }

    template <typename TEnum, std::enable_if_t<std::is_enum_v<TEnum>, int> = 0>
    [[nodiscard]] Result Write(TEnum value) {
        using TUnderlying = std::underlying_type_t<TEnum>;
        return Write(static_cast<TUnderlying>(value));
    }

    [[nodiscard]] virtual Result EndWrite() = 0;
};

class BlockReader {
public:
    BlockReader() = default;
    BlockReader(uint8_t* data, size_t size) : _data(data), _size(size) {
    }

    virtual ~BlockReader() = default;

    BlockReader(const BlockReader&) = delete;
    BlockReader& operator=(const BlockReader&) = delete;

    BlockReader(BlockReader&&) = default;
    BlockReader& operator=(BlockReader&&) = default;

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

    void Read(std::chrono::nanoseconds& simulationTime) {
        Read(reinterpret_cast<uint64_t&>(simulationTime));
    }

    template <typename TEnum, std::enable_if_t<std::is_enum_v<TEnum>, int> = 0>
    void Read(TEnum& value) {
        using TUnderlying = std::underlying_type_t<TEnum>;
        Read(reinterpret_cast<TUnderlying&>(value));
    }

private:
    uint8_t* _data{};
    size_t _size{};
};

class ChannelReader {
protected:
    ChannelReader() = default;

public:
    virtual ~ChannelReader() = default;

    ChannelReader(const ChannelReader&) = delete;
    ChannelReader& operator=(const ChannelReader&) = delete;

    ChannelReader(ChannelReader&&) = delete;
    ChannelReader& operator=(ChannelReader&&) = delete;

    [[nodiscard]] virtual Result ReadBlock(size_t size, BlockReader& blockReader) = 0;

    [[nodiscard]] virtual Result Read(uint16_t& value) = 0;
    [[nodiscard]] virtual Result Read(uint32_t& value) = 0;
    [[nodiscard]] virtual Result Read(uint64_t& value) = 0;
    [[nodiscard]] virtual Result Read(void* destination, size_t size) = 0;

    [[nodiscard]] Result Read(std::chrono::nanoseconds& simulationTime) {
        return Read(reinterpret_cast<uint64_t&>(simulationTime));
    }

    template <typename TEnum, std::enable_if_t<std::is_enum_v<TEnum>, int> = 0>
    [[nodiscard]] Result Read(TEnum& value) {
        using TUnderlying = std::underlying_type_t<TEnum>;
        return Read(reinterpret_cast<TUnderlying&>(value));
    }
};

class Channel {
protected:
    Channel() = default;

public:
    virtual ~Channel() = default;

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
    virtual ~ChannelServer() = default;

    ChannelServer(const ChannelServer&) = delete;
    ChannelServer& operator=(const ChannelServer&) = delete;

    ChannelServer(ChannelServer&&) = delete;
    ChannelServer& operator=(ChannelServer&&) = delete;

    [[nodiscard]] virtual uint16_t GetLocalPort() const = 0;

    [[nodiscard]] virtual Result TryAccept(std::unique_ptr<Channel>& acceptedChannel) = 0;
};

[[nodiscard]] Result TryConnectToLocalChannel(const std::string& name, std::unique_ptr<Channel>& connectedChannel);

[[nodiscard]] Result TryConnectToTcpChannel(const std::string& remoteIpAddress,
                                            uint16_t remotePort,
                                            uint16_t localPort,
                                            uint32_t timeoutInMilliseconds,
                                            std::unique_ptr<Channel>& connectedChannel);

[[nodiscard]] Result TryConnectToUdsChannel(const std::string& name, std::unique_ptr<Channel>& connectedChannel);

[[nodiscard]] Result CreateLocalChannelServer(const std::string& name, std::unique_ptr<ChannelServer>& channelServer);

[[nodiscard]] Result CreateTcpChannelServer(uint16_t port,
                                            bool enableRemoteAccess,
                                            std::unique_ptr<ChannelServer>& channelServer);

[[nodiscard]] Result CreateUdsChannelServer(const std::string& name, std::unique_ptr<ChannelServer>& channelServer);

}  // namespace DsVeosCoSim
