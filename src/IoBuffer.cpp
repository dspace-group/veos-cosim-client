// Copyright dSPACE GmbH. All rights reserved.

#include "IoBuffer.h"

#include <fmt/format.h>
#include <cstring>

#include "CoSimHelper.h"
#include "CoSimTypes.h"
#include "DsVeosCoSim/DsVeosCoSim.h"

namespace DsVeosCoSim {

namespace {

void CheckSizeKind(DsVeosCoSim_SizeKind sizeKind, const std::string& name) {
    switch (sizeKind) {  // NOLINT
        case DsVeosCoSim_SizeKind_Fixed:
        case DsVeosCoSim_SizeKind_Variable:
            return;
        default:
            throw CoSimException(fmt::format("Unknown size kind '{}' for IO signal '{}'.", ToString(sizeKind), name));
    }
}

}  // namespace

IoPartBufferBase::IoPartBufferBase(CoSimType coSimType, const std::vector<DsVeosCoSim_IoSignal>& signals)
    : _changedSignalsQueue(signals.size()) {
    _coSimType = coSimType;

    size_t nextSignalIndex = 0;
    for (const auto& signal : signals) {
        if (signal.length == 0) {
            throw CoSimException(fmt::format("Invalid length {} for IO signal '{}'.", signal.length, signal.name));
        }

        CheckSizeKind(signal.sizeKind, signal.name);

        const size_t dataTypeSize = GetDataTypeSize(signal.dataType);
        if (dataTypeSize == 0) {
            throw CoSimException(
                fmt::format("Invalid data type {} for IO signal '{}'.", ToString(signal.dataType), signal.name));
        }

        if (_metaDataLookup.contains(signal.id)) {
            throw CoSimException(fmt::format("Duplicated IO signal id {}.", signal.id));
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

void IoPartBufferBase::Write(DsVeosCoSim_IoSignalId signalId, uint32_t length, const void* value) {
    if (_coSimType == CoSimType::Client) {
        std::lock_guard lock(_mutex);
        WriteInternal(signalId, length, value);
        return;
    }

    WriteInternal(signalId, length, value);
}

void IoPartBufferBase::Read(DsVeosCoSim_IoSignalId signalId, uint32_t& length, void* value) {
    if (_coSimType == CoSimType::Client) {
        std::lock_guard lock(_mutex);
        ReadInternal(signalId, length, value);
        return;
    }

    ReadInternal(signalId, length, value);
}

void IoPartBufferBase::Read(DsVeosCoSim_IoSignalId signalId, uint32_t& length, const void** value) {
    if (_coSimType == CoSimType::Client) {
        std::lock_guard lock(_mutex);
        ReadInternal(signalId, length, value);
        return;
    }

    ReadInternal(signalId, length, value);
}

bool IoPartBufferBase::Serialize(ChannelWriter& writer) {
    if (_coSimType == CoSimType::Client) {
        std::lock_guard lock(_mutex);
        return SerializeInternal(writer);
    }

    return SerializeInternal(writer);
}

bool IoPartBufferBase::Deserialize(ChannelReader& reader,
                                   DsVeosCoSim_SimulationTime simulationTime,
                                   const Callbacks& callbacks) {
    if (_coSimType == CoSimType::Client) {
        std::lock_guard lock(_mutex);
        return DeserializeInternal(reader, simulationTime, callbacks);
    }

    return DeserializeInternal(reader, simulationTime, callbacks);
}

IoPartBufferBase::MetaData& IoPartBufferBase::FindMetaData(DsVeosCoSim_IoSignalId signalId) {
    const auto search = _metaDataLookup.find(signalId);
    if (search != _metaDataLookup.end()) {
        return search->second;
    }

    throw CoSimException(fmt::format("IO signal id {} is unknown.", signalId));
}

RemoteIoPartBuffer::RemoteIoPartBuffer(CoSimType coSimType,
                                       [[maybe_unused]] const std::string& name,
                                       const std::vector<DsVeosCoSim_IoSignal>& signals)
    : IoPartBufferBase(coSimType, signals) {
    _dataVector.resize(_metaDataLookup.size());
    for (auto& [signalId, metaData] : _metaDataLookup) {
        Data data{};
        data.buffer.resize(metaData.totalDataSize);
        if (metaData.info.sizeKind == DsVeosCoSim_SizeKind_Fixed) {
            data.currentLength = metaData.info.length;
        }

        _dataVector[metaData.signalIndex] = data;
    }
}

void RemoteIoPartBuffer::ClearDataInternal() {
    _changedSignalsQueue.Clear();

    for (auto& [signalId, metaData] : _metaDataLookup) {
        Data& data = _dataVector[metaData.signalIndex];
        data.isChanged = false;
        if (metaData.info.sizeKind == DsVeosCoSim_SizeKind_Variable) {
            data.currentLength = 0;
        }

        std::fill(data.buffer.begin(), data.buffer.end(), static_cast<uint8_t>(0));
    }
}

void RemoteIoPartBuffer::WriteInternal(DsVeosCoSim_IoSignalId signalId, uint32_t length, const void* value) {
    MetaData& metaData = FindMetaData(signalId);
    Data& data = _dataVector[metaData.signalIndex];

    if (metaData.info.sizeKind == DsVeosCoSim_SizeKind_Variable) {
        if (length > metaData.info.length) {
            throw CoSimException(
                fmt::format("Length of variable sized IO signal '{}' exceeds max size.", metaData.info.name));
        }

        if (data.currentLength != length) {
            if (!data.isChanged) {
                data.isChanged = true;
                _changedSignalsQueue.PushBack(&metaData);
            }
        }

        data.currentLength = length;
    } else {
        if (length != metaData.info.length) {
            throw CoSimException(fmt::format("Length of fixed sized IO signal '{}' must be {} but was {}.",
                                             metaData.info.name,
                                             metaData.info.length,
                                             length));
        }
    }

    const size_t totalSize = metaData.dataTypeSize * length;

    if (::memcmp(data.buffer.data(), value, totalSize) == 0) {
        return;
    }

    (void)::memcpy(data.buffer.data(), value, totalSize);

    if (!data.isChanged) {
        data.isChanged = true;
        _changedSignalsQueue.PushBack(&metaData);
    }
}

void RemoteIoPartBuffer::ReadInternal(DsVeosCoSim_IoSignalId signalId, uint32_t& length, void* value) {
    MetaData& metaData = FindMetaData(signalId);
    Data& data = _dataVector[metaData.signalIndex];

    length = data.currentLength;
    const size_t totalSize = metaData.dataTypeSize * length;
    (void)::memcpy(value, data.buffer.data(), totalSize);
}

void RemoteIoPartBuffer::ReadInternal(DsVeosCoSim_IoSignalId signalId, uint32_t& length, const void** value) {
    MetaData& metaData = FindMetaData(signalId);
    Data& data = _dataVector[metaData.signalIndex];

    length = data.currentLength;
    *value = data.buffer.data();
}

bool RemoteIoPartBuffer::SerializeInternal(ChannelWriter& writer) {
    auto size = static_cast<uint32_t>(_changedSignalsQueue.Size());
    CheckResultWithMessage(writer.Write(size), "Could not write count of changed signals.");
    if (_changedSignalsQueue.IsEmpty()) {
        return true;
    }

    while (!_changedSignalsQueue.IsEmpty()) {
        MetaData* metaData = _changedSignalsQueue.PopFront();
        Data& data = _dataVector[metaData->signalIndex];

        CheckResultWithMessage(writer.Write(metaData->info.id), "Could not write signal id.");

        if (metaData->info.sizeKind == DsVeosCoSim_SizeKind_Variable) {
            CheckResultWithMessage(writer.Write(data.currentLength), "Could not write current signal length.");
        }

        const size_t totalSize = metaData->dataTypeSize * data.currentLength;
        CheckResultWithMessage(writer.Write(data.buffer.data(), static_cast<uint32_t>(totalSize)),
                               "Could not write signal data.");
        data.isChanged = false;
    }

    return true;
}

bool RemoteIoPartBuffer::DeserializeInternal(ChannelReader& reader,
                                             DsVeosCoSim_SimulationTime simulationTime,
                                             const Callbacks& callbacks) {
    uint32_t ioSignalChangedCount = 0;
    CheckResultWithMessage(reader.Read(ioSignalChangedCount), "Could not read count of changed signals.");

    for (uint32_t i = 0; i < ioSignalChangedCount; i++) {
        DsVeosCoSim_IoSignalId signalId{};
        CheckResultWithMessage(reader.Read(signalId), "Could not read signal id.");

        MetaData& metaData = FindMetaData(signalId);
        Data& data = _dataVector[metaData.signalIndex];

        if (metaData.info.sizeKind == DsVeosCoSim_SizeKind_Variable) {
            uint32_t length = 0;
            CheckResultWithMessage(reader.Read(length), "Could not read current signal length.");
            if (length > metaData.info.length) {
                throw CoSimException(
                    fmt::format("Length of variable sized IO signal '{}' exceeds max size.", metaData.info.name));
            }

            data.currentLength = length;
        }

        const size_t totalSize = metaData.dataTypeSize * data.currentLength;
        CheckResultWithMessage(reader.Read(data.buffer.data(), totalSize), "Could not read signal data.");

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

LocalIoPartBuffer::LocalIoPartBuffer(CoSimType coSimType,
                                     const std::string& name,
                                     const std::vector<DsVeosCoSim_IoSignal>& signals)
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
        if (metaData.info.sizeKind == DsVeosCoSim_SizeKind_Fixed) {
            dataBuffer->currentLength = metaData.info.length;
            backupDataBuffer->currentLength = metaData.info.length;
        }
    }
}

void LocalIoPartBuffer::ClearDataInternal() {
    _changedSignalsQueue.Clear();

    for (auto& [signalId, metaData] : _metaDataLookup) {
        Data& data = _dataVector[metaData.signalIndex];
        data.isChanged = false;

        DataBuffer* dataBuffer = GetDataBuffer(data.offsetOfDataBufferInShm);
        DataBuffer* backupDataBuffer = GetDataBuffer(data.offsetOfBackupDataBufferInShm);

        if (data.offsetOfDataBufferInShm > data.offsetOfBackupDataBufferInShm) {
            FlipBuffers(data);
        }

        if (metaData.info.sizeKind == DsVeosCoSim_SizeKind_Variable) {
            dataBuffer->currentLength = 0;
            backupDataBuffer->currentLength = 0;
        }

        (void)::memset(dataBuffer->data, 0, metaData.dataTypeSize * metaData.info.length);
        (void)::memset(backupDataBuffer->data, 0, metaData.dataTypeSize * metaData.info.length);
    }
}

void LocalIoPartBuffer::WriteInternal(DsVeosCoSim_IoSignalId signalId, uint32_t length, const void* value) {
    MetaData& metaData = FindMetaData(signalId);
    Data& data = _dataVector[metaData.signalIndex];

    DataBuffer* dataBuffer = GetDataBuffer(data.offsetOfDataBufferInShm);

    bool currentLengthChanged{};
    if (metaData.info.sizeKind == DsVeosCoSim_SizeKind_Variable) {
        if (length > metaData.info.length) {
            throw CoSimException(
                fmt::format("Length of variable sized IO signal '{}' exceeds max size.", metaData.info.name));
        }

        currentLengthChanged = dataBuffer->currentLength != length;
    } else {
        if (length != metaData.info.length) {
            throw CoSimException(fmt::format("Length of fixed sized IO signal '{}' must be {} but was {}.",
                                             metaData.info.name,
                                             metaData.info.length,
                                             length));
        }
    }

    const size_t totalSize = metaData.dataTypeSize * length;

    bool dataChanged = ::memcmp(dataBuffer->data, value, totalSize) != 0;

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
        (void)::memcpy(dataBuffer->data, value, totalSize);
    }
}

void LocalIoPartBuffer::ReadInternal(DsVeosCoSim_IoSignalId signalId, uint32_t& length, void* value) {
    MetaData& metaData = FindMetaData(signalId);
    Data& data = _dataVector[metaData.signalIndex];

    DataBuffer* dataBuffer = GetDataBuffer(data.offsetOfDataBufferInShm);

    length = dataBuffer->currentLength;
    const size_t totalSize = metaData.dataTypeSize * length;
    (void)::memcpy(value, dataBuffer->data, totalSize);
}

void LocalIoPartBuffer::ReadInternal(DsVeosCoSim_IoSignalId signalId, uint32_t& length, const void** value) {
    MetaData& metaData = FindMetaData(signalId);
    Data& data = _dataVector[metaData.signalIndex];

    DataBuffer* dataBuffer = GetDataBuffer(data.offsetOfDataBufferInShm);

    length = dataBuffer->currentLength;
    *value = dataBuffer->data;
}

bool LocalIoPartBuffer::SerializeInternal(ChannelWriter& writer) {
    auto size = static_cast<uint32_t>(_changedSignalsQueue.Size());
    CheckResultWithMessage(writer.Write(size), "Could not write count of changed signals.");
    if (_changedSignalsQueue.IsEmpty()) {
        return true;
    }

    while (!_changedSignalsQueue.IsEmpty()) {
        MetaData* metaData = _changedSignalsQueue.PopFront();
        Data& data = _dataVector[metaData->signalIndex];

        CheckResultWithMessage(writer.Write(metaData->info.id), "Could not write signal id.");

        data.isChanged = false;
    }

    return true;
}

bool LocalIoPartBuffer::DeserializeInternal(ChannelReader& reader,
                                            DsVeosCoSim_SimulationTime simulationTime,
                                            const Callbacks& callbacks) {
    uint32_t ioSignalChangedCount = 0;
    CheckResultWithMessage(reader.Read(ioSignalChangedCount), "Could not read count of changed signals.");

    for (uint32_t i = 0; i < ioSignalChangedCount; i++) {
        DsVeosCoSim_IoSignalId signalId{};
        CheckResultWithMessage(reader.Read(signalId), "Could not read signal id.");

        MetaData& metaData = FindMetaData(signalId);
        Data& data = _dataVector[metaData.signalIndex];

        FlipBuffers(data);

        DataBuffer* dataBuffer = GetDataBuffer(data.offsetOfDataBufferInShm);

        if (callbacks.incomingSignalChangedCallback) {
            callbacks.incomingSignalChangedCallback(simulationTime,
                                                    metaData.info,
                                                    dataBuffer->currentLength,
                                                    dataBuffer->data);
        }
    }

    return true;
}

LocalIoPartBuffer::DataBuffer* LocalIoPartBuffer::GetDataBuffer(size_t offset) const {
    return reinterpret_cast<DataBuffer*>((static_cast<uint8_t*>(_sharedMemory.data()) + offset));
}

void LocalIoPartBuffer::FlipBuffers(Data& data) {
    std::swap(data.offsetOfDataBufferInShm, data.offsetOfBackupDataBufferInShm);
}

#endif

IoBuffer::IoBuffer(CoSimType coSimType,
                   [[maybe_unused]] ConnectionKind connectionKind,
                   const std::string& name,
                   const std::vector<DsVeosCoSim_IoSignal>& incomingSignals,
                   const std::vector<DsVeosCoSim_IoSignal>& outgoingSignals) {
    std::string suffixForWrite = ".Outgoing";
    std::string suffixForRead = ".Incoming";
    const std::vector<DsVeosCoSim_IoSignal>* writeSignals = &outgoingSignals;
    const std::vector<DsVeosCoSim_IoSignal>* readSignals = &incomingSignals;
    if (coSimType == CoSimType::Server) {
        std::swap(suffixForRead, suffixForWrite);
        writeSignals = &incomingSignals;
        readSignals = &outgoingSignals;
    }

#ifdef _WIN32
    if (connectionKind == ConnectionKind::Local) {
        _readBuffer = std::make_unique<LocalIoPartBuffer>(coSimType, name + suffixForRead, *readSignals);
        _writeBuffer = std::make_unique<LocalIoPartBuffer>(coSimType, name + suffixForWrite, *writeSignals);
    } else {
#endif
        _readBuffer = std::make_unique<RemoteIoPartBuffer>(coSimType, name + suffixForRead, *readSignals);
        _writeBuffer = std::make_unique<RemoteIoPartBuffer>(coSimType, name + suffixForWrite, *writeSignals);
#ifdef _WIN32
    }
#endif

    ClearData();
}

void IoBuffer::ClearData() const {
    _readBuffer->ClearData();
    _writeBuffer->ClearData();
}

void IoBuffer::Write(DsVeosCoSim_IoSignalId signalId, uint32_t length, const void* value) const {
    _writeBuffer->Write(signalId, length, value);
}

void IoBuffer::Read(DsVeosCoSim_IoSignalId signalId, uint32_t& length, void* value) const {
    _readBuffer->Read(signalId, length, value);
}

void IoBuffer::Read(DsVeosCoSim_IoSignalId signalId, uint32_t& length, const void** value) const {
    _readBuffer->Read(signalId, length, value);
}

bool IoBuffer::Serialize(ChannelWriter& writer) const {
    return _writeBuffer->Serialize(writer);
}

bool IoBuffer::Deserialize(ChannelReader& reader,
                           DsVeosCoSim_SimulationTime simulationTime,
                           const Callbacks& callbacks) const {
    return _readBuffer->Deserialize(reader, simulationTime, callbacks);
}

}  // namespace DsVeosCoSim
