// Copyright dSPACE GmbH. All rights reserved.

#include "SharedMemory.h"

#ifdef _WIN32

#include <Windows.h>
#undef min
#undef max

#include "Logger.h"

namespace DsVeosCoSim {

namespace {

constexpr int BufferSize = 64 * 1024;

}  // namespace

SharedMemory::SharedMemory(const std::string& name) : _name(name) {
}

SharedMemory::~SharedMemory() noexcept {
    (void)CloseHandle(_handle);
}

Result SharedMemory::Create() {
    constexpr auto sizeHigh = 0;
    constexpr auto sizeLow = static_cast<DWORD>(BufferSize);
    std::string fullName = "Local\\dSPACE.VEOS.CoSim.SharedMemory." + _name;
    _handle = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, sizeHigh, sizeLow, fullName.c_str());
    if (!_handle) {
        LogError("Could not create shared memory '" + _name + "'. Error code: " + std::to_string(GetLastError()) + ".");
        return Result::Error;
    }

    _buffer = MapViewOfFile(_handle, FILE_MAP_ALL_ACCESS, 0, 0, BufferSize);
    if (!_buffer) {
        (void)CloseHandle(_handle);
        LogError("Could not map view of shared memory '" + _name + "'. Error code: " + std::to_string(GetLastError()) + ".");
        return Result::Error;
    }

    return Result::Ok;
}

void* SharedMemory::GetBuffer() {
    if (!_buffer) {
        LogError("Shared memory '" + _name + "' is not initialized.");
        return nullptr;
    }

    return _buffer;
}

}  // namespace DsVeosCoSim

#endif
