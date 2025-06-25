// Copyright dSPACE GmbH. All rights reserved.

#include "IoBuffer.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "Channel.h"
#include "CoSimHelper.h"
#include "DsVeosCoSim/CoSimTypes.h"
#include "Environment.h"
#include "RingBuffer.h"

#ifdef _WIN32
#include "OsUtilities.h"
#endif

namespace DsVeosCoSim {

namespace {

void CheckSizeKind(SizeKind sizeKind, std::string_view name) {
    switch (sizeKind) {
        case SizeKind::Fixed:
        case SizeKind::Variable:
            return;
    }

    std::string message = "Unknown size kind '";
    message.append(ToString(sizeKind));
    message.append("' for IO signal '");
    message.append(name);
    message.append("'.");
    throw std::runtime_error(message);
}

class IoPartBufferBase {
protected:
    struct MetaData {
        IoSignal info{};
        size_t dataTypeSize{};
        size_t totalDataSize{};
        size_t signalIndex{};
    };

public:
    IoPartBufferBase(CoSimType coSimType, const std::vector<IoSignal>& signals)
        : _coSimType(coSimType), _changedSignalsQueue(signals.size()) {
        size_t nextSignalIndex = 0;
        for (const auto& signal : signals) {
            if (signal.length == 0) {
                std::string message = "Invalid length 0 for IO signal '";
                message.append(signal.name);
                message.append("'.");
                throw std::runtime_error(message);
            }

            CheckSizeKind(signal.sizeKind, signal.name);

            size_t dataTypeSize = GetDataTypeSize(signal.dataType);
            if (dataTypeSize == 0) {
                std::string message = "Invalid data type for IO signal '";
                message.append(signal.name);
                message.append("'.");
                throw std::runtime_error(message);
            }

            auto search = _metaDataLookup.find(signal.id);
            if (search != _metaDataLookup.end()) {
                std::string message = "Duplicated IO signal id ";
                message.append(ToString(signal.id));
                message.append(".");
                throw std::runtime_error(message);
            }

            size_t totalDataSize = dataTypeSize * signal.length;

            MetaData metaData{};
            metaData.info = signal;
            metaData.dataTypeSize = dataTypeSize;
            metaData.totalDataSize = totalDataSize;
            metaData.signalIndex = nextSignalIndex++;

            _metaDataLookup[signal.id] = metaData;
        }
    }

    virtual ~IoPartBufferBase() noexcept = default;

    IoPartBufferBase(const IoPartBufferBase&) = delete;
    IoPartBufferBase& operator=(const IoPartBufferBase&) = delete;

    IoPartBufferBase(IoPartBufferBase&&) = delete;
    IoPartBufferBase& operator=(IoPartBufferBase&&) = delete;

    void ClearData() {
        if (_coSimType == CoSimType::Client) {
            std::lock_guard lock(_mutex);
            ClearDataInternal();
            return;
        }

        ClearDataInternal();
    }

    void Write(IoSignalId signalId, uint32_t length, const void* value) {
        if (_coSimType == CoSimType::Client) {
            std::lock_guard lock(_mutex);
            WriteInternal(signalId, length, value);
            return;
        }

        WriteInternal(signalId, length, value);
    }

    void Read(IoSignalId signalId, uint32_t& length, void* value) {
        if (_coSimType == CoSimType::Client) {
            std::lock_guard lock(_mutex);
            ReadInternal(signalId, length, value);
            return;
        }

        ReadInternal(signalId, length, value);
    }

    void Read(IoSignalId signalId, uint32_t& length, const void** value) {
        if (_coSimType == CoSimType::Client) {
            std::lock_guard lock(_mutex);
            ReadInternal(signalId, length, value);
            return;
        }

        ReadInternal(signalId, length, value);
    }

    [[nodiscard]] bool Serialize(ChannelWriter& writer) {
        if (_coSimType == CoSimType::Client) {
            std::lock_guard lock(_mutex);
            return SerializeInternal(writer);
        }

        return SerializeInternal(writer);
    }

    [[nodiscard]] bool Deserialize(ChannelReader& reader, SimulationTime simulationTime, const Callbacks& callbacks) {
        if (_coSimType == CoSimType::Client) {
            std::lock_guard lock(_mutex);
            return DeserializeInternal(reader, simulationTime, callbacks);
        }

        return DeserializeInternal(reader, simulationTime, callbacks);
    }

protected:
    virtual void ClearDataInternal() = 0;

    virtual void WriteInternal(IoSignalId signalId, uint32_t length, const void* value) = 0;
    virtual void ReadInternal(IoSignalId signalId, uint32_t& length, void* value) = 0;
    virtual void ReadInternal(IoSignalId signalId, uint32_t& length, const void** value) = 0;

    [[nodiscard]] virtual bool SerializeInternal(ChannelWriter& writer) = 0;
    [[nodiscard]] virtual bool DeserializeInternal(ChannelReader& reader,
                                                   SimulationTime simulationTime,
                                                   const Callbacks& callbacks) = 0;

    [[nodiscard]] MetaData& FindMetaData(IoSignalId signalId) {
        auto search = _metaDataLookup.find(signalId);
        if (search != _metaDataLookup.end()) {
            return search->second;
        }

        std::string message = "IO signal id '";
        message.append(ToString(signalId));
        message.append("' is unknown.");
        throw std::runtime_error(message);
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
    RemoteIoPartBuffer(CoSimType coSimType,
                       [[maybe_unused]] std::string_view name,
                       const std::vector<IoSignal>& signals)
        : IoPartBufferBase(coSimType, signals) {
        _dataVector.resize(_metaDataLookup.size());
        for (auto& [signalId, metaData] : _metaDataLookup) {
            Data data{};
            data.buffer.resize(metaData.totalDataSize);
            if (metaData.info.sizeKind == SizeKind::Fixed) {
                data.currentLength = metaData.info.length;
            }

            _dataVector[metaData.signalIndex] = data;
        }
    }
    ~RemoteIoPartBuffer() noexcept override = default;

    RemoteIoPartBuffer(const RemoteIoPartBuffer&) = delete;
    RemoteIoPartBuffer& operator=(const RemoteIoPartBuffer&) = delete;

    RemoteIoPartBuffer(RemoteIoPartBuffer&&) = delete;
    RemoteIoPartBuffer& operator=(RemoteIoPartBuffer&&) = delete;

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

    void WriteInternal(IoSignalId signalId, uint32_t length, const void* value) override {
        MetaData& metaData = FindMetaData(signalId);
        auto& [currentLength, isChanged, buffer] = _dataVector[metaData.signalIndex];

        if (metaData.info.sizeKind == SizeKind::Variable) {
            if (length > metaData.info.length) {
                std::string message = "Length of variable sized IO signal '";
                message.append(metaData.info.name);
                message.append("' exceeds max size.");
                throw std::runtime_error(message);
            }

            if (currentLength != length) {
                if (!isChanged) {
                    isChanged = true;
                    _changedSignalsQueue.PushBack(&metaData);
                }
            }

            currentLength = length;
        } else {
            if (length != metaData.info.length) {
                std::string message = "Length of fixed sized IO signal '";
                message.append(metaData.info.name);
                message.append("' must be ");
                message.append(std::to_string(metaData.info.length));
                message.append(" but was ");
                message.append(std::to_string(length));
                message.append(".");
                throw std::runtime_error(message);
            }
        }

        size_t totalSize = metaData.dataTypeSize * length;

        int32_t compareResult = memcmp(buffer.data(), value, totalSize);
        if (compareResult == 0) {
            return;
        }

        (void)memcpy(buffer.data(), value, totalSize);

        if (!isChanged) {
            isChanged = true;
            _changedSignalsQueue.PushBack(&metaData);
        }
    }

    void ReadInternal(IoSignalId signalId, uint32_t& length, void* value) override {
        MetaData& metaData = FindMetaData(signalId);
        Data& data = _dataVector[metaData.signalIndex];

        length = data.currentLength;
        size_t totalSize = metaData.dataTypeSize * length;
        (void)memcpy(value, data.buffer.data(), totalSize);
    }

    void ReadInternal(IoSignalId signalId, uint32_t& length, const void** value) override {
        MetaData& metaData = FindMetaData(signalId);
        Data& data = _dataVector[metaData.signalIndex];

        length = data.currentLength;
        *value = data.buffer.data();
    }

    [[nodiscard]] bool SerializeInternal(ChannelWriter& writer) override {
        auto size = static_cast<uint32_t>(_changedSignalsQueue.Size());
        CheckResultWithMessage(writer.Write(size), "Could not write count of changed signals.");
        if (_changedSignalsQueue.IsEmpty()) {
            return true;
        }

        while (!_changedSignalsQueue.IsEmpty()) {
            MetaData* metaData = _changedSignalsQueue.PopFront();
            auto& [currentLength, isChanged, buffer] = _dataVector[metaData->signalIndex];

            CheckResultWithMessage(writer.Write(metaData->info.id), "Could not write signal id.");

            if (metaData->info.sizeKind == SizeKind::Variable) {
                CheckResultWithMessage(writer.Write(currentLength), "Could not write current signal length.");
            }

            size_t totalSize = metaData->dataTypeSize * currentLength;
            CheckResultWithMessage(writer.Write(buffer.data(), static_cast<uint32_t>(totalSize)),
                                   "Could not write signal data.");
            isChanged = false;

            if (IsProtocolTracingEnabled()) {
                std::string message = "Signal { Id: ";
                message.append(ToString(metaData->info.id));
                message.append(", Length: ");
                message.append(std::to_string(currentLength));
                message.append(", Data: ");
                message.append(ValueToString(metaData->info.dataType, currentLength, buffer.data()));
                message.append(" }");
                LogProtocolDataTrace(message);
            }
        }

        return true;
    }

    [[nodiscard]] bool DeserializeInternal(ChannelReader& reader,
                                           SimulationTime simulationTime,
                                           const Callbacks& callbacks) override {
        uint32_t ioSignalChangedCount = 0;
        CheckResultWithMessage(reader.Read(ioSignalChangedCount), "Could not read count of changed signals.");

        for (uint32_t i = 0; i < ioSignalChangedCount; i++) {
            IoSignalId signalId{};
            CheckResultWithMessage(reader.Read(signalId), "Could not read signal id.");

            MetaData& metaData = FindMetaData(signalId);
            Data& data = _dataVector[metaData.signalIndex];

            if (metaData.info.sizeKind == SizeKind::Variable) {
                uint32_t length = 0;
                CheckResultWithMessage(reader.Read(length), "Could not read current signal length.");
                if (length > metaData.info.length) {
                    std::string message = "Length of variable sized IO signal '";
                    message.append(metaData.info.name);
                    message.append("' exceeds max size.");
                    throw std::runtime_error(message);
                }

                data.currentLength = length;
            }

            size_t totalSize = metaData.dataTypeSize * data.currentLength;
            CheckResultWithMessage(reader.Read(data.buffer.data(), totalSize), "Could not read signal data.");

            if (IsProtocolTracingEnabled()) {
                std::string message = "Signal { Id: ";
                message.append(ToString(metaData.info.id));
                message.append(", Length: ");
                message.append(std::to_string(data.currentLength));
                message.append(", Data: ");
                message.append(ValueToString(metaData.info.dataType, data.currentLength, data.buffer.data()));
                message.append(" }");
                LogProtocolDataTrace(message);
            }

            if (callbacks.incomingSignalChangedCallback) {
                callbacks.incomingSignalChangedCallback(simulationTime,
                                                        metaData.info,
                                                        data.currentLength,
                                                        data.buffer.data());
            }
        }

        return true;
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
    LocalIoPartBuffer(CoSimType coSimType, std::string_view name, const std::vector<IoSignal>& signals)
        : IoPartBufferBase(coSimType, signals) {
        // The memory layout looks like this:
        // for each signal:
        //   [ current length ]
        //   [ data ]
        //   [ current length ]
        //   [ data ]

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
            _sharedMemory = SharedMemory::CreateOrOpen(name, totalSize);
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
    }

    ~LocalIoPartBuffer() noexcept override = default;

    LocalIoPartBuffer(const LocalIoPartBuffer&) = delete;
    LocalIoPartBuffer& operator=(const LocalIoPartBuffer&) = delete;

    LocalIoPartBuffer(LocalIoPartBuffer&&) = delete;
    LocalIoPartBuffer& operator=(LocalIoPartBuffer&&) = delete;

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

    void WriteInternal(IoSignalId signalId, uint32_t length, const void* value) override {
        MetaData& metaData = FindMetaData(signalId);
        Data& data = _dataVector[metaData.signalIndex];

        DataBuffer* dataBuffer = GetDataBuffer(data.offsetOfDataBufferInShm);

        bool currentLengthChanged{};
        if (metaData.info.sizeKind == SizeKind::Variable) {
            if (length > metaData.info.length) {
                std::string message = "Length of variable sized IO signal '";
                message.append(metaData.info.name);
                message.append("' exceeds max size.");
                throw std::runtime_error(message);
            }

            currentLengthChanged = dataBuffer->currentLength != length;
        } else {
            if (length != metaData.info.length) {
                std::string message = "Length of fixed sized IO signal '";
                message.append(metaData.info.name);
                message.append("' must be ");
                message.append(std::to_string(metaData.info.length));
                message.append(" but was ");
                message.append(std::to_string(length));
                message.append(".");
                throw std::runtime_error(message);
            }
        }

        size_t totalSize = metaData.dataTypeSize * length;

        bool dataChanged = memcmp(dataBuffer->data, value, totalSize) != 0;

        if (!currentLengthChanged && !dataChanged) {
            return;
        }

        if (!data.isChanged) {
            data.isChanged = true;
            _changedSignalsQueue.PushBack(&metaData);
            FlipBuffers(data);
            dataBuffer = GetDataBuffer(data.offsetOfDataBufferInShm);
        }

        dataBuffer->currentLength = length;
        if (dataChanged) {
            (void)memcpy(dataBuffer->data, value, totalSize);
        }
    }

    void ReadInternal(IoSignalId signalId, uint32_t& length, void* value) override {
        MetaData& metaData = FindMetaData(signalId);
        Data& data = _dataVector[metaData.signalIndex];

        DataBuffer* dataBuffer = GetDataBuffer(data.offsetOfDataBufferInShm);

        length = dataBuffer->currentLength;
        size_t totalSize = metaData.dataTypeSize * length;
        (void)memcpy(value, dataBuffer->data, totalSize);
    }

    void ReadInternal(IoSignalId signalId, uint32_t& length, const void** value) override {
        MetaData& metaData = FindMetaData(signalId);
        Data& data = _dataVector[metaData.signalIndex];

        DataBuffer* dataBuffer = GetDataBuffer(data.offsetOfDataBufferInShm);

        length = dataBuffer->currentLength;
        *value = dataBuffer->data;
    }

    [[nodiscard]] bool SerializeInternal(ChannelWriter& writer) override {
        auto size = static_cast<uint32_t>(_changedSignalsQueue.Size());
        CheckResultWithMessage(writer.Write(size), "Could not write count of changed signals.");
        if (_changedSignalsQueue.IsEmpty()) {
            return true;
        }

        while (!_changedSignalsQueue.IsEmpty()) {
            MetaData* metaData = _changedSignalsQueue.PopFront();
            Data& data = _dataVector[metaData->signalIndex];

            if (IsProtocolTracingEnabled()) {
                DataBuffer* dataBuffer = GetDataBuffer(data.offsetOfDataBufferInShm);

                std::string message = "Signal { Id: ";
                message.append(ToString(metaData->info.id));
                message.append(", Length: ");
                message.append(std::to_string(dataBuffer->currentLength));
                message.append(", Data: ");
                message.append(ValueToString(metaData->info.dataType, dataBuffer->currentLength, dataBuffer->data));
                message.append(" }");
                LogProtocolDataTrace(message);
            }

            CheckResultWithMessage(writer.Write(metaData->info.id), "Could not write signal id.");

            data.isChanged = false;
        }

        return true;
    }

    [[nodiscard]] bool DeserializeInternal(ChannelReader& reader,
                                           SimulationTime simulationTime,
                                           const Callbacks& callbacks) override {
        uint32_t ioSignalChangedCount = 0;
        CheckResultWithMessage(reader.Read(ioSignalChangedCount), "Could not read count of changed signals.");

        for (uint32_t i = 0; i < ioSignalChangedCount; i++) {
            IoSignalId signalId{};
            CheckResultWithMessage(reader.Read(signalId), "Could not read signal id.");

            MetaData& metaData = FindMetaData(signalId);
            Data& data = _dataVector[metaData.signalIndex];

            FlipBuffers(data);

            DataBuffer* dataBuffer = GetDataBuffer(data.offsetOfDataBufferInShm);

            if (IsProtocolTracingEnabled()) {
                std::string message = "Signal { Id: ";
                message.append(ToString(metaData.info.id));
                message.append(", Length: ");
                message.append(std::to_string(dataBuffer->currentLength));
                message.append(", Data: ");
                message.append(ValueToString(metaData.info.dataType, dataBuffer->currentLength, dataBuffer->data));
                message.append(" }");
                LogProtocolDataTrace(message);
            }

            if (callbacks.incomingSignalChangedCallback) {
                callbacks.incomingSignalChangedCallback(simulationTime,
                                                        metaData.info,
                                                        dataBuffer->currentLength,
                                                        dataBuffer->data);
            }
        }

        return true;
    }

private:
    [[nodiscard]] DataBuffer* GetDataBuffer(size_t offset) const {
        return reinterpret_cast<DataBuffer*>(static_cast<uint8_t*>(_sharedMemory.data()) + offset);
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
    IoBufferImpl(CoSimType coSimType,
                 [[maybe_unused]] ConnectionKind connectionKind,
                 std::string_view name,
                 const std::vector<IoSignal>& incomingSignals,
                 const std::vector<IoSignal>& outgoingSignals) {
        std::string nameString = std::string(name);
        std::string outgoingName = nameString;
        outgoingName.append(".Outgoing");
        std::string incomingName = nameString;
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
            _readBuffer = std::make_unique<LocalIoPartBuffer>(coSimType, incomingName, *readSignals);
            _writeBuffer = std::make_unique<LocalIoPartBuffer>(coSimType, outgoingName, *writeSignals);
        } else {
#endif
            _readBuffer = std::make_unique<RemoteIoPartBuffer>(coSimType, incomingName, *readSignals);
            _writeBuffer = std::make_unique<RemoteIoPartBuffer>(coSimType, outgoingName, *writeSignals);
#ifdef _WIN32
        }
#endif

        ClearData();
    }

    ~IoBufferImpl() noexcept override = default;

    IoBufferImpl(const IoBufferImpl&) = delete;
    IoBufferImpl& operator=(const IoBufferImpl&) = delete;

    IoBufferImpl(IoBufferImpl&&) = delete;
    IoBufferImpl& operator=(IoBufferImpl&&) = delete;

    void ClearData() const override {
        _readBuffer->ClearData();
        _writeBuffer->ClearData();
    }

    void Write(IoSignalId signalId, uint32_t length, const void* value) const override {
        _writeBuffer->Write(signalId, length, value);
    }

    void Read(IoSignalId signalId, uint32_t& length, void* value) const override {
        _readBuffer->Read(signalId, length, value);
    }

    void Read(IoSignalId signalId, uint32_t& length, const void** value) const override {
        _readBuffer->Read(signalId, length, value);
    }

    [[nodiscard]] bool Serialize(ChannelWriter& writer) const override {
        return _writeBuffer->Serialize(writer);
    }

    [[nodiscard]] bool Deserialize(ChannelReader& reader,
                                   SimulationTime simulationTime,
                                   const Callbacks& callbacks) const override {
        return _readBuffer->Deserialize(reader, simulationTime, callbacks);
    }

private:
    std::unique_ptr<IoPartBufferBase> _writeBuffer;
    std::unique_ptr<IoPartBufferBase> _readBuffer;
};

}  // namespace

[[nodiscard]] std::unique_ptr<IoBuffer> CreateIoBuffer(CoSimType coSimType,
                                                       ConnectionKind connectionKind,
                                                       std::string_view name,
                                                       const std::vector<IoSignal>& incomingSignals,
                                                       const std::vector<IoSignal>& outgoingSignals) {
    return std::make_unique<IoBufferImpl>(coSimType, connectionKind, name, incomingSignals, outgoingSignals);
}

}  // namespace DsVeosCoSim
