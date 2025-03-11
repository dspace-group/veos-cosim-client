// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32

#include "Handle.h"

#include <Windows.h>  // NOLINT

#include <cstdint>
#include <string>

#include "CoSimHelper.h"
#include "CoSimTypes.h"
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
        case WAIT_FAILED:
            throw CoSimException("Could not wait for handle. " + GetSystemErrorMessage(GetLastWindowsError()));
        default:
            throw CoSimException("Could not wait for handle. Invalid result: " + std::to_string(result) + ".");
    }
}

[[nodiscard]] bool SignalAndWait(const Handle& toSignal, const Handle& toWait, const uint32_t milliseconds) {
    switch (const DWORD result = SignalObjectAndWait(toSignal, toWait, milliseconds, FALSE)) {  // NOLINT
        case WAIT_OBJECT_0:
            return true;
        case WAIT_ABANDONED:
        case WAIT_TIMEOUT:
            return false;
        case WAIT_FAILED:
            throw CoSimException("Could not signal and wait for handle. " +
                                 GetSystemErrorMessage(GetLastWindowsError()));
        default:
            throw CoSimException("Could not signal and wait for handle. Invalid result: " + std::to_string(result) +
                                 ".");
    }
}

}  // namespace DsVeosCoSim

#endif
