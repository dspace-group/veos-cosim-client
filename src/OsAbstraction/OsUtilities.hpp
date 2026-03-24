// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#ifdef _WIN32

#include <atomic>

#include "Result.hpp"

#endif

namespace DsVeosCoSim {

[[maybe_unused]] constexpr uint32_t Infinite = UINT32_MAX;

#ifdef _WIN32

class Handle final {
public:
    using handle_t = void*;

    Handle() = default;
    explicit Handle(handle_t handle) : _handle(handle) {
    }

    ~Handle() noexcept {
        Reset();
    }

    Handle(const Handle&) = delete;
    Handle& operator=(const Handle&) = delete;

    Handle(Handle&& other) noexcept : _handle(other.Release()) {
    }

    Handle& operator=(Handle&& other) noexcept {
        if (this != &other) {
            Reset(other.Release());
        }

        return *this;
    }

    [[nodiscard]] handle_t Get() const noexcept {
        return _handle;
    }

    [[nodiscard]] handle_t Release() noexcept {
        return std::exchange(_handle, nullptr);
    }

    void Reset(void* newHandle = nullptr);

    [[nodiscard]] bool IsValid() const;

    [[nodiscard]] Result Wait() const;
    [[nodiscard]] Result Wait(uint32_t milliseconds) const;

private:
    void* _handle{};
};

class NamedEvent final {
    explicit NamedEvent(Handle handle);

public:
    NamedEvent() = default;
    ~NamedEvent() noexcept = default;

    NamedEvent(const NamedEvent&) = delete;
    NamedEvent& operator=(const NamedEvent&) = delete;

    NamedEvent(NamedEvent&&) noexcept = default;
    NamedEvent& operator=(NamedEvent&&) noexcept = default;

    [[nodiscard]] static Result CreateOrOpen(std::string_view name, NamedEvent& namedEvent);

    void Close();
    [[nodiscard]] Result Set() const;
    [[nodiscard]] Result Wait() const;
    [[nodiscard]] Result Wait(uint32_t milliseconds) const;

    [[nodiscard]] bool IsValid() const;

private:
    Handle _handle;
};

class NamedLock final {
    explicit NamedLock(Handle handle);

public:
    NamedLock() = default;
    ~NamedLock() noexcept;

    NamedLock(const NamedLock&) = delete;
    NamedLock& operator=(const NamedLock&) = delete;

    NamedLock(NamedLock&&) noexcept = default;
    NamedLock& operator=(NamedLock&&) noexcept = default;

    [[nodiscard]] static Result Create(std::string_view name, NamedLock& namedMutex);

private:
    Handle _handle;
};

class SharedMemory final {
    SharedMemory(Handle handle, size_t size, void* data);

public:
    SharedMemory() = default;
    ~SharedMemory() noexcept;

    SharedMemory(const SharedMemory&) = delete;
    SharedMemory& operator=(const SharedMemory&) = delete;

    SharedMemory(SharedMemory&& other) noexcept;
    SharedMemory& operator=(SharedMemory&& other) noexcept;

    [[nodiscard]] static Result CreateOrOpen(std::string_view name, size_t size, SharedMemory& sharedMemory);
    [[nodiscard]] static Result TryOpenExisting(std::string_view name, size_t size, SharedMemory& sharedMemory);

    void Close();

    template <typename T>
    [[nodiscard]] T* As() const {
        return static_cast<T*>(_data);
    }

    [[nodiscard]] uint8_t* GetData() const;
    [[nodiscard]] size_t GetSize() const;

    [[nodiscard]] bool IsValid() const;

private:
    Handle _handle;
    size_t _size{};
    void* _data{};
};

class ShmPipePart {
    static constexpr size_t LockFreeCacheLineBytes = 64;

    struct Header {
        std::atomic<uint32_t> serverPid{};
        std::atomic<uint32_t> clientPid{};
        alignas(LockFreeCacheLineBytes) std::atomic<uint32_t> writeIndex{};
        alignas(LockFreeCacheLineBytes) std::atomic<uint32_t> readIndex{};
    };

    ShmPipePart(NamedEvent newDataEvent, NamedEvent newSpaceEvent, SharedMemory sharedMemory, bool isWriter, bool isServer);

public:
    static constexpr uint32_t PipeBufferSize = 65536;

    ShmPipePart() = default;
    ~ShmPipePart() noexcept;

    ShmPipePart(const ShmPipePart&) = delete;
    ShmPipePart& operator=(const ShmPipePart&) = delete;

    ShmPipePart(ShmPipePart&& other) noexcept = default;
    ShmPipePart& operator=(ShmPipePart&& other) noexcept = default;

    [[nodiscard]] static Result Create(std::string_view name, bool isWriter, bool isServer, ShmPipePart& pipe);

    void Disconnect() const;

    [[nodiscard]] Result Read(void* destination, size_t size, size_t& receivedSize);
    [[nodiscard]] Result Write(const void* source, size_t size);

    [[nodiscard]] bool IsConnected() const;

private:
    [[nodiscard]] static uint32_t MaskIndex(uint32_t index);
    [[nodiscard]] static uint32_t GetAvailableSpace(const Header& header);
    [[nodiscard]] static uint32_t GetAvailableData(const Header& header);
    [[nodiscard]] Result EnsureConnected();
    [[nodiscard]] Result WaitForSpace();
    [[nodiscard]] Result WaitForData();
    [[nodiscard]] Result CheckIfConnectionIsAlive();
    void SetOwnPid(uint32_t pid) const;
    [[nodiscard]] uint32_t GetOwnPid() const;
    [[nodiscard]] uint32_t GetCounterPartPid() const;

    NamedEvent _newDataEvent;
    NamedEvent _newSpaceEvent;
    SharedMemory _sharedMemory;
    Handle _counterPartProcess{};
    uint32_t _detectionCounter{};
    uint32_t _spinCount{};
    bool _isWriter{};
    bool _isServer{};
};

class ShmPipeListener;

class ShmPipeClient final {
    static constexpr char ServerToClientPostFix[] = "ServerToClient";
    static constexpr char ClientToServerPostFix[] = "ClientToServer";

    friend class ShmPipeListener;

    ShmPipeClient(ShmPipePart writer, ShmPipePart reader);

public:
    ShmPipeClient() = default;
    ~ShmPipeClient() noexcept = default;

    ShmPipeClient(const ShmPipeClient&) = delete;
    ShmPipeClient& operator=(const ShmPipeClient&) = delete;

    ShmPipeClient(ShmPipeClient&& other) noexcept = default;
    ShmPipeClient& operator=(ShmPipeClient&& other) noexcept = default;

    [[nodiscard]] static Result TryConnect(std::string_view name, ShmPipeClient& client);

    void Disconnect();

    [[nodiscard]] Result Receive(void* destination, size_t size, size_t& receivedSize);
    [[nodiscard]] Result Send(const void* source, size_t size);

    [[nodiscard]] bool IsConnected() const;

private:
    ShmPipePart _writer{};
    ShmPipePart _reader{};
};

class ShmPipeListener final {
    ShmPipeListener(std::string name, SharedMemory sharedMemory);

public:
    ShmPipeListener() = default;
    ~ShmPipeListener() noexcept = default;

    ShmPipeListener(const ShmPipeListener&) = delete;
    ShmPipeListener& operator=(const ShmPipeListener&) = delete;

    ShmPipeListener(ShmPipeListener&& other) noexcept = default;
    ShmPipeListener& operator=(ShmPipeListener&& other) noexcept = default;

    [[nodiscard]] static Result Create(const std::string& name, ShmPipeListener& listener);

    void Stop();

    [[nodiscard]] Result TryAccept(ShmPipeClient& client);

    [[nodiscard]] bool IsRunning() const;

private:
    std::string _name;
    SharedMemory _sharedMemory;
    uint32_t _lastCounter{};
};

[[nodiscard]] bool IsProcessRunning(const Handle& processHandle);
[[nodiscard]] uint32_t GetCurrentProcessIdCached();

#endif

void SetThreadAffinity(std::string_view name);

}  // namespace DsVeosCoSim
