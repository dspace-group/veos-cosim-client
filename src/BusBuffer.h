// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Channel.h"
#include "DsVeosCoSim/CoSimTypes.h"

namespace DsVeosCoSim {

class BusBuffer {
protected:
    BusBuffer() = default;

public:
    virtual ~BusBuffer() = default;

    BusBuffer(const BusBuffer&) = delete;
    BusBuffer& operator=(const BusBuffer&) = delete;

    BusBuffer(BusBuffer&&) = delete;
    BusBuffer& operator=(BusBuffer&&) = delete;

    virtual void ClearData() const = 0;

    [[nodiscard]] virtual Result Transmit(const CanMessage& message) const = 0;
    [[nodiscard]] virtual Result Transmit(const EthMessage& message) const = 0;
    [[nodiscard]] virtual Result Transmit(const LinMessage& message) const = 0;

    [[nodiscard]] virtual Result Transmit(const CanMessageContainer& message) const = 0;
    [[nodiscard]] virtual Result Transmit(const EthMessageContainer& message) const = 0;
    [[nodiscard]] virtual Result Transmit(const LinMessageContainer& message) const = 0;

    [[nodiscard]] virtual Result Receive(CanMessage& message) const = 0;
    [[nodiscard]] virtual Result Receive(EthMessage& message) const = 0;
    [[nodiscard]] virtual Result Receive(LinMessage& message) const = 0;

    [[nodiscard]] virtual Result Receive(CanMessageContainer& message) const = 0;
    [[nodiscard]] virtual Result Receive(EthMessageContainer& message) const = 0;
    [[nodiscard]] virtual Result Receive(LinMessageContainer& message) const = 0;

    [[nodiscard]] virtual Result Serialize(ChannelWriter& writer) const = 0;
    [[nodiscard]] virtual Result Deserialize(ChannelReader& reader,
                                             SimulationTime simulationTime,
                                             const Callbacks& callbacks) const = 0;
};

[[nodiscard]] Result CreateBusBuffer(CoSimType coSimType,
                                     ConnectionKind connectionKind,
                                     const std::string& name,
                                     const std::vector<CanController>& canControllers,
                                     const std::vector<EthController>& ethControllers,
                                     const std::vector<LinController>& linControllers,
                                     std::unique_ptr<BusBuffer>& busBuffer);

}  // namespace DsVeosCoSim
