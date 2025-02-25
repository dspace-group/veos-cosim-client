// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32

#include "LocalChannel.h"

#include <cstdint>
#include <mutex>
#include <string>

#include "CoSimHelper.h"
#include "NamedMutex.h"
#include "OsUtilities.h"

namespace DsVeosCoSim {

namespace {

constexpr uint32_t ServerSharedMemorySize = 4;
constexpr uint32_t BufferSize = 64 * 1024;

std::string ServerToClientPostFix = "ServerToClient";
std::string ClientToServerPostFix = "ClientToServer";

[[nodiscard]] std::string GetWriterName(const std::string& name, const bool isServer) {
    const std::string postfix = isServer ? ServerToClientPostFix : ClientToServerPostFix;
    return name + "." + postfix;
}

[[nodiscard]] std::string GetReaderName(const std::string& name, const bool isServer) {
    const std::string postfix = isServer ? ClientToServerPostFix : ServerToClientPostFix;
    return name + "." + postfix;
}

[[nodiscard]] constexpr uint32_t MaskIndex(const uint32_t index) noexcept {
    return index & (BufferSize - 1);
}

}  // namespace

LocalChannelBase::LocalChannelBase(const std::string& name, const bool isServer) {
    NamedMutex mutex = NamedMutex::CreateOrOpen(name);

    std::lock_guard lock(mutex);

    bool initShm{};

    const std::string dataName = name + ".Data";
    const std::string newDataName = name + ".NewData";
    const std::string newSpaceName = name + ".NewSpace";

    constexpr uint32_t totalSize = BufferSize + sizeof(Header);

    std::optional<SharedMemory> sharedMemory = SharedMemory::TryOpenExisting(dataName, totalSize);  // NOLINT
    if (sharedMemory) {
        _sharedMemory = std::move(*sharedMemory);
        _newDataEvent = NamedEvent::OpenExisting(newDataName);
        _newSpaceEvent = NamedEvent::OpenExisting(newSpaceName);
        initShm = false;
    } else {
        _sharedMemory = SharedMemory::CreateOrOpen(dataName, totalSize);
        _newDataEvent = NamedEvent::CreateOrOpen(newDataName);
        _newSpaceEvent = NamedEvent::CreateOrOpen(newSpaceName);
        initShm = true;
    }

    _header = static_cast<Header*>(_sharedMemory.data());
    _data = static_cast<uint8_t*>(_sharedMemory.data()) + sizeof(Header);

    if (initShm) {
        _header->serverPid = 0;
        _header->clientPid = 0;
        _header->writeIndex.store(0);
        _header->readIndex.store(0);
    }

    if (isServer) {
        _ownPid = &_header->serverPid;
        _counterpartPid = &_header->clientPid;
    } else {
        _ownPid = &_header->clientPid;
        _counterpartPid = &_header->serverPid;
    }

    *_ownPid = GetCurrentProcessId();
}

LocalChannelBase::~LocalChannelBase() noexcept {
    Disconnect();
}

LocalChannelBase::LocalChannelBase(LocalChannelBase&& other) noexcept
    : _newDataEvent(std::move(other._newDataEvent)),
      _newSpaceEvent(std::move(other._newSpaceEvent)),
      _header(other._header),
      _data(other._data),
      _counterpartPid(other._counterpartPid),
      _ownPid(other._ownPid),
      _sharedMemory(std::move(other._sharedMemory)) {
    other._header = {};
    other._data = {};
    other._counterpartPid = {};
    other._ownPid = {};
}

LocalChannelBase& LocalChannelBase::operator=(LocalChannelBase&& other) noexcept {
    _newDataEvent = std::move(other._newDataEvent);
    _newSpaceEvent = std::move(other._newSpaceEvent);
    _sharedMemory = std::move(other._sharedMemory);

    _header = other._header;
    _data = other._data;
    _counterpartPid = other._counterpartPid;
    _ownPid = other._ownPid;

    other._header = {};
    other._data = {};
    other._counterpartPid = {};
    other._ownPid = {};

    return *this;
}

void LocalChannelBase::Disconnect() const {
    if (_ownPid) {
        *_ownPid = 0;
    }
}

[[nodiscard]] bool LocalChannelBase::CheckIfConnectionIsAlive() {
    const uint32_t counterpartPid = *_counterpartPid;
    if (counterpartPid != 0) {
        _connectionDetected = true;
    } else {
        if (!_connectionDetected) {
            _detectionCounter++;
            if (_detectionCounter == 5000) {
                LogError("Counterpart still not connected after 5 seconds.");
                return false;
            }

            return true;
        }

        LogTrace("Remote endpoint disconnected.");
        return false;
    }

    CheckResultWithMessage(IsProcessRunning(counterpartPid),
                           "Process with id " + std::to_string(counterpartPid) + " exited.");
    return true;
}

LocalChannelWriter::LocalChannelWriter(const std::string& name, const bool isServer)
    : LocalChannelBase(GetWriterName(name, isServer), isServer) {
}

[[nodiscard]] bool LocalChannelWriter::Write(const void* source, const size_t size) {
    const auto* bufferPointer = static_cast<const uint8_t*>(source);

    auto totalSizeToCopy = static_cast<uint32_t>(size);

    while (totalSizeToCopy > 0) {
        const uint32_t readIndex = _header->readIndex.load();
        uint32_t currentSize = _writeIndex - readIndex;
        if (currentSize == BufferSize) {
            if (!WaitForFreeSpace(currentSize)) {
                return false;
            }
        }

        _connectionDetected = true;
        uint32_t sizeToCopy = std::min(totalSizeToCopy, BufferSize - currentSize);

        const uint32_t sizeUntilBufferEnd = std::min(sizeToCopy, BufferSize - _maskedWriteIndex);
        (void)memcpy(&_data[_maskedWriteIndex], bufferPointer, sizeUntilBufferEnd);
        bufferPointer += sizeUntilBufferEnd;

        if (const uint32_t restSize = sizeToCopy - sizeUntilBufferEnd; restSize > 0) {
            (void)memcpy(&_data[0], bufferPointer, restSize);
            bufferPointer += restSize;
        }

        _writeIndex += sizeToCopy;
        _maskedWriteIndex = MaskIndex(_writeIndex);
        _header->writeIndex.store(_writeIndex);
        totalSizeToCopy -= sizeToCopy;
    }

    return true;
}

[[nodiscard]] bool LocalChannelWriter::EndWrite() {
    _newDataEvent.Set();
    return true;
}

[[nodiscard]] bool LocalChannelWriter::WaitForFreeSpace(uint32_t& currentSize) {
    (void)SignalAndWait(_newDataEvent, _newSpaceEvent, 1);
    currentSize = _writeIndex - _header->readIndex.load();
    if (currentSize < BufferSize) {
        return true;
    }

    while (!_newSpaceEvent.Wait(1)) {
        currentSize = _writeIndex - _header->readIndex.load();
        if (currentSize < BufferSize) {
            return true;
        }

        if (!CheckIfConnectionIsAlive()) {
            return false;
        }
    }

    currentSize = _writeIndex - _header->readIndex.load();
    return true;
}

LocalChannelReader::LocalChannelReader(const std::string& name, const bool isServer)
    : LocalChannelBase(GetReaderName(name, isServer), isServer) {
}

[[nodiscard]] bool LocalChannelReader::Read(void* destination, const size_t size) {
    auto* bufferPointer = static_cast<uint8_t*>(destination);

    auto totalSizeToCopy = static_cast<uint32_t>(size);

    while (totalSizeToCopy > 0) {
        const uint32_t writeIndex = _header->writeIndex.load();
        uint32_t currentSize = writeIndex - _readIndex;
        if (currentSize == 0) {
            if (!BeginRead(currentSize)) {
                return false;
            }
        }

        _connectionDetected = true;
        uint32_t sizeToCopy = std::min(totalSizeToCopy, currentSize);
        const uint32_t sizeUntilBufferEnd = std::min(sizeToCopy, BufferSize - _maskedReadIndex);
        (void)memcpy(bufferPointer, &_data[_maskedReadIndex], sizeUntilBufferEnd);
        bufferPointer += sizeUntilBufferEnd;

        if (const uint32_t restSize = sizeToCopy - sizeUntilBufferEnd; restSize > 0) {
            (void)memcpy(bufferPointer, &_data[0], restSize);
            bufferPointer += restSize;
        }

        _readIndex += sizeToCopy;
        _maskedReadIndex = MaskIndex(_readIndex);
        _header->readIndex.store(_readIndex);
        if (currentSize == BufferSize) {
            _newSpaceEvent.Set();
        }

        totalSizeToCopy -= sizeToCopy;
    }

    return true;
}

[[nodiscard]] bool LocalChannelReader::BeginRead(uint32_t& currentSize) {
    while (!_newDataEvent.Wait(1)) {
        currentSize = _header->writeIndex.load() - _readIndex;
        if (currentSize > 0) {
            return true;
        }

        if (!CheckIfConnectionIsAlive()) {
            return false;
        }
    }

    currentSize = _header->writeIndex.load() - _readIndex;
    return true;
}

LocalChannel::LocalChannel(const std::string& name, const bool isServer)
    : _writer(name, isServer), _reader(name, isServer) {
}

void LocalChannel::Disconnect() {
    _writer.Disconnect();
    _reader.Disconnect();
}

[[nodiscard]] ChannelWriter& LocalChannel::GetWriter() {
    return _writer;
}

[[nodiscard]] ChannelReader& LocalChannel::GetReader() {
    return _reader;
}

[[nodiscard]] std::optional<LocalChannel> TryConnectToLocalChannel(const std::string& name) {
    NamedMutex mutex = NamedMutex::CreateOrOpen(name);

    std::lock_guard lock(mutex);

    const std::optional<SharedMemory> sharedMemory = SharedMemory::TryOpenExisting(name, ServerSharedMemorySize);
    if (!sharedMemory) {
        return {};
    }

    auto& counter = *static_cast<std::atomic<int32_t>*>(sharedMemory->data());
    const int32_t currentCounter = counter.fetch_add(1);
    const std::string specificName = name + "." + std::to_string(currentCounter);

    return LocalChannel(specificName, false);
}

LocalChannelServer::LocalChannelServer(const std::string& name) : _name(name) {
    NamedMutex mutex = NamedMutex::CreateOrOpen(name);

    std::lock_guard lock(mutex);

    _sharedMemory = SharedMemory::CreateOrOpen(_name, ServerSharedMemorySize);
    _counter = static_cast<std::atomic<int32_t>*>(_sharedMemory.data());  // NOLINT
    _counter->store(0);
}

[[nodiscard]] std::optional<LocalChannel> LocalChannelServer::TryAccept() {
    if (const int32_t currentCounter = _counter->load(); currentCounter > _lastCounter) {
        const std::string specificName = _name + "." + std::to_string(_lastCounter);
        _lastCounter++;
        return LocalChannel(specificName, true);
    }

    return {};
}

}  // namespace DsVeosCoSim

#endif
