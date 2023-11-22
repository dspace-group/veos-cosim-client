// Copyright dSPACE GmbH. All rights reserved.

#include "IoBuffer.h"

#include <cstring>
#include <stdexcept>

#include "CoSimTypes.h"
#include "Logger.h"

namespace DsVeosCoSim {

namespace {

[[nodiscard]] Result CheckDataType(DataType dataType, const std::string& name) {
    switch (dataType) {
        case DataType::Bool:
        case DataType::Int8:
        case DataType::Int16:
        case DataType::Int32:
        case DataType::Int64:
        case DataType::UInt8:
        case DataType::UInt16:
        case DataType::UInt32:
        case DataType::UInt64:
        case DataType::Float32:
        case DataType::Float64:
            return Result::Ok;
        default:  // NOLINT(clang-diagnostic-covered-switch-default)
            LogError("Unknown data type '" + ToString(dataType) + "' for IO signal '" + name + "'.");
            return Result::Error;
    }
}

[[nodiscard]] Result CheckSizeKind(SizeKind sizeKind, const std::string& name) {
    switch (sizeKind) {
        case SizeKind::Fixed:
        case SizeKind::Variable:
            return Result::Ok;
        default:  // NOLINT(clang-diagnostic-covered-switch-default)
            LogError("Unknown size kind '" + ToString(sizeKind) + "' for IO signal '" + name + "'.");
            return Result::Error;
    }
}

}  // namespace

Result IoBuffer::Initialize(const std::vector<IoSignalContainer>& incomingSignals, const std::vector<IoSignalContainer>& outgoingSignals) {
    Reset();
    CheckResult(Initialize(incomingSignals, _readBuffers));
    CheckResult(Initialize(outgoingSignals, _writeBuffers));
    ClearData();
    return Result::Ok;
}

void IoBuffer::Reset() {
    ClearData();
    _readBuffers.clear();
    _writeBuffers.clear();
}

void IoBuffer::ClearData() {
    while (!_changed.empty()) {
        _changed.pop();
    }

    ClearData(_readBuffers);
    ClearData(_writeBuffers);
}

Result IoBuffer::Read(IoSignalId signalId, uint32_t& length, void* value) {
    InternalIoBuffer* readBuffer{};
    CheckResult(FindReadBuffer(signalId, &readBuffer));

    length = readBuffer->currentLength;
    const size_t totalSize = static_cast<size_t>(readBuffer->currentLength) * readBuffer->dataTypeSize;
    memcpy(value, readBuffer->data.data(), totalSize);
    return Result::Ok;
}

Result IoBuffer::Read(IoSignalId signalId, uint32_t& length, const void** value) {
    InternalIoBuffer* readBuffer{};
    CheckResult(FindReadBuffer(signalId, &readBuffer));

    length = readBuffer->currentLength;
    *value = readBuffer->data.data();
    return Result::Ok;
}

Result IoBuffer::Write(IoSignalId signalId, uint32_t length, const void* value) {
    InternalIoBuffer* writeBuffer{};
    CheckResult(FindWriteBuffer(signalId, &writeBuffer));

    const bool isVariableSized = writeBuffer->info.signal.sizeKind == SizeKind::Variable;
    if (isVariableSized) {
        if (length > writeBuffer->info.signal.length) {
            LogError("Length of variable sized IO signal '" + writeBuffer->info.name + "' exceeds max size.");
            return Result::Error;
        }

        if (writeBuffer->currentLength != length) {
            if (!writeBuffer->changed) {
                writeBuffer->changed = true;
                _changed.push(writeBuffer);
            }
        }

        writeBuffer->currentLength = length;
    } else {
        if (length != writeBuffer->info.signal.length) {
            LogError("Length of fixed sized IO signal '" + writeBuffer->info.name + "' must be " + std::to_string(writeBuffer->info.signal.length) +
                     " but was " + std::to_string(length) + ".");
            return Result::Error;
        }
    }

    const size_t totalSize = static_cast<size_t>(writeBuffer->currentLength) * writeBuffer->dataTypeSize;

    if (memcmp(writeBuffer->data.data(), value, totalSize) == 0) {
        return Result::Ok;
    }

    memcpy(writeBuffer->data.data(), value, totalSize);

    if (!writeBuffer->changed) {
        writeBuffer->changed = true;
        _changed.push(writeBuffer);
    }

    return Result::Ok;
}

Result IoBuffer::Deserialize(Channel& channel,  // NOLINT(readability-function-cognitive-complexity)
                             SimulationTime simulationTime,
                             const Callbacks& callbacks) {
    uint32_t ioSignalChangedCount = 0;
    CheckResult(channel.Read(ioSignalChangedCount));

    for (uint32_t i = 0; i < ioSignalChangedCount; i++) {
        uint32_t signalId = 0;
        CheckResult(channel.Read(signalId));

        InternalIoBuffer* readBuffer{};
        CheckResult(FindReadBuffer(signalId, &readBuffer));

        const bool isVariableSized = readBuffer->info.signal.sizeKind == SizeKind::Variable;
        if (isVariableSized) {
            uint32_t length = 0;
            CheckResult(channel.Read(length));
            if (length > readBuffer->info.signal.length) {
                LogError("Length of variable sized IO signal '" + readBuffer->info.name + "' exceeds max size.");
                return Result::Error;
            }

            readBuffer->currentLength = length;
        }

        const size_t totalSize = static_cast<size_t>(readBuffer->currentLength) * readBuffer->dataTypeSize;

        CheckResult(channel.Read(readBuffer->data.data(), static_cast<uint32_t>(totalSize)));

        if (callbacks.incomingSignalChangedCallback) {
            callbacks.incomingSignalChangedCallback(simulationTime, readBuffer->info.signal, readBuffer->currentLength, readBuffer->data.data());
        }
    }

    return Result::Ok;
}

Result IoBuffer::Serialize(Channel& channel) {
    CheckResult(channel.Write(static_cast<uint32_t>(_changed.size())));
    if (_changed.empty()) {
        return Result::Ok;
    }

    while (!_changed.empty()) {
        InternalIoBuffer* internalBuffer = _changed.front();
        CheckResult(channel.Write(internalBuffer->info.signal.id));

        const bool isVariableSized = internalBuffer->info.signal.sizeKind == SizeKind::Variable;
        if (isVariableSized) {
            CheckResult(channel.Write(internalBuffer->currentLength));
        }

        const size_t totalSize = static_cast<size_t>(internalBuffer->currentLength) * internalBuffer->dataTypeSize;

        CheckResult(channel.Write(internalBuffer->data.data(), static_cast<uint32_t>(totalSize)));

        internalBuffer->changed = false;

        _changed.pop();
    }

    return Result::Ok;
}

Result IoBuffer::Initialize(const std::vector<IoSignalContainer>& containers, std::unordered_map<IoSignalId, InternalIoBuffer>& buffer) {
    buffer.reserve(containers.size());

    for (const auto& container : containers) {
        InternalIoBuffer internalBuffer{};
        internalBuffer.info = container;

        CheckResult(CheckDataType(container.signal.dataType, container.signal.name));
        CheckResult(CheckSizeKind(container.signal.sizeKind, container.signal.name));

        internalBuffer.dataTypeSize = GetDataTypeSize(container.signal.dataType);

        const bool isFixedSize = container.signal.sizeKind == SizeKind::Fixed;
        if (isFixedSize) {
            internalBuffer.currentLength = container.signal.length;
        }

        if (container.signal.length == 0) {
            LogError("Invalid length " + std::to_string(container.signal.length) + " for IO signal '" + container.name + "'.");
            return Result::Error;
        }

        const size_t totalSize = static_cast<size_t>(container.signal.length) * internalBuffer.dataTypeSize;
        if (totalSize > UINT32_MAX) {
            LogError("Buffer size exceeds maximum for IO signal '" + container.name + "'.");
            return Result::Error;
        }

        const auto search = buffer.find(container.signal.id);
        if (search != buffer.end()) {
            LogError("Duplicated IO signal id " + std::to_string(container.signal.id) + ".");
            return Result::Error;
        }

        internalBuffer.data.resize(totalSize);

        buffer[container.signal.id] = internalBuffer;
    }

    return Result::Ok;
}

void IoBuffer::ClearData(std::unordered_map<IoSignalId, InternalIoBuffer>& buffer) {
    for (auto& [signalId, internalBuffer] : buffer) {
        internalBuffer.changed = false;

        std::fill(internalBuffer.data.begin(), internalBuffer.data.end(), static_cast<uint8_t>(0));

        if (internalBuffer.info.signal.sizeKind == SizeKind::Variable) {
            internalBuffer.currentLength = 0;
        }
    }
}

Result IoBuffer::FindReadBuffer(IoSignalId signalId, InternalIoBuffer** readBuffer) {
    const auto search = _readBuffers.find(signalId);
    if (search != _readBuffers.end()) {
        *readBuffer = &search->second;
        return Result::Ok;
    }

    LogError("IO signal id " + std::to_string(signalId) + " is unknown.");
    return Result::InvalidArgument;
}

Result IoBuffer::FindWriteBuffer(IoSignalId signalId, InternalIoBuffer** writeBuffer) {
    const auto search = _writeBuffers.find(signalId);
    if (search != _writeBuffers.end()) {
        *writeBuffer = &search->second;
        return Result::Ok;
    }

    LogError("IO signal id " + std::to_string(signalId) + " is unknown.");
    return Result::InvalidArgument;
}

}  // namespace DsVeosCoSim
