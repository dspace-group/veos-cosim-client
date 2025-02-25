// Copyright dSPACE GmbH. All rights reserved.

#include "BusBuffer.h"

#include "CoSimTypes.h"

#include <string>

namespace DsVeosCoSim {

namespace {

#ifdef _WIN32
using LocalCanBuffer = LocalBusProtocolBuffer<CanMessageContainer, CanMessage, CanController>;
using LocalEthBuffer = LocalBusProtocolBuffer<EthMessageContainer, EthMessage, EthController>;
using LocalLinBuffer = LocalBusProtocolBuffer<LinMessageContainer, LinMessage, LinController>;
#endif

using RemoteCanBuffer = RemoteBusProtocolBuffer<CanMessageContainer, CanMessage, CanController>;
using RemoteEthBuffer = RemoteBusProtocolBuffer<EthMessageContainer, EthMessage, EthController>;
using RemoteLinBuffer = RemoteBusProtocolBuffer<LinMessageContainer, LinMessage, LinController>;

}  // namespace

[[nodiscard]] bool CanMessageContainer::SerializeTo(ChannelWriter& writer) const {
    CheckResultWithMessage(writer.Write(timestamp), "Could not write timestamp.");
    CheckResultWithMessage(writer.Write(controllerId), "Could not write controller id.");
    CheckResultWithMessage(writer.Write(id), "Could not write id.");
    CheckResultWithMessage(writer.Write(flags), "Could not write flags.");
    CheckResultWithMessage(writer.Write(length), "Could not write length.");
    CheckResultWithMessage(writer.Write(data.data(), length), "Could not write data.");
    return true;
}

[[nodiscard]] bool CanMessageContainer::DeserializeFrom(ChannelReader& reader) {
    CheckResultWithMessage(reader.Read(timestamp), "Could not read timestamp.");
    CheckResultWithMessage(reader.Read(controllerId), "Could not read controller id.");
    CheckResultWithMessage(reader.Read(id), "Could not read id.");
    CheckResultWithMessage(reader.Read(flags), "Could not read flags.");
    CheckResultWithMessage(reader.Read(length), "Could not read length");
    CheckMaxLength();
    CheckResultWithMessage(reader.Read(data.data(), length), "Could not read data.");
    return true;
}

void CanMessageContainer::WriteTo(CanMessage& message) const {
    message.timestamp = timestamp;
    message.controllerId = controllerId;
    message.id = id;
    message.flags = flags;
    message.length = length;
    message.data = data.data();
}

void CanMessageContainer::ReadFrom(const CanMessage& message) {
    timestamp = message.timestamp;
    controllerId = message.controllerId;
    id = message.id;
    flags = message.flags;
    length = message.length;
    CheckMaxLength();
    CheckFlags();
    (void)memcpy(data.data(), message.data, message.length);
}

[[nodiscard]] CanMessageContainer::operator CanMessage() const {
    CanMessage message{};
    WriteTo(message);
    return message;
}

[[nodiscard]] inline std::string CanMessageContainer::ToString() const {
    return "CAN Message { Timestamp: " + SimulationTimeToString(timestamp) +
           ", ControllerId: " + DsVeosCoSim::ToString(controllerId) + ", Id: " + DsVeosCoSim::ToString(id) +
           ", Length: " + std::to_string(length) + ", Data: " + DataToString(data.data(), length, '-') +
           ", Flags: " + DsVeosCoSim::ToString(flags) + " }";
}

void CanMessageContainer::CheckMaxLength() const {
    if (length > CanMessageMaxLength) {
        throw CoSimException("CAN message data exceeds maximum length.");
    }
}

void CanMessageContainer::CheckFlags() const {
    if (!HasFlag(flags, CanMessageFlags::FlexibleDataRateFormat)) {
        if (length > 8) {
            throw CoSimException("CAN message flags invalid. A DLC > 8 requires the flexible data rate format flag.");
        }

        if (HasFlag(flags, CanMessageFlags::BitRateSwitch)) {
            throw CoSimException(
                "CAN message flags invalid. A bit rate switch flag requires the flexible data rate format flag.");
        }
    }
}

[[nodiscard]] bool EthMessageContainer::SerializeTo(ChannelWriter& writer) const {
    CheckResultWithMessage(writer.Write(timestamp), "Could not write timestamp.");
    CheckResultWithMessage(writer.Write(controllerId), "Could not write controller id.");
    CheckResultWithMessage(writer.Write(flags), "Could not write flags.");
    CheckResultWithMessage(writer.Write(length), "Could not write length.");
    CheckResultWithMessage(writer.Write(data.data(), length), "Could not write data.");
    return true;
}

[[nodiscard]] bool EthMessageContainer::DeserializeFrom(ChannelReader& reader) {
    CheckResultWithMessage(reader.Read(timestamp), "Could not read timestamp.");
    CheckResultWithMessage(reader.Read(controllerId), "Could not read controller id.");
    CheckResultWithMessage(reader.Read(flags), "Could not read flags.");
    CheckResultWithMessage(reader.Read(length), "Could not read length.");
    CheckMaxLength();
    CheckResultWithMessage(reader.Read(data.data(), length), "Could not read data.");
    return true;
}

void EthMessageContainer::WriteTo(EthMessage& message) const {
    message.timestamp = timestamp;
    message.controllerId = controllerId;
    message.flags = flags;
    message.length = length;
    message.data = data.data();
}

void EthMessageContainer::ReadFrom(const EthMessage& message) {
    timestamp = message.timestamp;
    controllerId = message.controllerId;
    flags = message.flags;
    length = message.length;
    CheckMaxLength();
    (void)memcpy(data.data(), message.data, message.length);
}

[[nodiscard]] EthMessageContainer::operator EthMessage() const {
    EthMessage message{};
    WriteTo(message);
    return message;
}

[[nodiscard]] inline std::string EthMessageContainer::ToString() const {
    return "ETH Message { Timestamp: " + SimulationTimeToString(timestamp) +
           ", ControllerId: " + DsVeosCoSim::ToString(controllerId) + ", Length: " + std::to_string(length) +
           ", Data: " + DataToString(data.data(), length, '-') + ", Flags: " + DsVeosCoSim::ToString(flags) + " }";
}

void EthMessageContainer::CheckMaxLength() const {
    if (length > EthMessageMaxLength) {
        throw CoSimException("Ethernet message data exceeds maximum length.");
    }
}

[[nodiscard]] bool LinMessageContainer::SerializeTo(ChannelWriter& writer) const {
    CheckResultWithMessage(writer.Write(timestamp), "Could not write timestamp.");
    CheckResultWithMessage(writer.Write(controllerId), "Could not write controller id.");
    CheckResultWithMessage(writer.Write(id), "Could not write id.");
    CheckResultWithMessage(writer.Write(flags), "Could not write flags.");
    CheckResultWithMessage(writer.Write(length), "Could not write length.");
    CheckResultWithMessage(writer.Write(data.data(), length), "Could not write data.");
    return true;
}

[[nodiscard]] bool LinMessageContainer::DeserializeFrom(ChannelReader& reader) {
    CheckResultWithMessage(reader.Read(timestamp), "Could not read timestamp.");
    CheckResultWithMessage(reader.Read(controllerId), "Could not read controller id.");
    CheckResultWithMessage(reader.Read(id), "Could not read id.");
    CheckResultWithMessage(reader.Read(flags), "Could not read flags.");
    CheckResultWithMessage(reader.Read(length), "Could not read length.");
    CheckMaxLength();
    CheckResultWithMessage(reader.Read(data.data(), length), "Could not read data.");
    return true;
}

void LinMessageContainer::WriteTo(LinMessage& message) const {
    message.timestamp = timestamp;
    message.controllerId = controllerId;
    message.id = id;
    message.flags = flags;
    message.length = length;
    message.data = data.data();
}

void LinMessageContainer::ReadFrom(const LinMessage& message) {
    timestamp = message.timestamp;
    controllerId = message.controllerId;
    id = message.id;
    flags = message.flags;
    length = message.length;
    CheckMaxLength();
    (void)memcpy(data.data(), message.data, message.length);
}

[[nodiscard]] LinMessageContainer::operator LinMessage() const {
    LinMessage message{};
    WriteTo(message);
    return message;
}

[[nodiscard]] inline std::string LinMessageContainer::ToString() const {
    return "LIN Message { Timestamp: " + SimulationTimeToString(timestamp) +
           ", ControllerId: " + DsVeosCoSim::ToString(controllerId) + ", Id: " + DsVeosCoSim::ToString(id) +
           ", Length: " + std::to_string(length) + ", Data: " + DataToString(data.data(), length, '-') +
           ", Flags: " + DsVeosCoSim::ToString(flags) + " }";
}

void LinMessageContainer::CheckMaxLength() const {
    if (length > LinMessageMaxLength) {
        throw CoSimException("LIN message data exceeds maximum length.");
    }
}

BusBuffer::BusBuffer(const CoSimType coSimType,
                     [[maybe_unused]] const ConnectionKind connectionKind,
                     const std::string& name,
                     const std::vector<CanController>& canControllers,
                     const std::vector<EthController>& ethControllers,
                     const std::vector<LinController>& linControllers) {
#ifdef _WIN32
    if (connectionKind == ConnectionKind::Local) {
        _canTransmitBuffer = std::make_unique<LocalCanBuffer>();
        _ethTransmitBuffer = std::make_unique<LocalEthBuffer>();
        _linTransmitBuffer = std::make_unique<LocalLinBuffer>();

        _canReceiveBuffer = std::make_unique<LocalCanBuffer>();
        _ethReceiveBuffer = std::make_unique<LocalEthBuffer>();
        _linReceiveBuffer = std::make_unique<LocalLinBuffer>();
    } else {
#endif
        _canTransmitBuffer = std::make_unique<RemoteCanBuffer>();
        _ethTransmitBuffer = std::make_unique<RemoteEthBuffer>();
        _linTransmitBuffer = std::make_unique<RemoteLinBuffer>();

        _canReceiveBuffer = std::make_unique<RemoteCanBuffer>();
        _ethReceiveBuffer = std::make_unique<RemoteEthBuffer>();
        _linReceiveBuffer = std::make_unique<RemoteLinBuffer>();
#ifdef _WIN32
    }
#endif

    const std::string suffixForTransmit = coSimType == CoSimType::Client ? "Transmit" : "Receive";
    const std::string suffixForReceive = coSimType == CoSimType::Client ? "Receive" : "Transmit";

    _canTransmitBuffer->Initialize(coSimType, name + ".Can." + suffixForTransmit, canControllers);
    _ethTransmitBuffer->Initialize(coSimType, name + ".Eth." + suffixForTransmit, ethControllers);
    _linTransmitBuffer->Initialize(coSimType, name + ".Lin." + suffixForTransmit, linControllers);
    _canReceiveBuffer->Initialize(coSimType, name + ".Can." + suffixForReceive, canControllers);
    _ethReceiveBuffer->Initialize(coSimType, name + ".Eth." + suffixForReceive, ethControllers);
    _linReceiveBuffer->Initialize(coSimType, name + ".Lin." + suffixForReceive, linControllers);
}

BusBuffer::BusBuffer(const CoSimType coSimType,
                     const ConnectionKind connectionKind,
                     const std::string& name,
                     const std::vector<CanController>& canControllers)
    : BusBuffer(coSimType, connectionKind, name, canControllers, {}, {}) {
}

BusBuffer::BusBuffer(const CoSimType coSimType,
                     const ConnectionKind connectionKind,
                     const std::string& name,
                     const std::vector<EthController>& ethControllers)
    : BusBuffer(coSimType, connectionKind, name, {}, ethControllers, {}) {
}

BusBuffer::BusBuffer(const CoSimType coSimType,
                     const ConnectionKind connectionKind,
                     const std::string& name,
                     const std::vector<LinController>& linControllers)
    : BusBuffer(coSimType, connectionKind, name, {}, {}, linControllers) {
}

void BusBuffer::ClearData() const {
    _canTransmitBuffer->ClearData();
    _ethTransmitBuffer->ClearData();
    _linTransmitBuffer->ClearData();

    _canReceiveBuffer->ClearData();
    _ethReceiveBuffer->ClearData();
    _linReceiveBuffer->ClearData();
}

[[nodiscard]] bool BusBuffer::Transmit(const CanMessage& message) const {
    return _canTransmitBuffer->Transmit(message);
}

[[nodiscard]] bool BusBuffer::Transmit(const EthMessage& message) const {
    return _ethTransmitBuffer->Transmit(message);
}

[[nodiscard]] bool BusBuffer::Transmit(const LinMessage& message) const {
    return _linTransmitBuffer->Transmit(message);
}

[[nodiscard]] bool BusBuffer::Receive(CanMessage& message) const {
    return _canReceiveBuffer->Receive(message);
}

[[nodiscard]] bool BusBuffer::Receive(EthMessage& message) const {
    return _ethReceiveBuffer->Receive(message);
}

[[nodiscard]] bool BusBuffer::Receive(LinMessage& message) const {
    return _linReceiveBuffer->Receive(message);
}

[[nodiscard]] bool BusBuffer::Serialize(ChannelWriter& writer) const {
    CheckResultWithMessage(_canTransmitBuffer->Serialize(writer), "Could not transmit CAN messages.");
    CheckResultWithMessage(_ethTransmitBuffer->Serialize(writer), "Could not transmit ETH messages.");
    CheckResultWithMessage(_linTransmitBuffer->Serialize(writer), "Could not transmit LIN messages.");
    return true;
}

[[nodiscard]] bool BusBuffer::Deserialize(ChannelReader& reader,
                                          const SimulationTime simulationTime,
                                          const Callbacks& callbacks) const {
    CheckResultWithMessage(_canReceiveBuffer->Deserialize(reader, simulationTime, callbacks.canMessageReceivedCallback),
                           "Could not receive CAN messages.");
    CheckResultWithMessage(_ethReceiveBuffer->Deserialize(reader, simulationTime, callbacks.ethMessageReceivedCallback),
                           "Could not receive ETH messages.");
    CheckResultWithMessage(_linReceiveBuffer->Deserialize(reader, simulationTime, callbacks.linMessageReceivedCallback),
                           "Could not receive LIN messages.");
    return true;
}

}  // namespace DsVeosCoSim
