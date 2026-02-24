// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "OsUtilities.hpp"

#include <atomic>
#include <stdexcept>
#include <string>

#ifdef _WIN32

#include <cstdint>
#include <utility>

#include <Windows.h>
#undef min

#include <sysinfoapi.h>  // IWYU pragma: keep

#include "Environment.hpp"
#include "Error.hpp"

#else

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <pthread.h>
#include <sched.h>

#endif

namespace DsVeosCoSim {

#ifdef _WIN32

namespace {

constexpr size_t ServerSharedMemorySize = 4;

[[nodiscard]] int32_t GetLastWindowsError() {
    return static_cast<int32_t>(GetLastError());
}

[[nodiscard]] Result Utf8ToWide(const std::string& utf8String, std::wstring& utf16String) {
    if (utf8String.empty()) {
        utf16String = L"";
        return CreateOk();
    }

    int32_t sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, utf8String.data(), static_cast<int32_t>(utf8String.size()), nullptr, 0);
    if (sizeNeeded <= 0) {
        return CreateError("Could not convert UTF-8 string to wide string.", GetLastWindowsError());
    }

    utf16String.resize(static_cast<size_t>(sizeNeeded));
    sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, utf8String.data(), static_cast<int32_t>(utf8String.size()), utf16String.data(), sizeNeeded);
    if (sizeNeeded <= 0) {
        return CreateError("Could not convert UTF-8 string to wide string.", GetLastWindowsError());
    }

    return CreateOk();
}

[[nodiscard]] Result GetFullNamedEventName(const std::string& name, std::wstring& fullName) {
    return Utf8ToWide("Local\\dSPACE.VEOS.CoSim.Event." + name, fullName);
}

[[nodiscard]] Result GetFullTestNamedLockName(const std::string& name, std::wstring& fullName) {
    return Utf8ToWide("Local\\dSPACE.VEOS.CoSim.Mutex." + name, fullName);
}

[[nodiscard]] Result GetFullSharedMemoryName(const std::string& name, std::wstring& fullName) {
    return Utf8ToWide("Local\\dSPACE.VEOS.CoSim.SharedMemory." + name, fullName);
}

// Spin wait with exponential backoff
template <typename Predicate>
bool SpinWait(Predicate&& predicate, uint32_t iterations) {
    for (uint32_t i = 0; i < iterations; ++i) {
        if (std::forward<Predicate>(predicate)()) {
            return true;
        }

        // Exponential backoff
        if (i < 10) {
            _mm_pause();
        } else if (i < 100) {
            for (int j = 0; j < 4; ++j) {
                _mm_pause();
            }
        } else {
            for (int j = 0; j < 16; ++j) {
                _mm_pause();
            }
        }
    }

    return false;
}

[[nodiscard]] bool IsProcessRunning(const Handle& processHandle) {
    if (!processHandle.IsValid()) {
        return false;
    }

    DWORD exitCode{};
    BOOL result = GetExitCodeProcess(processHandle.Get(), &exitCode);
    return (result != 0) && (exitCode == STILL_ACTIVE);
}

[[nodiscard]] uint32_t GetCurrentProcessId() {
    static const auto processId = static_cast<uint32_t>(::GetCurrentProcessId());
    return processId;
}

}  // namespace

void Handle::Reset(void* newHandle) {
    if (IsValid()) {
        CloseHandle(_handle);
    }

    _handle = newHandle;
}

[[nodiscard]] bool Handle::IsValid() const {
    return (_handle != nullptr) && (_handle != INVALID_HANDLE_VALUE);
}

[[nodiscard]] Result Handle::Wait() const {
    return Wait(Infinite);
}

[[nodiscard]] Result Handle::Wait(uint32_t milliseconds) const {
    if (!IsValid()) {
        return CreateError("Handle is not initialized.");
    }

    switch (DWORD result = WaitForSingleObject(_handle, milliseconds); result) {
        case WAIT_OBJECT_0:
            return CreateOk();
        case WAIT_TIMEOUT:
            return CreateTimeout();
        case WAIT_ABANDONED:
        case WAIT_FAILED: {
            return CreateError("Could not wait for handle.", GetLastWindowsError());
        }
        default: {
            return CreateError("Could not wait for handle. Invalid result.", GetLastWindowsError());
        }
    }
}

NamedEvent::NamedEvent(Handle handle) : _handle(std::move(handle)) {
}

[[nodiscard]] Result NamedEvent::CreateOrOpen(const std::string& name, NamedEvent& namedEvent) {
    std::wstring fullName;
    CheckResult(GetFullNamedEventName(name, fullName));

    Handle handle(CreateEventW(nullptr, FALSE, FALSE, fullName.c_str()));
    if (!handle.IsValid()) {
        return CreateError("Could not create or open event.", GetLastWindowsError());
    }

    namedEvent = NamedEvent(std::move(handle));
    return CreateOk();
}

void NamedEvent::Close() {
    _handle.Reset();
}

[[nodiscard]] Result NamedEvent::Set() const {
    if (!IsValid()) {
        throw std::runtime_error("Not initialized.");
    }

    BOOL result = SetEvent(_handle.Get());
    if (result == FALSE) {
        return CreateError("Could not set event.", GetLastWindowsError());
    }

    return CreateOk();
}

[[nodiscard]] Result NamedEvent::Wait() const {
    // Check if valid will be done in Handle::Wait
    return _handle.Wait();
}

[[nodiscard]] Result NamedEvent::Wait(uint32_t milliseconds) const {
    // Check if valid will be done in Handle::Wait
    return _handle.Wait(milliseconds);
}

[[nodiscard]] bool NamedEvent::IsValid() const {
    return _handle.IsValid();
}

NamedLock::NamedLock(Handle handle) : _handle(std::move(handle)) {
}

NamedLock::~NamedLock() noexcept {
    ReleaseMutex(_handle.Get());
}

[[nodiscard]] Result NamedLock::Create(const std::string& name, NamedLock& namedMutex) {
    std::wstring fullName;
    CheckResult(GetFullTestNamedLockName(name, fullName));

    Handle handle(CreateMutexW(nullptr, FALSE, fullName.c_str()));
    if (!handle.IsValid()) {
        return CreateError("Could not create or open mutex.", GetLastWindowsError());
    }

    CheckResult(handle.Wait());

    namedMutex = NamedLock(std::move(handle));
    return CreateOk();
}

SharedMemory::SharedMemory(Handle handle, size_t size, void* data) : _handle(std::move(handle)), _size(size), _data(data) {
}

SharedMemory::~SharedMemory() noexcept {
    Close();
}

SharedMemory::SharedMemory(SharedMemory&& other) noexcept : _handle(std::move(other._handle)), _size(other._size), _data(other._data) {
    other._size = {};
    other._data = {};
}

SharedMemory& SharedMemory::operator=(SharedMemory&& other) noexcept {
    if (this != &other) {
        Close();
        _size = other._size;
        _handle = std::move(other._handle);
        _data = other._data;
        other._size = {};
        other._data = {};
    }

    return *this;
}

[[nodiscard]] Result SharedMemory::CreateOrOpen(const std::string& name, size_t size, SharedMemory& sharedMemory) {
    std::wstring fullName;
    CheckResult(GetFullSharedMemoryName(name, fullName));

    DWORD sizeHigh{};
    auto sizeLow = static_cast<DWORD>(size);
    Handle handle(CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, sizeHigh, sizeLow, fullName.c_str()));
    if (!handle.IsValid()) {
        return CreateError("Could not create or open shared memory.", GetLastWindowsError());
    }

    void* data = MapViewOfFile(handle.Get(), FILE_MAP_ALL_ACCESS, 0, 0, size);
    if (!data) {
        return CreateError("Could not map view of shared memory.", GetLastWindowsError());
    }

    sharedMemory = SharedMemory(std::move(handle), size, data);
    return CreateOk();
}

[[nodiscard]] Result SharedMemory::TryOpenExisting(const std::string& name, size_t size, SharedMemory& sharedMemory) {
    std::wstring fullName;
    CheckResult(GetFullSharedMemoryName(name, fullName));

    Handle handle(OpenFileMappingW(FILE_MAP_WRITE, FALSE, fullName.c_str()));
    if (!handle.IsValid()) {
        return CreateNotConnected();
    }

    void* data = MapViewOfFile(handle.Get(), FILE_MAP_ALL_ACCESS, 0, 0, size);
    if (!data) {
        return CreateError("Could not map view of shared memory.", GetLastWindowsError());
    }

    sharedMemory = SharedMemory(std::move(handle), size, data);
    return CreateOk();
}

void SharedMemory::Close() {
    if (!IsValid()) {
        return;
    }

    if (_data != nullptr) {
        UnmapViewOfFile(_data);
    }

    _handle.Reset();
    _data = nullptr;
    _size = 0;
}

[[nodiscard]] uint8_t* SharedMemory::GetData() const {
    if (!IsValid()) {
        throw std::runtime_error("SharedMemory is not valid.");
    }

    return static_cast<uint8_t*>(_data);
}

[[nodiscard]] size_t SharedMemory::GetSize() const {
    if (!IsValid()) {
        throw std::runtime_error("SharedMemory is not valid.");
    }

    return _size;
}

[[nodiscard]] bool SharedMemory::IsValid() const {
    return _data != nullptr && _handle.IsValid();
}

ShmPipePart::ShmPipePart(NamedEvent newDataEvent, NamedEvent newSpaceEvent, SharedMemory sharedMemory, bool isWriter)
    : _newDataEvent(std::move(newDataEvent)), _newSpaceEvent(std::move(newSpaceEvent)), _sharedMemory(std::move(sharedMemory)), _isWriter(isWriter) {
    SetOwnPid(GetCurrentProcessId());
}

ShmPipePart::~ShmPipePart() noexcept {
    Disconnect();
}

[[nodiscard]] Result ShmPipePart::Create(const std::string& name, bool isWriter, ShmPipePart& pipe) {
    NamedLock mutex;
    CheckResult(NamedLock::Create(name, mutex));

    std::string dataName = name + ".Data";
    std::string newDataName = name + ".NewData";
    std::string newSpaceName = name + ".NewSpace";

    constexpr size_t totalSize = static_cast<size_t>(PipeBufferSize) + sizeof(Header);

    bool initShm{};
    SharedMemory sharedMemory;
    Result result = SharedMemory::TryOpenExisting(dataName, totalSize, sharedMemory);
    if (IsError(result)) {
        return result;
    }

    if (IsNotConnected(result)) {
        CheckResult(SharedMemory::CreateOrOpen(dataName, totalSize, sharedMemory));
        initShm = true;
    }

    NamedEvent newDataEvent;
    CheckResult(NamedEvent::CreateOrOpen(newDataName, newDataEvent));

    NamedEvent newSpaceEvent;
    CheckResult(NamedEvent::CreateOrOpen(newSpaceName, newSpaceEvent));

    auto& header = *sharedMemory.As<Header>();

    if (initShm) {
        header.writerPid.store(0, std::memory_order_release);
        header.readerPid.store(0, std::memory_order_release);
        header.writeIndex.store(0, std::memory_order_release);
        header.readIndex.store(0, std::memory_order_release);
    }

    pipe = ShmPipePart(std::move(newDataEvent), std::move(newSpaceEvent), std::move(sharedMemory), isWriter);
    return CreateOk();
}

void ShmPipePart::Disconnect() {
    if (!_sharedMemory.IsValid()) {
        return;
    }

    SetOwnPid(0);
    _sharedMemory.Close();
    _newDataEvent.Close();
    _newSpaceEvent.Close();
}

[[nodiscard]] Result ShmPipePart::Read(void* destination, size_t size, size_t& receivedSize) {
    if (_isWriter) {
        return CreateError("Pipe is writer.");
    }

    CheckResult(EnsureConnected());

    if (size == 0) {
        receivedSize = 0;
        return CreateOk();
    }

    auto* destinationBuffer = static_cast<uint8_t*>(destination);

    auto& header = *_sharedMemory.As<Header>();
    uint32_t readIndex = header.readIndex.load(std::memory_order_acquire);

    uint8_t* sourceBuffer = _sharedMemory.GetData() + sizeof(Header);

    uint32_t availableData = GetAvailableData(header);
    if (availableData == 0) {
        CheckResult(WaitForData());
        availableData = GetAvailableData(header);
    }

    uint32_t chunkSize = static_cast<uint32_t>(std::min<size_t>(size, UINT32_MAX));
    uint32_t sizeToCopy = std::min(chunkSize, availableData);
    uint32_t maskedReadIndex = MaskIndex(readIndex);
    uint32_t sizeUntilBufferEnd = std::min(sizeToCopy, PipeBufferSize - maskedReadIndex);
    uint32_t restSize = sizeToCopy - sizeUntilBufferEnd;

    memcpy(destinationBuffer, sourceBuffer + maskedReadIndex, sizeUntilBufferEnd);

    if (restSize > 0) {
        memcpy(destinationBuffer + sizeUntilBufferEnd, sourceBuffer, restSize);
    }

    header.readIndex.store(readIndex + sizeToCopy, std::memory_order_release);
    receivedSize = sizeToCopy;

    return _newSpaceEvent.Set();
}

[[nodiscard]] Result ShmPipePart::Write(const void* source, size_t size) {
    if (!_isWriter) {
        return CreateError("Pipe is reader.");
    }

    CheckResult(EnsureConnected());

    const auto* sourceBuffer = static_cast<const uint8_t*>(source);

    auto& header = *_sharedMemory.As<Header>();
    uint32_t writeIndex = header.writeIndex.load(std::memory_order_acquire);
    uint8_t* destinationBuffer = _sharedMemory.GetData() + sizeof(Header);

    while (size > 0) {
        uint32_t availableSpace = GetAvailableSpace(header);
        if (availableSpace == 0) {
            CheckResult(WaitForSpace());
            availableSpace = GetAvailableSpace(header);
        }

        uint32_t chunkSize = static_cast<uint32_t>(std::min<size_t>(size, UINT32_MAX));
        uint32_t sizeToCopy = std::min(chunkSize, availableSpace);
        uint32_t maskedWriteIndex = MaskIndex(writeIndex);
        uint32_t sizeUntilBufferEnd = std::min(sizeToCopy, PipeBufferSize - maskedWriteIndex);
        uint32_t restSize = sizeToCopy - sizeUntilBufferEnd;

        memcpy(destinationBuffer + maskedWriteIndex, sourceBuffer, sizeUntilBufferEnd);

        if (restSize > 0) {
            memcpy(destinationBuffer, sourceBuffer + sizeUntilBufferEnd, restSize);
        }

        writeIndex += sizeToCopy;
        header.writeIndex.store(writeIndex, std::memory_order_release);

        size -= sizeToCopy;
        sourceBuffer += sizeToCopy;
    }

    return _newDataEvent.Set();
}

[[nodiscard]] bool ShmPipePart::IsConnected() const {
    return _sharedMemory.IsValid();
}

[[nodiscard]] uint32_t ShmPipePart::MaskIndex(uint32_t index) {
    return index & (PipeBufferSize - 1);
}

[[nodiscard]] uint32_t ShmPipePart::GetAvailableSpace(Header& header) {
    return PipeBufferSize - GetAvailableData(header);
}

[[nodiscard]] uint32_t ShmPipePart::GetAvailableData(Header& header) {
    uint32_t writeIndex = header.writeIndex.load(std::memory_order_acquire);
    uint32_t readIndex = header.readIndex.load(std::memory_order_acquire);
    return writeIndex - readIndex;
}

[[nodiscard]] Result ShmPipePart::EnsureConnected() {
    if (!_sharedMemory.IsValid()) {
        return CreateNotConnected();
    }

    uint32_t counterPartPid = GetCounterPartPid();
    bool counterPartPidSet = counterPartPid != 0;
    bool counterPartProcessHandleSet = _counterPartProcess.IsValid();

    // Normal connected state
    if (counterPartPidSet && counterPartProcessHandleSet) {
        return CreateOk();
    }

    // Not yet connected fully. The pipe is used before the counterpart finished the initialization.
    // Handle as connected for now...
    if (!counterPartPidSet && !counterPartProcessHandleSet) {
        return CreateOk();
    }

    // !counterPartPidSet && counterPartProcessHandleSet
    if (!counterPartPidSet) {
        // Counterpart process is not connected anymore
        return CreateNotConnected();
    }

    // counterPartPidSet && !counterPartProcessHandleSet
    _counterPartProcess = Handle(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | SYNCHRONIZE, FALSE, counterPartPid));
    if (!_counterPartProcess.IsValid()) {
        return CreateNotConnected();
    }

    return CreateOk();
}

[[nodiscard]] Result ShmPipePart::WaitForSpace() {
    auto& header = *_sharedMemory.As<Header>();

    // Fast path
    if (SpinWait(
            [&header]() {
                return GetAvailableSpace(header) > 0;
            },
            SpinCount)) {
        return CreateOk();
    }

    CheckResult(_newDataEvent.Set());

    // Slow path
    while (GetAvailableSpace(header) == 0) {
        Result result = _newSpaceEvent.Wait(1);
        if (IsError(result)) {
            return result;
        }

        if (IsOk(result)) {
            // Event signaled, check space again
            continue;
        }

        CheckResult(CheckIfConnectionIsAlive());
    }

    return CreateOk();
}

[[nodiscard]] Result ShmPipePart::WaitForData() {
    auto& header = *_sharedMemory.As<Header>();

    // Fast path
    if (SpinWait(
            [&header]() {
                return GetAvailableData(header) > 0;
            },
            SpinCount)) {
        return CreateOk();
    }

    CheckResult(_newSpaceEvent.Set());

    // Slow path
    while (GetAvailableData(header) == 0) {
        Result result = _newDataEvent.Wait(1);
        if (IsError(result)) {
            return result;
        }

        if (IsOk(result)) {
            // Event signaled, check data again
            continue;
        }

        CheckResult(CheckIfConnectionIsAlive());
    }

    return CreateOk();
}

[[nodiscard]] Result ShmPipePart::CheckIfConnectionIsAlive() {
    static constexpr uint32_t ConnectionTimeoutInMilliseconds = 5000U;

    uint32_t counterPartPid = GetCounterPartPid();
    if (counterPartPid == 0) {
        if (!_counterPartProcess.IsValid()) {
            // Not connected yet. Give it up to 5 seconds ...
            _detectionCounter++;
            if (_detectionCounter == ConnectionTimeoutInMilliseconds) {
                return CreateError("Counterpart still not connected after 5 seconds.");
            }

            return CreateOk();
        }

        // Not connected anymore
        return CreateNotConnected();
    }

    if (!_counterPartProcess.IsValid()) {
        _counterPartProcess = Handle(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | SYNCHRONIZE, FALSE, counterPartPid));
    }

    if (IsProcessRunning(_counterPartProcess)) {
        return CreateOk();
    }

    return CreateError("Counterpart process is not running anymore.");
}

void ShmPipePart::SetOwnPid(uint32_t pid) {
    auto& header = *_sharedMemory.As<Header>();
    if (_isWriter) {
        header.writerPid.store(pid, std::memory_order_release);
    } else {
        header.readerPid.store(pid, std::memory_order_release);
    }
}

[[nodiscard]] uint32_t ShmPipePart::GetCounterPartPid() {
    auto& header = *_sharedMemory.As<Header>();
    return _isWriter ? header.readerPid.load(std::memory_order_acquire) : header.writerPid.load(std::memory_order_acquire);
}

ShmPipeClient::ShmPipeClient(ShmPipePart writer, ShmPipePart reader) : _writer(std::move(writer)), _reader(std::move(reader)) {
}

[[nodiscard]] Result ShmPipeClient::TryConnect(const std::string& name, ShmPipeClient& client) {
    NamedLock mutex;
    CheckResult(NamedLock::Create(name, mutex));

    SharedMemory sharedMemory;
    CheckResult(SharedMemory::TryOpenExisting(name, ServerSharedMemorySize, sharedMemory));

    auto& counter = *sharedMemory.As<std::atomic<uint32_t>>();
    uint32_t currentCounter = counter.fetch_add(1);

    std::string writerName = name + '.' + std::to_string(currentCounter) + '.' + ClientToServerPostFix;
    ShmPipePart writer;
    CheckResult(ShmPipePart::Create(writerName, true, writer));

    std::string readerName = name + '.' + std::to_string(currentCounter) + '.' + ServerToClientPostFix;
    ShmPipePart reader;
    CheckResult(ShmPipePart::Create(readerName, false, reader));

    client = ShmPipeClient(std::move(writer), std::move(reader));
    return CreateOk();
}

void ShmPipeClient::Disconnect() {
    _writer.Disconnect();
    _reader.Disconnect();
}

[[nodiscard]] Result ShmPipeClient::Receive(void* destination, size_t size, size_t& receivedSize) {
    return _reader.Read(destination, size, receivedSize);
}

[[nodiscard]] Result ShmPipeClient::Send(const void* source, size_t size) {
    return _writer.Write(source, size);
}

[[nodiscard]] bool ShmPipeClient::IsConnected() const {
    return _writer.IsConnected();
}

ShmPipeListener::ShmPipeListener(std::string name, SharedMemory sharedMemory) : _name(std::move(name)), _sharedMemory(std::move(sharedMemory)) {
}

[[nodiscard]] Result ShmPipeListener::Create(const std::string& name, ShmPipeListener& listener) {
    NamedLock mutex;
    CheckResult(NamedLock::Create(name, mutex));

    SharedMemory sharedMemory;
    CheckResult(SharedMemory::CreateOrOpen(name, ServerSharedMemorySize, sharedMemory));
    auto& counter = *sharedMemory.As<std::atomic<uint32_t>>();
    counter.store(0, std::memory_order_release);

    listener = ShmPipeListener(name, std::move(sharedMemory));
    return CreateOk();
}

void ShmPipeListener::Stop() {
    _sharedMemory.Close();
}

[[nodiscard]] Result ShmPipeListener::TryAccept(ShmPipeClient& client) {
    if (!IsRunning()) {
        return CreateError("Server is not running.");
    }

    auto& counter = *_sharedMemory.As<std::atomic<uint32_t>>();
    uint32_t currentCounter = counter.load(std::memory_order_acquire);
    if (currentCounter <= _lastCounter) {
        return CreateNotConnected();
    }

    uint32_t counterToUse = _lastCounter;
    _lastCounter++;

    std::string writerName = _name + '.' + std::to_string(counterToUse) + '.' + ShmPipeClient::ServerToClientPostFix;
    ShmPipePart writer;
    CheckResult(ShmPipePart::Create(writerName, true, writer));

    std::string readerName = _name + '.' + std::to_string(counterToUse) + '.' + ShmPipeClient::ClientToServerPostFix;
    ShmPipePart reader;
    CheckResult(ShmPipePart::Create(readerName, false, reader));

    client = ShmPipeClient(std::move(writer), std::move(reader));
    return CreateOk();
}

[[nodiscard]] bool ShmPipeListener::IsRunning() const {
    return _sharedMemory.IsValid();
}

void SetThreadAffinity(const std::string& name) {
    size_t mask{};

    if (!TryGetAffinityMask(name, mask)) {
        return;
    }

    SetThreadAffinityMask(GetCurrentThread(), mask);
}

#else

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

#endif

}  // namespace DsVeosCoSim
