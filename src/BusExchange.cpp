// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "BusExchange.hpp"

#include <memory>
#include <string_view>
#include <vector>

#include "BusExchangeSpecific.hpp"
#include "Protocol.hpp"
#include "Result.hpp"

namespace DsVeosCoSim {

namespace {

using CanBusExchange = BusExchangeDetail::BusExchangeSpecific<BusExchangeDetail::CanBus>;
using EthBusExchange = BusExchangeDetail::BusExchangeSpecific<BusExchangeDetail::EthBus>;
using LinBusExchange = BusExchangeDetail::BusExchangeSpecific<BusExchangeDetail::LinBus>;
using FrBusExchange = BusExchangeDetail::BusExchangeSpecific<BusExchangeDetail::FrBus>;

template <typename TBusExchange, typename TController>
[[nodiscard]] Result CreateBusExchangeTransport(CoSimType coSimType,
                                                ConnectionKind connectionKind,
                                                std::string_view name,
                                                const std::vector<TController>& controllers,
                                                IProtocol& protocol,
                                                std::unique_ptr<TBusExchange>& busExchangeTransport) {
    CheckResult(TBusExchange::Create(coSimType, connectionKind, name, controllers, protocol, busExchangeTransport));
    return CreateOk();
}

class BusExchangeImpl final : public BusExchange {
public:
    BusExchangeImpl(std::unique_ptr<CanBusExchange> canBusExchange,
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

    ~BusExchangeImpl() override = default;

    BusExchangeImpl(const BusExchangeImpl&) = delete;
    BusExchangeImpl& operator=(const BusExchangeImpl&) = delete;

    BusExchangeImpl(BusExchangeImpl&&) = delete;
    BusExchangeImpl& operator=(BusExchangeImpl&&) = delete;

    void ClearData() const override {
        _canBusExchange->ClearData();
        _ethBusExchange->ClearData();
        _linBusExchange->ClearData();
        _frBusExchange->ClearData();
    }

    [[nodiscard]] Result Transmit(const CanMessage& message) const override {
        return _canBusExchange->Transmit(message);
    }

    [[nodiscard]] Result Transmit(const EthMessage& message) const override {
        return _ethBusExchange->Transmit(message);
    }

    [[nodiscard]] Result Transmit(const LinMessage& message) const override {
        return _linBusExchange->Transmit(message);
    }

    [[nodiscard]] Result Transmit(const FrMessage& message) const override {
        return _frBusExchange->Transmit(message);
    }

    [[nodiscard]] Result Transmit(const CanMessageContainer& messageContainer) const override {
        return _canBusExchange->Transmit(messageContainer);
    }

    [[nodiscard]] Result Transmit(const EthMessageContainer& messageContainer) const override {
        return _ethBusExchange->Transmit(messageContainer);
    }

    [[nodiscard]] Result Transmit(const LinMessageContainer& messageContainer) const override {
        return _linBusExchange->Transmit(messageContainer);
    }

    [[nodiscard]] Result Transmit(const FrMessageContainer& messageContainer) const override {
        return _frBusExchange->Transmit(messageContainer);
    }

    [[nodiscard]] Result Receive(CanMessage& message) const override {
        return _canBusExchange->Receive(message);
    }

    [[nodiscard]] Result Receive(EthMessage& message) const override {
        return _ethBusExchange->Receive(message);
    }

    [[nodiscard]] Result Receive(LinMessage& message) const override {
        return _linBusExchange->Receive(message);
    }

    [[nodiscard]] Result Receive(FrMessage& message) const override {
        return _frBusExchange->Receive(message);
    }

    [[nodiscard]] Result Receive(CanMessageContainer& messageContainer) const override {
        return _canBusExchange->Receive(messageContainer);
    }

    [[nodiscard]] Result Receive(EthMessageContainer& messageContainer) const override {
        return _ethBusExchange->Receive(messageContainer);
    }

    [[nodiscard]] Result Receive(LinMessageContainer& messageContainer) const override {
        return _linBusExchange->Receive(messageContainer);
    }

    [[nodiscard]] Result Receive(FrMessageContainer& messageContainer) const override {
        return _frBusExchange->Receive(messageContainer);
    }

    [[nodiscard]] Result Serialize(ChannelWriter& writer) const override {
        CheckResultWithMessage(_canBusExchange->Serialize(writer), "Could not transmit CAN messages.");
        CheckResultWithMessage(_ethBusExchange->Serialize(writer), "Could not transmit Ethernet messages.");
        CheckResultWithMessage(_linBusExchange->Serialize(writer), "Could not transmit LIN messages.");

        if (_doFlexRayOperations) {
            CheckResultWithMessage(_frBusExchange->Serialize(writer), "Could not transmit FlexRay messages.");
        }

        return CreateOk();
    }

    [[nodiscard]] Result Deserialize(ChannelReader& reader, SimulationTime simulationTime, const Callbacks& callbacks) const override {
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

private:
    std::unique_ptr<CanBusExchange> _canBusExchange;
    std::unique_ptr<EthBusExchange> _ethBusExchange;
    std::unique_ptr<LinBusExchange> _linBusExchange;
    std::unique_ptr<FrBusExchange> _frBusExchange;

    bool _doFlexRayOperations{};
};

}  // namespace

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
    CheckResult(CreateBusExchangeTransport(coSimType, connectionKind, name, canControllers, protocol, canBusExchange));

    std::unique_ptr<EthBusExchange> ethBusExchange;
    CheckResult(CreateBusExchangeTransport(coSimType, connectionKind, name, ethControllers, protocol, ethBusExchange));

    std::unique_ptr<LinBusExchange> linBusExchange;
    CheckResult(CreateBusExchangeTransport(coSimType, connectionKind, name, linControllers, protocol, linBusExchange));

    std::unique_ptr<FrBusExchange> frBusExchange;
    CheckResult(CreateBusExchangeTransport(coSimType, connectionKind, name, frControllers, protocol, frBusExchange));

    busExchange = std::make_unique<BusExchangeImpl>(std::move(canBusExchange),
                                                    std::move(ethBusExchange),
                                                    std::move(linBusExchange),
                                                    std::move(frBusExchange),
                                                    protocol.DoFlexRayOperations());
    return CreateOk();
}

}  // namespace DsVeosCoSim
