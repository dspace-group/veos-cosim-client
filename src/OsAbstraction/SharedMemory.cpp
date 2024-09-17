// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32

#include "SharedMemory.h"

#include <Windows.h>
#include <fmt/format.h>

#include "OsUtilities.h"

namespace DsVeosCoSim {

namespace {

[[nodiscard]] std::wstring GetFullSharedMemoryName(const std::string& name) {
    return Utf8ToWide(fmt::format("Local\\dSPACE.VEOS.CoSim.SharedMemory.{}", name));
}

}  // namespace

SharedMemory::SharedMemory(const std::string& name, size_t size, Handle handle)
    : _size(size), _handle(std::move(handle)) {
    _data = ::MapViewOfFile(_handle, FILE_MAP_ALL_ACCESS, 0, 0, _size);
    if (!_data) {
        (void)::CloseHandle(_handle);
        throw OsAbstractionException(fmt::format("Could not map view of shared memory '{}'.", name),
                                     GetLastWindowsError());
    }
}

SharedMemory::SharedMemory(SharedMemory&& sharedMemory) noexcept {
    _size = sharedMemory._size;
    _handle = std::move(sharedMemory._handle);
    _data = sharedMemory._data;

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

SharedMemory SharedMemory::CreateOrOpen(const std::string& name, size_t size) {
    std::wstring fullName = GetFullSharedMemoryName(name);
    DWORD sizeHigh = 0U;
    auto sizeLow = static_cast<DWORD>(size);
    void* handle =
        ::CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, sizeHigh, sizeLow, fullName.c_str());
    if (!handle) {
        throw OsAbstractionException(fmt::format("Could not create or open shared memory '{}'.", name),
                                     GetLastWindowsError());
    }

    return {name, size, handle};
}

SharedMemory SharedMemory::OpenExisting(const std::string& name, size_t size) {
    std::wstring fullName = GetFullSharedMemoryName(name);
    void* handle = ::OpenFileMappingW(FILE_MAP_WRITE, FALSE, fullName.c_str());
    if (!handle) {
        throw OsAbstractionException(fmt::format("Could not open shared memory '{}'.", name), GetLastWindowsError());
    }

    return {name, size, handle};
}

std::optional<SharedMemory> SharedMemory::TryOpenExisting(const std::string& name, size_t size) {
    std::wstring fullName = GetFullSharedMemoryName(name);
    void* handle = ::OpenFileMappingW(FILE_MAP_WRITE, FALSE, fullName.c_str());
    if (!handle) {
        return {};
    }

    return SharedMemory(name, size, handle);
}

void* SharedMemory::data() const noexcept {
    return _data;
}

size_t SharedMemory::size() const noexcept {
    return _size;
}

}  // namespace DsVeosCoSim

#endif