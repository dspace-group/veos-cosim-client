// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#ifdef _WIN32

#include <cstdint>
#include <optional>
#include <string>

#include "Handle.h"

namespace DsVeosCoSim {

class NamedMutex final {
    explicit NamedMutex(Handle handle);

public:
    NamedMutex() = default;
    ~NamedMutex() noexcept = default;

    NamedMutex(const NamedMutex&) = delete;
    NamedMutex& operator=(const NamedMutex&) = delete;

    NamedMutex(NamedMutex&&) noexcept = default;
    NamedMutex& operator=(NamedMutex&&) noexcept = default;

    [[nodiscard]] static NamedMutex CreateOrOpen(const std::string& name);
    [[nodiscard]] static NamedMutex OpenExisting(const std::string& name);
    [[nodiscard]] static std::optional<NamedMutex> TryOpenExisting(const std::string& name);

    // Small case, so this mutex can directly be used in std::lock_guard
    void lock() const;                                     // NOLINT(readability-identifier-naming)
    [[nodiscard]] bool lock(uint32_t milliseconds) const;  // NOLINT(readability-identifier-naming)
    void unlock() const;                                   // NOLINT(readability-identifier-naming)

private:
    Handle _handle;
};

}  // namespace DsVeosCoSim

#endif
