// Copyright dSPACE SE & Co. KG. All rights reserved.

#ifdef _WIN32

#include "OsUtilities.h"

#include <cstdint>
#include <optional>
#include <string>
#include <utility>

#include <Windows.h>

#include <sysinfoapi.h>  // IWYU pragma: keep

#include "CoSimHelper.h"
#include "DsVeosCoSim/CoSimTypes.h"
#include "Environment.h"

namespace DsVeosCoSim {

namespace {

[[nodiscard]] int32_t GetLastWindowsError() {
    return static_cast<int32_t>(GetLastError());
}

[[nodiscard]] Result Utf8ToWide(const std::string& utf8String, std::wstring& utf16String) {
    if (utf8String.empty()) {
        utf16String = L"";
        return Result::Ok;
    }

    int32_t sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, utf8String.data(), static_cast<int32_t>(utf8String.size()), nullptr, 0);
    if (sizeNeeded <= 0) {
        LogSystemError("Could not convert UTF-8 string to wide string.", GetLastWindowsError());
        return Result::Error;
    }

    utf16String.resize(static_cast<size_t>(sizeNeeded));
    sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, utf8String.data(), static_cast<int32_t>(utf8String.size()), utf16String.data(), sizeNeeded);
    if (sizeNeeded <= 0) {
        LogSystemError("Could not convert UTF-8 string to wide string.", GetLastWindowsError());
        return Result::Error;
    }

    return Result::Ok;
}

[[nodiscard]] Result GetFullNamedEventName(const std::string& name, std::wstring& fullName) {
    std::string utf8Name = "Local\\dSPACE.VEOS.CoSim.Event.";
    utf8Name.append(name);
    return Utf8ToWide(utf8Name, fullName);
}

[[nodiscard]] Result GetFullNamedMutexName(const std::string& name, std::wstring& fullName) {
    std::string utf8Name = "Local\\dSPACE.VEOS.CoSim.Mutex.";
    utf8Name.append(name);
    return Utf8ToWide(utf8Name, fullName);
}

[[nodiscard]] Result GetFullSharedMemoryName(const std::string& name, std::wstring& fullName) {
    std::string utf8Name = "Local\\dSPACE.VEOS.CoSim.SharedMemory.";
    utf8Name.append(name);
    return Utf8ToWide(utf8Name, fullName);
}

}  // namespace

Handle::Handle(void* handle) : _handle(handle) {
}

Handle::~Handle() noexcept {
    (void)CloseHandle(_handle);
}

Handle::Handle(Handle&& other) noexcept : _handle(other._handle) {
    other._handle = {};
}

Handle& Handle::operator=(Handle&& other) noexcept {
    _handle = other._handle;

    other._handle = {};

    return *this;
}

Handle::operator void*() const {
    return _handle;
}

[[nodiscard]] Result Handle::Wait() const {
    bool success{};
    return Wait(Infinite, success);
}

[[nodiscard]] Result Handle::Wait(uint32_t milliseconds, bool& success) const {
    switch (DWORD result = WaitForSingleObject(_handle, milliseconds); result) {
        case WAIT_OBJECT_0:
            success = true;
            return Result::Ok;
        case WAIT_ABANDONED:
        case WAIT_TIMEOUT:
            success = false;
            return Result::Ok;
        case WAIT_FAILED: {
            LogSystemError("Could not wait for handle.", GetLastWindowsError());
            return Result::Error;
        }
        default: {
            LogSystemError("Could not wait for handle. Invalid result.", GetLastWindowsError());
            return Result::Error;
        }
    }
}

NamedEvent::NamedEvent(Handle handle) : _handle(std::move(handle)) {
}

[[nodiscard]] Result NamedEvent::CreateOrOpen(const std::string& name, NamedEvent& namedEvent) {
    std::wstring fullName{};
    CheckResult(GetFullNamedEventName(name, fullName));
    Handle handle = CreateEventW(nullptr, FALSE, FALSE, fullName.c_str());
    if (!handle) {
        LogSystemError("Could not create or open event.", GetLastWindowsError());
        return Result::Error;
    }

    namedEvent = NamedEvent(std::move(handle));
    return Result::Ok;
}

[[nodiscard]] Result NamedEvent::Set() const {
    BOOL result = SetEvent(_handle);
    if (result == FALSE) {
        LogSystemError("Could not set event.", GetLastWindowsError());
        return Result::Error;
    }

    return Result::Ok;
}

[[nodiscard]] Result NamedEvent::Wait() const {
    bool success{};
    return Wait(Infinite, success);
}

[[nodiscard]] Result NamedEvent::Wait(uint32_t milliseconds, bool& success) const {
    return _handle.Wait(milliseconds, success);
}

NamedMutex::NamedMutex(Handle handle) : _handle(std::move(handle)) {
}

NamedMutex::~NamedMutex() noexcept {
    if (_isLocked) {
        (void)ReleaseMutex(_handle);
    }
}

[[nodiscard]] Result NamedMutex::CreateOrOpen(const std::string& name, NamedMutex& namedMutex) {
    std::wstring fullName{};
    CheckResult(GetFullNamedMutexName(name, fullName));
    Handle handle = CreateMutexW(nullptr, FALSE, fullName.c_str());
    if (!handle) {
        LogSystemError("Could not create or open mutex.", GetLastWindowsError());
        return Result::Error;
    }

    namedMutex = NamedMutex(std::move(handle));
    return Result::Ok;
}

[[nodiscard]] Result NamedMutex::Lock() {
    return _handle.Wait(Infinite, _isLocked);
}

void NamedMutex::Unlock() {
    if (!_isLocked) {
        return;
    }

    (void)ReleaseMutex(_handle);
    _isLocked = false;
}

SharedMemory::SharedMemory(Handle handle, size_t size, void* data) : _handle(std::move(handle)), _size(size), _data(data) {
}

SharedMemory::SharedMemory(SharedMemory&& other) noexcept : _handle(std::move(other._handle)), _size(other._size), _data(other._data) {
    other._size = {};
    other._data = {};
}

SharedMemory& SharedMemory::operator=(SharedMemory&& other) noexcept {
    _size = other._size;
    _handle = std::move(other._handle);
    _data = other._data;

    other._size = {};
    other._data = {};

    return *this;
}

[[nodiscard]] Result SharedMemory::CreateOrOpen(const std::string& name, size_t size, SharedMemory& sharedMemory) {
    std::wstring fullName{};
    CheckResult(GetFullSharedMemoryName(name, fullName));
    DWORD sizeHigh{};
    auto sizeLow = static_cast<DWORD>(size);
    Handle handle = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, sizeHigh, sizeLow, fullName.c_str());
    if (!handle) {
        LogSystemError("Could not create or open shared memory.", GetLastWindowsError());
        return Result::Error;
    }

    void* data = MapViewOfFile(handle, FILE_MAP_ALL_ACCESS, 0, 0, size);
    if (!data) {
        LogSystemError("Could not map view of shared memory.", GetLastWindowsError());
        return Result::Error;
    }

    sharedMemory = SharedMemory(std::move(handle), size, data);
    return Result::Ok;
}

[[nodiscard]] Result SharedMemory::TryOpenExisting(const std::string& name, size_t size, std::optional<SharedMemory>& sharedMemory) {
    std::wstring fullName{};
    CheckResult(GetFullSharedMemoryName(name, fullName));
    Handle handle = OpenFileMappingW(FILE_MAP_WRITE, FALSE, fullName.c_str());
    if (!handle) {
        sharedMemory = std::nullopt;
        return Result::Ok;
    }

    void* data = MapViewOfFile(handle, FILE_MAP_ALL_ACCESS, 0, 0, size);
    if (!data) {
        LogSystemError("Could not map view of shared memory.", GetLastWindowsError());
        return Result::Error;
    }

    sharedMemory = SharedMemory(std::move(handle), size, data);
    return Result::Ok;
}

[[nodiscard]] void* SharedMemory::GetData() const {
    return _data;
}

[[nodiscard]] size_t SharedMemory::GetSize() const {
    return _size;
}

[[nodiscard]] uint32_t GetCurrentProcessId() {
    static uint32_t processId{};
    if (processId == 0) {
        processId = static_cast<uint32_t>(::GetCurrentProcessId());
    }

    return processId;
}

[[nodiscard]] bool IsProcessRunning(uint32_t processId) {
    Handle processHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | SYNCHRONIZE, FALSE, processId);
    if (!processHandle) {
        return false;
    }

    DWORD exitCode{};
    BOOL result = GetExitCodeProcess(processHandle, &exitCode);
    return (result != 0) && (exitCode == STILL_ACTIVE);
}

void SetThreadAffinity(const std::string& name) {
    size_t mask{};
    if (!TryGetAffinityMask(name, mask)) {
        return;
    }

    (void)SetThreadAffinityMask(GetCurrentThread(), mask);
}

[[nodiscard]] std::string GetEnglishErrorMessage(int32_t errorCode) {
    constexpr DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
    constexpr DWORD languageId = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
    LPSTR buffer = nullptr;
    DWORD size = FormatMessageA(flags, nullptr, static_cast<DWORD>(errorCode), languageId, reinterpret_cast<LPSTR>(&buffer), 0, nullptr);

    std::string message;
    if ((size > 0) && buffer) {
        message.assign(buffer, size);
        (void)LocalFree(buffer);

        while (!message.empty() && (message.back() == L'\r' || message.back() == L'\n' || message.back() == L' ')) {
            message.pop_back();
        }
    } else {
        message = "Unknown error.";
    }

    return message;
}

}  // namespace DsVeosCoSim

#else

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <string>

#include <pthread.h>
#include <sched.h>

#include "Environment.h"

namespace DsVeosCoSim {

void SetThreadAffinity(const std::string& name) {
    size_t mask{};
    if (!TryGetAffinityMask(name, mask)) {
        return;
    }

    int maxCpuCount = static_cast<int>(sizeof(size_t) * 8);

    cpu_set_t cpuSet{};
    CPU_ZERO(&cpuSet);

    for (int cpuId = 0; cpuId < maxCpuCount; ++cpuId) {
        size_t maskId = 1 << cpuId;
        if ((maskId & mask) != 0) {
            CPU_SET(cpuId, &cpuSet);
        }
    }

    pthread_t thread = pthread_self();
    (void)pthread_setaffinity_np(thread, sizeof(cpuSet), &cpuSet);
}

}  // namespace DsVeosCoSim

#endif
