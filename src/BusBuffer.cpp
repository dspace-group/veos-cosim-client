// Copyright dSPACE GmbH. All rights reserved.

#include "BusBuffer.h"

namespace DsVeosCoSim {

namespace {

#ifdef _WIN32
using LocalCanBuffer = LocalBusProtocolBuffer<DsVeosCoSim_CanMessage, CanMessage, DsVeosCoSim_CanController>;
using LocalEthBuffer = LocalBusProtocolBuffer<DsVeosCoSim_EthMessage, EthMessage, DsVeosCoSim_EthController>;
using LocalLinBuffer = LocalBusProtocolBuffer<DsVeosCoSim_LinMessage, LinMessage, DsVeosCoSim_LinController>;
#endif

using RemoteCanBuffer = RemoteBusProtocolBuffer<DsVeosCoSim_CanMessage, CanMessage, DsVeosCoSim_CanController>;
using RemoteEthBuffer = RemoteBusProtocolBuffer<DsVeosCoSim_EthMessage, EthMessage, DsVeosCoSim_EthController>;
using RemoteLinBuffer = RemoteBusProtocolBuffer<DsVeosCoSim_LinMessage, LinMessage, DsVeosCoSim_LinController>;

}  // namespace

bool CanMessage::SerializeTo(ChannelWriter& writer) const {
    CheckResultWithMessage(writer.Write(timestamp), "Could not write timestamp.");
    CheckResultWithMessage(writer.Write(controllerId), "Could not write controller id.");
    CheckResultWithMessage(writer.Write(id), "Could not write id.");
    CheckResultWithMessage(writer.Write(flags), "Could not write flags.");
    CheckResultWithMessage(writer.Write(length), "Could not write length.");
    CheckResultWithMessage(writer.Write(data.data(), length), "Could not write data.");
    return true;
}

bool CanMessage::DeserializeFrom(ChannelReader& reader) {
    CheckResultWithMessage(reader.Read(timestamp), "Could not read timestamp.");
    CheckResultWithMessage(reader.Read(controllerId), "Could not read controller id.");
    CheckResultWithMessage(reader.Read(id), "Could not read id.");
    CheckResultWithMessage(reader.Read(flags), "Could not read flags.");
    CheckResultWithMessage(reader.Read(length), "Could not read length");
    CheckMaxLength();
    CheckResultWithMessage(reader.Read(data.data(), length), "Could not read data.");
    return true;
}

void CanMessage::WriteTo(DsVeosCoSim_CanMessage& message) const {
    message.timestamp = timestamp;
    message.controllerId = controllerId;
    message.id = id;
    message.flags = flags;
    message.length = length;
    message.data = data.data();
}

void CanMessage::ReadFrom(const DsVeosCoSim_CanMessage& message) {
    timestamp = message.timestamp;
    controllerId = message.controllerId;
    id = message.id;
    flags = message.flags;
    length = message.length;
    CheckMaxLength();
    (void)::memcpy(data.data(), message.data, message.length);
}

CanMessage::operator DsVeosCoSim_CanMessage() const {
    DsVeosCoSim_CanMessage message{};
    WriteTo(message);
    return message;
}

void CanMessage::CheckMaxLength() const {
    if (length > CanMessageMaxLength) {
        throw CoSimException("CAN message data exceeds maximum length.");
    }
}

bool EthMessage::SerializeTo(ChannelWriter& writer) const {
    CheckResultWithMessage(writer.Write(timestamp), "Could not write timestamp.");
    CheckResultWithMessage(writer.Write(controllerId), "Could not write controller id.");
    CheckResultWithMessage(writer.Write(flags), "Could not write flags.");
    CheckResultWithMessage(writer.Write(length), "Could not write length.");
    CheckResultWithMessage(writer.Write(data.data(), length), "Could not write data.");
    return true;
}

bool EthMessage::DeserializeFrom(ChannelReader& reader) {
    CheckResultWithMessage(reader.Read(timestamp), "Could not read timestamp.");
    CheckResultWithMessage(reader.Read(controllerId), "Could not read controller id.");
    CheckResultWithMessage(reader.Read(flags), "Could not read flags.");
    CheckResultWithMessage(reader.Read(length), "Could not read length.");
    CheckMaxLength();
    CheckResultWithMessage(reader.Read(data.data(), length), "Could not read data.");
    return true;
}

void EthMessage::WriteTo(DsVeosCoSim_EthMessage& message) const {
    message.timestamp = timestamp;
    message.controllerId = controllerId;
    message.flags = flags;
    message.length = length;
    message.data = data.data();
}

void EthMessage::ReadFrom(const DsVeosCoSim_EthMessage& message) {
    timestamp = message.timestamp;
    controllerId = message.controllerId;
    flags = message.flags;
    length = message.length;
    CheckMaxLength();
    (void)::memcpy(data.data(), message.data, message.length);
}

EthMessage::operator DsVeosCoSim_EthMessage() const {
    DsVeosCoSim_EthMessage message{};
    WriteTo(message);
    return message;
}

void EthMessage::CheckMaxLength() const {
    if (length > EthMessageMaxLength) {
        throw CoSimException("Ethernet message data exceeds maximum length.");
    }
}

bool LinMessage::SerializeTo(ChannelWriter& writer) const {
    CheckResultWithMessage(writer.Write(timestamp), "Could not write timestamp.");
    CheckResultWithMessage(writer.Write(controllerId), "Could not write controller id.");
    CheckResultWithMessage(writer.Write(id), "Could not write id.");
    CheckResultWithMessage(writer.Write(flags), "Could not write flags.");
    CheckResultWithMessage(writer.Write(length), "Could not write length.");
    CheckResultWithMessage(writer.Write(data.data(), length), "Could not write data.");
    return true;
}

bool LinMessage::DeserializeFrom(ChannelReader& reader) {
    CheckResultWithMessage(reader.Read(timestamp), "Could not read timestamp.");
    CheckResultWithMessage(reader.Read(controllerId), "Could not read controller id.");
    CheckResultWithMessage(reader.Read(id), "Could not read id.");
    CheckResultWithMessage(reader.Read(flags), "Could not read flags.");
    CheckResultWithMessage(reader.Read(length), "Could not read length.");
    CheckMaxLength();
    CheckResultWithMessage(reader.Read(data.data(), length), "Could not read data.");
    return true;
}

void LinMessage::WriteTo(DsVeosCoSim_LinMessage& message) const {
    message.timestamp = timestamp;
    message.controllerId = controllerId;
    message.id = id;
    message.flags = flags;
    message.length = length;
    message.data = data.data();
}

void LinMessage::ReadFrom(const DsVeosCoSim_LinMessage& message) {
    timestamp = message.timestamp;
    controllerId = message.controllerId;
    id = message.id;
    flags = message.flags;
    length = message.length;
    CheckMaxLength();
    (void)::memcpy(data.data(), message.data, message.length);
}

LinMessage::operator DsVeosCoSim_LinMessage() const {
    DsVeosCoSim_LinMessage message{};
    WriteTo(message);
    return message;
}

void LinMessage::CheckMaxLength() const {
    if (length > LinMessageMaxLength) {
        throw CoSimException("LIN message data exceeds maximum length.");
    }
}

BusBuffer::BusBuffer(CoSimType coSimType,
                     [[maybe_unused]] ConnectionKind connectionKind,
                     const std::string& name,
                     const std::vector<DsVeosCoSim_CanController>& canControllers,
                     const std::vector<DsVeosCoSim_EthController>& ethControllers,
                     const std::vector<DsVeosCoSim_LinController>& linControllers) {
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

    std::string suffixForTransmit = coSimType == CoSimType::Client ? "Transmit" : "Receive";
    std::string suffixForReceive = coSimType == CoSimType::Client ? "Receive" : "Transmit";

    _canTransmitBuffer->Initialize(coSimType, fmt::format("{}.Can.{}", name, suffixForTransmit), canControllers);
    _ethTransmitBuffer->Initialize(coSimType, fmt::format("{}.Eth.{}", name, suffixForTransmit), ethControllers);
    _linTransmitBuffer->Initialize(coSimType, fmt::format("{}.Lin.{}", name, suffixForTransmit), linControllers);
    _canReceiveBuffer->Initialize(coSimType, fmt::format("{}.Can.{}", name, suffixForReceive), canControllers);
    _ethReceiveBuffer->Initialize(coSimType, fmt::format("{}.Eth.{}", name, suffixForReceive), ethControllers);
    _linReceiveBuffer->Initialize(coSimType, fmt::format("{}.Lin.{}", name, suffixForReceive), linControllers);
}

BusBuffer::BusBuffer(CoSimType coSimType,
                     ConnectionKind connectionKind,
                     const std::string& name,
                     const std::vector<DsVeosCoSim_CanController>& canControllers)
    : BusBuffer(coSimType, connectionKind, name, canControllers, {}, {}) {
}

BusBuffer::BusBuffer(CoSimType coSimType,
                     ConnectionKind connectionKind,
                     const std::string& name,
                     const std::vector<DsVeosCoSim_EthController>& ethControllers)
    : BusBuffer(coSimType, connectionKind, name, {}, ethControllers, {}) {
}

BusBuffer::BusBuffer(CoSimType coSimType,
                     ConnectionKind connectionKind,
                     const std::string& name,
                     const std::vector<DsVeosCoSim_LinController>& linControllers)
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

bool BusBuffer::Transmit(const DsVeosCoSim_CanMessage& message) const {
    return _canTransmitBuffer->Transmit(message);
}

bool BusBuffer::Transmit(const DsVeosCoSim_EthMessage& message) const {
    return _ethTransmitBuffer->Transmit(message);
}

bool BusBuffer::Transmit(const DsVeosCoSim_LinMessage& message) const {
    return _linTransmitBuffer->Transmit(message);
}

bool BusBuffer::Receive(DsVeosCoSim_CanMessage& message) const {
    return _canReceiveBuffer->Receive(message);
}

bool BusBuffer::Receive(DsVeosCoSim_EthMessage& message) const {
    return _ethReceiveBuffer->Receive(message);
}

bool BusBuffer::Receive(DsVeosCoSim_LinMessage& message) const {
    return _linReceiveBuffer->Receive(message);
}

bool BusBuffer::Serialize(ChannelWriter& writer) const {
    CheckResultWithMessage(_canTransmitBuffer->Serialize(writer), "Could not transmit CAN messages.");
    CheckResultWithMessage(_ethTransmitBuffer->Serialize(writer), "Could not transmit ETH messages.");
    CheckResultWithMessage(_linTransmitBuffer->Serialize(writer), "Could not transmit LIN messages.");
    return true;
}

bool BusBuffer::Deserialize(ChannelReader& reader,
                            DsVeosCoSim_SimulationTime simulationTime,
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
