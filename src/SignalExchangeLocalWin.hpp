// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#ifdef _WIN32

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

#include "Environment.hpp"
#include "Logger.hpp"
#include "OsUtilities.hpp"
#include "Protocol.hpp"
#include "RingBuffer.hpp"
#include "SignalExchangeCommon.hpp"

namespace DsVeosCoSim::SignalExchangeDetail {

class LocalSignalExchangePart final : public ISignalExchangePart {
    struct SharedData {
        uint32_t currentLength{};
        uint8_t data[1]{};
    };

    using SharedDataPtr = SharedData*;

    struct SignalState {
        size_t offsetOfActivePartInShm{};
        size_t offsetOfStagingPartInShm{};
        bool isChanged{};
    };

public:
    LocalSignalExchangePart(IProtocol& protocol,
                            std::string name,
                            SignalRegistry signalRegistry,
                            std::vector<SignalState> signalStates,
                            SharedMemory sharedMemory,
                            RingBuffer<SignalMetaDataPtr> changedSignalsQueue)
        : _protocol(protocol),
          _name(std::move(name)),
          _signalRegistry(std::move(signalRegistry)),
          _signalStates(std::move(signalStates)),
          _sharedMemory(std::move(sharedMemory)),
          _changedSignalsQueue(std::move(changedSignalsQueue)) {
    }

    ~LocalSignalExchangePart() noexcept override = default;

    LocalSignalExchangePart(const LocalSignalExchangePart&) = delete;
    LocalSignalExchangePart& operator=(const LocalSignalExchangePart&) = delete;

    LocalSignalExchangePart(LocalSignalExchangePart&&) = delete;
    LocalSignalExchangePart& operator=(LocalSignalExchangePart&&) = delete;

    [[nodiscard]] static Result Create(IProtocol& protocol,
                                       const std::vector<IoSignal>& ioSignals,
                                       std::string name,
                                       std::unique_ptr<ISignalExchangePart>& signalExchangePart) {
        SignalRegistry signalRegistry;
        CheckResult(SignalRegistry::Create(ioSignals, signalRegistry));

        std::unordered_map<IoSignalId, SignalMetaData>& metaDataLookup = signalRegistry.GetMetaDataLookup();
        auto changedSignalsQueue = RingBuffer<SignalMetaDataPtr>(metaDataLookup.size());
        std::vector<SignalState> signalStates(metaDataLookup.size());

        size_t totalSize{};
        for (auto& [signalId, metaData] : metaDataLookup) {
            SignalState signalState{};
            signalState.offsetOfActivePartInShm = totalSize;
            totalSize += sizeof(uint32_t) + metaData.totalDataSize;

            signalState.offsetOfStagingPartInShm = totalSize;
            totalSize += sizeof(uint32_t) + metaData.totalDataSize;

            signalStates[metaData.signalIndex] = signalState;
        }

        SharedMemory sharedMemory;
        if (totalSize > 0) {
            CheckResult(SharedMemory::CreateOrOpen(name, totalSize, sharedMemory));
        }

        for (auto& [signalId, metaData] : metaDataLookup) {
            SignalState& signalState = signalStates[metaData.signalIndex];
            SharedDataPtr activePart = reinterpret_cast<SharedDataPtr>(sharedMemory.GetData() + signalState.offsetOfActivePartInShm);
            SharedDataPtr stagingPart = reinterpret_cast<SharedDataPtr>(sharedMemory.GetData() + signalState.offsetOfStagingPartInShm);
            if (metaData.info.sizeKind == SizeKind::Fixed) {
                activePart->currentLength = metaData.info.length;
                stagingPart->currentLength = metaData.info.length;
            }
        }

        auto localSignalExchangePart = std::make_unique<LocalSignalExchangePart>(protocol,
                                                                                 std::move(name),
                                                                                 std::move(signalRegistry),
                                                                                 std::move(signalStates),
                                                                                 std::move(sharedMemory),
                                                                                 std::move(changedSignalsQueue));
        localSignalExchangePart->ClearData();
        signalExchangePart = std::move(localSignalExchangePart);
        return CreateOk();
    }

    // Each signal owns two parts inside shared memory. One is the active value
    // visible to readers, the other is a staging part for the next published update.
    void ClearData() override {
        _changedSignalsQueue.Clear();

        for (auto& [signalId, metaData] : _signalRegistry.GetMetaDataLookup()) {
            SignalState& signalState = _signalStates[metaData.signalIndex];
            signalState.isChanged = false;

            SharedDataPtr activePart = GetSharedData(signalState.offsetOfActivePartInShm);
            SharedDataPtr stagingPart = GetSharedData(signalState.offsetOfStagingPartInShm);

            if (signalState.offsetOfActivePartInShm > signalState.offsetOfStagingPartInShm) {
                SwapActiveAndStagingParts(signalState);
            }

            if (metaData.info.sizeKind == SizeKind::Variable) {
                activePart->currentLength = 0;
                stagingPart->currentLength = 0;
            }

            memset(activePart->data, 0, metaData.dataTypeSize * metaData.info.length);
            memset(stagingPart->data, 0, metaData.dataTypeSize * metaData.info.length);
        }
    }

    [[nodiscard]] Result Write(IoSignalId signalId, uint32_t length, const void* value) override {
        SignalMetaDataPtr metaData{};
        CheckResult(_signalRegistry.FindMetaData(signalId, metaData));
        SignalState& signalState = _signalStates[metaData->signalIndex];

        SharedDataPtr activePart = GetSharedData(signalState.offsetOfActivePartInShm);

        bool currentLengthChanged{};
        if (metaData->info.sizeKind == SizeKind::Variable) {
            if (length > metaData->info.length) {
                LogError("Length of variable sized IO signal '{}' exceeds max size.", metaData->info.name);
                return CreateError();
            }

            currentLengthChanged = activePart->currentLength != length;
        } else {
            if (length != metaData->info.length) {
                LogError("Length of fixed sized IO signal '{}' must be {} but was {}.", metaData->info.name, metaData->info.length, length);
                return CreateError();
            }
        }

        size_t totalSize = metaData->dataTypeSize * length;

        bool dataChanged = memcmp(activePart->data, value, totalSize) != 0;

        if (!currentLengthChanged && !dataChanged) {
            return CreateOk();
        }

        if (!signalState.isChanged) {
            signalState.isChanged = true;
            if (!_changedSignalsQueue.TryPushBack(metaData)) {
                LogError("Changed signals queue is full.");
                return CreateError();
            }

            SwapActiveAndStagingParts(signalState);
            activePart = GetSharedData(signalState.offsetOfActivePartInShm);
        }

        activePart->currentLength = length;
        if (dataChanged) {
            memcpy(activePart->data, value, totalSize);
        }

        return CreateOk();
    }

    [[nodiscard]] Result Read(IoSignalId signalId, uint32_t& length, void* value) override {
        SignalMetaDataPtr metaData{};
        CheckResult(_signalRegistry.FindMetaData(signalId, metaData));
        SignalState& signalState = _signalStates[metaData->signalIndex];

        SharedDataPtr activePart = GetSharedData(signalState.offsetOfActivePartInShm);

        length = activePart->currentLength;
        size_t totalSize = metaData->dataTypeSize * length;
        memcpy(value, activePart->data, totalSize);
        return CreateOk();
    }

    [[nodiscard]] Result Read(IoSignalId signalId, uint32_t& length, const void** value) override {
        SignalMetaDataPtr metaData{};
        CheckResult(_signalRegistry.FindMetaData(signalId, metaData));
        SignalState& signalState = _signalStates[metaData->signalIndex];

        SharedDataPtr activePart = GetSharedData(signalState.offsetOfActivePartInShm);

        length = activePart->currentLength;
        *value = activePart->data;
        return CreateOk();
    }

    // For local transport the payload bytes are already in shared memory. The
    // channel only publishes which signal ids changed since the last transfer.
    [[nodiscard]] Result Serialize(ChannelWriter& writer) override {
        CheckResultWithMessage(_protocol.WriteSize(writer, _changedSignalsQueue.Size()), "Could not write count of changed signals.");

        SignalMetaDataPtr metaData{};
        while (_changedSignalsQueue.TryPopFront(metaData)) {
            SignalState& signalState = _signalStates[metaData->signalIndex];

            if (IsProtocolTracingEnabled()) {
                SharedDataPtr activePart = GetSharedData(signalState.offsetOfActivePartInShm);
                LogProtData(IoDataToString(metaData->info, activePart->currentLength, activePart->data));
            }

            CheckResultWithMessage(_protocol.WriteSignalId(writer, metaData->info.id), "Could not write signal id.");

            signalState.isChanged = false;
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
            SignalState& signalState = _signalStates[metaData->signalIndex];

            SwapActiveAndStagingParts(signalState);

            SharedDataPtr activePart = GetSharedData(signalState.offsetOfActivePartInShm);

            if (IsProtocolTracingEnabled()) {
                LogProtData(IoDataToString(metaData->info, activePart->currentLength, activePart->data));
            }

            if (callbacks.incomingSignalChangedCallback) {
                callbacks.incomingSignalChangedCallback(simulationTime, metaData->info, activePart->currentLength, activePart->data);
            }
        }

        return CreateOk();
    }

private:
    [[nodiscard]] SharedDataPtr GetSharedData(size_t offset) const {
        return reinterpret_cast<SharedDataPtr>(_sharedMemory.GetData() + offset);
    }

    static void SwapActiveAndStagingParts(SignalState& signalState) {
        std::swap(signalState.offsetOfActivePartInShm, signalState.offsetOfStagingPartInShm);
    }

    IProtocol& _protocol;
    std::string _name;
    SignalRegistry _signalRegistry;
    std::vector<SignalState> _signalStates;
    SharedMemory _sharedMemory;
    RingBuffer<SignalMetaDataPtr> _changedSignalsQueue;
};

}  // namespace DsVeosCoSim::SignalExchangeDetail

#endif
