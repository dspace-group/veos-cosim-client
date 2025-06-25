// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#ifdef _WIN32

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace DsVeosCoSim {

[[maybe_unused]] constexpr uint32_t Infinite = UINT32_MAX;

class Handle final {
public:
    Handle() noexcept = default;
    Handle(void* handle) noexcept;  // NOLINT(google-explicit-constructor, hicpp-explicit-conversions)
    ~Handle() noexcept;

    Handle(const Handle&) = delete;
    Handle& operator=(const Handle&) = delete;

    Handle(Handle&&) noexcept;
    Handle& operator=(Handle&&) noexcept;

    operator void*() const noexcept;  // NOLINT(google-explicit-constructor, hicpp-explicit-conversions)

    void Wait() const;
    [[nodiscard]] bool Wait(uint32_t milliseconds) const;

private:
    void* _handle{};
};

class NamedEvent final {
    NamedEvent(Handle handle, std::string_view name);

public:
    NamedEvent() = default;
    ~NamedEvent() = default;

    NamedEvent(const NamedEvent&) = delete;
    NamedEvent& operator=(const NamedEvent&) = delete;

    NamedEvent(NamedEvent&&) noexcept = default;
    NamedEvent& operator=(NamedEvent&&) noexcept = default;

    [[nodiscard]] static NamedEvent CreateOrOpen(std::string_view name);

    void Set() const;
    void Wait() const;
    [[nodiscard]] bool Wait(uint32_t milliseconds) const;

private:
    Handle _handle;
    std::string _name;
};

class NamedMutex final {
    explicit NamedMutex(Handle handle);

public:
    NamedMutex() noexcept = default;
    ~NamedMutex() noexcept = default;

    NamedMutex(const NamedMutex&) = delete;
    NamedMutex& operator=(const NamedMutex&) = delete;

    NamedMutex(NamedMutex&&) noexcept = default;
    NamedMutex& operator=(NamedMutex&&) noexcept = default;

    [[nodiscard]] static NamedMutex CreateOrOpen(std::string_view name);

    // Small case, so this mutex can directly be used in std::lock_guard
    void lock() const;                                     // NOLINT(readability-identifier-naming)
    [[nodiscard]] bool lock(uint32_t milliseconds) const;  // NOLINT(readability-identifier-naming)
    void unlock() const;                                   // NOLINT(readability-identifier-naming)

private:
    Handle _handle;
};

class SharedMemory final {
    SharedMemory(std::string_view name, size_t size, Handle handle);

public:
    SharedMemory() = default;
    ~SharedMemory() noexcept = default;

    SharedMemory(const SharedMemory&) = delete;
    SharedMemory& operator=(const SharedMemory&) = delete;

    SharedMemory(SharedMemory&&) noexcept;
    SharedMemory& operator=(SharedMemory&&) noexcept;

    [[nodiscard]] static SharedMemory CreateOrOpen(std::string_view name, size_t size);
    [[nodiscard]] static std::optional<SharedMemory> TryOpenExisting(std::string_view name, size_t size);

    [[nodiscard]] void* data() const noexcept;   // NOLINT(readability-identifier-naming)
    [[nodiscard]] size_t size() const noexcept;  // NOLINT(readability-identifier-naming)

private:
    size_t _size{};
    Handle _handle;
    void* _data{};
};

[[nodiscard]] uint32_t GetCurrentProcessId();
[[nodiscard]] bool IsProcessRunning(uint32_t processId);

void SetThreadAffinity(std::string_view name);

[[nodiscard]] std::string GetEnglishErrorMessage(int32_t errorCode);

}  // namespace DsVeosCoSim

#else

#include <cstdint>
#include <string_view>

namespace DsVeosCoSim {

[[maybe_unused]] constexpr uint32_t Infinite = UINT32_MAX;

void SetThreadAffinity(std::string_view name);

}  // namespace DsVeosCoSim

#endif
