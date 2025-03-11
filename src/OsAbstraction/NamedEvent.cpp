// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32

#include "NamedEvent.h"

#include <Windows.h>  // NOLINT

#include <cstdint>
#include <optional>
#include <string>
#include <utility>

#include "CoSimHelper.h"
#include "CoSimTypes.h"
#include "Handle.h"
#include "OsUtilities.h"

namespace DsVeosCoSim {

namespace {

[[nodiscard]] std::wstring GetFullNamedEventName(const std::string& name) {
    return Utf8ToWide("Local\\dSPACE.VEOS.CoSim.Event." + name);
}

}  // namespace

NamedEvent::NamedEvent(Handle handle, const std::string& name) : _handle(std::move(handle)), _name(name) {
}

[[nodiscard]] NamedEvent NamedEvent::CreateOrOpen(const std::string& name) {
    const std::wstring fullName = GetFullNamedEventName(name);
    void* handle = CreateEventW(nullptr, FALSE, FALSE, fullName.c_str());  // NOLINT
    if (!handle) {
        throw CoSimException("Could not create event '" + name + "'. " + GetSystemErrorMessage(GetLastWindowsError()));
    }

    return {handle, name};
}

[[nodiscard]] NamedEvent NamedEvent::OpenExisting(const std::string& name) {
    const std::wstring fullName = GetFullNamedEventName(name);
    void* handle = OpenEventW(EVENT_ALL_ACCESS, FALSE, fullName.c_str());  // NOLINT
    if (!handle) {
        throw CoSimException("Could not open event '" + name + "'. " + GetSystemErrorMessage(GetLastWindowsError()));
    }

    return {handle, name};
}

[[nodiscard]] std::optional<NamedEvent> NamedEvent::TryOpenExisting(const std::string& name) {
    const std::wstring fullName = GetFullNamedEventName(name);
    void* handle = OpenEventW(EVENT_ALL_ACCESS, FALSE, fullName.c_str());
    if (!handle) {
        return {};
    }

    return NamedEvent(handle, name);
}

NamedEvent::operator Handle&() noexcept {
    return _handle;
}

void NamedEvent::Set() const {
    const BOOL result = SetEvent(_handle);  // NOLINT
    if (result == FALSE) {
        throw CoSimException("Could not set event '" + _name + "'. " + GetSystemErrorMessage(GetLastWindowsError()));
    }
}

void NamedEvent::Wait() const {
    (void)Wait(Infinite);
}

[[nodiscard]] bool NamedEvent::Wait(const uint32_t milliseconds) const {
    return _handle.Wait(milliseconds);
}

}  // namespace DsVeosCoSim

#endif
