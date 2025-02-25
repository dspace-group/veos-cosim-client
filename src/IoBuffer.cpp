// Copyright dSPACE GmbH. All rights reserved.

#include "IoBuffer.h"

#include <cstring>
#include <string>

#include "CoSimHelper.h"
#include "CoSimTypes.h"
#include "Environment.h"

namespace DsVeosCoSim {

namespace {

void CheckSizeKind(const SizeKind sizeKind, const std::string& name) {
    switch (sizeKind) {
        case SizeKind::Fixed:
        case SizeKind::Variable:
            return;
        default:  // NOLINT
            throw CoSimException("Unknown size kind '" + ToString(sizeKind) + "' for IO signal '" + name + "'.");
    }
}

}  // namespace

IoPartBufferBase::IoPartBufferBase(const CoSimType coSimType, const std::vector<IoSignal>& signals)
    : _coSimType(coSimType), _changedSignalsQueue(signals.size()) {
    size_t nextSignalIndex = 0;
    for (const auto& signal : signals) {
        if (signal.length == 0) {
            throw CoSimException("Invalid length 0 for IO signal '" + std::string(signal.name) + "'.");
        }

        CheckSizeKind(signal.sizeKind, signal.name);

        const size_t dataTypeSize = GetDataTypeSize(signal.dataType);
        if (dataTypeSize == 0) {
            throw CoSimException("Invalid data type for IO signal '" + std::string(signal.name) + "'.");
        }

        if (_metaDataLookup.find(signal.id) != _metaDataLookup.end()) {  // NOLINT
            throw CoSimException("Duplicated IO signal id " + ToString(signal.id) + ".");
        }

        const size_t totalDataSize = dataTypeSize * signal.length;

        MetaData metaData{};
        metaData.info = signal;
        metaData.dataTypeSize = dataTypeSize;
        metaData.totalDataSize = totalDataSize;
        metaData.signalIndex = nextSignalIndex++;

        _metaDataLookup[signal.id] = metaData;
    }
}

void IoPartBufferBase::ClearData() {
    if (_coSimType == CoSimType::Client) {
        std::lock_guard lock(_mutex);
        ClearDataInternal();
        return;
    }

    ClearDataInternal();
}

void IoPartBufferBase::Write(const IoSignalId signalId, const uint32_t length, const void* value) {
    if (_coSimType == CoSimType::Client) {
        std::lock_guard lock(_mutex);
        WriteInternal(signalId, length, value);
        return;
    }

    WriteInternal(signalId, length, value);
}

void IoPartBufferBase::Read(const IoSignalId signalId, uint32_t& length, void* value) {
    if (_coSimType == CoSimType::Client) {
        std::lock_guard lock(_mutex);
        ReadInternal(signalId, length, value);
        return;
    }

    ReadInternal(signalId, length, value);
}

void IoPartBufferBase::Read(const IoSignalId signalId, uint32_t& length, const void** value) {
    if (_coSimType == CoSimType::Client) {
        std::lock_guard lock(_mutex);
        ReadInternal(signalId, length, value);
        return;
    }

    ReadInternal(signalId, length, value);
}

[[nodiscard]] bool IoPartBufferBase::Serialize(ChannelWriter& writer) {
    if (_coSimType == CoSimType::Client) {
        std::lock_guard lock(_mutex);
        return SerializeInternal(writer);
    }

    return SerializeInternal(writer);
}

[[nodiscard]] bool IoPartBufferBase::Deserialize(ChannelReader& reader,
                                                 const SimulationTime simulationTime,
                                                 const Callbacks& callbacks) {
    if (_coSimType == CoSimType::Client) {
        std::lock_guard lock(_mutex);
        return DeserializeInternal(reader, simulationTime, callbacks);
    }

    return DeserializeInternal(reader, simulationTime, callbacks);
}

IoPartBufferBase::MetaData& IoPartBufferBase::FindMetaData(const IoSignalId signalId) {
    if (const auto search = _metaDataLookup.find(signalId); search != _metaDataLookup.end()) {
        return search->second;
    }

    throw CoSimException("IO signal id " + ToString(signalId) + " is unknown.");
}

RemoteIoPartBuffer::RemoteIoPartBuffer(const CoSimType coSimType,
                                       [[maybe_unused]] const std::string& name,
                                       const std::vector<IoSignal>& signals)
    : IoPartBufferBase(coSimType, signals) {
    _dataVector.resize(_metaDataLookup.size());
    for (auto& [signalId, metaData] : _metaDataLookup) {  // NOLINT
        Data data{};
        data.buffer.resize(metaData.totalDataSize);
        if (metaData.info.sizeKind == SizeKind::Fixed) {
            data.currentLength = metaData.info.length;
        }

        _dataVector[metaData.signalIndex] = data;
    }
}

void RemoteIoPartBuffer::ClearDataInternal() {
    _changedSignalsQueue.Clear();

    for (auto& [signalId, metaData] : _metaDataLookup) {  // NOLINT
        auto& [currentLength, isChanged, buffer] = _dataVector[metaData.signalIndex];
        isChanged = false;
        if (metaData.info.sizeKind == SizeKind::Variable) {
            currentLength = 0;
        }

        std::fill(buffer.begin(), buffer.end(), static_cast<uint8_t>(0));  // NOLINT
    }
}

void RemoteIoPartBuffer::WriteInternal(const IoSignalId signalId, const uint32_t length, const void* value) {
    MetaData& metaData = FindMetaData(signalId);
    auto& [currentLength, isChanged, buffer] = _dataVector[metaData.signalIndex];

    if (metaData.info.sizeKind == SizeKind::Variable) {
        if (length > metaData.info.length) {
            throw CoSimException("Length of variable sized IO signal '" + std::string(metaData.info.name) +
                                 "' exceeds max size.");
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
            throw CoSimException("Length of fixed sized IO signal '" + std::string(metaData.info.name) + "' must be " +
                                 std::to_string(metaData.info.length) + " but was " + std::to_string(length) + ".");
        }
    }

    const size_t totalSize = metaData.dataTypeSize * length;

    if (memcmp(buffer.data(), value, totalSize) == 0) {
        return;
    }

    (void)memcpy(buffer.data(), value, totalSize);

    if (!isChanged) {
        isChanged = true;
        _changedSignalsQueue.PushBack(&metaData);
    }
}

void RemoteIoPartBuffer::ReadInternal(const IoSignalId signalId, uint32_t& length, void* value) {
    const MetaData& metaData = FindMetaData(signalId);
    const Data& data = _dataVector[metaData.signalIndex];

    length = data.currentLength;
    const size_t totalSize = metaData.dataTypeSize * length;
    (void)memcpy(value, data.buffer.data(), totalSize);
}

void RemoteIoPartBuffer::ReadInternal(const IoSignalId signalId, uint32_t& length, const void** value) {
    const MetaData& metaData = FindMetaData(signalId);
    const Data& data = _dataVector[metaData.signalIndex];

    length = data.currentLength;
    *value = data.buffer.data();
}

[[nodiscard]] bool RemoteIoPartBuffer::SerializeInternal(ChannelWriter& writer) {
    const auto size = static_cast<uint32_t>(_changedSignalsQueue.Size());
    CheckResultWithMessage(writer.Write(size), "Could not write count of changed signals.");
    if (_changedSignalsQueue.IsEmpty()) {
        return true;
    }

    while (!_changedSignalsQueue.IsEmpty()) {
        const MetaData* metaData = _changedSignalsQueue.PopFront();
        auto& [currentLength, isChanged, buffer] = _dataVector[metaData->signalIndex];

        CheckResultWithMessage(writer.Write(metaData->info.id), "Could not write signal id.");

        if (metaData->info.sizeKind == SizeKind::Variable) {
            CheckResultWithMessage(writer.Write(currentLength), "Could not write current signal length.");
        }

        const size_t totalSize = metaData->dataTypeSize * currentLength;
        CheckResultWithMessage(writer.Write(buffer.data(), static_cast<uint32_t>(totalSize)),
                               "Could not write signal data.");
        isChanged = false;

        if (IsProtocolTracingEnabled()) {
            LogProtocolDataTrace("Signal { Id: " + std::to_string(static_cast<uint32_t>(metaData->info.id)) +
                                 ", Length: " + std::to_string(currentLength) + ", Data: " +
                                 ValueToString(metaData->info.dataType, currentLength, buffer.data()) + " }");
        }
    }

    return true;
}

[[nodiscard]] bool RemoteIoPartBuffer::DeserializeInternal(ChannelReader& reader,
                                                           const SimulationTime simulationTime,
                                                           const Callbacks& callbacks) {
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
                throw CoSimException("Length of variable sized IO signal '" + std::string(metaData.info.name) +
                                     "' exceeds max size.");
            }

            data.currentLength = length;
        }

        const size_t totalSize = metaData.dataTypeSize * data.currentLength;
        CheckResultWithMessage(reader.Read(data.buffer.data(), totalSize), "Could not read signal data.");

        if (IsProtocolTracingEnabled()) {
            LogProtocolDataTrace("Signal { Id: " + std::to_string(static_cast<uint32_t>(metaData.info.id)) +
                                 ", Length: " + std::to_string(data.currentLength) + ", Data: " +
                                 ValueToString(metaData.info.dataType, data.currentLength, data.buffer.data()) + " }");
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

#ifdef _WIN32

LocalIoPartBuffer::LocalIoPartBuffer(const CoSimType coSimType,
                                     const std::string& name,
                                     const std::vector<IoSignal>& signals)
    : IoPartBufferBase(coSimType, signals) {
    // The memory layout looks like this:
    // for each signal:
    //   [ current length ]
    //   [ data ]
    //   [ current length ]
    //   [ data ]

    _dataVector.resize(_metaDataLookup.size());

    size_t totalSize{};
    for (auto& [signalId, metaData] : _metaDataLookup) {  // NOLINT
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

    for (auto& [signalId, metaData] : _metaDataLookup) {  // NOLINT
        const Data& data = _dataVector[metaData.signalIndex];
        DataBuffer* dataBuffer = GetDataBuffer(data.offsetOfDataBufferInShm);
        DataBuffer* backupDataBuffer = GetDataBuffer(data.offsetOfBackupDataBufferInShm);
        if (metaData.info.sizeKind == SizeKind::Fixed) {
            dataBuffer->currentLength = metaData.info.length;
            backupDataBuffer->currentLength = metaData.info.length;
        }
    }
}

void LocalIoPartBuffer::ClearDataInternal() {
    _changedSignalsQueue.Clear();

    for (auto& [signalId, metaData] : _metaDataLookup) {  // NOLINT
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

void LocalIoPartBuffer::WriteInternal(const IoSignalId signalId, const uint32_t length, const void* value) {
    MetaData& metaData = FindMetaData(signalId);
    Data& data = _dataVector[metaData.signalIndex];

    DataBuffer* dataBuffer = GetDataBuffer(data.offsetOfDataBufferInShm);

    bool currentLengthChanged{};
    if (metaData.info.sizeKind == SizeKind::Variable) {
        if (length > metaData.info.length) {
            throw CoSimException("Length of variable sized IO signal '" + std::string(metaData.info.name) +
                                 "' exceeds max size.");
        }

        currentLengthChanged = dataBuffer->currentLength != length;
    } else {
        if (length != metaData.info.length) {
            throw CoSimException("Length of fixed sized IO signal '" + std::string(metaData.info.name) + "' must be " +
                                 std::to_string(metaData.info.length) + " but was " + std::to_string(length) + ".");
        }
    }

    const size_t totalSize = metaData.dataTypeSize * length;

    const bool dataChanged = memcmp(dataBuffer->data, value, totalSize) != 0;

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

void LocalIoPartBuffer::ReadInternal(const IoSignalId signalId, uint32_t& length, void* value) {
    const MetaData& metaData = FindMetaData(signalId);
    const Data& data = _dataVector[metaData.signalIndex];

    const DataBuffer* dataBuffer = GetDataBuffer(data.offsetOfDataBufferInShm);

    length = dataBuffer->currentLength;
    const size_t totalSize = metaData.dataTypeSize * length;
    (void)memcpy(value, dataBuffer->data, totalSize);
}

void LocalIoPartBuffer::ReadInternal(const IoSignalId signalId, uint32_t& length, const void** value) {
    const MetaData& metaData = FindMetaData(signalId);
    const Data& data = _dataVector[metaData.signalIndex];

    const DataBuffer* dataBuffer = GetDataBuffer(data.offsetOfDataBufferInShm);

    length = dataBuffer->currentLength;
    *value = dataBuffer->data;
}

[[nodiscard]] bool LocalIoPartBuffer::SerializeInternal(ChannelWriter& writer) {
    const auto size = static_cast<uint32_t>(_changedSignalsQueue.Size());
    CheckResultWithMessage(writer.Write(size), "Could not write count of changed signals.");
    if (_changedSignalsQueue.IsEmpty()) {
        return true;
    }

    while (!_changedSignalsQueue.IsEmpty()) {
        const MetaData* metaData = _changedSignalsQueue.PopFront();
        Data& data = _dataVector[metaData->signalIndex];

        if (IsProtocolTracingEnabled()) {
            const DataBuffer* dataBuffer = GetDataBuffer(data.offsetOfDataBufferInShm);

            LogProtocolDataTrace("Signal { Id: " + std::to_string(static_cast<uint32_t>(metaData->info.id)) +
                                 ", Length: " + std::to_string(dataBuffer->currentLength) + ", Data: " +
                                 ValueToString(metaData->info.dataType, dataBuffer->currentLength, dataBuffer->data) +
                                 " }");
        }

        CheckResultWithMessage(writer.Write(metaData->info.id), "Could not write signal id.");

        data.isChanged = false;
    }

    return true;
}

[[nodiscard]] bool LocalIoPartBuffer::DeserializeInternal(ChannelReader& reader,
                                                          const SimulationTime simulationTime,
                                                          const Callbacks& callbacks) {
    uint32_t ioSignalChangedCount = 0;
    CheckResultWithMessage(reader.Read(ioSignalChangedCount), "Could not read count of changed signals.");

    for (uint32_t i = 0; i < ioSignalChangedCount; i++) {
        IoSignalId signalId{};
        CheckResultWithMessage(reader.Read(signalId), "Could not read signal id.");

        MetaData& metaData = FindMetaData(signalId);
        Data& data = _dataVector[metaData.signalIndex];

        FlipBuffers(data);

        const DataBuffer* dataBuffer = GetDataBuffer(data.offsetOfDataBufferInShm);

        if (IsProtocolTracingEnabled()) {
            LogProtocolDataTrace(
                "Signal { Id: " + std::to_string(static_cast<uint32_t>(metaData.info.id)) +
                ", Length: " + std::to_string(dataBuffer->currentLength) +
                ", Data: " + ValueToString(metaData.info.dataType, dataBuffer->currentLength, dataBuffer->data) + " }");
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

[[nodiscard]] LocalIoPartBuffer::DataBuffer* LocalIoPartBuffer::GetDataBuffer(const size_t offset) const {
    return reinterpret_cast<DataBuffer*>(static_cast<uint8_t*>(_sharedMemory.data()) + offset);
}

void LocalIoPartBuffer::FlipBuffers(Data& data) {
    std::swap(data.offsetOfDataBufferInShm, data.offsetOfBackupDataBufferInShm);
}

#endif

IoBuffer::IoBuffer(CoSimType coSimType,
                   [[maybe_unused]] const ConnectionKind connectionKind,
                   const std::string& name,
                   const std::vector<IoSignal>& incomingSignals,
                   const std::vector<IoSignal>& outgoingSignals) {
    std::string outgoingName = name + ".Outgoing";
    std::string incomingName = name + ".Incoming";
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

void IoBuffer::ClearData() const {
    _readBuffer->ClearData();
    _writeBuffer->ClearData();
}

void IoBuffer::Write(const IoSignalId signalId, const uint32_t length, const void* value) const {
    _writeBuffer->Write(signalId, length, value);
}

void IoBuffer::Read(const IoSignalId signalId, uint32_t& length, void* value) const {
    _readBuffer->Read(signalId, length, value);
}

void IoBuffer::Read(const IoSignalId signalId, uint32_t& length, const void** value) const {
    _readBuffer->Read(signalId, length, value);
}

[[nodiscard]] bool IoBuffer::Serialize(ChannelWriter& writer) const {
    return _writeBuffer->Serialize(writer);
}

[[nodiscard]] bool IoBuffer::Deserialize(ChannelReader& reader,
                                         const SimulationTime simulationTime,
                                         const Callbacks& callbacks) const {
    return _readBuffer->Deserialize(reader, simulationTime, callbacks);
}

}  // namespace DsVeosCoSim
