// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32

#include "NamedMutex.h"

#include <Windows.h>

#include <cstdint>
#include <string>

#include "CoSimHelper.h"
#include "OsUtilities.h"

namespace DsVeosCoSim {

namespace {

[[nodiscard]] std::wstring GetFullNamedMutexName(const std::string& name) {
    return Utf8ToWide("Local\\dSPACE.VEOS.CoSim.Mutex." + name);
}

}  // namespace

NamedMutex::NamedMutex(Handle handle) : _handle(std::move(handle)) {
}

[[nodiscard]] NamedMutex NamedMutex::CreateOrOpen(const std::string& name) {
    const std::wstring fullName = GetFullNamedMutexName(name);
    void* handle = CreateMutexW(nullptr, FALSE, fullName.c_str());
    if (!handle) {
        throw CoSimException("Could not create or open mutex '" + name + "'.", GetLastWindowsError());
    }

    return NamedMutex(handle);
}

[[nodiscard]] NamedMutex NamedMutex::OpenExisting(const std::string& name) {
    const std::wstring fullName = GetFullNamedMutexName(name);
    void* handle = OpenMutexW(MUTEX_ALL_ACCESS, FALSE, fullName.c_str());
    if (!handle) {
        throw CoSimException("Could not open mutex '" + name + "'.", GetLastWindowsError());
    }

    return NamedMutex(handle);
}

[[nodiscard]] std::optional<NamedMutex> NamedMutex::TryOpenExisting(const std::string& name) {
    const std::wstring fullName = GetFullNamedMutexName(name);
    void* handle = OpenMutexW(MUTEX_ALL_ACCESS, FALSE, fullName.c_str());
    if (!handle) {
        return NamedMutex();
    }

    return NamedMutex(handle);
}

void NamedMutex::lock() const {  // NOLINT
    (void)lock(Infinite);
}

[[nodiscard]] bool NamedMutex::lock(uint32_t milliseconds) const {  // NOLINT
    return _handle.Wait(milliseconds);
}

void NamedMutex::unlock() const {  // NOLINT
    (void)ReleaseMutex(_handle);
}

}  // namespace DsVeosCoSim

#endif
