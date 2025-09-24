// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#ifdef _WIN32

#include <cstdint>
#include <optional>
#include <string>

#include "DsVeosCoSim/CoSimTypes.h"

namespace DsVeosCoSim {

[[maybe_unused]] constexpr uint32_t Infinite = UINT32_MAX;

class Handle final {
public:
    Handle() = default;
    Handle(void* handle);
    ~Handle() noexcept;

    Handle(const Handle&) = delete;
    Handle& operator=(const Handle&) = delete;

    Handle(Handle&& other) noexcept;
    Handle& operator=(Handle&& other) noexcept;

    operator void*() const;

    [[nodiscard]] Result Wait() const;
    [[nodiscard]] Result Wait(uint32_t milliseconds, bool& success) const;

private:
    void* _handle{};
};

class NamedEvent final {
    explicit NamedEvent(Handle handle);

public:
    NamedEvent() = default;
    ~NamedEvent() = default;

    NamedEvent(const NamedEvent&) = delete;
    NamedEvent& operator=(const NamedEvent&) = delete;

    NamedEvent(NamedEvent&&) = default;
    NamedEvent& operator=(NamedEvent&&) = default;

    [[nodiscard]] static Result CreateOrOpen(const std::string& name, NamedEvent& namedEvent);

    [[nodiscard]] Result Set() const;
    [[nodiscard]] Result Wait() const;
    [[nodiscard]] Result Wait(uint32_t milliseconds, bool& success) const;

private:
    Handle _handle;
};

class NamedMutex final {
    explicit NamedMutex(Handle handle);

public:
    NamedMutex() = default;
    ~NamedMutex() noexcept;

    NamedMutex(const NamedMutex&) = delete;
    NamedMutex& operator=(const NamedMutex&) = delete;

    NamedMutex(NamedMutex&&) = default;
    NamedMutex& operator=(NamedMutex&&) = default;

    [[nodiscard]] static Result CreateOrOpen(const std::string& name, NamedMutex& namedMutex);

    [[nodiscard]] Result Lock();
    void Unlock();

private:
    Handle _handle;
    bool _isLocked{};
};

class SharedMemory final {
    SharedMemory(Handle handle, size_t size, void* data);

public:
    SharedMemory() = default;
    ~SharedMemory() = default;

    SharedMemory(const SharedMemory&) = delete;
    SharedMemory& operator=(const SharedMemory&) = delete;

    SharedMemory(SharedMemory&& other) noexcept;
    SharedMemory& operator=(SharedMemory&& other) noexcept;

    [[nodiscard]] static Result CreateOrOpen(const std::string& name, size_t size, SharedMemory& sharedMemory);
    [[nodiscard]] static Result TryOpenExisting(const std::string& name,
                                                size_t size,
                                                std::optional<SharedMemory>& sharedMemory);

    [[nodiscard]] void* GetData() const;
    [[nodiscard]] size_t GetSize() const;

private:
    Handle _handle;
    size_t _size{};
    void* _data{};
};

[[nodiscard]] uint32_t GetCurrentProcessId();
[[nodiscard]] bool IsProcessRunning(uint32_t processId);

void SetThreadAffinity(const std::string& name);

[[nodiscard]] std::string GetEnglishErrorMessage(int32_t errorCode);

}  // namespace DsVeosCoSim

#else

#include <cstdint>
#include <string>

namespace DsVeosCoSim {

[[maybe_unused]] constexpr uint32_t Infinite = UINT32_MAX;

void SetThreadAffinity(const std::string& name);

}  // namespace DsVeosCoSim

#endif
