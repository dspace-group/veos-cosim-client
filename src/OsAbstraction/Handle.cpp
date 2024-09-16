// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32

#include "Handle.h"

#include <Windows.h>
#include <fmt/format.h>

#include "OsUtilities.h"

namespace DsVeosCoSim {

Handle::Handle(void* handle) : _handle(handle) {
}

Handle::~Handle() noexcept {
    (void)::CloseHandle(_handle);
}

Handle::Handle(Handle&& other) noexcept {
    _handle = other._handle;

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

bool Handle::Wait(uint32_t milliseconds) const {
    DWORD result = ::WaitForSingleObject(_handle, milliseconds);
    switch (result) {
        case WAIT_OBJECT_0:
            return true;
        case WAIT_ABANDONED:
        case WAIT_TIMEOUT:
            return false;
        case WAIT_FAILED:
            throw OsAbstractionException("Could not wait for handle.", GetLastWindowsError());
        default:
            throw OsAbstractionException(fmt::format("Could not wait for handle. Invalid result: {}.", result));
    }
}

bool SignalAndWait(const Handle& toSignal, const Handle& toWait, uint32_t milliseconds) {
    DWORD result = ::SignalObjectAndWait(toSignal, toWait, milliseconds, FALSE);
    switch (result) {
        case WAIT_OBJECT_0:
            return true;
        case WAIT_ABANDONED:
        case WAIT_TIMEOUT:
            return false;
        case WAIT_FAILED:
            throw OsAbstractionException("Could not signal and wait for handle.", GetLastWindowsError());
        default:
            throw OsAbstractionException(
                fmt::format("Could not signal and wait for handle. Invalid result: {}.", result));
    }
}

}  // namespace DsVeosCoSim

#endif
