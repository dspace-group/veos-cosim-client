// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <memory>
#include <string>
#include <string_view>
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
// independent of transport details.
template <typename TBus>
class BusExchangeSpecific final {
public:
    using TMessage = typename TBus::Message;
    using TMessageContainer = typename TBus::MessageContainer;
    using TController = typename TBus::Controller;

    BusExchangeSpecific(std::unique_ptr<IBusExchangePart<TBus>> outboundPart, std::unique_ptr<IBusExchangePart<TBus>> inboundPart)
        : _outboundPart(std::move(outboundPart)), _inboundPart(std::move(inboundPart)) {
    }

    ~BusExchangeSpecific() noexcept = default;

    BusExchangeSpecific(const BusExchangeSpecific&) = delete;
    BusExchangeSpecific& operator=(const BusExchangeSpecific&) = delete;

    BusExchangeSpecific(BusExchangeSpecific&&) = delete;
    BusExchangeSpecific& operator=(BusExchangeSpecific&&) = delete;

    [[nodiscard]] static Result Create(CoSimType coSimType,
                                       [[maybe_unused]] ConnectionKind connectionKind,
                                       [[maybe_unused]] std::string_view name,
                                       const std::vector<TController>& controllers,
                                       IProtocol& protocol,
                                       std::unique_ptr<BusExchangeSpecific>& busExchangeFor) {
        std::unique_ptr<IBusExchangePart<TBus>> outboundPart;
        std::unique_ptr<IBusExchangePart<TBus>> inboundPart;

#ifdef _WIN32
        if (connectionKind == ConnectionKind::Local) {
            std::string_view suffixForTransmit = coSimType == CoSimType::Client ? "Transmit" : "Receive";
            std::string_view suffixForReceive = coSimType == CoSimType::Client ? "Receive" : "Transmit";

            std::string transmitPartName = fmt::format("{}{}{}", name, TBus::ShmNamePart, suffixForTransmit);
            std::string receivePartName = fmt::format("{}{}{}", name, TBus::ShmNamePart, suffixForReceive);

            // Local transport uses two shared-memory regions so each side can publish
            // and consume independently without swapping queue ownership.
            CheckResult(LocalBusExchangePart<TBus>::Create(protocol, std::move(transmitPartName), controllers, outboundPart));
            CheckResult(LocalBusExchangePart<TBus>::Create(protocol, std::move(receivePartName), controllers, inboundPart));
        } else {
#endif
            CheckResult(RemoteBusExchangePart<TBus>::Create(protocol, controllers, outboundPart));
            CheckResult(RemoteBusExchangePart<TBus>::Create(protocol, controllers, inboundPart));
#ifdef _WIN32
        }
#endif
        if (coSimType == CoSimType::Client) {
            outboundPart = std::make_unique<LockedBusExchangePart<TBus>>(std::move(outboundPart));
            inboundPart = std::make_unique<LockedBusExchangePart<TBus>>(std::move(inboundPart));
        }

        busExchangeFor = std::make_unique<BusExchangeSpecific>(std::move(outboundPart), std::move(inboundPart));
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
