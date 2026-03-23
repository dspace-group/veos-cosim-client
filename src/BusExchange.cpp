// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "BusExchange.hpp"

#include <memory>
#include <string>
#include <vector>

#include "BusExchangeFor.hpp"
#include "Protocol.hpp"
#include "Result.hpp"

namespace DsVeosCoSim {

namespace {

using CanPart = BusExchangeDetail::BusExchangeFor<BusExchangeDetail::CanBus>;
using EthPart = BusExchangeDetail::BusExchangeFor<BusExchangeDetail::EthBus>;
using LinPart = BusExchangeDetail::BusExchangeFor<BusExchangeDetail::LinBus>;
using FrPart = BusExchangeDetail::BusExchangeFor<BusExchangeDetail::FrBus>;

class BusExchangeImpl final : public BusExchange {
public:
    BusExchangeImpl() = default;

    [[nodiscard]] Result Initialize(CoSimType coSimType,
                                    ConnectionKind connectionKind,
                                    const std::string& name,
                                    const std::vector<CanController>& canControllers,
                                    const std::vector<EthController>& ethControllers,
                                    const std::vector<LinController>& linControllers,
                                    const std::vector<FrController>& frControllers,
                                    IProtocol& protocol) {
        _doFlexRayOperations = protocol.DoFlexRayOperations();

        _canPart = std::make_unique<CanPart>();
        _ethPart = std::make_unique<EthPart>();
        _linPart = std::make_unique<LinPart>();
        _frPart = std::make_unique<FrPart>();

        CheckResult(_canPart->Initialize(coSimType, connectionKind, name, canControllers, protocol));
        CheckResult(_ethPart->Initialize(coSimType, connectionKind, name, ethControllers, protocol));
        CheckResult(_linPart->Initialize(coSimType, connectionKind, name, linControllers, protocol));
        CheckResult(_frPart->Initialize(coSimType, connectionKind, name, frControllers, protocol));

        return CreateOk();
    }

    ~BusExchangeImpl() override = default;

    BusExchangeImpl(const BusExchangeImpl&) = delete;
    BusExchangeImpl& operator=(const BusExchangeImpl&) = delete;

    BusExchangeImpl(BusExchangeImpl&&) = delete;
    BusExchangeImpl& operator=(BusExchangeImpl&&) = delete;

    void ClearData() const override {
        _canPart->ClearData();
        _ethPart->ClearData();
        _linPart->ClearData();
        _frPart->ClearData();
    }

    [[nodiscard]] Result Transmit(const CanMessage& message) const override {
        return _canPart->Transmit(message);
    }

    [[nodiscard]] Result Transmit(const EthMessage& message) const override {
        return _ethPart->Transmit(message);
    }

    [[nodiscard]] Result Transmit(const LinMessage& message) const override {
        return _linPart->Transmit(message);
    }

    [[nodiscard]] Result Transmit(const FrMessage& message) const override {
        return _frPart->Transmit(message);
    }

    [[nodiscard]] Result Transmit(const CanMessageContainer& messageContainer) const override {
        return _canPart->Transmit(messageContainer);
    }

    [[nodiscard]] Result Transmit(const EthMessageContainer& messageContainer) const override {
        return _ethPart->Transmit(messageContainer);
    }

    [[nodiscard]] Result Transmit(const LinMessageContainer& messageContainer) const override {
        return _linPart->Transmit(messageContainer);
    }

    [[nodiscard]] Result Transmit(const FrMessageContainer& messageContainer) const override {
        return _frPart->Transmit(messageContainer);
    }

    [[nodiscard]] Result Receive(CanMessage& message) const override {
        return _canPart->Receive(message);
    }

    [[nodiscard]] Result Receive(EthMessage& message) const override {
        return _ethPart->Receive(message);
    }

    [[nodiscard]] Result Receive(LinMessage& message) const override {
        return _linPart->Receive(message);
    }

    [[nodiscard]] Result Receive(FrMessage& message) const override {
        return _frPart->Receive(message);
    }

    [[nodiscard]] Result Receive(CanMessageContainer& messageContainer) const override {
        return _canPart->Receive(messageContainer);
    }

    [[nodiscard]] Result Receive(EthMessageContainer& messageContainer) const override {
        return _ethPart->Receive(messageContainer);
    }

    [[nodiscard]] Result Receive(LinMessageContainer& messageContainer) const override {
        return _linPart->Receive(messageContainer);
    }

    [[nodiscard]] Result Receive(FrMessageContainer& messageContainer) const override {
        return _frPart->Receive(messageContainer);
    }

    [[nodiscard]] Result Serialize(ChannelWriter& writer) const override {
        CheckResultWithMessage(_canPart->Serialize(writer), "Could not transmit CAN messages.");
        CheckResultWithMessage(_ethPart->Serialize(writer), "Could not transmit Ethernet messages.");
        CheckResultWithMessage(_linPart->Serialize(writer), "Could not transmit LIN messages.");

        if (_doFlexRayOperations) {
            CheckResultWithMessage(_frPart->Serialize(writer), "Could not transmit FlexRay messages.");
        }

        return CreateOk();
    }

    [[nodiscard]] Result Deserialize(ChannelReader& reader, SimulationTime simulationTime, const Callbacks& callbacks) const override {
        CheckResultWithMessage(
            _canPart->Deserialize(reader, simulationTime, callbacks.canMessageReceivedCallback, callbacks.canMessageContainerReceivedCallback),
            "Could not receive CAN messages.");
        CheckResultWithMessage(
            _ethPart->Deserialize(reader, simulationTime, callbacks.ethMessageReceivedCallback, callbacks.ethMessageContainerReceivedCallback),
            "Could not receive Ethernet messages.");
        CheckResultWithMessage(
            _linPart->Deserialize(reader, simulationTime, callbacks.linMessageReceivedCallback, callbacks.linMessageContainerReceivedCallback),
            "Could not receive LIN messages.");

        if (_doFlexRayOperations) {
            CheckResultWithMessage(
                _frPart->Deserialize(reader, simulationTime, callbacks.frMessageReceivedCallback, callbacks.frMessageContainerReceivedCallback),
                "Could not receive FlexRay messages.");
        }

        return CreateOk();
    }

private:
    std::unique_ptr<CanPart> _canPart;
    std::unique_ptr<EthPart> _ethPart;
    std::unique_ptr<LinPart> _linPart;
    std::unique_ptr<FrPart> _frPart;

    bool _doFlexRayOperations{};
};

}  // namespace

[[nodiscard]] Result CreateBusExchange(CoSimType coSimType,
                                       ConnectionKind connectionKind,
                                       const std::string& name,
                                       const std::vector<CanController>& canControllers,
                                       const std::vector<EthController>& ethControllers,
                                       const std::vector<LinController>& linControllers,
                                       const std::vector<FrController>& frControllers,
                                       IProtocol& protocol,
                                       std::unique_ptr<BusExchange>& busExchange) {
    auto busExchangeTmp = std::make_unique<BusExchangeImpl>();
    CheckResult(busExchangeTmp->Initialize(coSimType, connectionKind, name, canControllers, ethControllers, linControllers, frControllers, protocol));
    busExchange = std::move(busExchangeTmp);
    return CreateOk();
}

}  // namespace DsVeosCoSim
