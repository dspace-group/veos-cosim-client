// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <vector>

#include "Environment.hpp"
#include "Logger.hpp"
#include "Protocol.hpp"
#include "RingBuffer.hpp"
#include "SignalExchangeCommon.hpp"

namespace DsVeosCoSim::SignalExchangeDetail {

class RemoteSignalExchangePart final : public ISignalExchangePart {
    struct SignalValueState {
        uint32_t currentLength{};
        bool isChanged{};
        std::vector<uint8_t> buffer;
    };

public:
    RemoteSignalExchangePart(IProtocol& protocol,
                             SignalRegistry signalRegistry,
                             std::vector<SignalValueState> signalStates,
                             RingBuffer<SignalMetaDataPtr> changedSignalsQueue)
        : _protocol(protocol),
          _signalRegistry(std::move(signalRegistry)),
          _signalStates(std::move(signalStates)),
          _changedSignalsQueue(std::move(changedSignalsQueue)) {
    }

    ~RemoteSignalExchangePart() noexcept override = default;

    RemoteSignalExchangePart(const RemoteSignalExchangePart&) = delete;
    RemoteSignalExchangePart& operator=(const RemoteSignalExchangePart&) = delete;

    RemoteSignalExchangePart(RemoteSignalExchangePart&&) = delete;
    RemoteSignalExchangePart& operator=(RemoteSignalExchangePart&&) = delete;

    [[nodiscard]] static Result Create(IProtocol& protocol, const std::vector<IoSignal>& ioSignals, std::unique_ptr<ISignalExchangePart>& signalExchangePart) {
        SignalRegistry signalRegistry;
        CheckResult(SignalRegistry::Create(ioSignals, signalRegistry));

        std::unordered_map<IoSignalId, SignalMetaData>& metaDataLookup = signalRegistry.GetMetaDataLookup();
        auto changedSignalsQueue = RingBuffer<SignalMetaDataPtr>(metaDataLookup.size());
        std::vector<SignalValueState> signalStates(metaDataLookup.size());
        for (auto& [signalId, metaData] : metaDataLookup) {
            SignalValueState signalState{};
            signalState.buffer.resize(metaData.totalDataSize);
            if (metaData.info.sizeKind == SizeKind::Fixed) {
                signalState.currentLength = metaData.info.length;
            }

            signalStates[metaData.signalIndex] = std::move(signalState);
        }

        signalExchangePart =
            std::make_unique<RemoteSignalExchangePart>(protocol, std::move(signalRegistry), std::move(signalStates), std::move(changedSignalsQueue));
        return CreateOk();
    }

    void ClearData() override {
        _changedSignalsQueue.Clear();

        for (auto& [signalId, metaData] : _signalRegistry.GetMetaDataLookup()) {
            auto& [currentLength, isChanged, buffer] = _signalStates[metaData.signalIndex];
            isChanged = false;
            if (metaData.info.sizeKind == SizeKind::Variable) {
                currentLength = 0;
            }

            std::fill(buffer.begin(), buffer.end(), static_cast<uint8_t>(0));
        }
    }

    [[nodiscard]] Result Write(IoSignalId signalId, uint32_t length, const void* value) override {
        SignalMetaDataPtr metaData{};
        CheckResult(_signalRegistry.FindMetaData(signalId, metaData));
        auto& [currentLength, isChanged, buffer] = _signalStates[metaData->signalIndex];

        if (metaData->info.sizeKind == SizeKind::Variable) {
            if (length > metaData->info.length) {
                LogError("Length of variable sized IO signal '{}' exceeds max size.", metaData->info.name);
                return CreateError();
            }

            if (currentLength != length) {
                if (!isChanged) {
                    isChanged = true;
                    if (!_changedSignalsQueue.TryPushBack(metaData)) {
                        LogError("Changed signals queue is full.");
                        return CreateError();
                    }
                }
            }

            currentLength = length;
        } else {
            if (length != metaData->info.length) {
                LogError("Length of fixed sized IO signal '{}' must be {} but was {}.", metaData->info.name, metaData->info.length, length);
                return CreateError();
            }
        }

        size_t totalSize = metaData->dataTypeSize * length;

        int32_t compareResult = memcmp(buffer.data(), value, totalSize);
        if (compareResult == 0) {
            return CreateOk();
        }

        memcpy(buffer.data(), value, totalSize);

        if (!isChanged) {
            isChanged = true;
            if (!_changedSignalsQueue.TryPushBack(metaData)) {
                LogError("Changed signals queue is full.");
                return CreateError();
            }
        }

        return CreateOk();
    }

    [[nodiscard]] Result Read(IoSignalId signalId, uint32_t& length, void* value) override {
        SignalMetaDataPtr metaData{};
        CheckResult(_signalRegistry.FindMetaData(signalId, metaData));
        SignalValueState& signalState = _signalStates[metaData->signalIndex];

        length = signalState.currentLength;
        size_t totalSize = metaData->dataTypeSize * length;
        memcpy(value, signalState.buffer.data(), totalSize);
        return CreateOk();
    }

    [[nodiscard]] Result Read(IoSignalId signalId, uint32_t& length, const void** value) override {
        SignalMetaDataPtr metaData{};
        CheckResult(_signalRegistry.FindMetaData(signalId, metaData));
        SignalValueState& signalState = _signalStates[metaData->signalIndex];

        length = signalState.currentLength;
        *value = signalState.buffer.data();
        return CreateOk();
    }

    // Remote transport sends changed signal ids together with the current length
    // and payload bytes. No shared memory is involved on this path.
    [[nodiscard]] Result Serialize(ChannelWriter& writer) override {
        CheckResultWithMessage(_protocol.WriteSize(writer, _changedSignalsQueue.Size()), "Could not write count of changed signals.");

        SignalMetaDataPtr metaData{};
        while (_changedSignalsQueue.TryPopFront(metaData)) {
            auto& [currentLength, isChanged, buffer] = _signalStates[metaData->signalIndex];

            CheckResultWithMessage(_protocol.WriteSignalId(writer, metaData->info.id), "Could not write signal id.");

            if (metaData->info.sizeKind == SizeKind::Variable) {
                CheckResultWithMessage(_protocol.WriteLength(writer, currentLength), "Could not write signal length.");
            }

            size_t totalSize = metaData->dataTypeSize * currentLength;
            CheckResultWithMessage(_protocol.WriteData(writer, buffer.data(), totalSize), "Could not write signal data.");
            isChanged = false;

            if (IsProtocolTracingEnabled()) {
                LogProtData(IoDataToString(metaData->info, currentLength, buffer.data()));
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

            SignalMetaDataPtr metaData{};
            CheckResult(_signalRegistry.FindMetaData(signalId, metaData));
            SignalValueState& signalState = _signalStates[metaData->signalIndex];

            if (metaData->info.sizeKind == SizeKind::Variable) {
                uint32_t length = 0;
                CheckResultWithMessage(_protocol.ReadLength(reader, length), "Could not read signal length.");
                if (length > metaData->info.length) {
                    LogError("Length of variable sized IO signal '{}' exceeds max size.", metaData->info.name);
                    return CreateError();
                }

                signalState.currentLength = length;
            }

            size_t totalSize = metaData->dataTypeSize * signalState.currentLength;
            CheckResultWithMessage(_protocol.ReadData(reader, signalState.buffer.data(), totalSize), "Could not read signal data.");

            if (IsProtocolTracingEnabled()) {
                LogProtData(IoDataToString(metaData->info, signalState.currentLength, signalState.buffer.data()));
            }

            if (callbacks.incomingSignalChangedCallback) {
                callbacks.incomingSignalChangedCallback(simulationTime, metaData->info, signalState.currentLength, signalState.buffer.data());
            }
        }

        return CreateOk();
    }

private:
    IProtocol& _protocol;
    SignalRegistry _signalRegistry;
    std::vector<SignalValueState> _signalStates;
    RingBuffer<SignalMetaDataPtr> _changedSignalsQueue;
};

}  // namespace DsVeosCoSim::SignalExchangeDetail
