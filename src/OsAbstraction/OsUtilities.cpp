// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32

#include "OsUtilities.h"

#include <Windows.h>  // NOLINT

#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>

#include "CoSimHelper.h"

namespace DsVeosCoSim {

namespace {

[[nodiscard]] std::wstring Utf8ToWide(const std::string_view utf8String) {
    if (utf8String.empty()) {
        return {};
    }

    const int32_t sizeNeeded = MultiByteToWideChar(CP_UTF8,  // NOLINT
                                                   0,
                                                   utf8String.data(),
                                                   static_cast<int32_t>(utf8String.size()),
                                                   nullptr,
                                                   0);

    std::wstring wideString(sizeNeeded, L'\0');
    (void)MultiByteToWideChar(CP_UTF8,
                              0,
                              utf8String.data(),
                              static_cast<int32_t>(utf8String.size()),
                              wideString.data(),
                              sizeNeeded);

    return wideString;
}

[[nodiscard]] std::wstring GetFullNamedEventName(const std::string& name) {
    std::string utf8Name = "Local\\dSPACE.VEOS.CoSim.Event.";
    utf8Name.append(name);
    return Utf8ToWide(utf8Name);
}

[[nodiscard]] std::wstring GetFullNamedMutexName(const std::string& name) {
    std::string utf8Name = "Local\\dSPACE.VEOS.CoSim.Mutex.";
    utf8Name.append(name);
    return Utf8ToWide(utf8Name);
}

[[nodiscard]] std::wstring GetFullSharedMemoryName(const std::string& name) {
    std::string utf8Name = "Local\\dSPACE.VEOS.CoSim.SharedMemory.";
    utf8Name.append(name);
    return Utf8ToWide(utf8Name);
}

[[nodiscard]] int32_t GetLastWindowsError() {
    return static_cast<int32_t>(GetLastError());  // NOLINT
}

class Handle final {
public:
    Handle() noexcept = default;

    Handle(void* handle) : _handle(handle) {  // NOLINT
    }

    ~Handle() noexcept {
        (void)CloseHandle(_handle);  // NOLINT
    }

    Handle(const Handle&) = delete;
    Handle& operator=(const Handle&) = delete;

    Handle(Handle&& other) noexcept : _handle(other._handle) {
        other._handle = {};
    }

    Handle& operator=(Handle&& other) noexcept {
        _handle = other._handle;

        other._handle = {};

        return *this;
    }

    operator void*() const noexcept {  // NOLINT
        return _handle;
    }

    void Wait() const {
        (void)Wait(Infinite);
    }

    [[nodiscard]] bool Wait(uint32_t milliseconds) const {
        switch (const DWORD result = WaitForSingleObject(_handle, milliseconds)) {  // NOLINT
            case WAIT_OBJECT_0:
                return true;
            case WAIT_ABANDONED:
            case WAIT_TIMEOUT:
                return false;
            case WAIT_FAILED: {
                std::string message = "Could not wait for handle. ";
                message.append(GetSystemErrorMessage(GetLastWindowsError()));
                throw std::runtime_error(message);
            }
            default: {
                std::string message = "Could not wait for handle. Invalid result: ";
                message.append(std::to_string(result));
                message.append(".");
                throw std::runtime_error(message);
            }
        }
    }

private:
    void* _handle{};
};

class NamedEventImpl final : public NamedEvent {
public:
    NamedEventImpl(Handle handle, const std::string& name) : _handle(std::move(handle)), _name(name) {
    }

    ~NamedEventImpl() override = default;

    NamedEventImpl(const NamedEventImpl&) = delete;
    NamedEventImpl& operator=(const NamedEventImpl&) = delete;

    NamedEventImpl(NamedEventImpl&&) = delete;
    NamedEventImpl& operator=(NamedEventImpl&&) = delete;

    void Set() const override {
        const BOOL result = SetEvent(_handle);  // NOLINT
        if (result == FALSE) {
            std::string message = "Could not set event '";
            message.append(_name);
            message.append("'. ");
            message.append(GetSystemErrorMessage(GetLastWindowsError()));
            throw std::runtime_error(message);
        }
    }

    void Wait() const override {
        (void)Wait(Infinite);
    }

    [[nodiscard]] bool Wait(uint32_t milliseconds) const override {
        return _handle.Wait(milliseconds);
    }

private:
    Handle _handle;
    std::string _name;
};

class NamedMutexImpl final : public NamedMutex {
public:
    explicit NamedMutexImpl(Handle handle) : _handle(std::move(handle)) {
    }

    ~NamedMutexImpl() noexcept override = default;

    NamedMutexImpl(const NamedMutexImpl&) = delete;
    NamedMutexImpl& operator=(const NamedMutexImpl&) = delete;

    NamedMutexImpl(NamedMutexImpl&&) = delete;
    NamedMutexImpl& operator=(NamedMutexImpl&&) = delete;

    // Small case, so this mutex can directly be used in std::lock_guard
    void lock() const override {  // NOLINT
        (void)lock(Infinite);
    }

    [[nodiscard]] bool lock(uint32_t milliseconds) const override {  // NOLINT
        return _handle.Wait(milliseconds);
    }

    void unlock() const override {    // NOLINT
        (void)ReleaseMutex(_handle);  // NOLINT
    }

private:
    Handle _handle;
};

class SharedMemoryImpl final : public SharedMemory {
public:
    SharedMemoryImpl(const std::string& name, size_t size, Handle handle)
        : _size(size),
          _handle(std::move(handle)),
          _data(MapViewOfFile(_handle, FILE_MAP_ALL_ACCESS, 0, 0, _size)) {  // NOLINT
        if (!_data) {
            (void)CloseHandle(_handle);  // NOLINT
            std::string message = "Could not map view of shared memory '";
            message.append(name);
            message.append("'. ");
            message.append(GetSystemErrorMessage(GetLastWindowsError()));
            throw std::runtime_error(message);
        }
    }

    ~SharedMemoryImpl() noexcept override = default;

    SharedMemoryImpl(const SharedMemoryImpl&) = delete;
    SharedMemoryImpl& operator=(const SharedMemoryImpl&) = delete;

    SharedMemoryImpl(SharedMemoryImpl&&) = delete;
    SharedMemoryImpl& operator=(SharedMemoryImpl&&) = delete;

    [[nodiscard]] void* data() const noexcept override {  // NOLINT
        return _data;
    }

    [[nodiscard]] size_t size() const noexcept override {  // NOLINT
        return _size;
    }

private:
    size_t _size{};
    Handle _handle;
    void* _data{};
};

}  // namespace

[[nodiscard]] uint32_t GetCurrentProcessId() {
    static uint32_t processId{};
    if (processId == 0) {
        processId = static_cast<uint32_t>(::GetCurrentProcessId());  // NOLINT
    }

    return processId;
}

[[nodiscard]] bool IsProcessRunning(const uint32_t processId) {
    void* processHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | SYNCHRONIZE, FALSE, processId);  // NOLINT
    if (processHandle == nullptr) {
        return false;
    }

    DWORD exitCode{};                                                  // NOLINT
    const BOOL result = GetExitCodeProcess(processHandle, &exitCode);  // NOLINT
    return (result != 0) && (exitCode == STILL_ACTIVE);
}

[[nodiscard]] std::string GetEnglishErrorMessage(int32_t errorCode) {
    const DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
    const DWORD languageId = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
    LPSTR buffer = nullptr;
    const DWORD size =
        FormatMessageA(flags, nullptr, errorCode, languageId, reinterpret_cast<LPSTR>(&buffer), 0, nullptr);

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

[[nodiscard]] std::unique_ptr<NamedEvent> CreateOrOpenNamedEvent(const std::string& name) {
    const std::wstring fullName = GetFullNamedEventName(name);
    void* handle = CreateEventW(nullptr, FALSE, FALSE, fullName.c_str());  // NOLINT
    if (!handle) {
        std::string message = "Could not create or open event '";
        message.append(name);
        message.append("'. ");
        message.append(GetSystemErrorMessage(GetLastWindowsError()));
        throw std::runtime_error(message);
    }

    return std::make_unique<NamedEventImpl>(handle, name);
}

[[nodiscard]] std::unique_ptr<NamedMutex> CreateOrOpenNamedMutex(const std::string& name) {
    const std::wstring fullName = GetFullNamedMutexName(name);
    void* handle = CreateMutexW(nullptr, FALSE, fullName.c_str());  // NOLINT
    if (!handle) {
        std::string message = "Could not create or open mutex '";
        message.append(name);
        message.append("'. ");
        message.append(GetSystemErrorMessage(GetLastWindowsError()));
        throw std::runtime_error(message);
    }

    return std::make_unique<NamedMutexImpl>(handle);
}

[[nodiscard]] std::unique_ptr<SharedMemory> CreateOrOpenSharedMemory(const std::string& name, size_t size) {
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
        std::string message = "Could not create or open shared memory '";
        message.append(name);
        message.append("'. ");
        message.append(GetSystemErrorMessage(GetLastWindowsError()));
        throw std::runtime_error(message);
    }

    return std::make_unique<SharedMemoryImpl>(name, size, handle);
}

[[nodiscard]] std::unique_ptr<SharedMemory> TryOpenExistingSharedMemory(const std::string& name, size_t size) {
    const std::wstring fullName = GetFullSharedMemoryName(name);
    void* handle = OpenFileMappingW(FILE_MAP_WRITE, FALSE, fullName.c_str());  // NOLINT
    if (!handle) {
        return {};
    }

    return std::make_unique<SharedMemoryImpl>(name, size, handle);
}

}  // namespace DsVeosCoSim

#endif
