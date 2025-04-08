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
    virtual ~BusBuffer() noexcept = default;

    BusBuffer(const BusBuffer&) = delete;
    BusBuffer& operator=(const BusBuffer&) = delete;

    BusBuffer(BusBuffer&&) = delete;
    BusBuffer& operator=(BusBuffer&&) = delete;

    virtual void ClearData() const = 0;

    [[nodiscard]] virtual bool Transmit(const CanMessage& message) const = 0;
    [[nodiscard]] virtual bool Transmit(const EthMessage& message) const = 0;
    [[nodiscard]] virtual bool Transmit(const LinMessage& message) const = 0;

    [[nodiscard]] virtual bool Receive(CanMessage& message) const = 0;
    [[nodiscard]] virtual bool Receive(EthMessage& message) const = 0;
    [[nodiscard]] virtual bool Receive(LinMessage& message) const = 0;

    [[nodiscard]] virtual bool Serialize(ChannelWriter& writer) const = 0;
    [[nodiscard]] virtual bool Deserialize(ChannelReader& reader,
                                           SimulationTime simulationTime,
                                           const Callbacks& callbacks) const = 0;
};

[[nodiscard]] std::unique_ptr<BusBuffer> CreateBusBuffer(CoSimType coSimType,
                                                         ConnectionKind connectionKind,
                                                         const std::string& name,
                                                         const std::vector<CanController>& canControllers,
                                                         const std::vector<EthController>& ethControllers,
                                                         const std::vector<LinController>& linControllers);

}  // namespace DsVeosCoSim
