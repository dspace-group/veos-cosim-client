// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32

#include "NamedEvent.h"

#include <Windows.h>
#include <format>

#include "CoSimHelper.h"
#include "OsUtilities.h"

namespace DsVeosCoSim {

namespace {

[[nodiscard]] std::wstring GetFullNamedEventName(std::string_view name) {
    return Utf8ToWide(std::format("Local\\dSPACE.VEOS.CoSim.Event.{}", name));
}

}  // namespace

NamedEvent::NamedEvent(Handle handle) : _handle(std::move(handle)) {
}

NamedEvent NamedEvent::CreateOrOpen(std::string_view name) {
    std::wstring fullName = GetFullNamedEventName(name);
    void* handle = ::CreateEventW(nullptr, FALSE, FALSE, fullName.c_str());
    if (!handle) {
        throw CoSimException(std::format("Could not create event '{}'.", name), GetLastWindowsError());
    }

    return NamedEvent(handle);
}

NamedEvent NamedEvent::OpenExisting(std::string_view name) {
    std::wstring fullName = GetFullNamedEventName(name);
    void* handle = ::OpenEventW(EVENT_ALL_ACCESS, FALSE, fullName.c_str());
    if (!handle) {
        throw CoSimException(std::format("Could not open event '{}'.", name), GetLastWindowsError());
    }

    return NamedEvent(handle);
}

std::optional<NamedEvent> NamedEvent::TryOpenExisting(std::string_view name) {
    std::wstring fullName = GetFullNamedEventName(name);
    void* handle = ::OpenEventW(EVENT_ALL_ACCESS, FALSE, fullName.c_str());
    if (!handle) {
        return {};
    }

    return NamedEvent(handle);
}

NamedEvent::operator Handle&() noexcept {
    return _handle;
}

void NamedEvent::Set() {
    if (::SetEvent(_handle) == FALSE) {
        throw CoSimException("Could not set event.", GetLastWindowsError());
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
