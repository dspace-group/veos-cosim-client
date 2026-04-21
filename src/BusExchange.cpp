// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "BusExchange.hpp"

#include <memory>
#include <string_view>
#include <utility>
#include <vector>

#include "BusExchangeCommon.hpp"
#include "BusExchangeSpecific.hpp"
#include "Channel.hpp"
#include "CoSimTypes.hpp"
#include "Protocol.hpp"
#include "Result.hpp"

namespace DsVeosCoSim {

using CanBusExchange = BusExchangeDetail::BusExchangeSpecific<BusExchangeDetail::CanBus>;
using EthBusExchange = BusExchangeDetail::BusExchangeSpecific<BusExchangeDetail::EthBus>;
using LinBusExchange = BusExchangeDetail::BusExchangeSpecific<BusExchangeDetail::LinBus>;
using FrBusExchange = BusExchangeDetail::BusExchangeSpecific<BusExchangeDetail::FrBus>;

BusExchange::BusExchange(std::unique_ptr<CanBusExchange> canBusExchange,
                         std::unique_ptr<EthBusExchange> ethBusExchange,
                         std::unique_ptr<LinBusExchange> linBusExchange,
                         std::unique_ptr<FrBusExchange> frBusExchange,
                         bool doFlexRayOperations)
    : _canBusExchange(std::move(canBusExchange)),
      _ethBusExchange(std::move(ethBusExchange)),
      _linBusExchange(std::move(linBusExchange)),
      _frBusExchange(std::move(frBusExchange)),
      _doFlexRayOperations(doFlexRayOperations) {
}

BusExchange::~BusExchange() noexcept = default;

void BusExchange::ClearData() const {
    _canBusExchange->ClearData();
    _ethBusExchange->ClearData();
    _linBusExchange->ClearData();
    _frBusExchange->ClearData();
}

[[nodiscard]] Result BusExchange::Transmit(const CanMessage& message) const {
    return _canBusExchange->Transmit(message);
}

[[nodiscard]] Result BusExchange::Transmit(const EthMessage& message) const {
    return _ethBusExchange->Transmit(message);
}

[[nodiscard]] Result BusExchange::Transmit(const LinMessage& message) const {
    return _linBusExchange->Transmit(message);
}

[[nodiscard]] Result BusExchange::Transmit(const FrMessage& message) const {
    return _frBusExchange->Transmit(message);
}

[[nodiscard]] Result BusExchange::Transmit(const CanMessageContainer& messageContainer) const {
    return _canBusExchange->Transmit(messageContainer);
}

[[nodiscard]] Result BusExchange::Transmit(const EthMessageContainer& messageContainer) const {
    return _ethBusExchange->Transmit(messageContainer);
}

[[nodiscard]] Result BusExchange::Transmit(const LinMessageContainer& messageContainer) const {
    return _linBusExchange->Transmit(messageContainer);
}

[[nodiscard]] Result BusExchange::Transmit(const FrMessageContainer& messageContainer) const {
    return _frBusExchange->Transmit(messageContainer);
}

[[nodiscard]] Result BusExchange::Receive(CanMessage& message) const {
    return _canBusExchange->Receive(message);
}

[[nodiscard]] Result BusExchange::Receive(EthMessage& message) const {
    return _ethBusExchange->Receive(message);
}

[[nodiscard]] Result BusExchange::Receive(LinMessage& message) const {
    return _linBusExchange->Receive(message);
}

[[nodiscard]] Result BusExchange::Receive(FrMessage& message) const {
    return _frBusExchange->Receive(message);
}

[[nodiscard]] Result BusExchange::Receive(CanMessageContainer& messageContainer) const {
    return _canBusExchange->Receive(messageContainer);
}

[[nodiscard]] Result BusExchange::Receive(EthMessageContainer& messageContainer) const {
    return _ethBusExchange->Receive(messageContainer);
}

[[nodiscard]] Result BusExchange::Receive(LinMessageContainer& messageContainer) const {
    return _linBusExchange->Receive(messageContainer);
}

[[nodiscard]] Result BusExchange::Receive(FrMessageContainer& messageContainer) const {
    return _frBusExchange->Receive(messageContainer);
}

[[nodiscard]] Result BusExchange::Serialize(ChannelWriter& writer) const {
    CheckResultWithMessage(_canBusExchange->Serialize(writer), "Could not transmit CAN messages.");
    CheckResultWithMessage(_ethBusExchange->Serialize(writer), "Could not transmit Ethernet messages.");
    CheckResultWithMessage(_linBusExchange->Serialize(writer), "Could not transmit LIN messages.");

    if (_doFlexRayOperations) {
        CheckResultWithMessage(_frBusExchange->Serialize(writer), "Could not transmit FlexRay messages.");
    }

    return CreateOk();
}

[[nodiscard]] Result BusExchange::Deserialize(ChannelReader& reader, SimulationTime simulationTime, const Callbacks& callbacks) const {
    CheckResultWithMessage(
        _canBusExchange->Deserialize(reader, simulationTime, callbacks.canMessageReceivedCallback, callbacks.canMessageContainerReceivedCallback),
        "Could not receive CAN messages.");
    CheckResultWithMessage(
        _ethBusExchange->Deserialize(reader, simulationTime, callbacks.ethMessageReceivedCallback, callbacks.ethMessageContainerReceivedCallback),
        "Could not receive Ethernet messages.");
    CheckResultWithMessage(
        _linBusExchange->Deserialize(reader, simulationTime, callbacks.linMessageReceivedCallback, callbacks.linMessageContainerReceivedCallback),
        "Could not receive LIN messages.");

    if (_doFlexRayOperations) {
        CheckResultWithMessage(
            _frBusExchange->Deserialize(reader, simulationTime, callbacks.frMessageReceivedCallback, callbacks.frMessageContainerReceivedCallback),
            "Could not receive FlexRay messages.");
    }

    return CreateOk();
}

[[nodiscard]] Result CreateBusExchange(CoSimType coSimType,
                                       ConnectionKind connectionKind,
                                       std::string_view name,
                                       const std::vector<CanController>& canControllers,
                                       const std::vector<EthController>& ethControllers,
                                       const std::vector<LinController>& linControllers,
                                       const std::vector<FrController>& frControllers,
                                       IProtocol& protocol,
                                       std::unique_ptr<BusExchange>& busExchange) {
    std::unique_ptr<CanBusExchange> canBusExchange;
    CheckResult(CanBusExchange::Create(coSimType, connectionKind, name, canControllers, protocol, canBusExchange));

    std::unique_ptr<EthBusExchange> ethBusExchange;
    CheckResult(EthBusExchange::Create(coSimType, connectionKind, name, ethControllers, protocol, ethBusExchange));

    std::unique_ptr<LinBusExchange> linBusExchange;
    CheckResult(LinBusExchange::Create(coSimType, connectionKind, name, linControllers, protocol, linBusExchange));

    std::unique_ptr<FrBusExchange> frBusExchange;
    CheckResult(FrBusExchange::Create(coSimType, connectionKind, name, frControllers, protocol, frBusExchange));

    busExchange = std::make_unique<BusExchange>(std::move(canBusExchange),
                                                std::move(ethBusExchange),
                                                std::move(linBusExchange),
                                                std::move(frBusExchange),
                                                protocol.DoFlexRayOperations());
    return CreateOk();
}

}  // namespace DsVeosCoSim
