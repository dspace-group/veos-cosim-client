// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <queue>
#include <unordered_map>
#include <vector>

#include "CoSimTypes.h"
#include "Communication.h"

namespace DsVeosCoSim {

class IoBuffer {
    struct InternalIoBuffer {
        IoSignal info{};
        uint32_t currentLength = 0;
        uint32_t dataTypeSize = 0;
        bool changed = false;
        std::vector<uint8_t> data;
    };

public:
    IoBuffer() = default;
    ~IoBuffer() noexcept = default;

    IoBuffer(const IoBuffer&) = delete;
    IoBuffer& operator=(IoBuffer const&) = delete;

    IoBuffer(IoBuffer&&) noexcept = default;
    IoBuffer& operator=(IoBuffer&&) = default;

    [[nodiscard]] Result Initialize(const std::vector<IoSignal>& incomingSignals, const std::vector<IoSignal>& outgoingSignals);
    void Reset();

    void ClearData();

    [[nodiscard]] Result Read(IoSignalId signalId, uint32_t& length, void* value);
    [[nodiscard]] Result Read(IoSignalId signalId, uint32_t& length, const void** value);
    [[nodiscard]] Result Write(IoSignalId signalId, uint32_t length, const void* value);

    [[nodiscard]] Result Deserialize(Channel& channel, SimulationTime simulationTime, const Callbacks& callbacks);
    [[nodiscard]] Result Serialize(Channel& channel);

private:
    [[nodiscard]] static Result Initialize(const std::vector<IoSignal>& signals, std::unordered_map<IoSignalId, InternalIoBuffer>& buffer);
    static void ClearData(std::unordered_map<IoSignalId, InternalIoBuffer>& buffer);

    [[nodiscard]] Result FindReadBuffer(IoSignalId signalId, InternalIoBuffer** readBuffer);
    [[nodiscard]] Result FindWriteBuffer(IoSignalId signalId, InternalIoBuffer** writeBuffer);

    std::queue<InternalIoBuffer*> _changed;
    std::unordered_map<IoSignalId, InternalIoBuffer> _readBuffers;
    std::unordered_map<IoSignalId, InternalIoBuffer> _writeBuffers;
};

}  // namespace DsVeosCoSim
