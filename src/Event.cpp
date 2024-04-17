// Copyright dSPACE GmbH. All rights reserved.

#include "Event.h"

#ifdef _WIN32

#include <Windows.h>
#undef min
#undef max

#include "Logger.h"

namespace DsVeosCoSim {

namespace {

}  // namespace

Event::Event(const std::string& name) : _name(name) {
}

Event::~Event() noexcept {
    (void)CloseHandle(_handle);
}

Result Event::Create() {
    std::string fullName = "Local\\dSPACE.VEOS.CoSim.Event." + _name;
    _handle = CreateEventA(nullptr, 0, 0, fullName.c_str());
    if (!_handle) {
        LogError("Could not create event '" + _name + "'. Error code: " + std::to_string(GetLastError()) + ".");
        return Result::Error;
    }

    return Result::Ok;
}

Result Event::Set() {
    if (SetEvent(_handle) == 0) {
        LogError("Could not set event '" + _name + "'. Error code: " + std::to_string(GetLastError()) + ".");
        return Result::Error;
    }

    return Result::Ok;
}

Result Event::Wait() {
    if (WaitForSingleObject(_handle, INFINITE) != WAIT_OBJECT_0) {
        LogError("Could not wait for event '" + _name + "'. Error code: " + std::to_string(GetLastError()) + ".");
        return Result::Error;
    }

    return Result::Ok;
}

Result Event::Wait(int milliseconds) {
    if (WaitForSingleObject(_handle, milliseconds) != WAIT_OBJECT_0) {
        LogError("Could not wait for event '" + _name + "'. Error code: " + std::to_string(GetLastError()) + ".");
        return Result::Error;
    }

    return Result::Ok;
}

}  // namespace DsVeosCoSim

#endif
