// Copyright dSPACE GmbH. All rights reserved.

#include "BusBuffer.h"

#include <cstring>

#include "Logger.h"

namespace DsVeosCoSim {

namespace {

[[nodiscard]] Result CanMessageCheckMaxLength(uint32_t length) {
    if (length > CanMessageMaxLength) {
        LogError("CAN message data exceeds maximum length.");
        return Result::InvalidArgument;
    }

    return Result::Ok;
}

[[nodiscard]] Result EthMessageCheckMaxLength(uint32_t length) {
    if (length > EthMessageMaxLength) {
        LogError("ETH message data exceeds maximum length.");
        return Result::InvalidArgument;
    }

    return Result::Ok;
}

[[nodiscard]] Result LinMessageCheckMaxLength(uint32_t length) {
    if (length > LinMessageMaxLength) {
        LogError("LIN message data exceeds maximum length.");
        return Result::InvalidArgument;
    }

    return Result::Ok;
}

[[nodiscard]] Result SerializeMessage(Channel& channel, const CanMessageContainer& container) {
    CheckResult(channel.Write(container.message.timestamp));
    CheckResult(channel.Write(container.message.controllerId));
    CheckResult(channel.Write(container.message.id));
    CheckResult(channel.Write(container.message.flags));
    CheckResult(channel.Write(container.message.length));
    return channel.Write(container.data.data(), container.message.length);
}

[[nodiscard]] Result DeserializeMessage(Channel& channel, CanMessageContainer& container) {
    CheckResult(channel.Read(container.message.timestamp));
    CheckResult(channel.Read(container.message.controllerId));
    CheckResult(channel.Read(container.message.id));
    CheckResult(channel.Read(container.message.flags));
    CheckResult(channel.Read(container.message.length));
    CheckResult(CanMessageCheckMaxLength(container.message.length));
    container.data.resize(container.message.length);
    container.message.data = container.data.data();
    return channel.Read(container.data.data(), container.message.length);
}

[[nodiscard]] Result SerializeMessage(Channel& channel, const EthMessageContainer& container) {
    CheckResult(channel.Write(container.message.timestamp));
    CheckResult(channel.Write(container.message.controllerId));
    CheckResult(channel.Write(container.message.flags));
    CheckResult(channel.Write(container.message.length));
    return channel.Write(container.data.data(), container.message.length);
}

[[nodiscard]] Result DeserializeMessage(Channel& channel, EthMessageContainer& container) {
    CheckResult(channel.Read(container.message.timestamp));
    CheckResult(channel.Read(container.message.controllerId));
    CheckResult(channel.Read(container.message.flags));
    CheckResult(channel.Read(container.message.length));
    CheckResult(EthMessageCheckMaxLength(container.message.length));
    container.data.resize(container.message.length);
    container.message.data = container.data.data();
    return channel.Read(container.data.data(), container.message.length);
}

[[nodiscard]] Result SerializeMessage(Channel& channel, const LinMessageContainer& container) {
    CheckResult(channel.Write(container.message.timestamp));
    CheckResult(channel.Write(container.message.controllerId));
    CheckResult(channel.Write(container.message.id));
    CheckResult(channel.Write(container.message.flags));
    CheckResult(channel.Write(container.message.length));
    return channel.Write(container.data.data(), container.message.length);
}

[[nodiscard]] Result DeserializeMessage(Channel& channel, LinMessageContainer& container) {
    CheckResult(channel.Read(container.message.timestamp));
    CheckResult(channel.Read(container.message.controllerId));
    CheckResult(channel.Read(container.message.id));
    CheckResult(channel.Read(container.message.flags));
    CheckResult(channel.Read(container.message.length));
    CheckResult(LinMessageCheckMaxLength(container.message.length));
    container.data.resize(container.message.length);
    container.message.data = container.data.data();
    return channel.Read(container.data.data(), container.message.length);
}

}  // namespace

Result BusBuffer::Initialize(const std::vector<CanControllerContainer>& canControllers,
                             const std::vector<EthControllerContainer>& ethControllers,
                             const std::vector<LinControllerContainer>& linControllers) {
    ClearData();

    CheckResult(Initialize(canControllers));
    CheckResult(Initialize(ethControllers));
    return Initialize(linControllers);
}

Result BusBuffer::Initialize(const std::vector<CanControllerContainer>& containers) {
    _canControllers.clear();
    _canControllers.reserve(containers.size());

    size_t totalQueueSize = 0;
    for (const auto& container : containers) {
        const auto search = _canControllers.find(container.controller.id);
        if (search != _canControllers.end()) {
            LogError("Duplicated CAN controller id " + std::to_string(container.controller.id) + ".");
            return Result::Error;
        }

        ControllerExtension<CanControllerContainer> controllerExtension{};
        controllerExtension.info = container;
        _canControllers[container.controller.id] = controllerExtension;
        totalQueueSize += container.controller.queueSize;
    }

    _canReceiveBuffer.Clear();
    _canReceiveBuffer.Resize(totalQueueSize);
    _canTransmitBuffer.Clear();
    _canTransmitBuffer.Resize(totalQueueSize);
    return Result::Ok;
}

Result BusBuffer::Initialize(const std::vector<EthControllerContainer>& containers) {
    _ethControllers.clear();
    _ethControllers.reserve(containers.size());

    size_t totalQueueSize = 0;
    for (const auto& container : containers) {
        const auto search = _ethControllers.find(container.controller.id);
        if (search != _ethControllers.end()) {
            LogError("Duplicated ethernet controller id " + std::to_string(container.controller.id) + ".");
            return Result::Error;
        }

        ControllerExtension<EthControllerContainer> controllerExtension{};
        controllerExtension.info = container;
        _ethControllers[container.controller.id] = controllerExtension;
        totalQueueSize += container.controller.queueSize;
    }

    _ethReceiveBuffer.Clear();
    _ethReceiveBuffer.Resize(totalQueueSize);
    _ethTransmitBuffer.Clear();
    _ethTransmitBuffer.Resize(totalQueueSize);
    return Result::Ok;
}

Result BusBuffer::Initialize(const std::vector<LinControllerContainer>& containers) {
    _linControllers.clear();
    _linControllers.reserve(containers.size());

    size_t totalQueueSize = 0;
    for (const auto& container : containers) {
        const auto search = _linControllers.find(container.controller.id);
        if (search != _linControllers.end()) {
            LogError("Duplicated LIN controller id " + std::to_string(container.controller.id) + ".");
            return Result::Error;
        }

        ControllerExtension<LinControllerContainer> controllerExtension{};
        controllerExtension.info = container;
        _linControllers[container.controller.id] = controllerExtension;
        totalQueueSize += container.controller.queueSize;
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

    ControllerExtension<CanControllerContainer>* controller{};
    CheckResult(FindController(message.controllerId, &controller));

    if (controller->transmitCount == controller->info.controller.queueSize) {
        if (!controller->transmitWarningSent) {
            LogWarning("Queue for CAN controller '" + std::string(controller->info.name) + "' is full. Messages are dropped.");
            controller->transmitWarningSent = true;
        }

        return Result::Full;
    }

    CanMessageContainer container{};
    container.message = message;
    container.data.resize(message.length);
    std::memcpy(container.data.data(), message.data, message.length);

    CanMessageContainer& tmp = _canTransmitBuffer.Push(container);
    tmp.message.data = tmp.data.data();
    controller->transmitCount++;
    return Result::Ok;
}

Result BusBuffer::Receive(CanMessage& message) {
    if (_canReceiveBuffer.IsEmpty()) {
        return Result::Empty;
    }

    message = _canReceiveBuffer.Pop().message;
    ControllerExtension<CanControllerContainer>* controller;
    CheckResult(FindController(message.controllerId, &controller));
    controller->receiveCount--;
    return Result::Ok;
}

Result BusBuffer::AddMessageToReceiveBuffer(ControllerExtension<CanControllerContainer>& extension, const CanMessageContainer& container) {
    if (extension.receiveCount == extension.info.controller.queueSize) {
        if (!extension.receiveWarningSent) {
            LogWarning("Receive buffer for CAN controller '" + extension.info.name + "' is full.");
            extension.receiveWarningSent = true;
        }

        return Result::Ok;
    }

    extension.receiveCount++;
    CanMessageContainer& tmp = _canReceiveBuffer.Push(container);
    tmp.message.data = tmp.data.data();
    return Result::Ok;
}

Result BusBuffer::DeserializeCanMessages(Channel& channel, SimulationTime simulationTime, const Callbacks& callbacks) {
    uint32_t totalCanCount = 0;
    CheckResult(channel.Read(totalCanCount));

    for (uint32_t i = 0; i < totalCanCount; i++) {
        CanMessageContainer container{};
        CheckResult(DeserializeMessage(channel, container));

        ControllerExtension<CanControllerContainer>* controller{};
        CheckResult(FindController(container.message.controllerId, &controller));

        if (callbacks.canMessageReceivedCallback) {
            callbacks.canMessageReceivedCallback(simulationTime, controller->info.controller, container.message);
            continue;
        }

        CheckResult(AddMessageToReceiveBuffer(*controller, container));
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
    ControllerExtension<EthControllerContainer>* extension{};
    CheckResult(FindController(message.controllerId, &extension));

    if (extension->transmitCount == extension->info.controller.queueSize) {
        if (!extension->transmitWarningSent) {
            LogWarning("Queue for ETH controller '" + extension->info.name + "' is full. Messages are dropped.");
            extension->transmitWarningSent = true;
        }

        return Result::Full;
    }

    EthMessageContainer container{};
    container.message = message;
    container.data.resize(message.length);
    std::memcpy(container.data.data(), message.data, message.length);

    EthMessageContainer& tmp = _ethTransmitBuffer.Push(container);
    tmp.message.data = tmp.data.data();
    extension->transmitCount++;
    return Result::Ok;
}

Result BusBuffer::Receive(EthMessage& message) {
    if (_ethReceiveBuffer.IsEmpty()) {
        return Result::Empty;
    }

    message = _ethReceiveBuffer.Pop().message;
    ControllerExtension<EthControllerContainer>* extension{};
    CheckResult(FindController(message.controllerId, &extension));
    extension->receiveCount--;
    return Result::Ok;
}

Result BusBuffer::AddMessageToReceiveBuffer(ControllerExtension<EthControllerContainer>& extension, const EthMessageContainer& container) {
    if (extension.receiveCount == extension.info.controller.queueSize) {
        if (!extension.receiveWarningSent) {
            LogWarning("Receive buffer for ETH controller '" + extension.info.name + "' is full.");
            extension.receiveWarningSent = true;
        }

        return Result::Ok;
    }

    extension.receiveCount++;
    EthMessageContainer& tmp = _ethReceiveBuffer.Push(container);
    tmp.message.data = tmp.data.data();
    return Result::Ok;
}

Result BusBuffer::DeserializeEthMessages(Channel& channel, SimulationTime simulationTime, const Callbacks& callbacks) {
    uint32_t totalEthCount = 0;
    CheckResult(channel.Read(totalEthCount));

    for (uint32_t i = 0; i < totalEthCount; i++) {
        EthMessageContainer container{};
        CheckResult(DeserializeMessage(channel, container));

        ControllerExtension<EthControllerContainer>* extension{};
        CheckResult(FindController(container.message.controllerId, &extension));

        if (callbacks.ethMessageReceivedCallback) {
            callbacks.ethMessageReceivedCallback(simulationTime, extension->info.controller, container.message);
            continue;
        }

        CheckResult(AddMessageToReceiveBuffer(*extension, container));
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
    ControllerExtension<LinControllerContainer>* extension{};
    CheckResult(FindController(message.controllerId, &extension));

    if (extension->transmitCount == extension->info.controller.queueSize) {
        if (!extension->transmitWarningSent) {
            LogWarning("Queue for LIN controller '" + extension->info.name + "' is full. Messages are dropped.");
            extension->transmitWarningSent = true;
        }

        return Result::Full;
    }

    LinMessageContainer container{};
    container.message = message;
    container.data.resize(message.length);
    std::memcpy(container.data.data(), message.data, message.length);

    LinMessageContainer& tmp = _linTransmitBuffer.Push(container);
    tmp.message.data = tmp.data.data();
    extension->transmitCount++;
    return Result::Ok;
}

Result BusBuffer::Receive(LinMessage& message) {
    if (_linReceiveBuffer.IsEmpty()) {
        return Result::Empty;
    }

    message = _linReceiveBuffer.Pop().message;
    ControllerExtension<LinControllerContainer>* extension{};
    CheckResult(FindController(message.controllerId, &extension));
    extension->receiveCount--;
    return Result::Ok;
}

Result BusBuffer::AddMessageToReceiveBuffer(ControllerExtension<LinControllerContainer>& extension, const LinMessageContainer& container) {
    if (extension.receiveCount == extension.info.controller.queueSize) {
        if (!extension.receiveWarningSent) {
            LogWarning("Receive buffer for LIN controller '" + extension.info.name + "' is full.");
            extension.receiveWarningSent = true;
        }

        return Result::Ok;
    }

    extension.receiveCount++;
    LinMessageContainer& tmp = _linReceiveBuffer.Push(container);
    tmp.message.data = tmp.data.data();
    return Result::Ok;
}

Result BusBuffer::DeserializeLinMessages(Channel& channel, SimulationTime simulationTime, const Callbacks& callbacks) {
    uint32_t totalLinCount = 0;
    CheckResult(channel.Read(totalLinCount));

    for (uint32_t i = 0; i < totalLinCount; i++) {
        LinMessageContainer container{};
        CheckResult(DeserializeMessage(channel, container));

        ControllerExtension<LinControllerContainer>* extension{};
        CheckResult(FindController(container.message.controllerId, &extension));

        if (callbacks.linMessageReceivedCallback) {
            callbacks.linMessageReceivedCallback(simulationTime, extension->info.controller, container.message);
            continue;
        }

        CheckResult(AddMessageToReceiveBuffer(*extension, container));
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
    return DeserializeLinMessages(channel, simulationTime, callbacks);
}

Result BusBuffer::Serialize(Channel& channel) {
    CheckResult(SerializeCanMessages(channel));
    CheckResult(SerializeEthMessages(channel));
    return SerializeLinMessages(channel);
}

Result BusBuffer::FindController(BusControllerId controllerId, ControllerExtension<CanControllerContainer>** extension) {
    const auto search = _canControllers.find(controllerId);
    if (search != _canControllers.end()) {
        *extension = &search->second;
        return Result::Ok;
    }

    LogError("CAN controller id " + std::to_string(controllerId) + " is unknown.");
    return Result::InvalidArgument;
}

Result BusBuffer::FindController(BusControllerId controllerId, ControllerExtension<EthControllerContainer>** extension) {
    const auto search = _ethControllers.find(controllerId);
    if (search != _ethControllers.end()) {
        *extension = &search->second;
        return Result::Ok;
    }

    LogError("Ethernet controller id " + std::to_string(controllerId) + " is unknown.");
    return Result::InvalidArgument;
}

Result BusBuffer::FindController(BusControllerId controllerId, ControllerExtension<LinControllerContainer>** extension) {
    const auto search = _linControllers.find(controllerId);
    if (search != _linControllers.end()) {
        *extension = &search->second;
        return Result::Ok;
    }

    LogError("LIN controller id " + std::to_string(controllerId) + " is unknown.");
    return Result::InvalidArgument;
}

}  // namespace DsVeosCoSim
