// Copyright dSPACE GmbH. All rights reserved.

#include "BusBuffer.h"

#include <cstring>

#include "CoSimTypes.h"
#include "Logger.h"

namespace DsVeosCoSim {

Result CanMessage::SerializeTo(Channel& channel) const {
    CheckResult(channel.Write(timestamp));
    CheckResult(channel.Write(controllerId));
    CheckResult(channel.Write(id));
    CheckResult(channel.Write(flags));
    CheckResult(channel.Write(length));
    return channel.Write(data.data(), length);
}

Result CanMessage::DeserializeFrom(Channel& channel) {
    CheckResult(channel.Read(timestamp));
    CheckResult(channel.Read(controllerId));
    CheckResult(channel.Read(id));
    CheckResult(channel.Read(flags));
    CheckResult(channel.Read(length));
    CheckResult(CheckMaxLength());
    return channel.Read(data.data(), length);
}

void CanMessage::WriteTo(DsVeosCoSim_CanMessage& message) const {
    message.timestamp = timestamp;
    message.controllerId = controllerId;
    message.id = id;
    message.flags = (DsVeosCoSim_CanMessageFlags)flags;
    message.length = length;
    message.data = data.data();
}

Result CanMessage::ReadFrom(const DsVeosCoSim_CanMessage& message) {
    timestamp = message.timestamp;
    controllerId = message.controllerId;
    id = message.id;
    flags = (CanMessageFlags)message.flags;
    length = message.length;
    CheckResult(CheckMaxLength());
    (void)std::memcpy(data.data(), message.data, message.length);
    return Result::Ok;
}

DsVeosCoSim_CanMessage CanMessage::Convert() const {
    DsVeosCoSim_CanMessage message{};
    WriteTo(message);
    return message;
}

Result CanMessage::CheckMaxLength() const {
    if (length > CanMessageMaxLength) {
        LogError("CAN message data exceeds maximum length.");
        return Result::InvalidArgument;
    }

    return Result::Ok;
}

Result EthMessage::SerializeTo(Channel& channel) const {
    CheckResult(channel.Write(timestamp));
    CheckResult(channel.Write(controllerId));
    CheckResult(channel.Write(flags));
    CheckResult(channel.Write(length));
    return channel.Write(data.data(), length);
}

Result EthMessage::DeserializeFrom(Channel& channel) {
    CheckResult(channel.Read(timestamp));
    CheckResult(channel.Read(controllerId));
    CheckResult(channel.Read(flags));
    CheckResult(channel.Read(length));
    CheckResult(CheckMaxLength());
    return channel.Read(data.data(), length);
}

void EthMessage::WriteTo(DsVeosCoSim_EthMessage& message) const {
    message.timestamp = timestamp;
    message.controllerId = controllerId;
    message.flags = (DsVeosCoSim_EthMessageFlags)flags;
    message.length = length;
    message.data = data.data();
}

Result EthMessage::ReadFrom(const DsVeosCoSim_EthMessage& message) {
    timestamp = message.timestamp;
    controllerId = message.controllerId;
    flags = (EthMessageFlags)message.flags;
    length = message.length;
    CheckResult(CheckMaxLength());
    (void)std::memcpy(data.data(), message.data, message.length);
    return Result::Ok;
}

DsVeosCoSim_EthMessage EthMessage::Convert() const {
    DsVeosCoSim_EthMessage message{};
    WriteTo(message);
    return message;
}

Result EthMessage::CheckMaxLength() const {
    if (length > EthMessageMaxLength) {
        LogError("Ethernet message data exceeds maximum length.");
        return Result::InvalidArgument;
    }

    return Result::Ok;
}

Result LinMessage::SerializeTo(Channel& channel) const {
    CheckResult(channel.Write(timestamp));
    CheckResult(channel.Write(controllerId));
    CheckResult(channel.Write(id));
    CheckResult(channel.Write(flags));
    CheckResult(channel.Write(length));
    return channel.Write(data.data(), length);
}

Result LinMessage::DeserializeFrom(Channel& channel) {
    CheckResult(channel.Read(timestamp));
    CheckResult(channel.Read(controllerId));
    CheckResult(channel.Read(id));
    CheckResult(channel.Read(flags));
    CheckResult(channel.Read(length));
    CheckResult(CheckMaxLength());
    return channel.Read(data.data(), length);
}

void LinMessage::WriteTo(DsVeosCoSim_LinMessage& message) const {
    message.timestamp = timestamp;
    message.controllerId = controllerId;
    message.id = id;
    message.flags = (DsVeosCoSim_LinMessageFlags)flags;
    message.length = length;
    message.data = data.data();
}

Result LinMessage::ReadFrom(const DsVeosCoSim_LinMessage& message) {
    timestamp = message.timestamp;
    controllerId = message.controllerId;
    id = message.id;
    flags = (LinMessageFlags)message.flags;
    length = message.length;
    CheckResult(CheckMaxLength());
    (void)std::memcpy(data.data(), message.data, message.length);
    return Result::Ok;
}

DsVeosCoSim_LinMessage LinMessage::Convert() const {
    DsVeosCoSim_LinMessage message{};
    WriteTo(message);
    return message;
}

Result LinMessage::CheckMaxLength() const {
    if (length > LinMessageMaxLength) {
        LogError("LIN message data exceeds maximum length.");
        return Result::InvalidArgument;
    }

    return Result::Ok;
}

Result BusBuffer::Initialize(const std::vector<DsVeosCoSim_CanController>& canControllers,
                             const std::vector<DsVeosCoSim_EthController>& ethControllers,
                             const std::vector<DsVeosCoSim_LinController>& linControllers) {
    Clear();

    CheckResult(_canBuffer.Initialize(canControllers));
    CheckResult(_ethBuffer.Initialize(ethControllers));
    return _linBuffer.Initialize(linControllers);
}

void BusBuffer::Clear() {
    _canBuffer.Clear();
    _ethBuffer.Clear();
    _linBuffer.Clear();
}

Result BusBuffer::Transmit(const DsVeosCoSim_CanMessage& message) {
    return _canBuffer.Transmit(message);
}

Result BusBuffer::Transmit(const DsVeosCoSim_EthMessage& message) {
    return _ethBuffer.Transmit(message);
}

Result BusBuffer::Transmit(const DsVeosCoSim_LinMessage& message) {
    return _linBuffer.Transmit(message);
}

Result BusBuffer::Receive(DsVeosCoSim_CanMessage& message) {
    return _canBuffer.Receive(message);
}

Result BusBuffer::Receive(DsVeosCoSim_EthMessage& message) {
    return _ethBuffer.Receive(message);
}

Result BusBuffer::Receive(DsVeosCoSim_LinMessage& message) {
    return _linBuffer.Receive(message);
}

Result BusBuffer::Deserialize(Channel& channel, SimulationTime simulationTime, const Callbacks& callbacks) {
    CheckResult(_canBuffer.Deserialize(channel, simulationTime, callbacks.canMessageReceivedCallback));
    CheckResult(_ethBuffer.Deserialize(channel, simulationTime, callbacks.ethMessageReceivedCallback));
    return _linBuffer.Deserialize(channel, simulationTime, callbacks.linMessageReceivedCallback);
}

Result BusBuffer::Serialize(Channel& channel) {
    CheckResult(_canBuffer.Serialize(channel));
    CheckResult(_ethBuffer.Serialize(channel));
    return _linBuffer.Serialize(channel);
}

}  // namespace DsVeosCoSim
