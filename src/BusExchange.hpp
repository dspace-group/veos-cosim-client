// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <memory>
#include <string_view>
#include <vector>

#include "Channel.hpp"
#include "CoSimTypes.hpp"
#include "Protocol.hpp"
#include "Result.hpp"

namespace DsVeosCoSim {

class BusExchange {
protected:
    BusExchange() = default;

public:
    virtual ~BusExchange() noexcept = default;

    BusExchange(const BusExchange&) = delete;
    BusExchange& operator=(const BusExchange&) = delete;

    BusExchange(BusExchange&&) = delete;
    BusExchange& operator=(BusExchange&&) = delete;

    virtual void ClearData() const = 0;

    [[nodiscard]] virtual Result Transmit(const CanMessage& message) const = 0;
    [[nodiscard]] virtual Result Transmit(const EthMessage& message) const = 0;
    [[nodiscard]] virtual Result Transmit(const LinMessage& message) const = 0;
    [[nodiscard]] virtual Result Transmit(const FrMessage& message) const = 0;

    [[nodiscard]] virtual Result Transmit(const CanMessageContainer& messageContainer) const = 0;
    [[nodiscard]] virtual Result Transmit(const EthMessageContainer& messageContainer) const = 0;
    [[nodiscard]] virtual Result Transmit(const LinMessageContainer& messageContainer) const = 0;
    [[nodiscard]] virtual Result Transmit(const FrMessageContainer& messageContainer) const = 0;

    [[nodiscard]] virtual Result Receive(CanMessage& message) const = 0;
    [[nodiscard]] virtual Result Receive(EthMessage& message) const = 0;
    [[nodiscard]] virtual Result Receive(LinMessage& message) const = 0;
    [[nodiscard]] virtual Result Receive(FrMessage& message) const = 0;

    [[nodiscard]] virtual Result Receive(CanMessageContainer& messageContainer) const = 0;
    [[nodiscard]] virtual Result Receive(EthMessageContainer& messageContainer) const = 0;
    [[nodiscard]] virtual Result Receive(LinMessageContainer& messageContainer) const = 0;
    [[nodiscard]] virtual Result Receive(FrMessageContainer& messageContainer) const = 0;

    [[nodiscard]] virtual Result Serialize(ChannelWriter& writer) const = 0;
    [[nodiscard]] virtual Result Deserialize(ChannelReader& reader, SimulationTime simulationTime, const Callbacks& callbacks) const = 0;
};

[[nodiscard]] Result CreateBusExchange(CoSimType coSimType,
                                       ConnectionKind connectionKind,
                                       std::string_view name,
                                       const std::vector<CanController>& canControllers,
                                       const std::vector<EthController>& ethControllers,
                                       const std::vector<LinController>& linControllers,
                                       const std::vector<FrController>& frControllers,
                                       IProtocol& protocol,
                                       std::unique_ptr<BusExchange>& busExchange);

}  // namespace DsVeosCoSim
