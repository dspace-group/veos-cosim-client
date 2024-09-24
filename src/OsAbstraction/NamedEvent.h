// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#ifdef _WIN32

#include <cstdint>
#include <optional>
#include <string_view>

#include "Handle.h"

namespace DsVeosCoSim {

class NamedEvent final {
    explicit NamedEvent(Handle handle);

public:
    NamedEvent() = default;
    ~NamedEvent() = default;

    NamedEvent(const NamedEvent&) = delete;
    NamedEvent& operator=(const NamedEvent&) = delete;

    NamedEvent(NamedEvent&&) noexcept = default;
    NamedEvent& operator=(NamedEvent&&) noexcept = default;

    [[nodiscard]] static NamedEvent CreateOrOpen(std::string_view name);
    [[nodiscard]] static NamedEvent OpenExisting(std::string_view name);
    [[nodiscard]] static std::optional<NamedEvent> TryOpenExisting(std::string_view name);

    operator Handle&() noexcept;  // NOLINT

    void Set();
    void Wait() const;
    [[nodiscard]] bool Wait(uint32_t milliseconds) const;

private:
    Handle _handle;
};

}  // namespace DsVeosCoSim

#endif
