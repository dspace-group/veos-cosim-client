// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#ifdef _WIN32

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>  // IWYU pragma: keep

namespace DsVeosCoSim {

constexpr uint32_t Infinite = UINT32_MAX;  // NOLINT

[[nodiscard]] uint32_t GetCurrentProcessId();

[[nodiscard]] bool IsProcessRunning(uint32_t processId);

class NamedEvent {
protected:
    NamedEvent() = default;

public:
    virtual ~NamedEvent() noexcept = default;

    NamedEvent(const NamedEvent&) = delete;
    NamedEvent& operator=(const NamedEvent&) = delete;

    NamedEvent(NamedEvent&&) = delete;
    NamedEvent& operator=(NamedEvent&&) = delete;

    virtual void Set() const = 0;
    virtual void Wait() const = 0;
    [[nodiscard]] virtual bool Wait(uint32_t milliseconds) const = 0;
};

[[nodiscard]] std::unique_ptr<NamedEvent> CreateOrOpenNamedEvent(const std::string& name);
[[nodiscard]] std::unique_ptr<NamedEvent> OpenExistingNamedEvent(const std::string& name);
[[nodiscard]] std::unique_ptr<NamedEvent> TryOpenExistingNamedEvent(const std::string& name);

class NamedMutex {
protected:
    NamedMutex() = default;

public:
    virtual ~NamedMutex() noexcept = default;

    NamedMutex(const NamedMutex&) = delete;
    NamedMutex& operator=(const NamedMutex&) = delete;

    NamedMutex(NamedMutex&&) = delete;
    NamedMutex& operator=(NamedMutex&&) = delete;

    // Small case, so this mutex can directly be used in std::lock_guard
    virtual void lock() const = 0;                                     // NOLINT
    [[nodiscard]] virtual bool lock(uint32_t milliseconds) const = 0;  // NOLINT
    virtual void unlock() const = 0;                                   // NOLINT
};

[[nodiscard]] std::unique_ptr<NamedMutex> CreateOrOpenNamedMutex(const std::string& name);
[[nodiscard]] std::unique_ptr<NamedMutex> OpenExistingNamedMutex(const std::string& name);
[[nodiscard]] std::unique_ptr<NamedMutex> TryOpenExistingNamedMutex(const std::string& name);

class SharedMemory {
protected:
    SharedMemory() = default;

public:
    virtual ~SharedMemory() noexcept = default;

    SharedMemory(const SharedMemory&) = delete;
    SharedMemory& operator=(const SharedMemory&) = delete;

    SharedMemory(SharedMemory&&) = delete;
    SharedMemory& operator=(SharedMemory&&) = delete;

    // Small case, so it has the same interface as std::vector
    [[nodiscard]] virtual void* data() const noexcept = 0;   // NOLINT
    [[nodiscard]] virtual size_t size() const noexcept = 0;  // NOLINT
};

[[nodiscard]] std::unique_ptr<SharedMemory> CreateOrOpenSharedMemory(const std::string& name, size_t size);
[[nodiscard]] std::unique_ptr<SharedMemory> OpenExistingSharedMemory(const std::string& name, size_t size);
[[nodiscard]] std::unique_ptr<SharedMemory> TryOpenExistingSharedMemory(const std::string& name, size_t size);

}  // namespace DsVeosCoSim

#endif
