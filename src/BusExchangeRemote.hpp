// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include "BusExchangeCommon.hpp"
#include "Environment.hpp"
#include "Protocol.hpp"
#include "RingBuffer.hpp"

namespace DsVeosCoSim::BusExchangeDetail {

template <typename TBus>
class RemoteBusExchangePart final : public IBusExchangePart<TBus> {
public:
    using TMessage = typename TBus::Message;
    using TMessageContainer = typename TBus::MessageContainer;
    using TController = typename TBus::Controller;

    RemoteBusExchangePart(IProtocol& protocol,
                          ControllerRegistry<TBus> controllerRegistry,
                          std::vector<uint32_t> queuedMessageCountByController,
                          RingBuffer<TMessageContainer> queuedMessageContainers)
        : _protocol(protocol),
          _controllerRegistry(std::move(controllerRegistry)),
          _queuedMessageCountByController(std::move(queuedMessageCountByController)),
          _queuedMessageContainers(std::move(queuedMessageContainers)) {
    }

    ~RemoteBusExchangePart() noexcept override = default;

    RemoteBusExchangePart(const RemoteBusExchangePart&) = delete;
    RemoteBusExchangePart& operator=(const RemoteBusExchangePart&) = delete;

    RemoteBusExchangePart(RemoteBusExchangePart&&) = delete;
    RemoteBusExchangePart& operator=(RemoteBusExchangePart&&) = delete;

    [[nodiscard]] static Result Create(IProtocol& protocol,
                                       const std::vector<TController>& controllers,
                                       std::unique_ptr<IBusExchangePart<TBus>>& busExchangePart) {
        ControllerRegistry<TBus> controllerRegistry;
        CheckResult(ControllerRegistry<TBus>::Create(controllers, controllerRegistry));

        size_t combinedQueueCapacity = controllerRegistry.GetCombinedQueueCapacity();
        std::vector<uint32_t> queuedMessageCountByController(controllerRegistry.GetControllerStatesById().size());
        auto queuedMessageContainers = RingBuffer<TMessageContainer>(combinedQueueCapacity);

        busExchangePart = std::make_unique<RemoteBusExchangePart>(protocol,
                                                                  std::move(controllerRegistry),
                                                                  std::move(queuedMessageCountByController),
                                                                  std::move(queuedMessageContainers));
        return CreateOk();
    }

    void ClearData() override {
        _controllerRegistry.ClearWarnings();

        for (auto& queuedMessageCount : _queuedMessageCountByController) {
            queuedMessageCount = 0;
        }

        _queuedMessageContainers.Clear();
    }

    [[nodiscard]] Result Transmit(const TMessage& message) override {
        ControllerStatePtr<TBus> controllerState{};
        CheckResult(_controllerRegistry.FindController(message.controllerId, controllerState));
        CheckResult(CheckTransmitCapacity(*controllerState));

        TMessageContainer messageContainer{};
        message.WriteTo(messageContainer);
        if (!_queuedMessageContainers.TryPushBack(std::move(messageContainer))) {
            LogError("Message buffer is full.");
            return CreateError();
        }

        ++_queuedMessageCountByController[controllerState->controllerSlot];
        return CreateOk();
    }

    [[nodiscard]] Result Transmit(const TMessageContainer& messageContainer) override {
        ControllerStatePtr<TBus> controllerState{};
        CheckResult(_controllerRegistry.FindController(messageContainer.controllerId, controllerState));
        CheckResult(CheckTransmitCapacity(*controllerState));

        if (!_queuedMessageContainers.TryPushBack(messageContainer)) {
            LogError("Message buffer is full.");
            return CreateError();
        }

        ++_queuedMessageCountByController[controllerState->controllerSlot];
        return CreateOk();
    }

    // The plain message view points directly into the queue storage. The caller
    // must copy the payload before the queue slot is overwritten by a later push.
    [[nodiscard]] Result Receive(TMessage& message) override {
        TMessageContainer* messageContainer = _queuedMessageContainers.TryPeekFront();
        if (messageContainer == nullptr) {
            return CreateEmpty();
        }

        messageContainer->WriteTo(message);
        _queuedMessageContainers.RemoveFront();

        ControllerStatePtr<TBus> controllerState{};
        CheckResult(_controllerRegistry.FindController(messageContainer->controllerId, controllerState));
        --_queuedMessageCountByController[controllerState->controllerSlot];
        return CreateOk();
    }

    [[nodiscard]] Result Receive(TMessageContainer& messageContainer) override {
        if (!_queuedMessageContainers.TryPopFront(messageContainer)) {
            return CreateEmpty();
        }

        ControllerStatePtr<TBus> controllerState{};
        CheckResult(_controllerRegistry.FindController(messageContainer.controllerId, controllerState));
        --_queuedMessageCountByController[controllerState->controllerSlot];
        return CreateOk();
    }

    [[nodiscard]] Result Serialize(ChannelWriter& writer) override {
        size_t queuedMessageCount = _queuedMessageContainers.Size();
        CheckResultWithMessage(_protocol.WriteSize(writer, queuedMessageCount), "Could not write count of messages.");

        TMessageContainer messageContainer{};
        while (_queuedMessageContainers.TryPopFront(messageContainer)) {
            if (IsProtocolTracingEnabled()) {
                LogProtData(format_as(messageContainer));
            }

            CheckResultWithMessage(_protocol.WriteMessage(writer, messageContainer), "Could not serialize message.");
        }

        for (auto& [controllerId, controllerState] : _controllerRegistry.GetControllerStatesById()) {
            _queuedMessageCountByController[controllerState.controllerSlot] = 0;
        }

        return CreateOk();
    }

    [[nodiscard]] Result Deserialize(ChannelReader& reader,
                                     SimulationTime simulationTime,
                                     const BusMessageCallback<TBus>& messageCallback,
                                     const BusMessageContainerCallback<TBus>& messageContainerCallback) override {
        size_t totalCount{};
        CheckResultWithMessage(_protocol.ReadSize(reader, totalCount), "Could not read count of messages.");

        for (size_t i = 0; i < totalCount; i++) {
            TMessageContainer messageContainer{};
            CheckResultWithMessage(_protocol.ReadMessage(reader, messageContainer), "Could not deserialize message.");

            if (IsProtocolTracingEnabled()) {
                LogProtData(format_as(messageContainer));
            }

            ControllerStatePtr<TBus> controllerState{};
            CheckResult(_controllerRegistry.FindController(messageContainer.controllerId, controllerState));

            if (messageContainerCallback) {
                messageContainerCallback(simulationTime, controllerState->controller, messageContainer);
                continue;
            }

            if (messageCallback) {
                TMessage message{};
                messageContainer.WriteTo(message);
                messageCallback(simulationTime, controllerState->controller, message);
                continue;
            }

            if (_queuedMessageCountByController[controllerState->controllerSlot] == controllerState->controller.queueSize) {
                if (!controllerState->receiveWarningSent) {
                    LogWarning("Receive buffer for controller '{}' is full. Messages are dropped.", controllerState->controller.name);
                    controllerState->receiveWarningSent = true;
                }

                continue;
            }

            ++_queuedMessageCountByController[controllerState->controllerSlot];
            if (!_queuedMessageContainers.TryPushBack(std::move(messageContainer))) {
                LogError("Message buffer is full.");
                return CreateError();
            }
        }

        return CreateOk();
    }

private:
    [[nodiscard]] Result CheckTransmitCapacity(ControllerState<TBus>& controllerState) {
        if (_queuedMessageCountByController[controllerState.controllerSlot] == controllerState.controller.queueSize) {
            if (!controllerState.transmitWarningSent) {
                LogWarning("Transmit buffer for controller '{}' is full. Messages are dropped.", controllerState.controller.name);
                controllerState.transmitWarningSent = true;
            }

            return CreateFull();
        }

        return CreateOk();
    }

    IProtocol& _protocol;
    ControllerRegistry<TBus> _controllerRegistry;
    std::vector<uint32_t> _queuedMessageCountByController;
    RingBuffer<TMessageContainer> _queuedMessageContainers;
};

}  // namespace DsVeosCoSim::BusExchangeDetail
