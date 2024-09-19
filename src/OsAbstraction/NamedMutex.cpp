// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32

#include "NamedMutex.h"

#include <Windows.h>
#include <fmt/format.h>

#include "CoSimHelper.h"
#include "OsUtilities.h"

namespace DsVeosCoSim {

namespace {

[[nodiscard]] std::wstring GetFullNamedMutexName(std::string_view name) {
    return Utf8ToWide(fmt::format("Local\\dSPACE.VEOS.CoSim.Mutex.{}", name));
}

}  // namespace

NamedMutex::NamedMutex(Handle handle) : _handle(std::move(handle)) {
}

NamedMutex NamedMutex::CreateOrOpen(std::string_view name) {
    std::wstring fullName = GetFullNamedMutexName(name);
    void* handle = ::CreateMutexW(nullptr, FALSE, fullName.c_str());
    if (!handle) {
        throw CoSimException(fmt::format("Could not create or open mutex '{}'.", name), GetLastWindowsError());
    }

    return NamedMutex(handle);
}

NamedMutex NamedMutex::OpenExisting(std::string_view name) {
    std::wstring fullName = GetFullNamedMutexName(name);
    void* handle = ::OpenMutexW(MUTEX_ALL_ACCESS, FALSE, fullName.c_str());
    if (!handle) {
        throw CoSimException(fmt::format("Could not open mutex '{}'", name), GetLastWindowsError());
    }

    return NamedMutex(handle);
}

std::optional<NamedMutex> NamedMutex::TryOpenExisting(std::string_view name) {
    std::wstring fullName = GetFullNamedMutexName(name);
    void* handle = ::OpenMutexW(MUTEX_ALL_ACCESS, FALSE, fullName.c_str());
    if (!handle) {
        return NamedMutex();
    }

    return NamedMutex(handle);
}

void NamedMutex::lock() const {
    (void)lock(Infinite);
}

bool NamedMutex::lock(uint32_t milliseconds) const {
    return _handle.Wait(milliseconds);
}

void NamedMutex::unlock() const {
    (void)::ReleaseMutex(_handle);
}

}  // namespace DsVeosCoSim

#endif
