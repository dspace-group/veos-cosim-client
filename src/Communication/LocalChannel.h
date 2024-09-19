// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#ifdef _WIN32

#include <atomic>
#include <string>
#include <string_view>

#include "Channel.h"
#include "NamedEvent.h"
#include "SharedMemory.h"

namespace DsVeosCoSim {

constexpr uint32_t LockFreeCacheLineBytes = 64;

class LocalChannelBase {
protected:
    LocalChannelBase(std::string_view name, bool isServer);

public:
    virtual ~LocalChannelBase() noexcept;

    LocalChannelBase(const LocalChannelBase&) = delete;
    LocalChannelBase& operator=(const LocalChannelBase&) = delete;

    LocalChannelBase(LocalChannelBase&&) noexcept;
    LocalChannelBase& operator=(LocalChannelBase&&) noexcept;

    void Disconnect() const;

protected:
    [[nodiscard]] bool CheckIfConnectionIsAlive();

    struct Header {
        uint32_t serverPid{};
        uint32_t clientPid{};
        alignas(LockFreeCacheLineBytes) std::atomic<uint32_t> writeIndex;
        alignas(LockFreeCacheLineBytes) std::atomic<uint32_t> readIndex;
    };

    NamedEvent _newDataEvent;
    NamedEvent _newSpaceEvent;
    Header* _header{};
    bool _connectionDetected{};
    alignas(LockFreeCacheLineBytes) uint8_t* _data{};
    alignas(LockFreeCacheLineBytes) uint32_t* _counterpartPid{};
    alignas(LockFreeCacheLineBytes) uint32_t* _ownPid{};

private:
    SharedMemory _sharedMemory;
    uint32_t _detectionCounter{};
};

class LocalChannelWriter final : public LocalChannelBase, public ChannelWriter {
public:
    LocalChannelWriter(std::string_view name, bool isServer);
    ~LocalChannelWriter() noexcept override = default;

    LocalChannelWriter(const LocalChannelWriter&) = delete;
    LocalChannelWriter& operator=(const LocalChannelWriter&) = delete;

    LocalChannelWriter(LocalChannelWriter&&) noexcept = default;
    LocalChannelWriter& operator=(LocalChannelWriter&&) noexcept = default;

    [[nodiscard]] bool Write(const void* source, size_t size) override;

    [[nodiscard]] bool EndWrite() override;

private:
    [[nodiscard]] bool WaitForFreeSpace(uint32_t& currentSize);

    uint32_t _writeIndex{};
    uint32_t _maskedWriteIndex{};
};

class LocalChannelReader final : public LocalChannelBase, public ChannelReader {
public:
    LocalChannelReader(std::string_view name, bool isServer);
    ~LocalChannelReader() noexcept override = default;

    LocalChannelReader(const LocalChannelReader&) = delete;
    LocalChannelReader& operator=(const LocalChannelReader&) = delete;

    LocalChannelReader(LocalChannelReader&&) noexcept = default;
    LocalChannelReader& operator=(LocalChannelReader&&) noexcept = default;

    [[nodiscard]] bool Read(void* destination, size_t size) override;

private:
    [[nodiscard]] bool BeginRead(uint32_t& currentSize);

    uint32_t _readIndex{};
    uint32_t _maskedReadIndex{};
};

class LocalChannel final : public Channel {
public:
    LocalChannel(std::string_view name, bool isServer);
    ~LocalChannel() noexcept override = default;

    LocalChannel(const LocalChannel&) = delete;
    LocalChannel& operator=(const LocalChannel&) = delete;

    LocalChannel(LocalChannel&&) noexcept = default;
    LocalChannel& operator=(LocalChannel&&) noexcept = default;

    void Disconnect() override;

    [[nodiscard]] ChannelWriter& GetWriter() override;
    [[nodiscard]] ChannelReader& GetReader() override;

private:
    LocalChannelWriter _writer;
    LocalChannelReader _reader;
};

[[nodiscard]] std::optional<LocalChannel> TryConnectToLocalChannel(std::string_view name);

class LocalChannelServer final {
public:
    explicit LocalChannelServer(std::string_view name);
    ~LocalChannelServer() noexcept = default;

    LocalChannelServer(const LocalChannelServer&) = delete;
    LocalChannelServer& operator=(const LocalChannelServer&) = delete;

    LocalChannelServer(LocalChannelServer&&) = delete;
    LocalChannelServer& operator=(LocalChannelServer&&) = delete;

    [[nodiscard]] std::optional<LocalChannel> TryAccept();

private:
    std::string _name;
    SharedMemory _sharedMemory;

    std::atomic<int32_t>* _counter{};
    int32_t _lastCounter{};
};

}  // namespace DsVeosCoSim

#endif
