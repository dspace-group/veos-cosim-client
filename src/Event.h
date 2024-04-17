// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#ifdef _WIN32

#include <string>

#include "CoSimTypes.h"

namespace DsVeosCoSim {

class Event {
public:
    explicit Event(const std::string& name);
    ~Event() noexcept;

    Event(const Event&) = delete;
    Event& operator=(const Event&) = delete;

    Event(const Event&&) = delete;
    Event& operator=(const Event&&) = delete;

    [[nodiscard]] Result Create();

    [[nodiscard]] Result Set();
    [[nodiscard]] Result Wait();
    [[nodiscard]] Result Wait(int milliseconds);

private:
    std::string _name;
    void* _handle{};
};

}  // namespace DsVeosCoSim

#endif
