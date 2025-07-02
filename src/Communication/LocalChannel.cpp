// Copyright dSPACE GmbH. All rights reserved.

#include "Environment.h"
#ifdef _WIN32

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <memory>
#include <string>

#include "Channel.h"
#include "CoSimHelper.h"
#include "DsVeosCoSim/CoSimTypes.h"
#include "OsUtilities.h"

namespace DsVeosCoSim {

namespace {

constexpr uint32_t LockFreeCacheLineBytes = 64;
constexpr uint32_t ServerSharedMemorySize = 4;
constexpr uint32_t BufferSize = 64 * 1024;

const auto ServerToClientPostFix = ".ServerToClient";
const auto ClientToServerPostFix = ".ClientToServer";

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

    [[nodiscard]] Result InitializeBase(std::string_view name, bool isServer) {
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

    [[nodiscard]] Result Initialize(std::string_view name, uint32_t counter, bool isServer) {
        const auto postFix = isServer ? ServerToClientPostFix : ClientToServerPostFix;
        std::string writerName(name);
        writerName.append(".");
        writerName.append(std::to_string(counter));
        writerName.append(postFix);
        CheckResult(InitializeBase(writerName, isServer));

        std::string writerNameForSpinCount(name);
        writerNameForSpinCount.append(postFix);
        _spinCount = GetSpinCount(writerNameForSpinCount);
        return Result::Ok;
    }

    [[nodiscard]] Result Write(const void* source, size_t size) override {
        const auto* bufferPointer = static_cast<const uint8_t*>(source);

        auto totalSizeToCopy = static_cast<uint32_t>(size);

        while (totalSizeToCopy > 0) {
            uint32_t readIndex = _header->readIndex.load();
            uint32_t currentSize = _writeIndex - readIndex;
            if (currentSize == BufferSize) {
                CheckResult(WaitForFreeSpace(currentSize));
            }

            _connectionDetected = true;
            uint32_t sizeToCopy = std::min(totalSizeToCopy, BufferSize - currentSize);

            uint32_t sizeUntilBufferEnd = std::min(sizeToCopy, BufferSize - _maskedWriteIndex);
            (void)memcpy(&_data[_maskedWriteIndex], bufferPointer, sizeUntilBufferEnd);
            bufferPointer += sizeUntilBufferEnd;

            uint32_t restSize = sizeToCopy - sizeUntilBufferEnd;
            if (restSize > 0) {
                (void)memcpy(&_data[0], bufferPointer, restSize);
                bufferPointer += restSize;
            }

            _writeIndex += sizeToCopy;
            _maskedWriteIndex = MaskIndex(_writeIndex);
            _header->writeIndex.store(_writeIndex);
            totalSizeToCopy -= sizeToCopy;
        }

        return Result::Ok;
    }

    [[nodiscard]] Result EndWrite() override {
        return _newDataEvent.Set();
    }

private:
    [[nodiscard]] Result WaitForFreeSpace(uint32_t& currentSize) {
        CheckResult(_newDataEvent.Set());

        std::atomic<uint32_t>& readIndex = _header->readIndex;

        for (uint32_t i = 0; i < _spinCount; i++) {
            currentSize = _writeIndex - readIndex.load();
            if (currentSize < BufferSize) {
                return Result::Ok;
            }
        }

        while (true) {
            currentSize = _writeIndex - readIndex.load();
            if (currentSize < BufferSize) {
                return Result::Ok;
            }

            bool eventSet{};
            CheckResult(_newSpaceEvent.Wait(1, eventSet));
            if (eventSet) {
                break;
            }

            CheckResult(CheckIfConnectionIsAlive());
        }

        currentSize = _writeIndex - readIndex.load();
        return Result::Ok;
    }

    uint32_t _writeIndex{};
    uint32_t _maskedWriteIndex{};
    uint32_t _spinCount{};
};

class LocalChannelReader final : public LocalChannelBase, public ChannelReader {
public:
    LocalChannelReader() = default;

    ~LocalChannelReader() override = default;

    LocalChannelReader(const LocalChannelReader&) = delete;
    LocalChannelReader& operator=(const LocalChannelReader&) = delete;

    LocalChannelReader(LocalChannelReader&&) = delete;
    LocalChannelReader& operator=(LocalChannelReader&&) = delete;

    [[nodiscard]] Result Initialize(std::string_view name, uint32_t counter, bool isServer) {
        const auto postFix = isServer ? ClientToServerPostFix : ServerToClientPostFix;
        std::string readerName(name);
        readerName.append(".");
        readerName.append(std::to_string(counter));
        readerName.append(postFix);
        CheckResult(InitializeBase(readerName, isServer));

        std::string readerNameForSpinCount(name);
        readerNameForSpinCount.append(postFix);
        _spinCount = GetSpinCount(readerNameForSpinCount);
        return Result::Ok;
    }

    [[nodiscard]] Result Read(void* destination, size_t size) override {
        auto* bufferPointer = static_cast<uint8_t*>(destination);

        auto totalSizeToCopy = static_cast<uint32_t>(size);

        while (totalSizeToCopy > 0) {
            uint32_t writeIndex = _header->writeIndex.load();
            uint32_t currentSize = writeIndex - _readIndex;
            if (currentSize == 0) {
                CheckResult(BeginRead(currentSize));
            }

            _connectionDetected = true;
            uint32_t sizeToCopy = std::min(totalSizeToCopy, currentSize);
            uint32_t sizeUntilBufferEnd = std::min(sizeToCopy, BufferSize - _maskedReadIndex);
            (void)memcpy(bufferPointer, &_data[_maskedReadIndex], sizeUntilBufferEnd);
            bufferPointer += sizeUntilBufferEnd;

            uint32_t restSize = sizeToCopy - sizeUntilBufferEnd;
            if (restSize > 0) {
                (void)memcpy(bufferPointer, &_data[0], restSize);
                bufferPointer += restSize;
            }

            _readIndex += sizeToCopy;
            _maskedReadIndex = MaskIndex(_readIndex);
            _header->readIndex.store(_readIndex);
            if (currentSize == BufferSize) {
                CheckResult(_newSpaceEvent.Set());
            }

            totalSizeToCopy -= sizeToCopy;
        }

        return Result::Ok;
    }

private:
    [[nodiscard]] Result BeginRead(uint32_t& currentSize) {
        std::atomic<uint32_t>& writeIndex = _header->writeIndex;

        for (uint32_t i = 0; i < _spinCount; i++) {
            currentSize = writeIndex.load() - _readIndex;
            if (currentSize > 0) {
                return Result::Ok;
            }
        }

        while (true) {
            currentSize = writeIndex.load() - _readIndex;
            if (currentSize > 0) {
                return Result::Ok;
            }

            bool eventSet{};
            CheckResult(_newDataEvent.Wait(1, eventSet));
            if (eventSet) {
                break;
            }

            CheckResult(CheckIfConnectionIsAlive());
        }

        currentSize = writeIndex.load() - _readIndex;
        return Result::Ok;
    }

    uint32_t _readIndex{};
    uint32_t _maskedReadIndex{};
    uint32_t _spinCount{};
};

class LocalChannel final : public Channel {
public:
    LocalChannel() = default;
    ~LocalChannel() override = default;

    LocalChannel(const LocalChannel&) = delete;
    LocalChannel& operator=(const LocalChannel&) = delete;

    LocalChannel(LocalChannel&&) = delete;
    LocalChannel& operator=(LocalChannel&&) = delete;

    [[nodiscard]] Result Initialize(std::string_view name, uint32_t counter, bool isServer) {
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

    [[nodiscard]] Result Initialize(std::string_view name) {
        _name = name;
        NamedMutex mutex;
        CheckResult(NamedMutex::CreateOrOpen(name, mutex));
        CheckResult(mutex.Lock());

        CheckResult(SharedMemory::CreateOrOpen(name, ServerSharedMemorySize, _sharedMemory));
        _counter = static_cast<std::atomic<uint32_t>*>(_sharedMemory.GetData());
        _counter->store(0);
        return Result::Ok;
    }

    [[nodiscard]] uint16_t GetLocalPort() const override {
        return {};
    }

    [[nodiscard]] Result TryAccept(std::unique_ptr<Channel>& acceptedChannel) override {
        uint32_t currentCounter = _counter->load();
        if (currentCounter > _lastCounter) {
            uint32_t counterToUse = _lastCounter;
            _lastCounter++;
            std::unique_ptr<LocalChannel> tmpChannel = std::make_unique<LocalChannel>();
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

[[nodiscard]] Result TryConnectToLocalChannel(std::string_view name, std::unique_ptr<Channel>& connectedChannel) {
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

    std::unique_ptr<LocalChannel> tmpChannel = std::make_unique<LocalChannel>();
    CheckResult(tmpChannel->Initialize(name, currentCounter, false));
    connectedChannel = std::move(tmpChannel);
    return Result::Ok;
}

[[nodiscard]] Result CreateLocalChannelServer(std::string_view name,
                                              std::unique_ptr<ChannelServer>& localChannelServer) {
    std::unique_ptr<LocalChannelServer> server = std::make_unique<LocalChannelServer>();
    CheckResult(server->Initialize(name));
    localChannelServer = std::move(server);
    return Result::Ok;
}

}  // namespace DsVeosCoSim

#endif
