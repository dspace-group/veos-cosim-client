// Copyright dSPACE GmbH. All rights reserved.

#include "BusBuffer.h"

#include <cstring>

#include "CoSimTypes.h"
#include "Logger.h"

namespace DsVeosCoSim {

Result CanMessageShm::SerializeTo(Channel& channel) const {
    CheckResult(channel.Write(timestamp));
    CheckResult(channel.Write(controllerId));
    CheckResult(channel.Write(id));
    CheckResult(channel.Write(flags));
    CheckResult(channel.Write(length));
    return channel.Write(data.data(), length);
}

Result CanMessageShm::DeserializeFrom(Channel& channel) {
    CheckResult(channel.Read(timestamp));
    CheckResult(channel.Read(controllerId));
    CheckResult(channel.Read(id));
    CheckResult(channel.Read(flags));
    CheckResult(channel.Read(length));
    CheckResult(CheckMaxLength());
    return channel.Read(data.data(), length);
}

void CanMessageShm::WriteTo(DsVeosCoSim_CanMessage& message) const {
    message.timestamp = timestamp;
    message.controllerId = controllerId;
    message.id = id;
    message.flags = (DsVeosCoSim_CanMessageFlags)flags;
    message.length = length;
    message.data = data.data();
}

Result CanMessageShm::ReadFrom(const DsVeosCoSim_CanMessage& message) {
    timestamp = message.timestamp;
    controllerId = message.controllerId;
    id = message.id;
    flags = (CanMessageFlags)message.flags;
    length = message.length;
    CheckResult(CheckMaxLength());
    (void)std::memcpy(data.data(), message.data, message.length);
    return Result::Ok;
}

Result CanMessageShm::CheckMaxLength() const {
    if (length > CanMessageMaxLength) {
        LogError("CAN message data exceeds maximum length.");
        return Result::InvalidArgument;
    }

    return Result::Ok;
}

Result EthMessageShm::SerializeTo(Channel& channel) const {
    CheckResult(channel.Write(timestamp));
    CheckResult(channel.Write(controllerId));
    CheckResult(channel.Write(flags));
    CheckResult(channel.Write(length));
    return channel.Write(data.data(), length);
}

Result EthMessageShm::DeserializeFrom(Channel& channel) {
    CheckResult(channel.Read(timestamp));
    CheckResult(channel.Read(controllerId));
    CheckResult(channel.Read(flags));
    CheckResult(channel.Read(length));
    CheckResult(CheckMaxLength());
    return channel.Read(data.data(), length);
}

void EthMessageShm::WriteTo(DsVeosCoSim_EthMessage& message) const {
    message.timestamp = timestamp;
    message.controllerId = controllerId;
    message.flags = (DsVeosCoSim_EthMessageFlags)flags;
    message.length = length;
    message.data = data.data();
}

Result EthMessageShm::ReadFrom(const DsVeosCoSim_EthMessage& message) {
    timestamp = message.timestamp;
    controllerId = message.controllerId;
    flags = (EthMessageFlags)message.flags;
    length = message.length;
    CheckResult(CheckMaxLength());
    (void)std::memcpy(data.data(), message.data, message.length);
    return Result::Ok;
}

Result EthMessageShm::CheckMaxLength() const {
    if (length > EthMessageMaxLength) {
        LogError("Ethernet message data exceeds maximum length.");
        return Result::InvalidArgument;
    }

    return Result::Ok;
}

Result LinMessageShm::SerializeTo(Channel& channel) const {
    CheckResult(channel.Write(timestamp));
    CheckResult(channel.Write(controllerId));
    CheckResult(channel.Write(id));
    CheckResult(channel.Write(flags));
    CheckResult(channel.Write(length));
    return channel.Write(data.data(), length);
}

Result LinMessageShm::DeserializeFrom(Channel& channel) {
    CheckResult(channel.Read(timestamp));
    CheckResult(channel.Read(controllerId));
    CheckResult(channel.Read(id));
    CheckResult(channel.Read(flags));
    CheckResult(channel.Read(length));
    CheckResult(CheckMaxLength());
    return channel.Read(data.data(), length);
}

void LinMessageShm::WriteTo(DsVeosCoSim_LinMessage& message) const {
    message.timestamp = timestamp;
    message.controllerId = controllerId;
    message.id = id;
    message.flags = (DsVeosCoSim_LinMessageFlags)flags;
    message.length = length;
    message.data = data.data();
}

Result LinMessageShm::ReadFrom(const DsVeosCoSim_LinMessage& message) {
    timestamp = message.timestamp;
    controllerId = message.controllerId;
    id = message.id;
    flags = (LinMessageFlags)message.flags;
    length = message.length;
    CheckResult(CheckMaxLength());
    (void)std::memcpy(data.data(), message.data, message.length);
    return Result::Ok;
}

Result LinMessageShm::CheckMaxLength() const {
    if (length > LinMessageMaxLength) {
        LogError("LIN message data exceeds maximum length.");
        return Result::InvalidArgument;
    }

    return Result::Ok;
}

Result BusBuffer::Initialize(const std::vector<CanController>& canControllers,
                             const std::vector<EthController>& ethControllers,
                             const std::vector<LinController>& linControllers) {
    ClearData();

    CheckResult(_canBuffer.Initialize(canControllers));
    CheckResult(_ethBuffer.Initialize(ethControllers));
    return _linBuffer.Initialize(linControllers);
}

void BusBuffer::ClearData() {
    _canBuffer.ClearData();
    _ethBuffer.ClearData();
    _linBuffer.ClearData();
}

Result BusBuffer::Transmit(const DsVeosCoSim_CanMessage& message) {
    return _canBuffer.Transmit(message);
}

Result BusBuffer::Receive(DsVeosCoSim_CanMessage& message) {
    return _canBuffer.Receive(message);
}

Result BusBuffer::Transmit(const DsVeosCoSim_EthMessage& message) {
    return _ethBuffer.Transmit(message);
}

Result BusBuffer::Receive(DsVeosCoSim_EthMessage& message) {
    return _ethBuffer.Receive(message);
}

Result BusBuffer::Transmit(const DsVeosCoSim_LinMessage& message) {
    return _linBuffer.Transmit(message);
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
