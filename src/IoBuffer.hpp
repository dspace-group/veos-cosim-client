// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "Channel.hpp"
#include "CoSimTypes.hpp"
#include "Protocol.hpp"
#include "Result.hpp"

namespace DsVeosCoSim {

class IoBuffer {
protected:
    IoBuffer() = default;

public:
    virtual ~IoBuffer() noexcept = default;

    IoBuffer(const IoBuffer&) = delete;
    IoBuffer& operator=(const IoBuffer&) = delete;

    IoBuffer(IoBuffer&&) = delete;
    IoBuffer& operator=(IoBuffer&&) = delete;

    virtual void ClearData() const = 0;

    [[nodiscard]] virtual Result Write(IoSignalId signalId, uint32_t length, const void* value) const = 0;
    [[nodiscard]] virtual Result Read(IoSignalId signalId, uint32_t& length, void* value) const = 0;
    [[nodiscard]] virtual Result Read(IoSignalId signalId, uint32_t& length, const void** value) const = 0;

    [[nodiscard]] virtual Result Serialize(ChannelWriter& writer) const = 0;
    [[nodiscard]] virtual Result Deserialize(ChannelReader& reader, SimulationTime simulationTime, const Callbacks& callbacks) const = 0;
};

[[nodiscard]] Result CreateIoBuffer(CoSimType coSimType,
                                    ConnectionKind connectionKind,
                                    const std::string& name,
                                    const std::vector<IoSignal>& incomingSignals,
                                    const std::vector<IoSignal>& outgoingSignals,
                                    IProtocol& protocol,
                                    std::unique_ptr<IoBuffer>& ioBuffer);

}  // namespace DsVeosCoSim
