// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32

#include "SharedMemory.h"

#include <Windows.h>  // NOLINT

#include <cstddef>
#include <optional>
#include <string>
#include <utility>

#include "CoSimHelper.h"
#include "DsVeosCoSim/CoSimTypes.h"
#include "Handle.h"
#include "OsUtilities.h"

namespace DsVeosCoSim {

namespace {

[[nodiscard]] std::wstring GetFullSharedMemoryName(const std::string& name) {
    return Utf8ToWide("Local\\dSPACE.VEOS.CoSim.SharedMemory." + name);
}

}  // namespace

SharedMemory::SharedMemory(const std::string& name, const size_t size, Handle handle)
    : _size(size),
      _handle(std::move(handle)),
      _data(MapViewOfFile(_handle, FILE_MAP_ALL_ACCESS, 0, 0, _size)) {  // NOLINT
    if (!_data) {
        (void)CloseHandle(_handle);  // NOLINT
        throw CoSimException("Could not map view of shared memory '" + name + "'. " +
                             GetSystemErrorMessage(GetLastWindowsError()));
    }
}

SharedMemory::SharedMemory(SharedMemory&& sharedMemory) noexcept
    : _size(sharedMemory._size), _handle(std::move(sharedMemory._handle)), _data(sharedMemory._data) {
    sharedMemory._size = {};
    sharedMemory._data = {};
}

SharedMemory& SharedMemory::operator=(SharedMemory&& sharedMemory) noexcept {
    _size = sharedMemory._size;
    _handle = std::move(sharedMemory._handle);
    _data = sharedMemory._data;

    sharedMemory._size = {};
    sharedMemory._data = {};

    return *this;
}

[[nodiscard]] SharedMemory SharedMemory::CreateOrOpen(const std::string& name, size_t size) {
    const std::wstring fullName = GetFullSharedMemoryName(name);
    constexpr DWORD sizeHigh{};                              // NOLINT
    const auto sizeLow = static_cast<DWORD>(size);           // NOLINT
    void* handle = CreateFileMappingW(INVALID_HANDLE_VALUE,  // NOLINT
                                      nullptr,
                                      PAGE_READWRITE,
                                      sizeHigh,
                                      sizeLow,
                                      fullName.c_str());
    if (!handle) {
        throw CoSimException("Could not create or open shared memory '" + name + "'. " +
                             GetSystemErrorMessage(GetLastWindowsError()));
    }

    return {name, size, handle};
}

[[nodiscard]] SharedMemory SharedMemory::OpenExisting(const std::string& name, size_t size) {
    const std::wstring fullName = GetFullSharedMemoryName(name);
    void* handle = OpenFileMappingW(FILE_MAP_WRITE, FALSE, fullName.c_str());  // NOLINT
    if (!handle) {
        throw CoSimException("Could not open shared memory '" + name + "'. " +
                             GetSystemErrorMessage(GetLastWindowsError()));
    }

    return {name, size, handle};
}

[[nodiscard]] std::optional<SharedMemory> SharedMemory::TryOpenExisting(const std::string& name, const size_t size) {
    const std::wstring fullName = GetFullSharedMemoryName(name);
    void* handle = OpenFileMappingW(FILE_MAP_WRITE, FALSE, fullName.c_str());  // NOLINT
    if (!handle) {
        return {};
    }

    return SharedMemory(name, size, handle);
}

[[nodiscard]] void* SharedMemory::data() const noexcept {
    return _data;
}

[[nodiscard]] size_t SharedMemory::size() const noexcept {
    return _size;
}

}  // namespace DsVeosCoSim

#endif
