// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <cstddef>
#include <type_traits>

namespace DsVeosCoSim {

class ChannelWriter {
protected:
    ChannelWriter() = default;

public:
    virtual ~ChannelWriter() noexcept = default;

    ChannelWriter(const ChannelWriter&) = delete;
    ChannelWriter& operator=(const ChannelWriter&) = delete;

    ChannelWriter(ChannelWriter&&) noexcept = default;
    ChannelWriter& operator=(ChannelWriter&&) noexcept = default;

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
    ChannelReader() = default;

public:
    virtual ~ChannelReader() noexcept = default;

    ChannelReader(const ChannelReader&) = delete;
    ChannelReader& operator=(const ChannelReader&) = delete;

    ChannelReader(ChannelReader&&) noexcept = default;
    ChannelReader& operator=(ChannelReader&&) noexcept = default;

    template <typename T>
    [[nodiscard]] bool Read(T& value) {
        static_assert(std::is_trivially_copyable_v<T>);

        return Read(&value, sizeof(T));
    }

    [[nodiscard]] virtual bool Read(void* destination, size_t size) = 0;
};

class Channel {
protected:
    Channel() = default;

public:
    virtual ~Channel() noexcept = default;

    Channel(const Channel&) = delete;
    Channel& operator=(const Channel&) = delete;

    Channel(Channel&&) noexcept = default;
    Channel& operator=(Channel&&) noexcept = default;

    virtual void Disconnect() = 0;

    [[nodiscard]] virtual ChannelWriter& GetWriter() = 0;
    [[nodiscard]] virtual ChannelReader& GetReader() = 0;
};

}  // namespace DsVeosCoSim
