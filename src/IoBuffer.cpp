// Copyright dSPACE GmbH. All rights reserved.

#include "IoBuffer.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "Channel.h"
#include "CoSimHelper.h"
#include "DsVeosCoSim/CoSimTypes.h"
#include "Environment.h"
#include "Protocol.h"
#include "RingBuffer.h"

#ifdef _WIN32
#include "OsUtilities.h"
#endif

namespace DsVeosCoSim {

namespace {

class IoPartBufferBase {
protected:
    struct MetaData {
        IoSignal info{};
        size_t dataTypeSize{};
        size_t totalDataSize{};
        size_t signalIndex{};
    };

public:
    IoPartBufferBase() = default;
    virtual ~IoPartBufferBase() = default;

    IoPartBufferBase(const IoPartBufferBase&) = delete;
    IoPartBufferBase& operator=(const IoPartBufferBase&) = delete;

    IoPartBufferBase(IoPartBufferBase&&) = delete;
    IoPartBufferBase& operator=(IoPartBufferBase&&) = delete;

    [[nodiscard]] virtual Result Initialize(CoSimType coSimType,
                                            [[maybe_unused]] const std::string& name,
                                            const std::vector<IoSignal>& signals) {
        _coSimType = coSimType;
        _changedSignalsQueue = RingBuffer<MetaData*>(signals.size());
        size_t nextSignalIndex = 0;
        for (const auto& signal : signals) {
            if (signal.length == 0) {
                std::string message = "Invalid length 0 for IO signal '";
                message.append(signal.name);
                message.append("'.");
                LogError(message);
                return Result::Error;
            }

            size_t dataTypeSize = GetDataTypeSize(signal.dataType);
            if (dataTypeSize == 0) {
                std::string message = "Invalid data type for IO signal '";
                message.append(signal.name);
                message.append("'.");
                LogError(message);
                return Result::Error;
            }

            auto search = _metaDataLookup.find(signal.id);
            if (search != _metaDataLookup.end()) {
                std::string message = "Duplicated IO signal id ";
                message.append(ToString(signal.id));
                message.append(".");
                LogError(message);
                return Result::Error;
            }

            size_t totalDataSize = dataTypeSize * signal.length;

            MetaData metaData{};
            metaData.info = signal;
            metaData.dataTypeSize = dataTypeSize;
            metaData.totalDataSize = totalDataSize;
            metaData.signalIndex = nextSignalIndex++;

            _metaDataLookup[signal.id] = metaData;
        }

        return Result::Ok;
    }

    void ClearData() {
        if (_coSimType == CoSimType::Client) {
            std::lock_guard lock(_mutex);
            ClearDataInternal();
            return;
        }

        ClearDataInternal();
    }

    [[nodiscard]] Result Write(IoSignalId signalId, uint32_t length, const void* value) {
        if (_coSimType == CoSimType::Client) {
            std::lock_guard lock(_mutex);
            return WriteInternal(signalId, length, value);
        }

        return WriteInternal(signalId, length, value);
    }

    [[nodiscard]] Result Read(IoSignalId signalId, uint32_t& length, void* value) {
        if (_coSimType == CoSimType::Client) {
            std::lock_guard lock(_mutex);
            return ReadInternal(signalId, length, value);
        }

        return ReadInternal(signalId, length, value);
    }

    [[nodiscard]] Result Read(IoSignalId signalId, uint32_t& length, const void** value) {
        if (_coSimType == CoSimType::Client) {
            std::lock_guard lock(_mutex);
            return ReadInternal(signalId, length, value);
        }

        return ReadInternal(signalId, length, value);
    }

    [[nodiscard]] Result Serialize(ChannelWriter& writer) {
        if (_coSimType == CoSimType::Client) {
            std::lock_guard lock(_mutex);
            return SerializeInternal(writer);
        }

        return SerializeInternal(writer);
    }

    [[nodiscard]] Result Deserialize(ChannelReader& reader, SimulationTime simulationTime, const Callbacks& callbacks) {
        if (_coSimType == CoSimType::Client) {
            std::lock_guard lock(_mutex);
            return DeserializeInternal(reader, simulationTime, callbacks);
        }

        return DeserializeInternal(reader, simulationTime, callbacks);
    }

protected:
    virtual void ClearDataInternal() = 0;

    [[nodiscard]] virtual Result WriteInternal(IoSignalId signalId, uint32_t length, const void* value) = 0;
    [[nodiscard]] virtual Result ReadInternal(IoSignalId signalId, uint32_t& length, void* value) = 0;
    [[nodiscard]] virtual Result ReadInternal(IoSignalId signalId, uint32_t& length, const void** value) = 0;

    [[nodiscard]] virtual Result SerializeInternal(ChannelWriter& writer) = 0;
    [[nodiscard]] virtual Result DeserializeInternal(ChannelReader& reader,
                                                     SimulationTime simulationTime,
                                                     const Callbacks& callbacks) = 0;

    [[nodiscard]] Result FindMetaData(IoSignalId signalId, MetaData*& metaData) {
        auto search = _metaDataLookup.find(signalId);
        if (search != _metaDataLookup.end()) {
            metaData = &search->second;
            return Result::Ok;
        }

        std::string message = "IO signal id '";
        message.append(ToString(signalId));
        message.append("' is unknown.");
        LogError(message);
        return Result::Error;
    }

    CoSimType _coSimType{};
    std::unordered_map<IoSignalId, MetaData> _metaDataLookup;
    RingBuffer<MetaData*> _changedSignalsQueue;

private:
    std::mutex _mutex;
};

class RemoteIoPartBuffer final : public IoPartBufferBase {
    struct Data {
        uint32_t currentLength{};
        bool isChanged{};
        std::vector<uint8_t> buffer;
    };

public:
    RemoteIoPartBuffer() = default;
    ~RemoteIoPartBuffer() override = default;

    RemoteIoPartBuffer(const RemoteIoPartBuffer&) = delete;
    RemoteIoPartBuffer& operator=(const RemoteIoPartBuffer&) = delete;

    RemoteIoPartBuffer(RemoteIoPartBuffer&&) = delete;
    RemoteIoPartBuffer& operator=(RemoteIoPartBuffer&&) = delete;

    [[nodiscard]] Result Initialize(CoSimType coSimType,
                                    const std::string& name,
                                    const std::vector<IoSignal>& signals) override {
        CheckResult(IoPartBufferBase::Initialize(coSimType, name, signals));

        _dataVector.resize(_metaDataLookup.size());
        for (auto& [signalId, metaData] : _metaDataLookup) {
            Data data{};
            data.buffer.resize(metaData.totalDataSize);
            if (metaData.info.sizeKind == SizeKind::Fixed) {
                data.currentLength = metaData.info.length;
            }

            _dataVector[metaData.signalIndex] = data;
        }

        return Result::Ok;
    }

protected:
    void ClearDataInternal() override {
        _changedSignalsQueue.Clear();

        for (auto& [signalId, metaData] : _metaDataLookup) {
            auto& [currentLength, isChanged, buffer] = _dataVector[metaData.signalIndex];
            isChanged = false;
            if (metaData.info.sizeKind == SizeKind::Variable) {
                currentLength = 0;
            }

            std::fill(buffer.begin(), buffer.end(), static_cast<uint8_t>(0));
        }
    }

    [[nodiscard]] Result WriteInternal(IoSignalId signalId, uint32_t length, const void* value) override {
        MetaData* metaData{};
        CheckResult(FindMetaData(signalId, metaData));
        auto& [currentLength, isChanged, buffer] = _dataVector[metaData->signalIndex];

        if (metaData->info.sizeKind == SizeKind::Variable) {
            if (length > metaData->info.length) {
                std::string message = "Length of variable sized IO signal '";
                message.append(metaData->info.name);
                message.append("' exceeds max size.");
                LogError(message);
                return Result::Error;
            }

            if (currentLength != length) {
                if (!isChanged) {
                    isChanged = true;
                    _changedSignalsQueue.PushBack(&*metaData);
                }
            }

            currentLength = length;
        } else {
            if (length != metaData->info.length) {
                std::string message = "Length of fixed sized IO signal '";
                message.append(metaData->info.name);
                message.append("' must be ");
                message.append(std::to_string(metaData->info.length));
                message.append(" but was ");
                message.append(std::to_string(length));
                message.append(".");
                LogError(message);
                return Result::Error;
            }
        }

        size_t totalSize = metaData->dataTypeSize * length;

        int32_t compareResult = memcmp(buffer.data(), value, totalSize);
        if (compareResult == 0) {
            return Result::Ok;
        }

        (void)memcpy(buffer.data(), value, totalSize);

        if (!isChanged) {
            isChanged = true;
            _changedSignalsQueue.PushBack(&*metaData);
        }

        return Result::Ok;
    }

    [[nodiscard]] Result ReadInternal(IoSignalId signalId, uint32_t& length, void* value) override {
        MetaData* metaData{};
        CheckResult(FindMetaData(signalId, metaData));
        Data& data = _dataVector[metaData->signalIndex];

        length = data.currentLength;
        size_t totalSize = metaData->dataTypeSize * length;
        (void)memcpy(value, data.buffer.data(), totalSize);
        return Result::Ok;
    }

    [[nodiscard]] Result ReadInternal(IoSignalId signalId, uint32_t& length, const void** value) override {
        MetaData* metaData{};
        CheckResult(FindMetaData(signalId, metaData));
        Data& data = _dataVector[metaData->signalIndex];

        length = data.currentLength;
        *value = data.buffer.data();
        return Result::Ok;
    }

    [[nodiscard]] Result SerializeInternal(ChannelWriter& writer) override {
        CheckResultWithMessage(Protocol::WriteSize(writer, _changedSignalsQueue.Size()),
                               "Could not write count of changed signals.");
        if (_changedSignalsQueue.IsEmpty()) {
            return Result::Ok;
        }

        while (!_changedSignalsQueue.IsEmpty()) {
            MetaData*& metaData = _changedSignalsQueue.PopFront();
            auto& [currentLength, isChanged, buffer] = _dataVector[metaData->signalIndex];

            CheckResultWithMessage(Protocol::WriteSignalId(writer, metaData->info.id), "Could not write signal id.");

            if (metaData->info.sizeKind == SizeKind::Variable) {
                CheckResultWithMessage(Protocol::WriteLength(writer, currentLength), "Could not write signal length.");
            }

            size_t totalSize = metaData->dataTypeSize * currentLength;
            CheckResultWithMessage(writer.Write(buffer.data(), totalSize), "Could not write signal data.");
            isChanged = false;

            if (IsProtocolTracingEnabled()) {
                LogProtocolDataTraceSignal(metaData->info.id, currentLength, metaData->info.dataType, buffer.data());
            }
        }

        return Result::Ok;
    }

    [[nodiscard]] Result DeserializeInternal(ChannelReader& reader,
                                             SimulationTime simulationTime,
                                             const Callbacks& callbacks) override {
        size_t ioSignalChangedCount = 0;
        CheckResultWithMessage(Protocol::ReadSize(reader, ioSignalChangedCount),
                               "Could not read count of changed signals.");

        for (size_t i = 0; i < ioSignalChangedCount; i++) {
            IoSignalId signalId{};
            CheckResultWithMessage(Protocol::ReadSignalId(reader, signalId), "Could not read signal id.");

            MetaData* metaData{};
            CheckResult(FindMetaData(signalId, metaData));
            Data& data = _dataVector[metaData->signalIndex];

            if (metaData->info.sizeKind == SizeKind::Variable) {
                uint32_t length = 0;
                CheckResultWithMessage(Protocol::ReadLength(reader, length), "Could not read signal length.");
                if (length > metaData->info.length) {
                    std::string message = "Length of variable sized IO signal '";
                    message.append(metaData->info.name);
                    message.append("' exceeds max size.");
                    LogError(message);
                    return Result::Error;
                }

                data.currentLength = length;
            }

            size_t totalSize = metaData->dataTypeSize * data.currentLength;
            CheckResultWithMessage(reader.Read(data.buffer.data(), totalSize), "Could not read signal data.");

            if (IsProtocolTracingEnabled()) {
                LogProtocolDataTraceSignal(metaData->info.id,
                                           data.currentLength,
                                           metaData->info.dataType,
                                           data.buffer.data());
            }

            if (callbacks.incomingSignalChangedCallback) {
                callbacks.incomingSignalChangedCallback(simulationTime,
                                                        metaData->info,
                                                        data.currentLength,
                                                        data.buffer.data());
            }
        }

        return Result::Ok;
    }

private:
    std::vector<Data> _dataVector;
};

#ifdef _WIN32

class LocalIoPartBuffer final : public IoPartBufferBase {
    struct DataBuffer {
        uint32_t currentLength{};
        uint8_t data[1]{};
    };

    struct Data {
        size_t offsetOfDataBufferInShm{};
        size_t offsetOfBackupDataBufferInShm{};
        bool isChanged{};
    };

public:
    LocalIoPartBuffer() = default;
    ~LocalIoPartBuffer() override = default;

    LocalIoPartBuffer(const LocalIoPartBuffer&) = delete;
    LocalIoPartBuffer& operator=(const LocalIoPartBuffer&) = delete;

    LocalIoPartBuffer(LocalIoPartBuffer&&) = delete;
    LocalIoPartBuffer& operator=(LocalIoPartBuffer&&) = delete;

    [[nodiscard]] Result Initialize(CoSimType coSimType,
                                    const std::string& name,
                                    const std::vector<IoSignal>& signals) override {
        CheckResult(IoPartBufferBase::Initialize(coSimType, name, signals));
        _dataVector.resize(_metaDataLookup.size());

        size_t totalSize{};
        for (auto& [signalId, metaData] : _metaDataLookup) {
            Data data{};
            data.offsetOfDataBufferInShm = totalSize;
            totalSize += sizeof(uint32_t) + metaData.totalDataSize;  // Current length + data buffer

            data.offsetOfBackupDataBufferInShm = totalSize;
            totalSize += sizeof(uint32_t) + metaData.totalDataSize;  // Current length + data buffer

            _dataVector[metaData.signalIndex] = data;
        }

        if (totalSize > 0) {
            CheckResult(SharedMemory::CreateOrOpen(name, totalSize, _sharedMemory));
        }

        for (auto& [signalId, metaData] : _metaDataLookup) {
            Data& data = _dataVector[metaData.signalIndex];
            DataBuffer* dataBuffer = GetDataBuffer(data.offsetOfDataBufferInShm);
            DataBuffer* backupDataBuffer = GetDataBuffer(data.offsetOfBackupDataBufferInShm);
            if (metaData.info.sizeKind == SizeKind::Fixed) {
                dataBuffer->currentLength = metaData.info.length;
                backupDataBuffer->currentLength = metaData.info.length;
            }
        }

        return Result::Ok;
    }

protected:
    void ClearDataInternal() override {
        _changedSignalsQueue.Clear();

        for (auto& [signalId, metaData] : _metaDataLookup) {
            Data& data = _dataVector[metaData.signalIndex];
            data.isChanged = false;

            DataBuffer* dataBuffer = GetDataBuffer(data.offsetOfDataBufferInShm);
            DataBuffer* backupDataBuffer = GetDataBuffer(data.offsetOfBackupDataBufferInShm);

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

    [[nodiscard]] Result WriteInternal(IoSignalId signalId, uint32_t length, const void* value) override {
        MetaData* metaData{};
        CheckResult(FindMetaData(signalId, metaData));
        Data& data = _dataVector[metaData->signalIndex];

        DataBuffer* dataBuffer = GetDataBuffer(data.offsetOfDataBufferInShm);

        bool currentLengthChanged{};
        if (metaData->info.sizeKind == SizeKind::Variable) {
            if (length > metaData->info.length) {
                std::string message = "Length of variable sized IO signal '";
                message.append(metaData->info.name);
                message.append("' exceeds max size.");
                LogError(message);
                return Result::Error;
            }

            currentLengthChanged = dataBuffer->currentLength != length;
        } else {
            if (length != metaData->info.length) {
                std::string message = "Length of fixed sized IO signal '";
                message.append(metaData->info.name);
                message.append("' must be ");
                message.append(std::to_string(metaData->info.length));
                message.append(" but was ");
                message.append(std::to_string(length));
                message.append(".");
                LogError(message);
                return Result::Error;
            }
        }

        size_t totalSize = metaData->dataTypeSize * length;

        bool dataChanged = memcmp(dataBuffer->data, value, totalSize) != 0;

        if (!currentLengthChanged && !dataChanged) {
            return Result::Ok;
        }

        if (!data.isChanged) {
            data.isChanged = true;
            _changedSignalsQueue.PushBack(&*metaData);
            FlipBuffers(data);
            dataBuffer = GetDataBuffer(data.offsetOfDataBufferInShm);
        }

        dataBuffer->currentLength = length;
        if (dataChanged) {
            (void)memcpy(dataBuffer->data, value, totalSize);
        }

        return Result::Ok;
    }

    [[nodiscard]] Result ReadInternal(IoSignalId signalId, uint32_t& length, void* value) override {
        MetaData* metaData{};
        CheckResult(FindMetaData(signalId, metaData));
        Data& data = _dataVector[metaData->signalIndex];

        DataBuffer* dataBuffer = GetDataBuffer(data.offsetOfDataBufferInShm);

        length = dataBuffer->currentLength;
        size_t totalSize = metaData->dataTypeSize * length;
        (void)memcpy(value, dataBuffer->data, totalSize);
        return Result::Ok;
    }

    [[nodiscard]] Result ReadInternal(IoSignalId signalId, uint32_t& length, const void** value) override {
        MetaData* metaData{};
        CheckResult(FindMetaData(signalId, metaData));
        Data& data = _dataVector[metaData->signalIndex];

        DataBuffer* dataBuffer = GetDataBuffer(data.offsetOfDataBufferInShm);

        length = dataBuffer->currentLength;
        *value = dataBuffer->data;
        return Result::Ok;
    }

    [[nodiscard]] Result SerializeInternal(ChannelWriter& writer) override {
        CheckResultWithMessage(Protocol::WriteSize(writer, _changedSignalsQueue.Size()),
                               "Could not write count of changed signals.");
        if (_changedSignalsQueue.IsEmpty()) {
            return Result::Ok;
        }

        while (!_changedSignalsQueue.IsEmpty()) {
            MetaData*& metaData = _changedSignalsQueue.PopFront();
            Data& data = _dataVector[metaData->signalIndex];

            if (IsProtocolTracingEnabled()) {
                DataBuffer* dataBuffer = GetDataBuffer(data.offsetOfDataBufferInShm);
                LogProtocolDataTraceSignal(metaData->info.id,
                                           dataBuffer->currentLength,
                                           metaData->info.dataType,
                                           dataBuffer->data);
            }

            CheckResultWithMessage(Protocol::WriteSignalId(writer, metaData->info.id), "Could not write signal id.");

            data.isChanged = false;
        }

        return Result::Ok;
    }

    [[nodiscard]] Result DeserializeInternal(ChannelReader& reader,
                                             SimulationTime simulationTime,
                                             const Callbacks& callbacks) override {
        size_t ioSignalChangedCount = 0;
        CheckResultWithMessage(Protocol::ReadSize(reader, ioSignalChangedCount),
                               "Could not read count of changed signals.");

        for (size_t i = 0; i < ioSignalChangedCount; i++) {
            IoSignalId signalId{};
            CheckResultWithMessage(Protocol::ReadSignalId(reader, signalId), "Could not read signal id.");

            MetaData* metaData{};
            CheckResult(FindMetaData(signalId, metaData));
            Data& data = _dataVector[metaData->signalIndex];

            FlipBuffers(data);

            DataBuffer* dataBuffer = GetDataBuffer(data.offsetOfDataBufferInShm);

            if (IsProtocolTracingEnabled()) {
                LogProtocolDataTraceSignal(metaData->info.id,
                                           dataBuffer->currentLength,
                                           metaData->info.dataType,
                                           dataBuffer->data);
            }

            if (callbacks.incomingSignalChangedCallback) {
                callbacks.incomingSignalChangedCallback(simulationTime,
                                                        metaData->info,
                                                        dataBuffer->currentLength,
                                                        dataBuffer->data);
            }
        }

        return Result::Ok;
    }

private:
    [[nodiscard]] DataBuffer* GetDataBuffer(size_t offset) const {
        return reinterpret_cast<DataBuffer*>(static_cast<uint8_t*>(_sharedMemory.GetData()) + offset);
    }

    static void FlipBuffers(Data& data) {
        std::swap(data.offsetOfDataBufferInShm, data.offsetOfBackupDataBufferInShm);
    }

    std::vector<Data> _dataVector;
    SharedMemory _sharedMemory;
};

#endif

class IoBufferImpl final : public IoBuffer {
public:
    IoBufferImpl() = default;
    ~IoBufferImpl() override = default;

    IoBufferImpl(const IoBufferImpl&) = delete;
    IoBufferImpl& operator=(const IoBufferImpl&) = delete;

    IoBufferImpl(IoBufferImpl&&) = delete;
    IoBufferImpl& operator=(IoBufferImpl&&) = delete;

    [[nodiscard]] Result Initialize(CoSimType coSimType,
                                    [[maybe_unused]] ConnectionKind connectionKind,
                                    const std::string& name,
                                    const std::vector<IoSignal>& incomingSignals,
                                    const std::vector<IoSignal>& outgoingSignals) {
        std::string outgoingName(name);
        outgoingName.append(".Outgoing");
        std::string incomingName(name);
        incomingName.append(".Incoming");
        const std::vector<IoSignal>* writeSignals = &outgoingSignals;
        const std::vector<IoSignal>* readSignals = &incomingSignals;
        if (coSimType == CoSimType::Server) {
            std::swap(incomingName, outgoingName);
            writeSignals = &incomingSignals;
            readSignals = &outgoingSignals;
        }

#ifdef _WIN32
        if (connectionKind == ConnectionKind::Local) {
            _readBuffer = std::make_unique<LocalIoPartBuffer>();
            CheckResult(_readBuffer->Initialize(coSimType, incomingName, *readSignals));
            _writeBuffer = std::make_unique<LocalIoPartBuffer>();
            CheckResult(_writeBuffer->Initialize(coSimType, outgoingName, *writeSignals));
        } else {
#endif
            _readBuffer = std::make_unique<RemoteIoPartBuffer>();
            CheckResult(_readBuffer->Initialize(coSimType, incomingName, *readSignals));
            _writeBuffer = std::make_unique<RemoteIoPartBuffer>();
            CheckResult(_writeBuffer->Initialize(coSimType, outgoingName, *writeSignals));
#ifdef _WIN32
        }
#endif

        ClearData();
        return Result::Ok;
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

    [[nodiscard]] Result Deserialize(ChannelReader& reader,
                                     SimulationTime simulationTime,
                                     const Callbacks& callbacks) const override {
        return _readBuffer->Deserialize(reader, simulationTime, callbacks);
    }

private:
    std::unique_ptr<IoPartBufferBase> _writeBuffer;
    std::unique_ptr<IoPartBufferBase> _readBuffer;
};

}  // namespace

[[nodiscard]] Result CreateIoBuffer(CoSimType coSimType,
                                    ConnectionKind connectionKind,
                                    const std::string& name,
                                    const std::vector<IoSignal>& incomingSignals,
                                    const std::vector<IoSignal>& outgoingSignals,
                                    std::unique_ptr<IoBuffer>& ioBuffer) {
    std::unique_ptr<IoBufferImpl> tmpIoBuffer = std::make_unique<IoBufferImpl>();
    CheckResult(tmpIoBuffer->Initialize(coSimType, connectionKind, name, incomingSignals, outgoingSignals));
    ioBuffer = std::move(tmpIoBuffer);
    return Result::Ok;
}

}  // namespace DsVeosCoSim
