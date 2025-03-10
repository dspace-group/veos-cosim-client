// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#ifdef _WIN32

#include <cstdint>

namespace DsVeosCoSim {

class Handle final {
public:
    Handle() = default;
    Handle(void* handle);  // NOLINT
    ~Handle() noexcept;

    Handle(const Handle&) = delete;
    Handle& operator=(const Handle&) = delete;

    Handle(Handle&&) noexcept;
    Handle& operator=(Handle&&) noexcept;

    operator void*() const noexcept;  // NOLINT

    void Wait() const;
    [[nodiscard]] bool Wait(uint32_t milliseconds) const;

private:
    void* _handle{};
};

[[nodiscard]] bool SignalAndWait(const Handle& toSignal, const Handle& toWait, uint32_t milliseconds);

}  // namespace DsVeosCoSim

#endif
