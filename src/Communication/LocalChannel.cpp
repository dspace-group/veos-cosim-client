// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32

#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include <emmintrin.h>  // IWYU pragma: keep

#include "Channel.h"
#include "CoSimHelper.h"
#include "DsVeosCoSim/CoSimTypes.h"
#include "Environment.h"
#include "OsUtilities.h"

namespace DsVeosCoSim {

namespace {

constexpr size_t LockFreeCacheLineBytes = 64;
constexpr size_t ServerSharedMemorySize = 4;
constexpr int32_t BufferSize = 65536;

constexpr char ServerToClientPostFix[] = "ServerToClient";
constexpr char ClientToServerPostFix[] = "ClientToServer";

[[nodiscard]] constexpr uint32_t MaskIndex(uint32_t index) {
    return index & (BufferSize - 1);
}

class LocalChannelBase {
protected:
    LocalChannelBase() = default;

public:
    virtual ~LocalChannelBase() noexcept {
        Disconnect();
    }

    [[nodiscard]] Result InitializeBase(const std::string& name, bool isServer) {
        NamedMutex mutex;
        CheckResult(NamedMutex::CreateOrOpen(name, mutex));

        CheckResult(mutex.Lock());

        std::string dataName(name);
        dataName.append(".Data");
        std::string newDataName(name);
        newDataName.append(".NewData");
        std::string newSpaceName(name);
        newSpaceName.append(".NewSpace");

        constexpr size_t totalSize = static_cast<size_t>(BufferSize) + sizeof(Header);

        bool initShm{};
        std::optional<SharedMemory> sharedMemory;
        CheckResult(SharedMemory::TryOpenExisting(dataName, totalSize, sharedMemory));
        if (sharedMemory) {
            _sharedMemory = std::move(*sharedMemory);
        } else {
            CheckResult(SharedMemory::CreateOrOpen(dataName, totalSize, _sharedMemory));
            initShm = true;
        }

        CheckResult(NamedEvent::CreateOrOpen(newDataName, _newDataEvent));
        CheckResult(NamedEvent::CreateOrOpen(newSpaceName, _newSpaceEvent));

        _header = static_cast<Header*>(_sharedMemory.GetData());
        _data = static_cast<uint8_t*>(_sharedMemory.GetData()) + sizeof(Header);

        if (initShm) {
            _header->serverPid = 0;
            _header->clientPid = 0;
            _header->writeIndex.store(0, std::memory_order_release);
            _header->readIndex.store(0, std::memory_order_release);
        }

        if (isServer) {
            _ownPid = &_header->serverPid;
            _counterpartPid = &_header->clientPid;
        } else {
            _ownPid = &_header->clientPid;
            _counterpartPid = &_header->serverPid;
        }

        *_ownPid = GetCurrentProcessId();
        return Result::Ok;
    }

    LocalChannelBase(const LocalChannelBase&) = delete;
    LocalChannelBase& operator=(const LocalChannelBase&) = delete;

    LocalChannelBase(LocalChannelBase&&) = delete;
    LocalChannelBase& operator=(LocalChannelBase&&) = delete;

    void Disconnect() const {
        if (_ownPid) {
            *_ownPid = 0;
        }
    }

protected:
    [[nodiscard]] Result CheckIfConnectionIsAlive() {
        uint32_t counterpartPid = *_counterpartPid;
        if (counterpartPid != 0) {
            _connectionDetected = true;
        } else {
            if (!_connectionDetected) {
                _detectionCounter++;
                if (_detectionCounter == 5000U) {
                    LogError("Counterpart still not connected after 5 seconds.");
                    return Result::Error;
                }

                return Result::Ok;
            }

            LogTrace("Remote endpoint disconnected.");
            return Result::Disconnected;
        }

        if (IsProcessRunning(counterpartPid)) {
            return Result::Ok;
        }

        LogError("Counterpart process is not running anymore.");
        return Result::Error;
    }

    struct Header {
        uint32_t serverPid{};
        uint32_t clientPid{};
        alignas(LockFreeCacheLineBytes) std::atomic<uint32_t> writeIndex{};
        alignas(LockFreeCacheLineBytes) std::atomic<uint32_t> readIndex{};
    };

    NamedEvent _newDataEvent;
    NamedEvent _newSpaceEvent;
    Header* _header{};
    bool _connectionDetected{};
    alignas(LockFreeCacheLineBytes) uint8_t* _data{};

private:
    alignas(LockFreeCacheLineBytes) uint32_t* _counterpartPid{};
    alignas(LockFreeCacheLineBytes) uint32_t* _ownPid{};

    SharedMemory _sharedMemory;
    uint32_t _detectionCounter{};
};

class LocalChannelWriter final : public LocalChannelBase, public ChannelWriter {
public:
    LocalChannelWriter() = default;

    ~LocalChannelWriter() override = default;

    LocalChannelWriter(const LocalChannelWriter&) = delete;
    LocalChannelWriter& operator=(const LocalChannelWriter&) = delete;

    LocalChannelWriter(LocalChannelWriter&&) = delete;
    LocalChannelWriter& operator=(LocalChannelWriter&&) = delete;

    [[nodiscard]] Result Initialize(const std::string& name, uint32_t counter, bool isServer) {
        const auto* postFix = isServer ? ServerToClientPostFix : ClientToServerPostFix;
        std::string writerName(name);
        writerName.append(".").append(std::to_string(counter)).append(".").append(postFix);
        CheckResult(InitializeBase(writerName, isServer));

        _spinCount = GetSpinCount(name, postFix, "Write");
        return Result::Ok;
    }

    [[nodiscard]] Result Reserve(size_t size, BlockWriter& blockWriter) override {
        auto sizeToReserve = static_cast<int32_t>(size);
        if (BufferSize - _writeIndex < sizeToReserve) {
            CheckResult(EndWrite());

            if (BufferSize - _writeIndex < sizeToReserve) {
                throw std::runtime_error("No more space available.");
            }
        }

        blockWriter = BlockWriter(&_writeBuffer[static_cast<size_t>(_writeIndex)], size);
        _writeIndex += sizeToReserve;

        return Result::Ok;
    }

    [[nodiscard]] Result Write(uint16_t value) override {
        auto size = static_cast<int32_t>(sizeof(value));
        if (BufferSize - _writeIndex < size) {
            CheckResult(EndWrite());

            if (BufferSize - _writeIndex < size) {
                throw std::runtime_error("No more space available.");
            }
        }

        *(reinterpret_cast<decltype(value)*>(&_writeBuffer[static_cast<size_t>(_writeIndex)])) = value;
        _writeIndex += size;

        return Result::Ok;
    }

    [[nodiscard]] Result Write(uint32_t value) override {
        auto size = static_cast<int32_t>(sizeof(value));
        if (BufferSize - _writeIndex < size) {
            CheckResult(EndWrite());

            if (BufferSize - _writeIndex < size) {
                throw std::runtime_error("No more space available.");
            }
        }

        *(reinterpret_cast<decltype(value)*>(&_writeBuffer[static_cast<size_t>(_writeIndex)])) = value;
        _writeIndex += size;

        return Result::Ok;
    }

    [[nodiscard]] Result Write(uint64_t value) override {
        auto size = static_cast<int32_t>(sizeof(value));
        if (BufferSize - _writeIndex < size) {
            CheckResult(EndWrite());

            if (BufferSize - _writeIndex < size) {
                throw std::runtime_error("No more space available.");
            }
        }

        *(reinterpret_cast<decltype(value)*>(&_writeBuffer[static_cast<size_t>(_writeIndex)])) = value;
        _writeIndex += size;

        return Result::Ok;
    }

    [[nodiscard]] Result Write(const void* source, size_t size) override {
        const auto* bufferPointer = static_cast<const uint8_t*>(source);
        auto sizeToCopy = static_cast<int32_t>(size);

        while (sizeToCopy > 0) {
            if (BufferSize == _writeIndex) {
                CheckResult(EndWrite());
                continue;
            }

            int32_t sizeOfChunkToCopy = std::min(sizeToCopy, BufferSize - _writeIndex);
            (void)memcpy(&_writeBuffer[static_cast<size_t>(_writeIndex)],
                         bufferPointer,
                         static_cast<size_t>(sizeOfChunkToCopy));
            _writeIndex += sizeOfChunkToCopy;
            bufferPointer += sizeOfChunkToCopy;
            sizeToCopy -= sizeOfChunkToCopy;
        }

        return Result::Ok;
    }

    [[nodiscard]] Result EndWrite() override {
        auto* bufferPointer = _writeBuffer.data();

        auto totalSizeToCopy = static_cast<uint32_t>(_writeIndex);

        while (totalSizeToCopy > 0) {
            uint32_t currentSizeOfShm = _cachedWriteIndex - _header->readIndex.load(std::memory_order_acquire);
            if (currentSizeOfShm == BufferSize) {
                CheckResult(WaitForFreeSpace(currentSizeOfShm));
            }

            _connectionDetected = true;
            uint32_t sizeToCopy = std::min(totalSizeToCopy, static_cast<uint32_t>(BufferSize) - currentSizeOfShm);
            uint32_t maskedWriteIndex = MaskIndex(_cachedWriteIndex);
            uint32_t sizeUntilBufferEnd = std::min(sizeToCopy, static_cast<uint32_t>(BufferSize) - maskedWriteIndex);
            (void)memcpy(&_data[maskedWriteIndex], bufferPointer, sizeUntilBufferEnd);
            bufferPointer += sizeUntilBufferEnd;

            uint32_t restSize = sizeToCopy - sizeUntilBufferEnd;
            if (restSize > 0) {
                (void)memcpy(&_data[0], bufferPointer, restSize);
                bufferPointer += restSize;
            }

            _cachedWriteIndex += sizeToCopy;
            _header->writeIndex.store(_cachedWriteIndex, std::memory_order_release);
            totalSizeToCopy -= sizeToCopy;
        }

        _writeIndex = 0;
        return _newDataEvent.Set();
    }

private:
    [[nodiscard]] Result WaitForFreeSpace(uint32_t& currentSizeOfShm) {
        CheckResult(_newDataEvent.Set());

        for (uint32_t i = 0; i < _spinCount; i++) {
            currentSizeOfShm = _cachedWriteIndex - _header->readIndex.load(std::memory_order_acquire);
            if (currentSizeOfShm < BufferSize) {
                return Result::Ok;
            }

            _mm_pause();
        }

        while (true) {
            currentSizeOfShm = _cachedWriteIndex - _header->readIndex.load(std::memory_order_acquire);
            if (currentSizeOfShm < BufferSize) {
                return Result::Ok;
            }

            bool eventSet{};
            CheckResult(_newSpaceEvent.Wait(1, eventSet));
            if (eventSet) {
                break;
            }

            CheckResult(CheckIfConnectionIsAlive());
        }

        currentSizeOfShm = _cachedWriteIndex - _header->readIndex.load(std::memory_order_acquire);
        return Result::Ok;
    }

    uint32_t _cachedWriteIndex{};
    uint32_t _spinCount{};

    int32_t _writeIndex{};
    std::array<uint8_t, BufferSize> _writeBuffer{};
};

class LocalChannelReader final : public LocalChannelBase, public ChannelReader {
public:
    LocalChannelReader() = default;

    ~LocalChannelReader() override = default;

    LocalChannelReader(const LocalChannelReader&) = delete;
    LocalChannelReader& operator=(const LocalChannelReader&) = delete;

    LocalChannelReader(LocalChannelReader&&) = delete;
    LocalChannelReader& operator=(LocalChannelReader&&) = delete;

    [[nodiscard]] Result Initialize(const std::string& name, uint32_t counter, bool isServer) {
        const auto* postFix = isServer ? ClientToServerPostFix : ServerToClientPostFix;
        std::string readerName(name);
        readerName.append(".").append(std::to_string(counter)).append(".").append(postFix);
        CheckResult(InitializeBase(readerName, isServer));

        _spinCount = GetSpinCount(name, postFix, "Read");
        return Result::Ok;
    }

    [[nodiscard]] Result ReadBlock(size_t size, BlockReader& blockReader) override {
        auto blockSize = static_cast<int32_t>(size);
        while (_writeIndex - _readIndex < blockSize) {
            CheckResult(BeginRead());
        }

        blockReader = BlockReader(&_readBuffer[static_cast<size_t>(_readIndex)], size);
        _readIndex += blockSize;
        return Result::Ok;
    }

    [[nodiscard]] Result Read(uint16_t& value) override {
        auto size = static_cast<int32_t>(sizeof(value));
        while (_writeIndex - _readIndex < size) {
            CheckResult(BeginRead());
        }

        value =
            *reinterpret_cast<std::remove_reference_t<decltype(value)>*>(&_readBuffer[static_cast<size_t>(_readIndex)]);
        _readIndex += size;
        return Result::Ok;
    }

    [[nodiscard]] Result Read(uint32_t& value) override {
        auto size = static_cast<int32_t>(sizeof(value));
        while (_writeIndex - _readIndex < size) {
            CheckResult(BeginRead());
        }

        value =
            *reinterpret_cast<std::remove_reference_t<decltype(value)>*>(&_readBuffer[static_cast<size_t>(_readIndex)]);
        _readIndex += size;
        return Result::Ok;
    }

    [[nodiscard]] Result Read(uint64_t& value) override {
        auto size = static_cast<int32_t>(sizeof(value));
        while (_writeIndex - _readIndex < size) {
            CheckResult(BeginRead());
        }

        value =
            *reinterpret_cast<std::remove_reference_t<decltype(value)>*>(&_readBuffer[static_cast<size_t>(_readIndex)]);
        _readIndex += size;
        return Result::Ok;
    }

    [[nodiscard]] Result Read(void* destination, size_t size) override {
        auto* bufferPointer = static_cast<uint8_t*>(destination);
        auto sizeToCopy = static_cast<int32_t>(size);

        while (sizeToCopy > 0) {
            if (_writeIndex <= _readIndex) {
                CheckResult(BeginRead());
                continue;
            }

            int32_t sizeOfChunkToCopy = std::min(sizeToCopy, _writeIndex - _readIndex);
            (void)memcpy(bufferPointer,
                         &_readBuffer[static_cast<size_t>(_readIndex)],
                         static_cast<size_t>(sizeOfChunkToCopy));
            _readIndex += sizeOfChunkToCopy;
            bufferPointer += sizeOfChunkToCopy;
            sizeToCopy -= sizeOfChunkToCopy;
        }

        return Result::Ok;
    }

private:
    [[nodiscard]] Result BeginRead() {
        int32_t unreadSize = _writeIndex - _readIndex;
        if (unreadSize > 0) {
            (void)memmove(_readBuffer.data(),
                          &_readBuffer[static_cast<size_t>(_readIndex)],
                          static_cast<size_t>(unreadSize));
        }

        _writeIndex -= _readIndex;
        _readIndex = 0;

        auto maxSizeToRead = static_cast<uint32_t>(BufferSize - unreadSize);

        uint32_t currentSizeOfShm = _header->writeIndex.load(std::memory_order_acquire) - _cachedReadIndex;
        if (currentSizeOfShm == 0) {
            CheckResult(BeginRead(currentSizeOfShm));
        }

        _connectionDetected = true;

        uint32_t sizeToCopy = std::min(maxSizeToRead, currentSizeOfShm);
        uint32_t maskedReadIndex = MaskIndex(_cachedReadIndex);
        uint32_t sizeUntilBufferEnd = std::min(sizeToCopy, static_cast<uint32_t>(BufferSize) - maskedReadIndex);

        (void)memcpy(&_readBuffer[static_cast<size_t>(_writeIndex)], &_data[maskedReadIndex], sizeUntilBufferEnd);
        _writeIndex += static_cast<int32_t>(sizeUntilBufferEnd);

        uint32_t restSize = sizeToCopy - sizeUntilBufferEnd;
        if (restSize > 0) {
            (void)memcpy(&_readBuffer[static_cast<size_t>(_writeIndex)], _data, restSize);
            _writeIndex += static_cast<int32_t>(restSize);
        }

        _cachedReadIndex += sizeToCopy;
        _header->readIndex.store(_cachedReadIndex, std::memory_order_release);
        CheckResult(_newSpaceEvent.Set());

        return Result::Ok;
    }

    [[nodiscard]] Result BeginRead(uint32_t& currentSizeOfShm) {
        for (uint32_t i = 0; i < _spinCount; i++) {
            currentSizeOfShm = _header->writeIndex.load(std::memory_order_acquire) - _cachedReadIndex;
            if (currentSizeOfShm > 0) {
                return Result::Ok;
            }

            _mm_pause();
        }

        while (true) {
            currentSizeOfShm = _header->writeIndex.load(std::memory_order_acquire) - _cachedReadIndex;
            if (currentSizeOfShm > 0) {
                return Result::Ok;
            }

            bool eventSet{};
            CheckResult(_newDataEvent.Wait(1, eventSet));
            if (eventSet) {
                break;
            }

            CheckResult(CheckIfConnectionIsAlive());
        }

        currentSizeOfShm = _header->writeIndex.load(std::memory_order_acquire) - _cachedReadIndex;
        return Result::Ok;
    }

    uint32_t _cachedReadIndex{};
    uint32_t _spinCount{};

    int32_t _readIndex{};
    int32_t _writeIndex{};
    std::array<uint8_t, BufferSize> _readBuffer{};
};

class LocalChannel final : public Channel {
public:
    LocalChannel() = default;
    ~LocalChannel() override = default;

    LocalChannel(const LocalChannel&) = delete;
    LocalChannel& operator=(const LocalChannel&) = delete;

    LocalChannel(LocalChannel&&) = delete;
    LocalChannel& operator=(LocalChannel&&) = delete;

    [[nodiscard]] Result Initialize(const std::string& name, uint32_t counter, bool isServer) {
        CheckResult(_writer.Initialize(name, counter, isServer));
        return _reader.Initialize(name, counter, isServer);
    }

    [[nodiscard]] Result GetRemoteAddress(std::string& remoteAddress) const override {
        remoteAddress = "";
        return Result::Ok;
    }

    void Disconnect() override {
        _writer.Disconnect();
        _reader.Disconnect();
    }

    [[nodiscard]] ChannelWriter& GetWriter() override {
        return _writer;
    }

    [[nodiscard]] ChannelReader& GetReader() override {
        return _reader;
    }

private:
    LocalChannelWriter _writer;
    LocalChannelReader _reader;
};

class LocalChannelServer final : public ChannelServer {
public:
    LocalChannelServer() = default;
    ~LocalChannelServer() override = default;

    LocalChannelServer(const LocalChannelServer&) = delete;
    LocalChannelServer& operator=(const LocalChannelServer&) = delete;

    LocalChannelServer(LocalChannelServer&&) = delete;
    LocalChannelServer& operator=(LocalChannelServer&&) = delete;

    [[nodiscard]] Result Initialize(const std::string& name) {
        _name = name;
        NamedMutex mutex;
        CheckResult(NamedMutex::CreateOrOpen(name, mutex));
        CheckResult(mutex.Lock());

        CheckResult(SharedMemory::CreateOrOpen(name, ServerSharedMemorySize, _sharedMemory));
        _counter = static_cast<std::atomic<uint32_t>*>(_sharedMemory.GetData());
        _counter->store(0, std::memory_order_release);
        return Result::Ok;
    }

    [[nodiscard]] uint16_t GetLocalPort() const override {
        return {};
    }

    [[nodiscard]] Result TryAccept(std::unique_ptr<Channel>& acceptedChannel) override {
        uint32_t currentCounter = _counter->load(std::memory_order_acquire);
        if (currentCounter > _lastCounter) {
            uint32_t counterToUse = _lastCounter;
            _lastCounter++;
            auto tmpChannel = std::make_unique<LocalChannel>();
            CheckResult(tmpChannel->Initialize(_name, counterToUse, true));
            acceptedChannel = std::move(tmpChannel);
        }

        return Result::Ok;
    }

private:
    std::string _name;
    SharedMemory _sharedMemory;

    std::atomic<uint32_t>* _counter{};
    uint32_t _lastCounter{};
};

}  // namespace

[[nodiscard]] Result TryConnectToLocalChannel(const std::string& name, std::unique_ptr<Channel>& connectedChannel) {
    NamedMutex mutex;
    CheckResult(NamedMutex::CreateOrOpen(name, mutex));
    CheckResult(mutex.Lock());

    std::optional<SharedMemory> sharedMemory;
    CheckResult(SharedMemory::TryOpenExisting(name, ServerSharedMemorySize, sharedMemory));
    if (!sharedMemory) {
        return Result::Ok;
    }

    auto& counter = *static_cast<std::atomic<uint32_t>*>(sharedMemory->GetData());
    uint32_t currentCounter = counter.fetch_add(1);

    auto tmpChannel = std::make_unique<LocalChannel>();
    CheckResult(tmpChannel->Initialize(name, currentCounter, false));
    connectedChannel = std::move(tmpChannel);
    return Result::Ok;
}

[[nodiscard]] Result CreateLocalChannelServer(const std::string& name, std::unique_ptr<ChannelServer>& channelServer) {
    auto server = std::make_unique<LocalChannelServer>();
    CheckResult(server->Initialize(name));
    channelServer = std::move(server);
    return Result::Ok;
}

}  // namespace DsVeosCoSim

#endif
