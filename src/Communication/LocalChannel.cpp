// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

#include "Channel.h"
#include "CoSimHelper.h"
#include "Handle.h"
#include "NamedEvent.h"
#include "NamedMutex.h"
#include "OsUtilities.h"
#include "SharedMemory.h"

namespace DsVeosCoSim {

namespace {

constexpr uint32_t LockFreeCacheLineBytes = 64;
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

class LocalChannelBase {
protected:
    LocalChannelBase(const std::string& name, const bool isServer) {
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

public:
    virtual ~LocalChannelBase() noexcept {
        Disconnect();
    }

    LocalChannelBase(const LocalChannelBase&) = delete;
    LocalChannelBase& operator=(const LocalChannelBase&) = delete;

    LocalChannelBase(LocalChannelBase&&) noexcept = delete;
    LocalChannelBase& operator=(LocalChannelBase&&) noexcept = delete;

    void Disconnect() const {
        if (_ownPid) {
            *_ownPid = 0;
        }
    }

protected:
    [[nodiscard]] bool CheckIfConnectionIsAlive() {
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
    alignas(LockFreeCacheLineBytes) uint32_t* _counterpartPid{};
    alignas(LockFreeCacheLineBytes) uint32_t* _ownPid{};

private:
    SharedMemory _sharedMemory;
    uint32_t _detectionCounter{};
};

class LocalChannelWriter final : public LocalChannelBase, public ChannelWriter {
public:
    LocalChannelWriter(const std::string& name, const bool isServer)
        : LocalChannelBase(GetWriterName(name, isServer), isServer) {
    }

    ~LocalChannelWriter() noexcept override = default;

    LocalChannelWriter(const LocalChannelWriter&) = delete;
    LocalChannelWriter& operator=(const LocalChannelWriter&) = delete;

    LocalChannelWriter(LocalChannelWriter&&) noexcept = delete;
    LocalChannelWriter& operator=(LocalChannelWriter&&) noexcept = delete;

    [[nodiscard]] bool Write(const void* source, const size_t size) override {
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

            const uint32_t restSize = sizeToCopy - sizeUntilBufferEnd;
            if (restSize > 0) {
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

    [[nodiscard]] bool EndWrite() override {
        _newDataEvent.Set();
        return true;
    }

private:
    [[nodiscard]] bool WaitForFreeSpace(uint32_t& currentSize) {
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

    uint32_t _writeIndex{};
    uint32_t _maskedWriteIndex{};
};

class LocalChannelReader final : public LocalChannelBase, public ChannelReader {
public:
    LocalChannelReader(const std::string& name, const bool isServer)
        : LocalChannelBase(GetReaderName(name, isServer), isServer) {
    }

    ~LocalChannelReader() noexcept override = default;

    LocalChannelReader(const LocalChannelReader&) = delete;
    LocalChannelReader& operator=(const LocalChannelReader&) = delete;

    LocalChannelReader(LocalChannelReader&&) noexcept = delete;
    LocalChannelReader& operator=(LocalChannelReader&&) noexcept = delete;

    [[nodiscard]] bool Read(void* destination, const size_t size) override {
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

            const uint32_t restSize = sizeToCopy - sizeUntilBufferEnd;
            if (restSize > 0) {
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

private:
    [[nodiscard]] bool BeginRead(uint32_t& currentSize) {
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

    uint32_t _readIndex{};
    uint32_t _maskedReadIndex{};
};

class LocalChannel final : public Channel {
public:
    LocalChannel(const std::string& name, const bool isServer) : _writer(name, isServer), _reader(name, isServer) {
    }

    ~LocalChannel() noexcept override = default;

    LocalChannel(const LocalChannel&) = delete;
    LocalChannel& operator=(const LocalChannel&) = delete;

    LocalChannel(LocalChannel&&) noexcept = delete;
    LocalChannel& operator=(LocalChannel&&) noexcept = delete;

    [[nodiscard]] std::string GetRemoteAddress() const override {
        return {};
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
    explicit LocalChannelServer(const std::string& name) : _name(name) {
        NamedMutex mutex = NamedMutex::CreateOrOpen(name);

        std::lock_guard lock(mutex);

        _sharedMemory = SharedMemory::CreateOrOpen(_name, ServerSharedMemorySize);
        _counter = static_cast<std::atomic<int32_t>*>(_sharedMemory.data());  // NOLINT
        _counter->store(0);
    }

    ~LocalChannelServer() noexcept override = default;

    LocalChannelServer(const LocalChannelServer&) = delete;
    LocalChannelServer& operator=(const LocalChannelServer&) = delete;

    LocalChannelServer(LocalChannelServer&&) = delete;
    LocalChannelServer& operator=(LocalChannelServer&&) = delete;

    [[nodiscard]] uint16_t GetLocalPort() const override {
        return {};
    }

    [[nodiscard]] std::unique_ptr<Channel> TryAccept() override {
        const int32_t currentCounter = _counter->load();
        if (currentCounter > _lastCounter) {
            const std::string specificName = _name + "." + std::to_string(_lastCounter);
            _lastCounter++;
            return std::make_unique<LocalChannel>(specificName, true);
        }

        return {};
    }

    [[nodiscard]] std::unique_ptr<Channel> TryAccept([[maybe_unused]] uint32_t timeoutInMilliseconds) override {
        return TryAccept();
    }

private:
    std::string _name;
    SharedMemory _sharedMemory;

    std::atomic<int32_t>* _counter{};
    int32_t _lastCounter{};
};

}  // namespace

[[nodiscard]] std::unique_ptr<Channel> TryConnectToLocalChannel(const std::string& name) {
    NamedMutex mutex = NamedMutex::CreateOrOpen(name);

    std::lock_guard lock(mutex);

    const std::optional<SharedMemory> sharedMemory = SharedMemory::TryOpenExisting(name, ServerSharedMemorySize);
    if (!sharedMemory) {
        return {};
    }

    auto& counter = *static_cast<std::atomic<int32_t>*>(sharedMemory->data());
    const int32_t currentCounter = counter.fetch_add(1);
    const std::string specificName = name + "." + std::to_string(currentCounter);

    return std::make_unique<LocalChannel>(specificName, false);
}

[[nodiscard]] std::unique_ptr<ChannelServer> CreateLocalChannelServer(const std::string& name) {
    return std::make_unique<LocalChannelServer>(name);
}

}  // namespace DsVeosCoSim

#endif
