// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <memory>
#include <string>
#include <vector>

#include <fmt/format.h>

#include "BusExchangeCommon.hpp"
#include "BusExchangeLocked.hpp"
#include "BusExchangeRemote.hpp"

#ifdef _WIN32
#include "BusExchangeLocalWin.hpp"
#endif

namespace DsVeosCoSim::BusExchangeDetail {

// This layer selects the transport backend and keeps bus-specific length checks
// independent from transport details.
template <typename TBus>
class BusExchangeFor final {
public:
    using TMessage = typename TBus::Message;
    using TMessageContainer = typename TBus::MessageContainer;
    using TController = typename TBus::Controller;

    BusExchangeFor() = default;
    ~BusExchangeFor() noexcept = default;

    BusExchangeFor(const BusExchangeFor&) = delete;
    BusExchangeFor& operator=(const BusExchangeFor&) = delete;

    BusExchangeFor(BusExchangeFor&&) = delete;
    BusExchangeFor& operator=(BusExchangeFor&&) = delete;

    [[nodiscard]] Result Initialize(CoSimType coSimType,
                                    [[maybe_unused]] ConnectionKind connectionKind,
                                    [[maybe_unused]] const std::string& name,
                                    const std::vector<TController>& controllers,
                                    IProtocol& protocol) {
#ifdef _WIN32
        if (connectionKind == ConnectionKind::Local) {
            std::string_view suffixForTransmit = coSimType == CoSimType::Client ? "Transmit" : "Receive";
            std::string_view suffixForReceive = coSimType == CoSimType::Client ? "Receive" : "Transmit";

            std::string transmitPartName = fmt::format("{}{}{}", name, TBus::ShmNamePart, suffixForTransmit);
            std::string receivePartName = fmt::format("{}{}{}", name, TBus::ShmNamePart, suffixForReceive);

            // Local transport uses two shared-memory regions so each side can publish
            // and consume independently without swapping queue ownership.
            _outboundPart = std::make_unique<LocalBusExchangePart<TBus>>(protocol, transmitPartName);
            _inboundPart = std::make_unique<LocalBusExchangePart<TBus>>(protocol, receivePartName);
        } else {
#endif
            _outboundPart = std::make_unique<RemoteBusExchangePart<TBus>>(protocol);
            _inboundPart = std::make_unique<RemoteBusExchangePart<TBus>>(protocol);
#ifdef _WIN32
        }
#endif
        if (coSimType == CoSimType::Client) {
            _outboundPart = std::make_unique<LockedBusExchangePart<TBus>>(std::move(_outboundPart));
            _inboundPart = std::make_unique<LockedBusExchangePart<TBus>>(std::move(_inboundPart));
        }

        CheckResult(_outboundPart->Initialize(controllers));
        CheckResult(_inboundPart->Initialize(controllers));

        return CreateOk();
    }

    void ClearData() const {
        _outboundPart->ClearData();
        _inboundPart->ClearData();
    }

    [[nodiscard]] Result Transmit(const TMessage& message) const {
        CheckResult(CheckMessageLength(message.length));
        return _outboundPart->Transmit(message);
    }

    [[nodiscard]] Result Transmit(const TMessageContainer& messageContainer) const {
        CheckResult(CheckMessageLength(messageContainer.length));
        return _outboundPart->Transmit(messageContainer);
    }

    [[nodiscard]] Result Receive(TMessage& message) const {
        return _inboundPart->Receive(message);
    }

    [[nodiscard]] Result Receive(TMessageContainer& messageContainer) const {
        return _inboundPart->Receive(messageContainer);
    }

    [[nodiscard]] Result Serialize(ChannelWriter& writer) const {
        return _outboundPart->Serialize(writer);
    }

    [[nodiscard]] Result Deserialize(ChannelReader& reader,
                                     SimulationTime simulationTime,
                                     const BusMessageCallback<TBus>& messageCallback,
                                     const BusMessageContainerCallback<TBus>& messageContainerCallback) const {
        return _inboundPart->Deserialize(reader, simulationTime, messageCallback, messageContainerCallback);
    }

private:
    [[nodiscard]] static Result CheckMessageLength(uint32_t length) {
        if (length > TBus::MessageMaxLength) {
            LogError("{} message data exceeds maximum length.", TBus::DisplayName);
            return CreateInvalidArgument();
        }

        return CreateOk();
    }

    std::unique_ptr<IBusExchangePart<TBus>> _outboundPart;
    std::unique_ptr<IBusExchangePart<TBus>> _inboundPart;
};

}  // namespace DsVeosCoSim::BusExchangeDetail