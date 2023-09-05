// Copyright dSPACE GmbH. All rights reserved.

#include "BusBuffer.h"

#include "Logger.h"

namespace DsVeosCoSim {

namespace {

[[nodiscard]] Result CanMessageCheckMaxLength(uint8_t length) {
    if (length > CanMessageMaxLength) {
        LogError("CAN message data exceeds maximum length.");
        return Result::InvalidArgument;
    }

    return Result::Ok;
}

[[nodiscard]] Result LinMessageCheckMaxLength(uint8_t length) {
    if (length > LinMessageMaxLength) {
        LogError("LIN message data exceeds maximum length.");
        return Result::InvalidArgument;
    }

    return Result::Ok;
}

[[nodiscard]] Result EthMessageCheckMaxLength(uint16_t length) {
    if (length > EthMessageMaxLength) {
        LogError("ETH message data exceeds maximum length.");
        return Result::InvalidArgument;
    }

    return Result::Ok;
}

[[nodiscard]] Result SerializeMessage(Channel& channel, const CanMessage& message) {
    CheckResult(channel.Write(message.timestamp));
    CheckResult(channel.Write(message.controllerId));
    CheckResult(channel.Write(message.id));
    CheckResult(channel.Write(message.flags));
    CheckResult(channel.Write(message.length));
    return channel.Write(message.data, message.length);
}

[[nodiscard]] Result DeserializeMessage(Channel& channel, CanMessage& message) {
    CheckResult(channel.Read(message.timestamp));
    CheckResult(channel.Read(message.controllerId));
    CheckResult(channel.Read(message.id));
    CheckResult(channel.Read(message.flags));
    CheckResult(channel.Read(message.length));
    CheckResult(CanMessageCheckMaxLength(message.length));
    return channel.Read(message.data, message.length);
}

[[nodiscard]] Result SerializeMessage(Channel& channel, const LinMessage& message) {
    CheckResult(channel.Write(message.timestamp));
    CheckResult(channel.Write(message.controllerId));
    CheckResult(channel.Write(message.id));
    CheckResult(channel.Write(message.flags));
    CheckResult(channel.Write(message.length));
    return channel.Write(message.data, message.length);
}

[[nodiscard]] Result DeserializeMessage(Channel& channel, LinMessage& message) {
    CheckResult(channel.Read(message.timestamp));
    CheckResult(channel.Read(message.controllerId));
    CheckResult(channel.Read(message.id));
    CheckResult(channel.Read(message.flags));
    CheckResult(channel.Read(message.length));
    CheckResult(LinMessageCheckMaxLength(message.length));
    return channel.Read(message.data, message.length);
}

[[nodiscard]] Result SerializeMessage(Channel& channel, const EthMessage& message) {
    CheckResult(channel.Write(message.timestamp));
    CheckResult(channel.Write(message.controllerId));
    CheckResult(channel.Write(message.flags));
    CheckResult(channel.Write(message.length));
    return channel.Write(message.data, message.length);
}

[[nodiscard]] Result DeserializeMessage(Channel& channel, EthMessage& message) {
    CheckResult(channel.Read(message.timestamp));
    CheckResult(channel.Read(message.controllerId));
    CheckResult(channel.Read(message.flags));
    CheckResult(channel.Read(message.length));
    CheckResult(EthMessageCheckMaxLength(message.length));
    return channel.Read(message.data, message.length);
}

}  // namespace

Result BusBuffer::Initialize(const std::vector<CanController>& canControllers,
                             const std::vector<EthController>& ethControllers,
                             const std::vector<LinController>& linControllers) {
    ClearData();

    CheckResult(Initialize(canControllers));
    CheckResult(Initialize(ethControllers));
    CheckResult(Initialize(linControllers));

    return Result::Ok;
}

Result BusBuffer::Initialize(const std::vector<CanController>& controllers) {
    _canControllers.clear();
    _canControllers.reserve(controllers.size());

    size_t totalQueueSize = 0;
    for (const auto& controller : controllers) {
        const auto search = _canControllers.find(controller.id);
        if (search != _canControllers.end()) {
            LogError("Duplicated CAN controller id " + std::to_string(controller.id) + ".");
            return Result::Error;
        }

        ControllerExtension<CanController> controllerExtension{};
        controllerExtension.info = controller;
        _canControllers[controller.id] = controllerExtension;
        totalQueueSize += controller.queueSize;
    }

    _canReceiveBuffer.Clear();
    _canReceiveBuffer.Resize(totalQueueSize);
    _canTransmitBuffer.Clear();
    _canTransmitBuffer.Resize(totalQueueSize);

    return Result::Ok;
}

Result BusBuffer::Initialize(const std::vector<EthController>& controllers) {
    _ethControllers.clear();
    _ethControllers.reserve(controllers.size());

    size_t totalQueueSize = 0;
    for (const auto& controller : controllers) {
        const auto search = _ethControllers.find(controller.id);
        if (search != _ethControllers.end()) {
            LogError("Duplicated ethernet controller id " + std::to_string(controller.id) + ".");
            return Result::Error;
        }

        ControllerExtension<EthController> controllerExtension{};
        controllerExtension.info = controller;
        _ethControllers[controller.id] = controllerExtension;
        totalQueueSize += controller.queueSize;
    }

    _ethReceiveBuffer.Clear();
    _ethReceiveBuffer.Resize(totalQueueSize);
    _ethTransmitBuffer.Clear();
    _ethTransmitBuffer.Resize(totalQueueSize);

    return Result::Ok;
}

Result BusBuffer::Initialize(const std::vector<LinController>& controllers) {
    _linControllers.clear();
    _linControllers.reserve(controllers.size());

    size_t totalQueueSize = 0;
    for (const auto& controller : controllers) {
        const auto search = _linControllers.find(controller.id);
        if (search != _linControllers.end()) {
            LogError("Duplicated LIN controller id " + std::to_string(controller.id) + ".");
            return Result::Error;
        }

        ControllerExtension<LinController> controllerExtension{};
        controllerExtension.info = controller;
        _linControllers[controller.id] = controllerExtension;
        totalQueueSize += controller.queueSize;
    }

    _linReceiveBuffer.Clear();
    _linReceiveBuffer.Resize(totalQueueSize);
    _linTransmitBuffer.Clear();
    _linTransmitBuffer.Resize(totalQueueSize);

    return Result::Ok;
}

void BusBuffer::ClearData() {
    for (auto& [controllerId, dataPerController] : _canControllers) {
        dataPerController.ClearData();
    }

    for (auto& [controllerId, dataPerController] : _ethControllers) {
        dataPerController.ClearData();
    }

    for (auto& [controllerId, dataPerController] : _linControllers) {
        dataPerController.ClearData();
    }

    _canReceiveBuffer.ClearData();
    _ethReceiveBuffer.ClearData();
    _linReceiveBuffer.ClearData();
    _canTransmitBuffer.ClearData();
    _ethTransmitBuffer.ClearData();
    _linTransmitBuffer.ClearData();
}

Result BusBuffer::Transmit(const CanMessage& message) {
    CheckResult(CanMessageCheckMaxLength(message.length));

    ControllerExtension<CanController>* controller{};
    CheckResult(FindController(message.controllerId, &controller));

    if (controller->transmitCount == controller->info.queueSize) {
        if (!controller->transmitWarningSent) {
            LogWarning("Queue for CAN controller '" + std::string(controller->info.name) + "' is full. Messages are dropped.");
            controller->transmitWarningSent = true;
        }

        return Result::Full;
    }

    _canTransmitBuffer.Push(message);
    controller->transmitCount++;

    return Result::Ok;
}

Result BusBuffer::Receive(CanMessage& message) {
    if (_canReceiveBuffer.IsEmpty()) {
        return Result::Empty;
    }

    message = _canReceiveBuffer.Pop();
    ControllerExtension<CanController>* controller;
    CheckResult(FindController(message.controllerId, &controller));
    controller->receiveCount--;

    return Result::Ok;
}

Result BusBuffer::AddMessageToReceiveBuffer(ControllerExtension<CanController>& controller, const CanMessage& message) {
    if (controller.receiveCount == controller.info.queueSize) {
        if (!controller.receiveWarningSent) {
            LogWarning("Receive buffer for CAN controller '" + std::string(controller.info.name) + "' is full.");
            controller.receiveWarningSent = true;
        }

        return Result::Ok;
    }

    controller.receiveCount++;
    _canReceiveBuffer.Push(message);

    return Result::Ok;
}

Result BusBuffer::DeserializeCanMessages(Channel& channel, SimulationTime simulationTime, const Callbacks& callbacks) {
    uint32_t totalCanCount = 0;
    CheckResult(channel.Read(totalCanCount));

    for (uint32_t i = 0; i < totalCanCount; i++) {
        CanMessage message{};
        CheckResult(DeserializeMessage(channel, message));

        ControllerExtension<CanController>* controller{};
        CheckResult(FindController(message.controllerId, &controller));

        if (callbacks.canMessageReceivedCallback) {
            callbacks.canMessageReceivedCallback(simulationTime, controller->info, message);
            continue;
        }

        CheckResult(AddMessageToReceiveBuffer(*controller, message));
    }

    return Result::Ok;
}

Result BusBuffer::SerializeCanMessages(Channel& channel) {
    const auto count = static_cast<uint32_t>(_canTransmitBuffer.Size());
    CheckResult(channel.Write(count));
    for (uint32_t i = 0; i < count; i++) {
        CheckResult(SerializeMessage(channel, _canTransmitBuffer.Pop()));
    }

    for (auto& [id, controller] : _canControllers) {
        controller.transmitCount = 0;
    }

    return Result::Ok;
}

Result BusBuffer::Transmit(const EthMessage& message) {
    CheckResult(EthMessageCheckMaxLength(message.length));
    ControllerExtension<EthController>* controller{};
    CheckResult(FindController(message.controllerId, &controller));

    if (controller->transmitCount == controller->info.queueSize) {
        if (!controller->transmitWarningSent) {
            LogWarning("Queue for ETH controller '" + std::string(controller->info.name) + "' is full. Messages are dropped.");
            controller->transmitWarningSent = true;
        }

        return Result::Full;
    }

    _ethTransmitBuffer.Push(message);
    controller->transmitCount++;

    return Result::Ok;
}

Result BusBuffer::Receive(EthMessage& message) {
    if (_ethReceiveBuffer.IsEmpty()) {
        return Result::Empty;
    }

    message = _ethReceiveBuffer.Pop();
    ControllerExtension<EthController>* controller{};
    CheckResult(FindController(message.controllerId, &controller));
    controller->receiveCount--;

    return Result::Ok;
}

Result BusBuffer::AddMessageToReceiveBuffer(ControllerExtension<EthController>& controller, const EthMessage& message) {
    if (controller.receiveCount == controller.info.queueSize) {
        if (!controller.receiveWarningSent) {
            LogWarning("Receive buffer for ETH controller '" + std::string(controller.info.name) + "' is full.");
            controller.receiveWarningSent = true;
        }

        return Result::Ok;
    }

    controller.receiveCount++;
    _ethReceiveBuffer.Push(message);

    return Result::Ok;
}

Result BusBuffer::DeserializeEthMessages(Channel& channel, SimulationTime simulationTime, const Callbacks& callbacks) {
    uint32_t totalEthCount = 0;
    CheckResult(channel.Read(totalEthCount));

    for (uint32_t i = 0; i < totalEthCount; i++) {
        EthMessage message{};
        CheckResult(DeserializeMessage(channel, message));

        ControllerExtension<EthController>* controller{};
        CheckResult(FindController(message.controllerId, &controller));

        if (callbacks.ethMessageReceivedCallback) {
            callbacks.ethMessageReceivedCallback(simulationTime, controller->info, message);
            continue;
        }

        CheckResult(AddMessageToReceiveBuffer(*controller, message));
    }

    return Result::Ok;
}

Result BusBuffer::SerializeEthMessages(Channel& channel) {
    const auto count = static_cast<uint32_t>(_ethTransmitBuffer.Size());
    CheckResult(channel.Write(count));
    for (uint32_t i = 0; i < count; i++) {
        CheckResult(SerializeMessage(channel, _ethTransmitBuffer.Pop()));
    }

    return Result::Ok;
}

Result BusBuffer::Transmit(const LinMessage& message) {
    CheckResult(LinMessageCheckMaxLength(message.length));
    ControllerExtension<LinController>* controller{};
    CheckResult(FindController(message.controllerId, &controller));

    if (controller->transmitCount == controller->info.queueSize) {
        if (!controller->transmitWarningSent) {
            LogWarning("Queue for LIN controller '" + std::string(controller->info.name) + "' is full. Messages are dropped.");
            controller->transmitWarningSent = true;
        }

        return Result::Full;
    }

    _linTransmitBuffer.Push(message);
    controller->transmitCount++;

    return Result::Ok;
}

Result BusBuffer::Receive(LinMessage& message) {
    if (_linReceiveBuffer.IsEmpty()) {
        return Result::Empty;
    }

    message = _linReceiveBuffer.Pop();
    ControllerExtension<LinController>* controller{};
    CheckResult(FindController(message.controllerId, &controller));
    controller->receiveCount--;

    return Result::Ok;
}

Result BusBuffer::AddMessageToReceiveBuffer(ControllerExtension<LinController>& controller, const LinMessage& message) {
    if (controller.receiveCount == controller.info.queueSize) {
        if (!controller.receiveWarningSent) {
            LogWarning("Receive buffer for LIN controller '" + std::string(controller.info.name) + "' is full.");
            controller.receiveWarningSent = true;
        }

        return Result::Ok;
    }

    controller.receiveCount++;
    _linReceiveBuffer.Push(message);

    return Result::Ok;
}

Result BusBuffer::DeserializeLinMessages(Channel& channel, SimulationTime simulationTime, const Callbacks& callbacks) {
    uint32_t totalLinCount = 0;
    CheckResult(channel.Read(totalLinCount));

    for (uint32_t i = 0; i < totalLinCount; i++) {
        LinMessage message{};
        CheckResult(DeserializeMessage(channel, message));

        ControllerExtension<LinController>* controller{};
        CheckResult(FindController(message.controllerId, &controller));

        if (callbacks.linMessageReceivedCallback) {
            callbacks.linMessageReceivedCallback(simulationTime, controller->info, message);
            continue;
        }

        CheckResult(AddMessageToReceiveBuffer(*controller, message));
    }

    return Result::Ok;
}

Result BusBuffer::SerializeLinMessages(Channel& channel) {
    const auto count = static_cast<uint32_t>(_linTransmitBuffer.Size());
    CheckResult(channel.Write(count));
    for (uint32_t i = 0; i < count; i++) {
        CheckResult(SerializeMessage(channel, _linTransmitBuffer.Pop()));
    }

    return Result::Ok;
}

Result BusBuffer::Deserialize(Channel& channel, SimulationTime simulationTime, const Callbacks& callbacks) {
    CheckResult(DeserializeCanMessages(channel, simulationTime, callbacks));
    CheckResult(DeserializeEthMessages(channel, simulationTime, callbacks));
    CheckResult(DeserializeLinMessages(channel, simulationTime, callbacks));

    return Result::Ok;
}

Result BusBuffer::Serialize(Channel& channel) {
    CheckResult(SerializeCanMessages(channel));
    CheckResult(SerializeEthMessages(channel));
    CheckResult(SerializeLinMessages(channel));

    return Result::Ok;
}

Result BusBuffer::FindController(BusControllerId controllerId, ControllerExtension<CanController>** controller) {
    const auto search = _canControllers.find(controllerId);
    if (search != _canControllers.end()) {
        *controller = &search->second;
        return Result::Ok;
    }

    LogError("CAN controller id " + std::to_string(controllerId) + " is unknown.");
    return Result::InvalidArgument;
}

Result BusBuffer::FindController(BusControllerId controllerId, ControllerExtension<EthController>** controller) {
    const auto search = _ethControllers.find(controllerId);
    if (search != _ethControllers.end()) {
        *controller = &search->second;
        return Result::Ok;
    }

    LogError("Ethernet controller id " + std::to_string(controllerId) + " is unknown.");
    return Result::InvalidArgument;
}

Result BusBuffer::FindController(BusControllerId controllerId, ControllerExtension<LinController>** controller) {
    const auto search = _linControllers.find(controllerId);
    if (search != _linControllers.end()) {
        *controller = &search->second;
        return Result::Ok;
    }

    LogError("LIN controller id " + std::to_string(controllerId) + " is unknown.");
    return Result::InvalidArgument;
}

}  // namespace DsVeosCoSim
