// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32

#include "Handle.h"

#include <Windows.h>  // NOLINT

#include <cstdint>
#include <stdexcept>
#include <string>

#include "CoSimHelper.h"
#include "OsUtilities.h"

namespace DsVeosCoSim {

Handle::Handle(void* handle) : _handle(handle) {
}

Handle::~Handle() noexcept {
    (void)CloseHandle(_handle);  // NOLINT
}

Handle::Handle(Handle&& other) noexcept : _handle(other._handle) {
    other._handle = {};
}

Handle& Handle::operator=(Handle&& other) noexcept {
    _handle = other._handle;

    other._handle = {};

    return *this;
}

Handle::operator void*() const noexcept {
    return _handle;
}

void Handle::Wait() const {
    (void)Wait(Infinite);
}

[[nodiscard]] bool Handle::Wait(const uint32_t milliseconds) const {
    switch (const DWORD result = WaitForSingleObject(_handle, milliseconds)) {  // NOLINT
        case WAIT_OBJECT_0:
            return true;
        case WAIT_ABANDONED:
        case WAIT_TIMEOUT:
            return false;
        case WAIT_FAILED: {
            std::string message = "Could not wait for handle. ";
            message.append(GetSystemErrorMessage(GetLastWindowsError()));
            throw std::runtime_error(message);
        }
        default: {
            std::string message = "Could not wait for handle. Invalid result: ";
            message.append(std::to_string(result));
            message.append(".");
            throw std::runtime_error(message);
        }
    }
}

}  // namespace DsVeosCoSim

#endif
