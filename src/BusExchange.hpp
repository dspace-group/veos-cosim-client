// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <memory>
#include <string_view>
#include <vector>

#include "Channel.hpp"
#include "CoSimTypes.hpp"
#include "Protocol.hpp"
#include "Result.hpp"

// Forward declarations to avoid including implementation headers
namespace DsVeosCoSim::BusExchangeDetail {

template <typename TBus>
class BusExchangeSpecific;
struct CanBus;
struct EthBus;
struct LinBus;
struct FrBus;

}  // namespace DsVeosCoSim::BusExchangeDetail

namespace DsVeosCoSim {

class BusExchange final {
    using CanBusExchange = BusExchangeDetail::BusExchangeSpecific<BusExchangeDetail::CanBus>;
    using EthBusExchange = BusExchangeDetail::BusExchangeSpecific<BusExchangeDetail::EthBus>;
    using LinBusExchange = BusExchangeDetail::BusExchangeSpecific<BusExchangeDetail::LinBus>;
    using FrBusExchange = BusExchangeDetail::BusExchangeSpecific<BusExchangeDetail::FrBus>;

public:
    BusExchange(std::unique_ptr<CanBusExchange> canBusExchange,
                std::unique_ptr<EthBusExchange> ethBusExchange,
                std::unique_ptr<LinBusExchange> linBusExchange,
                std::unique_ptr<FrBusExchange> frBusExchange,
                bool doFlexRayOperations);
    ~BusExchange() noexcept;

    BusExchange(const BusExchange&) = delete;
    BusExchange& operator=(const BusExchange&) = delete;

    BusExchange(BusExchange&&) = delete;
    BusExchange& operator=(BusExchange&&) = delete;

    void ClearData() const;

    [[nodiscard]] Result Transmit(const CanMessage& message) const;
    [[nodiscard]] Result Transmit(const EthMessage& message) const;
    [[nodiscard]] Result Transmit(const LinMessage& message) const;
    [[nodiscard]] Result Transmit(const FrMessage& message) const;

    [[nodiscard]] Result Transmit(const CanMessageContainer& messageContainer) const;
    [[nodiscard]] Result Transmit(const EthMessageContainer& messageContainer) const;
    [[nodiscard]] Result Transmit(const LinMessageContainer& messageContainer) const;
    [[nodiscard]] Result Transmit(const FrMessageContainer& messageContainer) const;

    [[nodiscard]] Result Receive(CanMessage& message) const;
    [[nodiscard]] Result Receive(EthMessage& message) const;
    [[nodiscard]] Result Receive(LinMessage& message) const;
    [[nodiscard]] Result Receive(FrMessage& message) const;

    [[nodiscard]] Result Receive(CanMessageContainer& messageContainer) const;
    [[nodiscard]] Result Receive(EthMessageContainer& messageContainer) const;
    [[nodiscard]] Result Receive(LinMessageContainer& messageContainer) const;
    [[nodiscard]] Result Receive(FrMessageContainer& messageContainer) const;

    [[nodiscard]] Result Serialize(ChannelWriter& writer) const;
    [[nodiscard]] Result Deserialize(ChannelReader& reader, SimulationTime simulationTime, const Callbacks& callbacks) const;

private:
    std::unique_ptr<CanBusExchange> _canBusExchange;
    std::unique_ptr<EthBusExchange> _ethBusExchange;
    std::unique_ptr<LinBusExchange> _linBusExchange;
    std::unique_ptr<FrBusExchange> _frBusExchange;

    bool _doFlexRayOperations{};
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
