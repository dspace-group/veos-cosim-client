// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <memory>
#include <string>
#include <type_traits>

#include "DsVeosCoSim/CoSimTypes.h"

namespace DsVeosCoSim {

class ChannelWriter {
protected:
    ChannelWriter() = default;

public:
    virtual ~ChannelWriter() = default;

    ChannelWriter(const ChannelWriter&) = delete;
    ChannelWriter& operator=(const ChannelWriter&) = delete;

    ChannelWriter(ChannelWriter&&) = delete;
    ChannelWriter& operator=(ChannelWriter&&) = delete;

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

class ChannelReader {
protected:
    ChannelReader() = default;

public:
    virtual ~ChannelReader() = default;

    ChannelReader(const ChannelReader&) = delete;
    ChannelReader& operator=(const ChannelReader&) = delete;

    ChannelReader(ChannelReader&&) = delete;
    ChannelReader& operator=(ChannelReader&&) = delete;

    [[nodiscard]] virtual Result Read(uint16_t& value) = 0;
    [[nodiscard]] virtual Result Read(uint32_t& value) = 0;
    [[nodiscard]] virtual Result Read(uint64_t& value) = 0;
    [[nodiscard]] virtual Result Read(void* destination, size_t size) = 0;

    [[nodiscard]] Result Read(SimulationTime& simulationTime) {
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
