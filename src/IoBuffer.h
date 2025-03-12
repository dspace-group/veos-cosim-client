// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "Channel.h"
#include "DsVeosCoSim/CoSimTypes.h"

namespace DsVeosCoSim {

class IoBuffer {  // NOLINT
public:
    virtual ~IoBuffer() noexcept = default;

    virtual void ClearData() const = 0;

    virtual void Write(IoSignalId signalId, uint32_t length, const void* value) const = 0;
    virtual void Read(IoSignalId signalId, uint32_t& length, void* value) const = 0;
    virtual void Read(IoSignalId signalId, uint32_t& length, const void** value) const = 0;

    [[nodiscard]] virtual bool Serialize(ChannelWriter& writer) const = 0;
    [[nodiscard]] virtual bool Deserialize(ChannelReader& reader,
                                           SimulationTime simulationTime,
                                           const Callbacks& callbacks) const = 0;
};

[[nodiscard]] std::unique_ptr<IoBuffer> CreateIoBuffer(CoSimType coSimType,
                                                       ConnectionKind connectionKind,
                                                       const std::string& name,
                                                       const std::vector<IoSignal>& incomingSignals,
                                                       const std::vector<IoSignal>& outgoingSignals);

}  // namespace DsVeosCoSim
