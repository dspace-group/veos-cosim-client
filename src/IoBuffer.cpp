// Copyright dSPACE GmbH. All rights reserved.

#include "IoBuffer.h"

#include <cstring>

#include "CoSimTypes.h"
#include "Logger.h"

namespace DsVeosCoSim {

namespace {

[[nodiscard]] Result CheckSizeKind(DsVeosCoSim_SizeKind sizeKind, const std::string& name) {
    switch (sizeKind) {
        case DsVeosCoSim_SizeKind_Fixed:
        case DsVeosCoSim_SizeKind_Variable:
            return Result::Ok;
        default:  // NOLINT(clang-diagnostic-covered-switch-default)
            LogError("Unknown size kind '" + ToString(sizeKind) + "' for IO signal '" + name + "'.");
            return Result::Error;
    }
}

}  // namespace

Result IoBuffer::Initialize(const std::vector<DsVeosCoSim_IoSignal>& incomingSignals, const std::vector<DsVeosCoSim_IoSignal>& outgoingSignals) {
    Reset();
    CheckResult(Initialize(incomingSignals, _readBuffers));
    CheckResult(Initialize(outgoingSignals, _writeBuffers));
    Clear();
    return Result::Ok;
}

void IoBuffer::Reset() {
    Clear();
    _readBuffers.clear();
    _writeBuffers.clear();
}

void IoBuffer::Clear() {
    while (!_changed.empty()) {
        _changed.pop();
    }

    Clear(_readBuffers);
    Clear(_writeBuffers);
}

Result IoBuffer::Read(IoSignalId signalId, uint32_t& length, void* value) {
    InternalIoBuffer* readBuffer{};
    CheckResult(FindReadBuffer(signalId, &readBuffer));

    length = readBuffer->currentLength;
    const size_t totalSize = static_cast<size_t>(readBuffer->currentLength) * readBuffer->dataTypeSize;
    (void)std::memcpy(value, readBuffer->data.data(), totalSize);
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

    const bool isVariableSized = writeBuffer->info.sizeKind == DsVeosCoSim_SizeKind_Variable;
    if (isVariableSized) {
        if (length > writeBuffer->info.length) {
            LogError("Length of variable sized IO signal '" + std::string(writeBuffer->info.name) + "' exceeds max size.");
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
        if (length != writeBuffer->info.length) {
            LogError("Length of fixed sized IO signal '" + std::string(writeBuffer->info.name) + "' must be " + std::to_string(writeBuffer->info.length) +
                     " but was " + std::to_string(length) + ".");
            return Result::Error;
        }
    }

    const size_t totalSize = static_cast<size_t>(writeBuffer->currentLength) * writeBuffer->dataTypeSize;

    if (std::memcmp(writeBuffer->data.data(), value, totalSize) == 0) {
        return Result::Ok;
    }

    (void)std::memcpy(writeBuffer->data.data(), value, totalSize);

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
        IoSignalId signalId{};
        CheckResult(channel.Read(signalId));

        InternalIoBuffer* readBuffer{};
        CheckResult(FindReadBuffer(signalId, &readBuffer));

        const bool isVariableSized = readBuffer->info.sizeKind == DsVeosCoSim_SizeKind_Variable;
        if (isVariableSized) {
            uint32_t length = 0;
            CheckResult(channel.Read(length));
            if (length > readBuffer->info.length) {
                LogError("Length of variable sized IO signal '" + std::string(readBuffer->info.name) + "' exceeds max size.");
                return Result::Error;
            }

            readBuffer->currentLength = length;
        }

        const size_t totalSize = static_cast<size_t>(readBuffer->currentLength) * readBuffer->dataTypeSize;

        CheckResult(channel.Read(readBuffer->data.data(), static_cast<uint32_t>(totalSize)));

        if (callbacks.incomingSignalChangedCallback) {
            callbacks.incomingSignalChangedCallback(simulationTime, readBuffer->info, readBuffer->currentLength, readBuffer->data.data());
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
        CheckResult(channel.Write(internalBuffer->info.id));

        const bool isVariableSized = internalBuffer->info.sizeKind == DsVeosCoSim_SizeKind_Variable;
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

Result IoBuffer::Initialize(const std::vector<DsVeosCoSim_IoSignal>& signals, std::unordered_map<IoSignalId, InternalIoBuffer>& buffer) {
    buffer.reserve(signals.size());

    for (const auto& signal : signals) {
        InternalIoBuffer internalBuffer{};
        internalBuffer.info = signal;

        CheckResult(CheckSizeKind(signal.sizeKind, signal.name));

        internalBuffer.dataTypeSize = GetDataTypeSize(signal.dataType);

        if (internalBuffer.dataTypeSize == 0) {
            LogError("Invalid data type " + ToString(signal.dataType) + " for IO signal '" + signal.name + "'.");
            return Result::Error;
        }

        const bool isFixedSize = signal.sizeKind == DsVeosCoSim_SizeKind_Fixed;
        if (isFixedSize) {
            internalBuffer.currentLength = signal.length;
        }

        if (signal.length == 0) {
            LogError("Invalid length " + std::to_string(signal.length) + " for IO signal '" + signal.name + "'.");
            return Result::Error;
        }

        const size_t totalSize = static_cast<size_t>(signal.length) * internalBuffer.dataTypeSize;
        if (totalSize > UINT32_MAX) {
            LogError("Buffer size exceeds maximum for IO signal '" + std::string(signal.name) + "'.");
            return Result::Error;
        }

        const auto search = buffer.find(signal.id);
        if (search != buffer.end()) {
            LogError("Duplicated IO signal id " + std::to_string(signal.id) + ".");
            return Result::Error;
        }

        internalBuffer.data.resize(totalSize);

        buffer[signal.id] = internalBuffer;
    }

    return Result::Ok;
}

void IoBuffer::Clear(std::unordered_map<IoSignalId, InternalIoBuffer>& buffer) {
    for (auto& [signalId, internalBuffer] : buffer) {
        internalBuffer.changed = false;

        std::fill(internalBuffer.data.begin(), internalBuffer.data.end(), static_cast<uint8_t>(0));

        if (internalBuffer.info.sizeKind == DsVeosCoSim_SizeKind_Variable) {
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
