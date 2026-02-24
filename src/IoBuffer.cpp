// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "IoBuffer.hpp"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Channel.hpp"
#include "CoSimTypes.hpp"
#include "Environment.hpp"
#include "Format.hpp"
#include "Protocol.hpp"
#include "ProtocolLogger.hpp"
#include "RingBuffer.hpp"

#ifdef _WIN32
#include "OsUtilities.hpp"
#endif

namespace DsVeosCoSim {

namespace {

struct IoMetaData {
    IoSignal info{};
    size_t dataTypeSize{};
    size_t totalDataSize{};
    size_t signalIndex{};
};

using IoMetaDataPtr = IoMetaData*;

class IoData final {
public:
    IoData() = default;
    ~IoData() noexcept = default;

    IoData(const IoData&) = delete;
    IoData& operator=(const IoData&) = delete;

    IoData(IoData&&) = delete;
    IoData& operator=(IoData&&) = delete;

    [[nodiscard]] Result Initialize(const std::vector<IoSignal>& ioSignals) {
        size_t nextSignalIndex = 0;
        for (const auto& ioSignal : ioSignals) {
            if (ioSignal.length == 0) {
                return CreateError(Format("Invalid length 0 for IO signal '{}'.", ioSignal.name));
            }

            size_t dataTypeSize = GetDataTypeSize(ioSignal.dataType);
            if (dataTypeSize == 0) {
                return CreateError(Format("Invalid data type for IO signal '{}'.", ioSignal.name));
            }

            auto search = _metaDataLookup.find(ioSignal.id);
            if (search != _metaDataLookup.end()) {
                return CreateError(Format("Duplicated IO signal id {}.", ioSignal.id));
            }

            size_t totalDataSize = dataTypeSize * ioSignal.length;

            IoMetaData metaData{};
            metaData.info = ioSignal;
            metaData.dataTypeSize = dataTypeSize;
            metaData.totalDataSize = totalDataSize;
            metaData.signalIndex = nextSignalIndex++;

            _metaDataLookup[ioSignal.id] = metaData;
        }

        return CreateOk();
    }

    [[nodiscard]] Result FindMetaData(IoSignalId signalId, IoMetaDataPtr& metaData) {
        auto search = _metaDataLookup.find(signalId);
        if (search != _metaDataLookup.end()) {
            metaData = &search->second;
            return CreateOk();
        }

        return CreateError(Format("IO signal id {} is unknown.", signalId));
    }

    [[nodiscard]] std::unordered_map<IoSignalId, IoMetaData>& GetMetaDataLookup() {
        return _metaDataLookup;
    }

private:
    std::unordered_map<IoSignalId, IoMetaData> _metaDataLookup;
};

class IIoPart {
public:
    IIoPart() = default;
    virtual ~IIoPart() noexcept = default;

    IIoPart(const IIoPart&) = delete;
    IIoPart& operator=(const IIoPart&) = delete;

    IIoPart(IIoPart&&) = delete;
    IIoPart& operator=(IIoPart&&) = delete;

    [[nodiscard]] virtual Result Initialize() = 0;
    virtual void ClearData() = 0;
    [[nodiscard]] virtual Result Write(IoSignalId signalId, uint32_t length, const void* value) = 0;
    [[nodiscard]] virtual Result Read(IoSignalId signalId, uint32_t& length, void* value) = 0;
    [[nodiscard]] virtual Result Read(IoSignalId signalId, uint32_t& length, const void** value) = 0;
    [[nodiscard]] virtual Result Serialize(ChannelWriter& writer) = 0;
    [[nodiscard]] virtual Result Deserialize(ChannelReader& reader, SimulationTime simulationTime, const Callbacks& callbacks) = 0;
};

class LockedIoPart final : public IIoPart {
public:
    explicit LockedIoPart(std::unique_ptr<IIoPart> proxiedPart) : _proxiedPart(std::move(proxiedPart)) {
    }

    ~LockedIoPart() noexcept override = default;

    LockedIoPart(const LockedIoPart&) = delete;
    LockedIoPart& operator=(const LockedIoPart&) = delete;

    LockedIoPart(LockedIoPart&&) = delete;
    LockedIoPart& operator=(LockedIoPart&&) = delete;

    [[nodiscard]] Result Initialize() override {
        std::lock_guard lock(_mutex);
        return _proxiedPart->Initialize();
    }

    void ClearData() override {
        std::lock_guard lock(_mutex);
        _proxiedPart->ClearData();
    }

    [[nodiscard]] Result Write(IoSignalId signalId, uint32_t length, const void* value) override {
        std::lock_guard lock(_mutex);
        return _proxiedPart->Write(signalId, length, value);
    }

    [[nodiscard]] Result Read(IoSignalId signalId, uint32_t& length, void* value) override {
        std::lock_guard lock(_mutex);
        return _proxiedPart->Read(signalId, length, value);
    }

    [[nodiscard]] Result Read(IoSignalId signalId, uint32_t& length, const void** value) override {
        std::lock_guard lock(_mutex);
        return _proxiedPart->Read(signalId, length, value);
    }

    [[nodiscard]] Result Serialize(ChannelWriter& writer) override {
        std::lock_guard lock(_mutex);
        return _proxiedPart->Serialize(writer);
    }

    [[nodiscard]] Result Deserialize(ChannelReader& reader, SimulationTime simulationTime, const Callbacks& callbacks) override {
        std::lock_guard lock(_mutex);
        return _proxiedPart->Deserialize(reader, simulationTime, callbacks);
    }

private:
    std::unique_ptr<IIoPart> _proxiedPart;
    std::mutex _mutex;
};

class RemoteIoPart final : public IIoPart {
    struct Data {
        uint32_t currentLength{};
        bool isChanged{};
        std::vector<uint8_t> buffer;
    };

public:
    RemoteIoPart(IProtocol& protocol, const std::vector<IoSignal>& ioSignals) : _protocol(protocol), _ioSignals(ioSignals) {};
    ~RemoteIoPart() noexcept override = default;

    RemoteIoPart(const RemoteIoPart&) = delete;
    RemoteIoPart& operator=(const RemoteIoPart&) = delete;

    RemoteIoPart(RemoteIoPart&&) = delete;
    RemoteIoPart& operator=(RemoteIoPart&&) = delete;

    [[nodiscard]] Result Initialize() override {
        CheckResult(_ioData.Initialize(_ioSignals));
        std::unordered_map<IoSignalId, IoMetaData>& metaDataLookup = _ioData.GetMetaDataLookup();
        _changedSignalsQueue = RingBuffer<IoMetaDataPtr>(metaDataLookup.size());
        _dataVector.resize(metaDataLookup.size());
        for (auto& [signalId, metaData] : metaDataLookup) {
            Data data{};
            data.buffer.resize(metaData.totalDataSize);
            if (metaData.info.sizeKind == SizeKind::Fixed) {
                data.currentLength = metaData.info.length;
            }

            _dataVector[metaData.signalIndex] = data;
        }

        return CreateOk();
    }

    void ClearData() override {
        _changedSignalsQueue.Clear();

        for (auto& [signalId, metaData] : _ioData.GetMetaDataLookup()) {
            auto& [currentLength, isChanged, buffer] = _dataVector[metaData.signalIndex];
            isChanged = false;
            if (metaData.info.sizeKind == SizeKind::Variable) {
                currentLength = 0;
            }

            std::fill(buffer.begin(), buffer.end(), static_cast<uint8_t>(0));
        }
    }

    [[nodiscard]] Result Write(IoSignalId signalId, uint32_t length, const void* value) override {
        IoMetaDataPtr metaData{};
        CheckResult(_ioData.FindMetaData(signalId, metaData));
        auto& [currentLength, isChanged, buffer] = _dataVector[metaData->signalIndex];

        if (metaData->info.sizeKind == SizeKind::Variable) {
            if (length > metaData->info.length) {
                return CreateError(Format("Length of variable sized IO signal '{}' exceeds max size.", metaData->info.name));
            }

            if (currentLength != length) {
                if (!isChanged) {
                    isChanged = true;
                    _changedSignalsQueue.PushBack(metaData);
                }
            }

            currentLength = length;
        } else {
            if (length != metaData->info.length) {
                return CreateError(Format("Length of fixed sized IO signal '{}' must be {} but was {}.", metaData->info.name, metaData->info.length, length));
            }
        }

        size_t totalSize = metaData->dataTypeSize * length;

        int32_t compareResult = memcmp(buffer.data(), value, totalSize);
        if (compareResult == 0) {
            return CreateOk();
        }

        (void)memcpy(buffer.data(), value, totalSize);

        if (!isChanged) {
            isChanged = true;
            _changedSignalsQueue.PushBack(metaData);
        }

        return CreateOk();
    }

    [[nodiscard]] Result Read(IoSignalId signalId, uint32_t& length, void* value) override {
        IoMetaDataPtr metaData{};
        CheckResult(_ioData.FindMetaData(signalId, metaData));
        Data& data = _dataVector[metaData->signalIndex];

        length = data.currentLength;
        size_t totalSize = metaData->dataTypeSize * length;
        (void)memcpy(value, data.buffer.data(), totalSize);
        return CreateOk();
    }

    [[nodiscard]] Result Read(IoSignalId signalId, uint32_t& length, const void** value) override {
        IoMetaDataPtr metaData{};
        CheckResult(_ioData.FindMetaData(signalId, metaData));
        Data& data = _dataVector[metaData->signalIndex];

        length = data.currentLength;
        *value = data.buffer.data();
        return CreateOk();
    }

    [[nodiscard]] Result Serialize(ChannelWriter& writer) override {
        CheckResultWithMessage(_protocol.WriteSize(writer, _changedSignalsQueue.Size()), "Could not write count of changed signals.");
        if (_changedSignalsQueue.IsEmpty()) {
            return CreateOk();
        }

        while (!_changedSignalsQueue.IsEmpty()) {
            IoMetaDataPtr& metaData = _changedSignalsQueue.PopFront();
            auto& [currentLength, isChanged, buffer] = _dataVector[metaData->signalIndex];

            CheckResultWithMessage(_protocol.WriteSignalId(writer, metaData->info.id), "Could not write signal id.");

            if (metaData->info.sizeKind == SizeKind::Variable) {
                CheckResultWithMessage(_protocol.WriteLength(writer, currentLength), "Could not write signal length.");
            }

            size_t totalSize = metaData->dataTypeSize * currentLength;
            CheckResultWithMessage(_protocol.WriteData(writer, buffer.data(), totalSize), "Could not write signal data.");
            isChanged = false;

            if (IsProtocolTracingEnabled()) {
                LogProtocolDataTraceSignal(metaData->info.id, currentLength, metaData->info.dataType, buffer.data());
            }
        }

        return CreateOk();
    }

    [[nodiscard]] Result Deserialize(ChannelReader& reader, SimulationTime simulationTime, const Callbacks& callbacks) override {
        size_t ioSignalChangedCount = 0;
        CheckResultWithMessage(_protocol.ReadSize(reader, ioSignalChangedCount), "Could not read count of changed signals.");

        for (size_t i = 0; i < ioSignalChangedCount; i++) {
            IoSignalId signalId{};
            CheckResultWithMessage(_protocol.ReadSignalId(reader, signalId), "Could not read signal id.");

            IoMetaDataPtr metaData{};
            CheckResult(_ioData.FindMetaData(signalId, metaData));
            Data& data = _dataVector[metaData->signalIndex];

            if (metaData->info.sizeKind == SizeKind::Variable) {
                uint32_t length = 0;
                CheckResultWithMessage(_protocol.ReadLength(reader, length), "Could not read signal length.");
                if (length > metaData->info.length) {
                    return CreateError(Format("Length of variable sized IO signal '{}' exceeds max size.", metaData->info.name));
                }

                data.currentLength = length;
            }

            size_t totalSize = metaData->dataTypeSize * data.currentLength;
            CheckResultWithMessage(_protocol.ReadData(reader, data.buffer.data(), totalSize), "Could not read signal data.");

            if (IsProtocolTracingEnabled()) {
                LogProtocolDataTraceSignal(metaData->info.id, data.currentLength, metaData->info.dataType, data.buffer.data());
            }

            if (callbacks.incomingSignalChangedCallback) {
                callbacks.incomingSignalChangedCallback(simulationTime, metaData->info, data.currentLength, data.buffer.data());
            }
        }

        return CreateOk();
    }

private:
    IProtocol& _protocol;
    const std::vector<IoSignal>& _ioSignals;
    IoData _ioData;
    std::vector<Data> _dataVector;
    RingBuffer<IoMetaDataPtr> _changedSignalsQueue;
};

#ifdef _WIN32

class LocalIoPart final : public IIoPart {
    struct DataBuffer {
        uint32_t currentLength{};
        uint8_t data[1]{};
    };

    using DataBufferPtr = DataBuffer*;

    struct Data {
        size_t offsetOfDataBufferInShm{};
        size_t offsetOfBackupDataBufferInShm{};
        bool isChanged{};
    };

public:
    ~LocalIoPart() noexcept override = default;
    LocalIoPart(IProtocol& protocol, const std::vector<IoSignal>& ioSignals, std::string name)
        : _protocol(protocol), _ioSignals(ioSignals), _name(std::move(name)) {};

    LocalIoPart(const LocalIoPart&) = delete;
    LocalIoPart& operator=(const LocalIoPart&) = delete;

    LocalIoPart(LocalIoPart&&) = delete;
    LocalIoPart& operator=(LocalIoPart&&) = delete;

    [[nodiscard]] Result Initialize() override {
        CheckResult(_ioData.Initialize(_ioSignals));
        std::unordered_map<IoSignalId, IoMetaData>& metaDataLookup = _ioData.GetMetaDataLookup();
        _changedSignalsQueue = RingBuffer<IoMetaDataPtr>(metaDataLookup.size());
        _dataVector.resize(metaDataLookup.size());

        size_t totalSize{};
        for (auto& [signalId, metaData] : metaDataLookup) {
            Data data{};
            data.offsetOfDataBufferInShm = totalSize;
            totalSize += sizeof(uint32_t) + metaData.totalDataSize;  // Current length + data buffer

            data.offsetOfBackupDataBufferInShm = totalSize;
            totalSize += sizeof(uint32_t) + metaData.totalDataSize;  // Current length + data buffer

            _dataVector[metaData.signalIndex] = data;
        }

        if (totalSize > 0) {
            CheckResult(SharedMemory::CreateOrOpen(_name, totalSize, _sharedMemory));
        }

        for (auto& [signalId, metaData] : metaDataLookup) {
            Data& data = _dataVector[metaData.signalIndex];
            DataBufferPtr dataBuffer = GetDataBuffer(data.offsetOfDataBufferInShm);
            DataBufferPtr backupDataBuffer = GetDataBuffer(data.offsetOfBackupDataBufferInShm);
            if (metaData.info.sizeKind == SizeKind::Fixed) {
                dataBuffer->currentLength = metaData.info.length;
                backupDataBuffer->currentLength = metaData.info.length;
            }
        }

        return CreateOk();
    }

    void ClearData() override {
        _changedSignalsQueue.Clear();

        for (auto& [signalId, metaData] : _ioData.GetMetaDataLookup()) {
            Data& data = _dataVector[metaData.signalIndex];
            data.isChanged = false;

            DataBufferPtr dataBuffer = GetDataBuffer(data.offsetOfDataBufferInShm);
            DataBufferPtr backupDataBuffer = GetDataBuffer(data.offsetOfBackupDataBufferInShm);

            if (data.offsetOfDataBufferInShm > data.offsetOfBackupDataBufferInShm) {
                FlipBuffers(data);
            }

            if (metaData.info.sizeKind == SizeKind::Variable) {
                dataBuffer->currentLength = 0;
                backupDataBuffer->currentLength = 0;
            }

            (void)memset(dataBuffer->data, 0, metaData.dataTypeSize * metaData.info.length);
            (void)memset(backupDataBuffer->data, 0, metaData.dataTypeSize * metaData.info.length);
        }
    }

    [[nodiscard]] Result Write(IoSignalId signalId, uint32_t length, const void* value) override {
        IoMetaDataPtr metaData{};
        CheckResult(_ioData.FindMetaData(signalId, metaData));
        Data& data = _dataVector[metaData->signalIndex];

        DataBufferPtr dataBuffer = GetDataBuffer(data.offsetOfDataBufferInShm);

        bool currentLengthChanged{};
        if (metaData->info.sizeKind == SizeKind::Variable) {
            if (length > metaData->info.length) {
                return CreateError(Format("Length of variable sized IO signal '{}' exceeds max size.", metaData->info.name));
            }

            currentLengthChanged = dataBuffer->currentLength != length;
        } else {
            if (length != metaData->info.length) {
                return CreateError(Format("Length of fixed sized IO signal '{}' must be {} but was {}.", metaData->info.name, metaData->info.length, length));
            }
        }

        size_t totalSize = metaData->dataTypeSize * length;

        bool dataChanged = memcmp(dataBuffer->data, value, totalSize) != 0;

        if (!currentLengthChanged && !dataChanged) {
            return CreateOk();
        }

        if (!data.isChanged) {
            data.isChanged = true;
            _changedSignalsQueue.PushBack(metaData);
            FlipBuffers(data);
            dataBuffer = GetDataBuffer(data.offsetOfDataBufferInShm);
        }

        dataBuffer->currentLength = length;
        if (dataChanged) {
            (void)memcpy(dataBuffer->data, value, totalSize);
        }

        return CreateOk();
    }

    [[nodiscard]] Result Read(IoSignalId signalId, uint32_t& length, void* value) override {
        IoMetaDataPtr metaData{};
        CheckResult(_ioData.FindMetaData(signalId, metaData));
        Data& data = _dataVector[metaData->signalIndex];

        DataBufferPtr dataBuffer = GetDataBuffer(data.offsetOfDataBufferInShm);

        length = dataBuffer->currentLength;
        size_t totalSize = metaData->dataTypeSize * length;
        (void)memcpy(value, dataBuffer->data, totalSize);
        return CreateOk();
    }

    [[nodiscard]] Result Read(IoSignalId signalId, uint32_t& length, const void** value) override {
        IoMetaDataPtr metaData{};
        CheckResult(_ioData.FindMetaData(signalId, metaData));
        Data& data = _dataVector[metaData->signalIndex];

        DataBufferPtr dataBuffer = GetDataBuffer(data.offsetOfDataBufferInShm);

        length = dataBuffer->currentLength;
        *value = dataBuffer->data;
        return CreateOk();
    }

    [[nodiscard]] Result Serialize(ChannelWriter& writer) override {
        CheckResultWithMessage(_protocol.WriteSize(writer, _changedSignalsQueue.Size()), "Could not write count of changed signals.");
        if (_changedSignalsQueue.IsEmpty()) {
            return CreateOk();
        }

        while (!_changedSignalsQueue.IsEmpty()) {
            IoMetaDataPtr& metaData = _changedSignalsQueue.PopFront();
            Data& data = _dataVector[metaData->signalIndex];

            if (IsProtocolTracingEnabled()) {
                DataBufferPtr dataBuffer = GetDataBuffer(data.offsetOfDataBufferInShm);
                LogProtocolDataTraceSignal(metaData->info.id, dataBuffer->currentLength, metaData->info.dataType, dataBuffer->data);
            }

            CheckResultWithMessage(_protocol.WriteSignalId(writer, metaData->info.id), "Could not write signal id.");

            data.isChanged = false;
        }

        return CreateOk();
    }

    [[nodiscard]] Result Deserialize(ChannelReader& reader, SimulationTime simulationTime, const Callbacks& callbacks) override {
        size_t ioSignalChangedCount = 0;
        CheckResultWithMessage(_protocol.ReadSize(reader, ioSignalChangedCount), "Could not read count of changed signals.");

        for (size_t i = 0; i < ioSignalChangedCount; i++) {
            IoSignalId signalId{};
            CheckResultWithMessage(_protocol.ReadSignalId(reader, signalId), "Could not read signal id.");

            IoMetaDataPtr metaData{};
            CheckResult(_ioData.FindMetaData(signalId, metaData));
            Data& data = _dataVector[metaData->signalIndex];

            FlipBuffers(data);

            DataBufferPtr dataBuffer = GetDataBuffer(data.offsetOfDataBufferInShm);

            if (IsProtocolTracingEnabled()) {
                LogProtocolDataTraceSignal(metaData->info.id, dataBuffer->currentLength, metaData->info.dataType, dataBuffer->data);
            }

            if (callbacks.incomingSignalChangedCallback) {
                callbacks.incomingSignalChangedCallback(simulationTime, metaData->info, dataBuffer->currentLength, dataBuffer->data);
            }
        }

        return CreateOk();
    }

private:
    [[nodiscard]] DataBufferPtr GetDataBuffer(size_t offset) const {
        return reinterpret_cast<DataBufferPtr>(_sharedMemory.GetData() + offset);
    }

    static void FlipBuffers(Data& data) {
        std::swap(data.offsetOfDataBufferInShm, data.offsetOfBackupDataBufferInShm);
    }

    IProtocol& _protocol;
    const std::vector<IoSignal>& _ioSignals;
    std::string _name;
    IoData _ioData;
    std::vector<Data> _dataVector;
    SharedMemory _sharedMemory;
    RingBuffer<IoMetaDataPtr> _changedSignalsQueue;
};

#endif

class IoBufferImpl final : public IoBuffer {
public:
    IoBufferImpl() = default;
    ~IoBufferImpl() noexcept override = default;

    IoBufferImpl(const IoBufferImpl&) = delete;
    IoBufferImpl& operator=(const IoBufferImpl&) = delete;

    IoBufferImpl(IoBufferImpl&&) = delete;
    IoBufferImpl& operator=(IoBufferImpl&&) = delete;

    [[nodiscard]] Result Initialize(CoSimType coSimType,
                                    [[maybe_unused]] ConnectionKind connectionKind,
                                    const std::string& name,
                                    const std::vector<IoSignal>& incomingSignals,
                                    const std::vector<IoSignal>& outgoingSignals,
                                    IProtocol& protocol) {
        const std::vector<IoSignal>* writeSignals = &outgoingSignals;
        const std::vector<IoSignal>* readSignals = &incomingSignals;
        if (coSimType == CoSimType::Server) {
            writeSignals = &incomingSignals;
            readSignals = &outgoingSignals;
        }

#ifdef _WIN32
        if (connectionKind == ConnectionKind::Local) {
            std::string outgoingName = name + ".Outgoing";
            std::string incomingName = name + ".Incoming";
            if (coSimType == CoSimType::Server) {
                std::swap(incomingName, outgoingName);
            }

            _readBuffer = std::make_unique<LocalIoPart>(protocol, *readSignals, incomingName);
            _writeBuffer = std::make_unique<LocalIoPart>(protocol, *writeSignals, outgoingName);
        } else {
#endif
            _readBuffer = std::make_unique<RemoteIoPart>(protocol, *readSignals);
            _writeBuffer = std::make_unique<RemoteIoPart>(protocol, *writeSignals);
#ifdef _WIN32
        }
#endif

        if (coSimType == CoSimType::Client) {
            _readBuffer = std::make_unique<LockedIoPart>(std::move(_readBuffer));
            _writeBuffer = std::make_unique<LockedIoPart>(std::move(_writeBuffer));
        }

        CheckResult(_readBuffer->Initialize());
        CheckResult(_writeBuffer->Initialize());

        ClearData();
        return CreateOk();
    }

    void ClearData() const override {
        _readBuffer->ClearData();
        _writeBuffer->ClearData();
    }

    [[nodiscard]] Result Write(IoSignalId signalId, uint32_t length, const void* value) const override {
        return _writeBuffer->Write(signalId, length, value);
    }

    [[nodiscard]] Result Read(IoSignalId signalId, uint32_t& length, void* value) const override {
        return _readBuffer->Read(signalId, length, value);
    }

    [[nodiscard]] Result Read(IoSignalId signalId, uint32_t& length, const void** value) const override {
        return _readBuffer->Read(signalId, length, value);
    }

    [[nodiscard]] Result Serialize(ChannelWriter& writer) const override {
        return _writeBuffer->Serialize(writer);
    }

    [[nodiscard]] Result Deserialize(ChannelReader& reader, SimulationTime simulationTime, const Callbacks& callbacks) const override {
        return _readBuffer->Deserialize(reader, simulationTime, callbacks);
    }

private:
    std::unique_ptr<IIoPart> _writeBuffer;
    std::unique_ptr<IIoPart> _readBuffer;
};

}  // namespace

[[nodiscard]] Result CreateIoBuffer(CoSimType coSimType,
                                    ConnectionKind connectionKind,
                                    const std::string& name,
                                    const std::vector<IoSignal>& incomingSignals,
                                    const std::vector<IoSignal>& outgoingSignals,
                                    IProtocol& protocol,
                                    std::unique_ptr<IoBuffer>& ioBuffer) {
    auto tmpIoBuffer = std::make_unique<IoBufferImpl>();
    CheckResult(tmpIoBuffer->Initialize(coSimType, connectionKind, name, incomingSignals, outgoingSignals, protocol));
    ioBuffer = std::move(tmpIoBuffer);
    return CreateOk();
}

}  // namespace DsVeosCoSim
