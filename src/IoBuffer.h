// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

#include "Channel.h"
#include "DsVeosCoSim/CoSimTypes.h"

namespace DsVeosCoSim {

class IoBuffer {
protected:
    IoBuffer() = default;

public:
    virtual ~IoBuffer() = default;

    IoBuffer(const IoBuffer&) = delete;
    IoBuffer& operator=(const IoBuffer&) = delete;

    IoBuffer(IoBuffer&&) = delete;
    IoBuffer& operator=(IoBuffer&&) = delete;

    virtual void ClearData() const = 0;

    [[nodiscard]] virtual Result Write(IoSignalId signalId, uint32_t length, const void* value) const = 0;
    [[nodiscard]] virtual Result Read(IoSignalId signalId, uint32_t& length, void* value) const = 0;
    [[nodiscard]] virtual Result Read(IoSignalId signalId, uint32_t& length, const void** value) const = 0;

    [[nodiscard]] virtual Result Serialize(ChannelWriter& writer) const = 0;
    [[nodiscard]] virtual Result Deserialize(ChannelReader& reader,
                                             SimulationTime simulationTime,
                                             const Callbacks& callbacks) const = 0;
};

[[nodiscard]] Result CreateIoBuffer(CoSimType coSimType,
                                    ConnectionKind connectionKind,
                                    std::string_view name,
                                    const std::vector<IoSignal>& incomingSignals,
                                    const std::vector<IoSignal>& outgoingSignals,
                                    std::unique_ptr<IoBuffer>& ioBuffer);

}  // namespace DsVeosCoSim
