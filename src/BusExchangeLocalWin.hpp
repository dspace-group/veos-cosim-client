// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#ifdef _WIN32

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>

#include "BusExchangeCommon.hpp"
#include "Environment.hpp"
#include "OsUtilities.hpp"
#include "Protocol.hpp"
#include "RingBufferView.hpp"

namespace DsVeosCoSim::BusExchangeDetail {

template <typename TBus>
class LocalBusExchangePart final : public IBusExchangePart<TBus> {
public:
    using TMessage = typename TBus::Message;
    using TMessageContainer = typename TBus::MessageContainer;
    using TController = typename TBus::Controller;

    LocalBusExchangePart(IProtocol& protocol, std::string name) : _protocol(protocol), _name(std::move(name)) {
    }

    ~LocalBusExchangePart() noexcept override = default;

    LocalBusExchangePart(const LocalBusExchangePart&) = delete;
    LocalBusExchangePart& operator=(const LocalBusExchangePart&) = delete;

    LocalBusExchangePart(LocalBusExchangePart&&) = delete;
    LocalBusExchangePart& operator=(LocalBusExchangePart&&) = delete;

    [[nodiscard]] Result Initialize(const std::vector<TController>& controllers) override {
        // The shared memory region is split into per-controller counters followed
        // by one shared ring buffer containing the actual message payloads.
        CheckResult(_controllerRegistry.Initialize(controllers));

        size_t combinedQueueCapacity = _controllerRegistry.GetCombinedQueueCapacity();
        size_t sizeOfMessageCountPerController =
            _controllerRegistry.GetControllerStatesById().size() * sizeof(std::atomic<uint32_t>);
        size_t sizeOfMessageQueue = sizeof(RingBufferView<TMessageContainer>) + (combinedQueueCapacity * sizeof(TMessageContainer));

        size_t sizeOfSharedMemory = 0;
        sizeOfSharedMemory += sizeOfMessageCountPerController;
        sizeOfSharedMemory += sizeOfMessageQueue;

        CheckResult(SharedMemory::CreateOrOpen(_name, sizeOfSharedMemory, _sharedMemory));

        auto* pointerToMessageCountPerController = _sharedMemory.GetData();
        auto* pointerToMessageQueue = pointerToMessageCountPerController + sizeOfMessageCountPerController;

        _sharedMessageCountByController = reinterpret_cast<std::atomic<uint32_t>*>(pointerToMessageCountPerController);
        _sharedMessageQueue = reinterpret_cast<RingBufferView<TMessageContainer>*>(pointerToMessageQueue);

        _sharedMessageQueue->Initialize(static_cast<uint32_t>(combinedQueueCapacity));

        ClearData();
        return CreateOk();
    }

    void ClearData() override {
        _controllerRegistry.ClearWarnings();

        _pendingReceiveCount = 0;
        _pendingTransmitNotificationCount = 0;

        for (size_t i = 0; i < _controllerRegistry.GetControllerStatesById().size(); i++) {
            _sharedMessageCountByController[i].store(0, std::memory_order_release);
        }

        if (_sharedMessageQueue) {
            _sharedMessageQueue->Clear();
        }
    }

    [[nodiscard]] Result Transmit(const TMessage& message) override {
        ControllerStatePtr<TBus> controllerState{};
        CheckResult(_controllerRegistry.FindController(message.controllerId, controllerState));
        std::atomic<uint32_t>& sharedQueuedMessageCount = _sharedMessageCountByController[controllerState->controllerSlot];
        CheckResult(CheckControllerQueueCapacity(sharedQueuedMessageCount, *controllerState));

        TMessageContainer messageContainer{};
        message.WriteTo(messageContainer);
        _sharedMessageQueue->PushBack(messageContainer);

        sharedQueuedMessageCount.fetch_add(1);
        _pendingTransmitNotificationCount++;
        return CreateOk();
    }

    [[nodiscard]] Result Transmit(const TMessageContainer& messageContainer) override {
        ControllerStatePtr<TBus> controllerState{};
        CheckResult(_controllerRegistry.FindController(messageContainer.controllerId, controllerState));
        std::atomic<uint32_t>& sharedQueuedMessageCount = _sharedMessageCountByController[controllerState->controllerSlot];
        CheckResult(CheckControllerQueueCapacity(sharedQueuedMessageCount, *controllerState));

        _sharedMessageQueue->PushBack(messageContainer);

        sharedQueuedMessageCount.fetch_add(1);
        _pendingTransmitNotificationCount++;
        return CreateOk();
    }

    [[nodiscard]] Result Receive(TMessage& message) override {
        if (_pendingReceiveCount == 0) {
            return CreateEmpty();
        }

        TMessageContainer& messageContainer = _sharedMessageQueue->PopFront();
        messageContainer.WriteTo(message);

        ControllerStatePtr<TBus> controllerState{};
        CheckResult(_controllerRegistry.FindController(message.controllerId, controllerState));
        std::atomic<uint32_t>& sharedQueuedMessageCount = _sharedMessageCountByController[controllerState->controllerSlot];
        sharedQueuedMessageCount.fetch_sub(1);
        _pendingReceiveCount--;
        return CreateOk();
    }

    [[nodiscard]] Result Receive(TMessageContainer& messageContainer) override {
        if (_pendingReceiveCount == 0) {
            return CreateEmpty();
        }

        messageContainer = _sharedMessageQueue->PopFront();

        ControllerStatePtr<TBus> controllerState{};
        CheckResult(_controllerRegistry.FindController(messageContainer.controllerId, controllerState));
        std::atomic<uint32_t>& sharedQueuedMessageCount = _sharedMessageCountByController[controllerState->controllerSlot];
        sharedQueuedMessageCount.fetch_sub(1);
        _pendingReceiveCount--;
        return CreateOk();
    }

    [[nodiscard]] Result Serialize(ChannelWriter& writer) override {
        CheckResultWithMessage(
            _protocol.WriteSize(writer, _pendingTransmitNotificationCount),
            "Could not write transmit count.");
        _pendingTransmitNotificationCount = 0;
        return CreateOk();
    }

    // For local transport the messages already live in shared memory. The channel
    // only transfers how many new queue entries became visible since the last read.
    [[nodiscard]] Result Deserialize(ChannelReader& reader,
                                     SimulationTime simulationTime,
                                     const BusMessageCallback<TBus>& messageCallback,
                                     const BusMessageContainerCallback<TBus>& messageContainerCallback) override {
        size_t receivedNotificationCount{};
        CheckResultWithMessage(_protocol.ReadSize(reader, receivedNotificationCount), "Could not read receive count.");
        _pendingReceiveCount += receivedNotificationCount;

        if (!messageCallback && !messageContainerCallback) {
            return CreateOk();
        }

        while (_pendingReceiveCount > 0) {
            TMessageContainer& messageContainer = _sharedMessageQueue->PopFront();

            if (IsProtocolTracingEnabled()) {
                LogProtData(format_as(messageContainer));
            }

            ControllerStatePtr<TBus> controllerState{};
            CheckResult(_controllerRegistry.FindController(messageContainer.controllerId, controllerState));
            std::atomic<uint32_t>& sharedQueuedMessageCount = _sharedMessageCountByController[controllerState->controllerSlot];
            sharedQueuedMessageCount.fetch_sub(1);
            _pendingReceiveCount--;

            if (messageContainerCallback) {
                messageContainerCallback(simulationTime, controllerState->controller, messageContainer);
                continue;
            }

            if (messageCallback) {
                TMessage message{};
                messageContainer.WriteTo(message);
                messageCallback(simulationTime, controllerState->controller, message);
            }
        }

        return CreateOk();
    }

private:
    [[nodiscard]] static Result CheckControllerQueueCapacity(const std::atomic<uint32_t>& sharedQueuedMessageCount,
                                                            ControllerState<TBus>& controllerState) {
        if (sharedQueuedMessageCount.load(std::memory_order_acquire) == controllerState.controller.queueSize) {
            if (!controllerState.transmitWarningSent) {
                LogWarning("Transmit buffer for controller '{}' is full. Messages are dropped.", controllerState.controller.name);
                controllerState.transmitWarningSent = true;
            }

            return CreateFull();
        }

        return CreateOk();
    }

    IProtocol& _protocol;
    std::string _name;
    ControllerRegistry<TBus> _controllerRegistry;
    size_t _pendingReceiveCount{};
    size_t _pendingTransmitNotificationCount{};
    std::atomic<uint32_t>* _sharedMessageCountByController{};
    RingBufferView<TMessageContainer>* _sharedMessageQueue{};
    SharedMemory _sharedMemory;
};

}  // namespace DsVeosCoSim::BusExchangeDetail

#endif