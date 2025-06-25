// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <memory>
#include <string>
#include <type_traits>

namespace DsVeosCoSim {

class ChannelWriter {
protected:
    ChannelWriter() noexcept = default;

public:
    virtual ~ChannelWriter() noexcept = default;

    ChannelWriter(const ChannelWriter&) = delete;
    ChannelWriter& operator=(const ChannelWriter&) = delete;

    ChannelWriter(ChannelWriter&&) = delete;
    ChannelWriter& operator=(ChannelWriter&&) = delete;

    template <typename T>
    [[nodiscard]] bool Write(const T& value) {
        static_assert(std::is_trivially_copyable_v<T>);

        return Write(&value, sizeof(T));
    }

    [[nodiscard]] virtual bool Write(const void* source, size_t size) = 0;

    [[nodiscard]] virtual bool EndWrite() = 0;
};

class ChannelReader {
protected:
    ChannelReader() noexcept = default;

public:
    virtual ~ChannelReader() noexcept = default;

    ChannelReader(const ChannelReader&) = delete;
    ChannelReader& operator=(const ChannelReader&) = delete;

    ChannelReader(ChannelReader&&) = delete;
    ChannelReader& operator=(ChannelReader&&) = delete;

    template <typename T>
    [[nodiscard]] bool Read(T& value) {
        static_assert(std::is_trivially_copyable_v<T>);

        return Read(&value, sizeof(T));
    }

    [[nodiscard]] virtual bool Read(void* destination, size_t size) = 0;
};

class Channel {
protected:
    Channel() noexcept = default;

public:
    virtual ~Channel() noexcept = default;

    Channel(const Channel&) = delete;
    Channel& operator=(const Channel&) = delete;

    Channel(Channel&&) = delete;
    Channel& operator=(Channel&&) = delete;

    [[nodiscard]] virtual std::string GetRemoteAddress() const = 0;

    virtual void Disconnect() = 0;

    [[nodiscard]] virtual ChannelWriter& GetWriter() = 0;
    [[nodiscard]] virtual ChannelReader& GetReader() = 0;
};

class ChannelServer {
protected:
    ChannelServer() noexcept = default;

public:
    virtual ~ChannelServer() noexcept = default;

    ChannelServer(const ChannelServer&) = delete;
    ChannelServer& operator=(const ChannelServer&) = delete;

    ChannelServer(ChannelServer&&) = delete;
    ChannelServer& operator=(ChannelServer&&) = delete;

    [[nodiscard]] virtual uint16_t GetLocalPort() const = 0;

    [[nodiscard]] virtual std::unique_ptr<Channel> TryAccept() = 0;
    [[nodiscard]] virtual std::unique_ptr<Channel> TryAccept(uint32_t timeoutInMilliseconds) = 0;
};

[[nodiscard]] std::unique_ptr<Channel> TryConnectToLocalChannel(std::string_view name);

[[nodiscard]] std::unique_ptr<Channel> TryConnectToTcpChannel(std::string_view remoteIpAddress,
                                                              uint16_t remotePort,
                                                              uint16_t localPort,
                                                              uint32_t timeoutInMilliseconds);

[[nodiscard]] std::unique_ptr<Channel> TryConnectToUdsChannel(std::string_view name);

[[nodiscard]] std::unique_ptr<ChannelServer> CreateLocalChannelServer(std::string_view name);

[[nodiscard]] std::unique_ptr<ChannelServer> CreateTcpChannelServer(uint16_t port, bool enableRemoteAccess);

[[nodiscard]] std::unique_ptr<ChannelServer> CreateUdsChannelServer(std::string_view name);

}  // namespace DsVeosCoSim
