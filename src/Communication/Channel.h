// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <type_traits>

namespace DsVeosCoSim {

class ChannelWriter {  // NOLINT
public:
    virtual ~ChannelWriter() noexcept = default;

    template <typename T>
    [[nodiscard]] bool Write(const T& value) {
        static_assert(std::is_trivially_copyable_v<T>);

        return Write(&value, sizeof(T));
    }

    [[nodiscard]] virtual bool Write(const void* source, size_t size) = 0;

    [[nodiscard]] virtual bool EndWrite() = 0;
};

class ChannelReader {  // NOLINT
public:
    virtual ~ChannelReader() noexcept = default;

    template <typename T>
    [[nodiscard]] bool Read(T& value) {
        static_assert(std::is_trivially_copyable_v<T>);

        return Read(&value, sizeof(T));
    }

    [[nodiscard]] virtual bool Read(void* destination, size_t size) = 0;
};

class Channel {  // NOLINT
public:
    virtual ~Channel() noexcept = default;

    [[nodiscard]] virtual std::string GetRemoteAddress() const = 0;

    virtual void Disconnect() = 0;

    [[nodiscard]] virtual ChannelWriter& GetWriter() = 0;
    [[nodiscard]] virtual ChannelReader& GetReader() = 0;
};

class ChannelServer {  // NOLINT
public:
    virtual ~ChannelServer() noexcept = default;

    [[nodiscard]] virtual uint16_t GetLocalPort() const = 0;

    [[nodiscard]] virtual std::unique_ptr<Channel> TryAccept() = 0;
    [[nodiscard]] virtual std::unique_ptr<Channel> TryAccept(uint32_t timeoutInMilliseconds) = 0;
};

[[nodiscard]] std::unique_ptr<Channel> TryConnectToLocalChannel(const std::string& name);

[[nodiscard]] std::unique_ptr<Channel> TryConnectToTcpChannel(std::string_view remoteIpAddress,
                                                              uint16_t remotePort,
                                                              uint16_t localPort,
                                                              uint32_t timeoutInMilliseconds);

[[nodiscard]] std::unique_ptr<Channel> TryConnectToUdsChannel(const std::string& name);

[[nodiscard]] std::unique_ptr<ChannelServer> CreateLocalChannelServer(const std::string& name);

[[nodiscard]] std::unique_ptr<ChannelServer> CreateTcpChannelServer(uint16_t port, bool enableRemoteAccess);

[[nodiscard]] std::unique_ptr<ChannelServer> CreateUdsChannelServer(const std::string& name);

}  // namespace DsVeosCoSim
