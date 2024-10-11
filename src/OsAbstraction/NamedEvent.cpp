// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32

#include "NamedEvent.h"

#include <Windows.h>
#include <cstdint>
#include <string>

#include "CoSimHelper.h"
#include "OsUtilities.h"

namespace DsVeosCoSim {

namespace {

[[nodiscard]] std::wstring GetFullNamedEventName(const std::string& name) {
    return Utf8ToWide("Local\\dSPACE.VEOS.CoSim.Event." + name);
}

}  // namespace

NamedEvent::NamedEvent(Handle handle, const std::string& name) : _handle(std::move(handle)), _name(name) {
}

NamedEvent NamedEvent::CreateOrOpen(const std::string& name) {
    std::wstring fullName = GetFullNamedEventName(name);
    void* handle = ::CreateEventW(nullptr, FALSE, FALSE, fullName.c_str());
    if (!handle) {
        throw CoSimException("Could not create event '" + name + "'.", GetLastWindowsError());
    }

    return {handle, name};
}

NamedEvent NamedEvent::OpenExisting(const std::string& name) {
    std::wstring fullName = GetFullNamedEventName(name);
    void* handle = ::OpenEventW(EVENT_ALL_ACCESS, FALSE, fullName.c_str());
    if (!handle) {
        throw CoSimException("Could not open event '" + name + "'.", GetLastWindowsError());
    }

    return {handle, name};
}

std::optional<NamedEvent> NamedEvent::TryOpenExisting(const std::string& name) {
    std::wstring fullName = GetFullNamedEventName(name);
    void* handle = ::OpenEventW(EVENT_ALL_ACCESS, FALSE, fullName.c_str());
    if (!handle) {
        return {};
    }

    return NamedEvent(handle, name);
}

NamedEvent::operator Handle&() noexcept {
    return _handle;
}

void NamedEvent::Set() {
    if (::SetEvent(_handle) == FALSE) {
        throw CoSimException("Could not set event '" + _name + "'.", GetLastWindowsError());
    }
}

void NamedEvent::Wait() const {
    (void)Wait(Infinite);
}

bool NamedEvent::Wait(uint32_t milliseconds) const {
    return _handle.Wait(milliseconds);
}

}  // namespace DsVeosCoSim

#endif
