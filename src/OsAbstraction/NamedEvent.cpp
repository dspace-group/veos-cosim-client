// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32

#include "NamedEvent.h"

#include <Windows.h>
#include <fmt/format.h>

#include "OsUtilities.h"

namespace DsVeosCoSim {

namespace {

[[nodiscard]] std::string GetFullNamedEventName(const std::string& name) {
    return fmt::format("Local\\dSPACE.VEOS.CoSim.Event.{}", name);
}

}  // namespace

NamedEvent::NamedEvent(const std::string& name, Handle handle) : _name(name), _handle(std::move(handle)) {
}

NamedEvent NamedEvent::CreateOrOpen(const std::string& name) {
    std::string fullName = GetFullNamedEventName(name);
    void* handle = ::CreateEventA(nullptr, FALSE, FALSE, fullName.c_str());
    if (!handle) {
        throw OsAbstractionException(fmt::format("Could not create event '{}'.", name), GetLastWindowsError());
    }

    return {fullName, handle};
}

NamedEvent NamedEvent::OpenExisting(const std::string& name) {
    std::string fullName = GetFullNamedEventName(name);
    void* handle = ::OpenEventA(EVENT_ALL_ACCESS, FALSE, fullName.c_str());
    if (!handle) {
        throw OsAbstractionException(fmt::format("Could not open event '{}'.", name), GetLastWindowsError());
    }

    return {fullName, handle};
}

std::optional<NamedEvent> NamedEvent::TryOpenExisting(const std::string& name) {
    std::string fullName = GetFullNamedEventName(name);
    void* handle = ::OpenEventA(EVENT_ALL_ACCESS, FALSE, fullName.c_str());
    if (!handle) {
        return {};
    }

    return NamedEvent(fullName, handle);
}

NamedEvent::operator Handle&() noexcept {
    return _handle;
}

void NamedEvent::Set() {
    if (::SetEvent(_handle) == FALSE) {
        throw OsAbstractionException(fmt::format("Could not set event '{}'.", _name), GetLastWindowsError());
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
