// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#ifdef _WIN32

#include <cstdint>
#include <optional>
#include <string>

#include "Handle.h"

namespace DsVeosCoSim {

class NamedEvent final {
    NamedEvent(Handle handle, const std::string& name);

public:
    NamedEvent() = default;
    ~NamedEvent() = default;

    NamedEvent(const NamedEvent&) = delete;
    NamedEvent& operator=(const NamedEvent&) = delete;

    NamedEvent(NamedEvent&&) noexcept = default;
    NamedEvent& operator=(NamedEvent&&) noexcept = default;

    [[nodiscard]] static NamedEvent CreateOrOpen(const std::string& name);
    [[nodiscard]] static NamedEvent OpenExisting(const std::string& name);
    [[nodiscard]] static std::optional<NamedEvent> TryOpenExisting(const std::string& name);

    operator Handle&() noexcept;  // NOLINT

    void Set();
    void Wait() const;
    [[nodiscard]] bool Wait(uint32_t milliseconds) const;

private:
    Handle _handle;
    std::string _name;
};

}  // namespace DsVeosCoSim

#endif
