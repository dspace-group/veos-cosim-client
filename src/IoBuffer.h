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
        DsVeosCoSim_IoSignal info{};
        uint32_t currentLength = 0;
        uint32_t dataTypeSize = 0;
        bool changed = false;
        std::vector<uint8_t> data;
    };

public:
    IoBuffer() = default;
    ~IoBuffer() noexcept = default;

    IoBuffer(const IoBuffer&) = delete;
    IoBuffer& operator=(const IoBuffer&) = delete;

    IoBuffer(IoBuffer&&) noexcept = default;
    IoBuffer& operator=(IoBuffer&&) noexcept = default;

    [[nodiscard]] Result Initialize(const std::vector<DsVeosCoSim_IoSignal>& incomingSignals, const std::vector<DsVeosCoSim_IoSignal>& outgoingSignals);
    void Reset();

    void Clear();

    [[nodiscard]] Result Read(IoSignalId signalId, uint32_t& length, void* value);
    [[nodiscard]] Result Read(IoSignalId signalId, uint32_t& length, const void** value);
    [[nodiscard]] Result Write(IoSignalId signalId, uint32_t length, const void* value);

    [[nodiscard]] Result Deserialize(Channel& channel, SimulationTime simulationTime, const Callbacks& callbacks);
    [[nodiscard]] Result Serialize(Channel& channel);

private:
    [[nodiscard]] static Result Initialize(const std::vector<DsVeosCoSim_IoSignal>& signals, std::unordered_map<IoSignalId, InternalIoBuffer>& buffer);
    static void Clear(std::unordered_map<IoSignalId, InternalIoBuffer>& buffer);

    [[nodiscard]] Result FindReadBuffer(IoSignalId signalId, InternalIoBuffer** readBuffer);
    [[nodiscard]] Result FindWriteBuffer(IoSignalId signalId, InternalIoBuffer** writeBuffer);

    std::queue<InternalIoBuffer*> _changed;
    std::unordered_map<IoSignalId, InternalIoBuffer> _readBuffers;
    std::unordered_map<IoSignalId, InternalIoBuffer> _writeBuffers;
};

}  // namespace DsVeosCoSim
