// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#ifdef _WIN32

#include <optional>
#include <string>

#include "Handle.h"

namespace DsVeosCoSim {

class NamedEvent final {
    NamedEvent(const std::string& name, Handle handle);

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

    operator Handle&() noexcept;  // NOLINT(google-explicit-constructor, hicpp-explicit-conversions)

    void Set();
    void Wait() const;
    [[nodiscard]] bool Wait(uint32_t milliseconds) const;

private:
    std::string _name;
    Handle _handle;
};

}  // namespace DsVeosCoSim

#endif
