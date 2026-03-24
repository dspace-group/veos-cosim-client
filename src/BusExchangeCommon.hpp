// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <cstddef>  // IWYU pragma: keep
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>

#include "Channel.hpp"
#include "CoSimTypes.hpp"
#include "Logger.hpp"
#include "Result.hpp"

namespace DsVeosCoSim::BusExchangeDetail {

struct CanBus {
    using Message = CanMessage;
    using MessageContainer = CanMessageContainer;
    using Controller = CanController;
#ifdef _WIN32
    static constexpr char ShmNamePart[] = ".Can.";
#endif
    static constexpr char DisplayName[] = "CAN";
    static constexpr uint32_t MessageMaxLength = CanMessageMaxLength;
};

struct EthBus {
    using Message = EthMessage;
    using MessageContainer = EthMessageContainer;
    using Controller = EthController;
#ifdef _WIN32
    static constexpr char ShmNamePart[] = ".Eth.";
#endif
    static constexpr char DisplayName[] = "Ethernet";
    static constexpr uint32_t MessageMaxLength = EthMessageMaxLength;
};

struct LinBus {
    using Message = LinMessage;
    using MessageContainer = LinMessageContainer;
    using Controller = LinController;
#ifdef _WIN32
    static constexpr char ShmNamePart[] = ".Lin.";
#endif
    static constexpr char DisplayName[] = "LIN";
    static constexpr uint32_t MessageMaxLength = LinMessageMaxLength;
};

struct FrBus {
    using Message = FrMessage;
    using MessageContainer = FrMessageContainer;
    using Controller = FrController;
#ifdef _WIN32
    static constexpr char ShmNamePart[] = ".FlexRay.";
#endif
    static constexpr char DisplayName[] = "FlexRay";
    static constexpr uint32_t MessageMaxLength = FrMessageMaxLength;
};

template <typename TBus>
using BusMessageCallback = std::function<void(SimulationTime, const typename TBus::Controller&, const typename TBus::Message&)>;

template <typename TBus>
using BusMessageContainerCallback = std::function<void(SimulationTime, const typename TBus::Controller&, const typename TBus::MessageContainer&)>;

template <typename TBus>
struct ControllerState {
    typename TBus::Controller controller{};
    size_t controllerSlot{};
    bool receiveWarningSent{};
    bool transmitWarningSent{};

    void ClearWarnings() {
        receiveWarningSent = false;
        transmitWarningSent = false;
    }
};

template <typename TBus>
using ControllerStatePtr = ControllerState<TBus>*;

// The registry keeps a stable vector-style index for each controller while still
// supporting lookup by controller id.
template <typename TBus>
class ControllerRegistry final {
public:
    using TController = typename TBus::Controller;

    ControllerRegistry() = default;
    ~ControllerRegistry() noexcept = default;

    ControllerRegistry(const ControllerRegistry&) = delete;
    ControllerRegistry& operator=(const ControllerRegistry&) = delete;

    ControllerRegistry(ControllerRegistry&&) noexcept = default;
    ControllerRegistry& operator=(ControllerRegistry&&) noexcept = default;

    [[nodiscard]] static Result Create(const std::vector<TController>& controllers, ControllerRegistry& controllerRegistry) {
        size_t combinedQueueCapacity = 0;
        std::unordered_map<BusControllerId, ControllerState<TBus>> controllerStatesById;
        controllerStatesById.reserve(controllers.size());

        size_t nextControllerSlot = 0;
        for (const auto& controller : controllers) {
            auto search = controllerStatesById.find(controller.id);
            if (search != controllerStatesById.end()) {
                LogError("Duplicated controller id {}.", controller.id);
                return CreateError();
            }

            ControllerState<TBus> controllerState{};
            controllerState.controller = controller;
            controllerState.controllerSlot = nextControllerSlot++;
            controllerStatesById.emplace(controller.id, std::move(controllerState));
            combinedQueueCapacity += controller.queueSize;
        }

        controllerRegistry = ControllerRegistry(combinedQueueCapacity, std::move(controllerStatesById));
        return CreateOk();
    }

    void ClearWarnings() {
        for (auto& [controllerId, controllerState] : _controllerStatesById) {
            controllerState.ClearWarnings();
        }
    }

    [[nodiscard]] Result FindController(BusControllerId controllerId, ControllerStatePtr<TBus>& controllerState) {
        auto search = _controllerStatesById.find(controllerId);
        if (search != _controllerStatesById.end()) {
            controllerState = &search->second;
            return CreateOk();
        }

        LogError("Controller id {} is unknown.", controllerId);
        return CreateError();
    }

    [[nodiscard]] std::unordered_map<BusControllerId, ControllerState<TBus>>& GetControllerStatesById() {
        return _controllerStatesById;
    }

    [[nodiscard]] size_t GetCombinedQueueCapacity() const {
        return _combinedQueueCapacity;
    }

private:
    ControllerRegistry(size_t combinedQueueCapacity, std::unordered_map<BusControllerId, ControllerState<TBus>> controllerStatesById)
        : _combinedQueueCapacity(combinedQueueCapacity), _controllerStatesById(std::move(controllerStatesById)) {
    }

    size_t _combinedQueueCapacity{};
    std::unordered_map<BusControllerId, ControllerState<TBus>> _controllerStatesById;
};

template <typename TBus>
class IBusExchangePart {
public:
    using TMessage = typename TBus::Message;
    using TMessageContainer = typename TBus::MessageContainer;
    using TController = typename TBus::Controller;

    IBusExchangePart() = default;
    virtual ~IBusExchangePart() noexcept = default;

    IBusExchangePart(const IBusExchangePart&) = delete;
    IBusExchangePart& operator=(const IBusExchangePart&) = delete;

    IBusExchangePart(IBusExchangePart&&) = delete;
    IBusExchangePart& operator=(IBusExchangePart&&) = delete;

    virtual void ClearData() = 0;
    [[nodiscard]] virtual Result Transmit(const TMessage& message) = 0;
    [[nodiscard]] virtual Result Transmit(const TMessageContainer& messageContainer) = 0;
    [[nodiscard]] virtual Result Receive(TMessage& message) = 0;
    [[nodiscard]] virtual Result Receive(TMessageContainer& messageContainer) = 0;
    [[nodiscard]] virtual Result Serialize(ChannelWriter& writer) = 0;
    [[nodiscard]] virtual Result Deserialize(ChannelReader& reader,
                                             SimulationTime simulationTime,
                                             const BusMessageCallback<TBus>& messageCallback,
                                             const BusMessageContainerCallback<TBus>& messageContainerCallback) = 0;
};

}  // namespace DsVeosCoSim::BusExchangeDetail
