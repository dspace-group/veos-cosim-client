// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32

#include "NamedMutex.h"

#include <Windows.h>  // NOLINT

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include "CoSimHelper.h"
#include "Handle.h"
#include "OsUtilities.h"

namespace DsVeosCoSim {

namespace {

[[nodiscard]] std::wstring GetFullNamedMutexName(const std::string& name) {
    std::string utf8Name = "Local\\dSPACE.VEOS.CoSim.Mutex.";
    utf8Name.append(name);
    return Utf8ToWide(utf8Name);
}

}  // namespace

NamedMutex::NamedMutex(Handle handle) : _handle(std::move(handle)) {
}

[[nodiscard]] NamedMutex NamedMutex::CreateOrOpen(const std::string& name) {
    const std::wstring fullName = GetFullNamedMutexName(name);
    void* handle = CreateMutexW(nullptr, FALSE, fullName.c_str());  // NOLINT
    if (!handle) {
        std::string message = "Could not create or open mutex '";
        message.append(name);
        message.append("'. ");
        message.append(GetSystemErrorMessage(GetLastWindowsError()));
        throw std::runtime_error(message);
    }

    return NamedMutex(handle);
}

[[nodiscard]] NamedMutex NamedMutex::OpenExisting(const std::string& name) {
    const std::wstring fullName = GetFullNamedMutexName(name);
    void* handle = OpenMutexW(MUTEX_ALL_ACCESS, FALSE, fullName.c_str());  // NOLINT
    if (!handle) {
        std::string message = "Could not open mutex '";
        message.append(name);
        message.append("'. ");
        message.append(GetSystemErrorMessage(GetLastWindowsError()));
        throw std::runtime_error(message);
    }

    return NamedMutex(handle);
}

[[nodiscard]] std::optional<NamedMutex> NamedMutex::TryOpenExisting(const std::string& name) {
    const std::wstring fullName = GetFullNamedMutexName(name);
    void* handle = OpenMutexW(MUTEX_ALL_ACCESS, FALSE, fullName.c_str());  // NOLINT
    if (!handle) {
        return NamedMutex();
    }

    return NamedMutex(handle);
}

void NamedMutex::lock() const {  // NOLINT
    (void)lock(Infinite);
}

[[nodiscard]] bool NamedMutex::lock(const uint32_t milliseconds) const {  // NOLINT
    return _handle.Wait(milliseconds);
}

void NamedMutex::unlock() const {  // NOLINT
    (void)ReleaseMutex(_handle);   // NOLINT
}

}  // namespace DsVeosCoSim

#endif
