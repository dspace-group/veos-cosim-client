// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <memory>
#include <mutex>

#include "BusExchangeCommon.hpp"

namespace DsVeosCoSim::BusExchangeDetail {

template <typename TBus>
class LockedBusExchangePart final : public IBusExchangePart<TBus> {
public:
    using TMessage = typename TBus::Message;
    using TMessageContainer = typename TBus::MessageContainer;
    using TController = typename TBus::Controller;

    explicit LockedBusExchangePart(std::unique_ptr<IBusExchangePart<TBus>> proxiedPart) : _proxiedPart(std::move(proxiedPart)) {
    }

    ~LockedBusExchangePart() noexcept override = default;

    LockedBusExchangePart(const LockedBusExchangePart&) = delete;
    LockedBusExchangePart& operator=(const LockedBusExchangePart&) = delete;

    LockedBusExchangePart(LockedBusExchangePart&&) = delete;
    LockedBusExchangePart& operator=(LockedBusExchangePart&&) = delete;

    void ClearData() override {
        std::scoped_lock lock(_mutex);
        _proxiedPart->ClearData();
    }

    [[nodiscard]] Result Transmit(const TMessage& message) override {
        std::scoped_lock lock(_mutex);
        return _proxiedPart->Transmit(message);
    }

    [[nodiscard]] Result Transmit(const TMessageContainer& messageContainer) override {
        std::scoped_lock lock(_mutex);
        return _proxiedPart->Transmit(messageContainer);
    }

    [[nodiscard]] Result Receive(TMessage& message) override {
        std::scoped_lock lock(_mutex);
        return _proxiedPart->Receive(message);
    }

    [[nodiscard]] Result Receive(TMessageContainer& messageContainer) override {
        std::scoped_lock lock(_mutex);
        return _proxiedPart->Receive(messageContainer);
    }

    [[nodiscard]] Result Serialize(ChannelWriter& writer) override {
        std::scoped_lock lock(_mutex);
        return _proxiedPart->Serialize(writer);
    }

    [[nodiscard]] Result Deserialize(ChannelReader& reader,
                                     SimulationTime simulationTime,
                                     const BusMessageCallback<TBus>& messageCallback,
                                     const BusMessageContainerCallback<TBus>& messageContainerCallback) override {
        std::scoped_lock lock(_mutex);
        return _proxiedPart->Deserialize(reader, simulationTime, messageCallback, messageContainerCallback);
    }

private:
    std::unique_ptr<IBusExchangePart<TBus>> _proxiedPart;
    std::mutex _mutex;
};

}  // namespace DsVeosCoSim::BusExchangeDetail
