// Copyright dSPACE GmbH. All rights reserved.

#include "Protocol.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "Channel.h"
#include "CoSimHelper.h"
#include "DsVeosCoSim/CoSimTypes.h"
#include "Environment.h"

namespace DsVeosCoSim {
namespace {
constexpr size_t IoSignalInfoSize = sizeof(IoSignalId) + sizeof(uint32_t) + sizeof(DataType) + sizeof(SizeKind);
constexpr size_t CanControllerSize = sizeof(BusControllerId) + sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint64_t);
constexpr size_t EthControllerSize = sizeof(BusControllerId) + sizeof(uint32_t) + sizeof(uint64_t) + EthAddressLength;
constexpr size_t LinControllerSize = sizeof(BusControllerId) + sizeof(uint32_t) + sizeof(uint64_t) + sizeof(
                                         LinControllerType);
constexpr size_t FrControllerSize = sizeof(BusControllerId) + sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint64_t);
constexpr size_t CanMessageSize = sizeof(SimulationTime) + sizeof(BusControllerId) + sizeof(BusMessageId) +
                                  sizeof(CanMessageFlags) + sizeof(uint32_t);
constexpr size_t EthMessageSize =
    sizeof(SimulationTime) + sizeof(BusControllerId) + sizeof(EthMessageFlags) + sizeof(uint32_t);
constexpr size_t LinMessageSize = sizeof(SimulationTime) + sizeof(BusControllerId) + sizeof(BusMessageId) +
                                  sizeof(LinMessageFlags) + sizeof(uint32_t);
constexpr size_t FrMessageSize = sizeof(SimulationTime) + sizeof(BusControllerId) + sizeof(BusMessageId) +
                                 sizeof(FrMessageFlags) + sizeof(uint32_t);
} // namespace

[[nodiscard]] Result V1::Protocol::ReadSize(ChannelReader& reader, size_t& size) {
    uint32_t intSize{};
    CheckResultWithMessage(reader.Read(intSize), "Could not read size.");
    size = static_cast<size_t>(intSize);
    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::WriteSize(ChannelWriter& writer, size_t size) {
    auto intSize = static_cast<uint32_t>(size);
    CheckResultWithMessage(writer.Write(intSize), "Could not write size.");
    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::ReadLength(ChannelReader& reader, uint32_t& length) {
    CheckResultWithMessage(reader.Read(length), "Could not read length.");
    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::WriteLength(ChannelWriter& writer, uint32_t length) {
    CheckResultWithMessage(writer.Write(length), "Could not write length.");
    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::ReadData(ChannelReader& reader, void* data, size_t size) {
    CheckResultWithMessage(reader.Read(data, size), "Could not read data.");
    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::WriteData(ChannelWriter& writer, const void* data, size_t size) {
    CheckResultWithMessage(writer.Write(data, size), "Could not write data.");
    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::ReadSignalId(ChannelReader& reader, IoSignalId& signalId) {
    CheckResultWithMessage(reader.Read(signalId), "Could not read signal id.");
    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::WriteSignalId(ChannelWriter& writer, IoSignalId signalId) {
    CheckResultWithMessage(writer.Write(signalId), "Could not write signal id.");
    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::ReadMessage(ChannelReader& reader, CanMessageContainer& messageContainer) {
    BlockReader blockReader;
    CheckResultWithMessage(reader.ReadBlock(CanMessageSize, blockReader),
                           "Could not read block for CanMessageContainer.");

    blockReader.Read(messageContainer.timestamp);
    blockReader.Read(messageContainer.controllerId);
    blockReader.Read(messageContainer.id);
    blockReader.Read(messageContainer.flags);
    blockReader.Read(messageContainer.length);

    CheckResult(messageContainer.Check());

    CheckResultWithMessage(reader.Read(messageContainer.data.data(), messageContainer.length), "Could not read data.");
    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::WriteMessage(ChannelWriter& writer, const CanMessageContainer& messageContainer) {
    BlockWriter blockWriter;
    CheckResultWithMessage(writer.Reserve(CanMessageSize + messageContainer.length, blockWriter),
                           "Could not reserve memory for CanMessageContainer.");

    blockWriter.Write(messageContainer.timestamp);
    blockWriter.Write(messageContainer.controllerId);
    blockWriter.Write(messageContainer.id);
    blockWriter.Write(messageContainer.flags);
    blockWriter.Write(messageContainer.length);
    blockWriter.Write(messageContainer.data.data(), messageContainer.length);
    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::ReadMessage(ChannelReader& reader, EthMessageContainer& messageContainer) {
    BlockReader blockReader;
    CheckResultWithMessage(reader.ReadBlock(EthMessageSize, blockReader),
                           "Could not read block for EthMessageContainer.");

    blockReader.Read(messageContainer.timestamp);
    blockReader.Read(messageContainer.controllerId);
    blockReader.Read(messageContainer.flags);
    blockReader.Read(messageContainer.length);

    CheckResult(messageContainer.Check());

    CheckResultWithMessage(reader.Read(messageContainer.data.data(), messageContainer.length), "Could not read data.");
    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::WriteMessage(ChannelWriter& writer, const EthMessageContainer& messageContainer) {
    BlockWriter blockWriter;
    CheckResultWithMessage(writer.Reserve(EthMessageSize + messageContainer.length, blockWriter),
                           "Could not reserve memory for EthMessageContainer.");

    blockWriter.Write(messageContainer.timestamp);
    blockWriter.Write(messageContainer.controllerId);
    blockWriter.Write(messageContainer.flags);
    blockWriter.Write(messageContainer.length);
    blockWriter.Write(messageContainer.data.data(), messageContainer.length);
    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::ReadMessage(ChannelReader& reader, LinMessageContainer& messageContainer) {
    BlockReader blockReader;
    CheckResultWithMessage(reader.ReadBlock(LinMessageSize, blockReader),
                           "Could not read block for LinMessageContainer.");

    blockReader.Read(messageContainer.timestamp);
    blockReader.Read(messageContainer.controllerId);
    blockReader.Read(messageContainer.id);
    blockReader.Read(messageContainer.flags);
    blockReader.Read(messageContainer.length);

    CheckResult(messageContainer.Check());

    CheckResultWithMessage(reader.Read(messageContainer.data.data(), messageContainer.length), "Could not read data.");
    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::WriteMessage(ChannelWriter& writer, const LinMessageContainer& messageContainer) {
    BlockWriter blockWriter;
    CheckResultWithMessage(writer.Reserve(LinMessageSize + messageContainer.length, blockWriter),
                           "Could not reserve memory for LinMessageContainer.");

    blockWriter.Write(messageContainer.timestamp);
    blockWriter.Write(messageContainer.controllerId);
    blockWriter.Write(messageContainer.id);
    blockWriter.Write(messageContainer.flags);
    blockWriter.Write(messageContainer.length);
    blockWriter.Write(messageContainer.data.data(), messageContainer.length);
    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::ReadMessage([[maybe_unused]] ChannelReader& reader, [[maybe_unused]] FrMessageContainer& messageContainer) {
    // V1 does not have this functionality
    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::WriteMessage([[maybe_unused]] ChannelWriter& writer, [[maybe_unused]] const FrMessageContainer& messageContainer) {
    // V1 does not have this functionality
    return Result::Ok;
}

[[nodiscard]] Result V2::Protocol::ReadMessage(ChannelReader& reader, FrMessageContainer& messageContainer) {
    BlockReader blockReader;
    CheckResultWithMessage(reader.ReadBlock(FrMessageSize, blockReader),
                           "Could not read block for FrMessageContainer.");

    blockReader.Read(messageContainer.timestamp);
    blockReader.Read(messageContainer.controllerId);
    blockReader.Read(messageContainer.id);
    blockReader.Read(messageContainer.flags);
    blockReader.Read(messageContainer.length);

    CheckResult(messageContainer.Check());

    CheckResultWithMessage(reader.Read(messageContainer.data.data(), messageContainer.length), "Could not read data.");
    return Result::Ok;
}

[[nodiscard]] Result V2::Protocol::WriteMessage(ChannelWriter& writer, const FrMessageContainer& messageContainer) {
    BlockWriter blockWriter;
    CheckResultWithMessage(writer.Reserve(FrMessageSize + messageContainer.length, blockWriter),
                           "Could not reserve memory for FrMessageContainer.");

    blockWriter.Write(messageContainer.timestamp);
    blockWriter.Write(messageContainer.controllerId);
    blockWriter.Write(messageContainer.id);
    blockWriter.Write(messageContainer.flags);
    blockWriter.Write(messageContainer.length);
    blockWriter.Write(messageContainer.data.data(), messageContainer.length);
    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::ReceiveHeader(ChannelReader& reader, FrameKind& frameKind) {
    if (IsProtocolHeaderTracingEnabled()) {
        LogProtocolBeginTraceReceiveHeader();
    }

    CheckResultWithMessage(reader.Read(frameKind), "Could not receive frame kind.");

    if (IsProtocolHeaderTracingEnabled()) {
        LogProtocolEndTraceReceiveHeader(frameKind);
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::SendOk(ChannelWriter& writer) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceSendOk();
    }

    CheckResultWithMessage(writer.Write(FrameKind::Ok), "Could not write frame kind.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceSendOk();
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::ReadError(ChannelReader& reader, std::string& errorMessage) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceReadError();
    }

    CheckResultWithMessage(ReadString(reader, errorMessage), "Could not read error message.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceReadError(errorMessage);
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::SendError(ChannelWriter& writer, const std::string& errorMessage) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceSendError(errorMessage);
    }

    CheckResultWithMessage(writer.Write(FrameKind::Error), "Could not write frame kind.");
    CheckResultWithMessage(WriteString(writer, errorMessage), "Could not write error message.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceSendError();
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::SendPing(ChannelWriter& writer) {
    if (IsProtocolPingTracingEnabled()) {
        LogProtocolBeginTraceSendPing();
    }

    CheckResultWithMessage(writer.Write(FrameKind::Ping), "Could not write frame kind.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolPingTracingEnabled()) {
        LogProtocolEndTraceSendPing();
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::ReadPingOk(ChannelReader& reader, Command& command) {
    if (IsProtocolPingTracingEnabled()) {
        LogProtocolBeginTraceReadPingOk();
    }

    CheckResultWithMessage(reader.Read(command), "Could not read command.");

    if (IsProtocolPingTracingEnabled()) {
        LogProtocolEndTraceReadPingOk(command);
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::SendPingOk(ChannelWriter& writer, Command command) {
    if (IsProtocolPingTracingEnabled()) {
        LogProtocolBeginTraceSendPingOk(command);
    }

    constexpr size_t size = sizeof(FrameKind) + sizeof(command);

    BlockWriter blockWriter;
    CheckResultWithMessage(writer.Reserve(size, blockWriter), "Could not reserve memory for PingOk frame.");

    blockWriter.Write(FrameKind::PingOk);
    blockWriter.Write(command);

    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolPingTracingEnabled()) {
        LogProtocolEndTraceSendPingOk();
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::ReadConnect(ChannelReader& reader,
                                               uint32_t& protocolVersion,
                                               Mode& clientMode,
                                               std::string& serverName,
                                               std::string& clientName) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceReadConnect();
    }

    constexpr size_t size = sizeof(protocolVersion) + sizeof(clientMode);

    BlockReader blockReader;
    CheckResultWithMessage(reader.ReadBlock(size, blockReader), "Could not read block for Connect frame.");

    blockReader.Read(protocolVersion);
    blockReader.Read(clientMode);

    CheckResultWithMessage(ReadString(reader, serverName), "Could not read server name.");
    CheckResultWithMessage(ReadString(reader, clientName), "Could not read client name.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceReadConnect(protocolVersion, clientMode, serverName, clientName);
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::SendConnect(ChannelWriter& writer,
                                               uint32_t protocolVersion,
                                               Mode clientMode,
                                               const std::string& serverName,
                                               const std::string& clientName) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceSendConnect(protocolVersion, clientMode, serverName, clientName);
    }

    constexpr size_t size = sizeof(FrameKind) + sizeof(protocolVersion) + sizeof(clientMode);

    BlockWriter blockWriter;
    CheckResultWithMessage(writer.Reserve(size, blockWriter), "Could not reserve memory for Connect frame.");

    blockWriter.Write(FrameKind::Connect);
    blockWriter.Write(protocolVersion);
    blockWriter.Write(clientMode);

    CheckResultWithMessage(WriteString(writer, serverName), "Could not write server name.");
    CheckResultWithMessage(WriteString(writer, clientName), "Could not write client name.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceSendConnect();
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::ReadConnectOkVersion(ChannelReader& reader, uint32_t& protocolVersion) {
    constexpr size_t size = sizeof(protocolVersion);
    BlockReader blockReader;
    CheckResultWithMessage(reader.ReadBlock(size, blockReader),
                           "Could not read protocolVersion block for ConnectOkVersion frame.");
    blockReader.Read(protocolVersion);

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceReadConnectOkVersion(protocolVersion);
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::ReadConnectOk(ChannelReader& reader,
                                                 Mode& clientMode,
                                                 SimulationTime& stepSize,
                                                 SimulationState& simulationState,
                                                 std::vector<IoSignalContainer>& incomingSignals,
                                                 std::vector<IoSignalContainer>& outgoingSignals,
                                                 std::vector<CanControllerContainer>& canControllers,
                                                 std::vector<EthControllerContainer>& ethControllers,
                                                 std::vector<LinControllerContainer>& linControllers,
                                                 std::vector<FrControllerContainer>& frControllers) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceReadConnectOk();
    }

    constexpr size_t size = sizeof(clientMode) + sizeof(stepSize) + sizeof(simulationState);

    BlockReader blockReader;
    CheckResultWithMessage(reader.ReadBlock(size, blockReader), "Could not read block for ConnectOk frame.");

    blockReader.Read(clientMode);
    blockReader.Read(stepSize);
    blockReader.Read(simulationState);

    CheckResultWithMessage(ReadIoSignalInfos(reader, incomingSignals), "Could not read incoming signals.");
    CheckResultWithMessage(ReadIoSignalInfos(reader, outgoingSignals), "Could not read outgoing signals.");
    CheckResultWithMessage(ReadControllerInfos(reader, canControllers), "Could not read CAN controllers.");
    CheckResultWithMessage(ReadControllerInfos(reader, ethControllers), "Could not read ETH controllers.");
    CheckResultWithMessage(ReadControllerInfos(reader, linControllers), "Could not read LIN controllers.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceReadConnectOk(clientMode,
                                         stepSize,
                                         simulationState,
                                         incomingSignals,
                                         outgoingSignals,
                                         canControllers,
                                         ethControllers,
                                         linControllers,
                                         frControllers);
    }

    return Result::Ok;
}

[[nodiscard]] Result V2::Protocol::ReadConnectOk(ChannelReader& reader,
                                                 Mode& clientMode,
                                                 SimulationTime& stepSize,
                                                 SimulationState& simulationState,
                                                 std::vector<IoSignalContainer>& incomingSignals,
                                                 std::vector<IoSignalContainer>& outgoingSignals,
                                                 std::vector<CanControllerContainer>& canControllers,
                                                 std::vector<EthControllerContainer>& ethControllers,
                                                 std::vector<LinControllerContainer>& linControllers,
                                                 std::vector<FrControllerContainer>& frControllers) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceReadConnectOk();
    }

    constexpr size_t size = sizeof(clientMode) + sizeof(stepSize) + sizeof(simulationState);

    BlockReader blockReader;
    CheckResultWithMessage(reader.ReadBlock(size, blockReader), "Could not read block for ConnectOk frame.");

    blockReader.Read(clientMode);
    blockReader.Read(stepSize);
    blockReader.Read(simulationState);

    CheckResultWithMessage(ReadIoSignalInfos(reader, incomingSignals), "Could not read incoming signals.");
    CheckResultWithMessage(ReadIoSignalInfos(reader, outgoingSignals), "Could not read outgoing signals.");
    CheckResultWithMessage(V1::Protocol::ReadControllerInfos(reader, canControllers),
                           "Could not read CAN controllers.");
    CheckResultWithMessage(V1::Protocol::ReadControllerInfos(reader, ethControllers),
                           "Could not read ETH controllers.");
    CheckResultWithMessage(V1::Protocol::ReadControllerInfos(reader, linControllers),
                           "Could not read LIN controllers.");
    CheckResultWithMessage(ReadControllerInfos(reader, frControllers), "Could not read FLEXRAY controllers.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceReadConnectOk(clientMode,
                                         stepSize,
                                         simulationState,
                                         incomingSignals,
                                         outgoingSignals,
                                         canControllers,
                                         ethControllers,
                                         linControllers,
                                         frControllers);
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::SendConnectOk(ChannelWriter& writer,
                                                 uint32_t protocolVersion,
                                                 Mode clientMode,
                                                 SimulationTime stepSize,
                                                 SimulationState simulationState,
                                                 const std::vector<IoSignalContainer>& incomingSignals,
                                                 const std::vector<IoSignalContainer>& outgoingSignals,
                                                 const std::vector<CanControllerContainer>& canControllers,
                                                 const std::vector<EthControllerContainer>& ethControllers,
                                                 const std::vector<LinControllerContainer>& linControllers,
                                                 const std::vector<FrControllerContainer>& frControllers) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceSendConnectOk(protocolVersion,
                                           clientMode,
                                           stepSize,
                                           simulationState,
                                           incomingSignals,
                                           outgoingSignals,
                                           canControllers,
                                           ethControllers,
                                           linControllers,
                                           frControllers);
    }
    constexpr size_t size =
        sizeof(FrameKind) + sizeof(protocolVersion) + sizeof(clientMode) + sizeof(stepSize) + sizeof(simulationState);

    BlockWriter blockWriter;
    CheckResultWithMessage(writer.Reserve(size, blockWriter), "Could not reserve memory for ConnectOk frame.");

    blockWriter.Write(FrameKind::ConnectOk);
    blockWriter.Write(protocolVersion);
    blockWriter.Write(clientMode);
    blockWriter.Write(stepSize);
    blockWriter.Write(simulationState);

    CheckResultWithMessage(WriteIoSignalInfos(writer, incomingSignals), "Could not write incoming signals.");
    CheckResultWithMessage(WriteIoSignalInfos(writer, outgoingSignals), "Could not write outgoing signals.");
    CheckResultWithMessage(WriteControllerInfos(writer, canControllers), "Could not write CAN controllers.");
    CheckResultWithMessage(WriteControllerInfos(writer, ethControllers), "Could not write ETH controllers.");
    CheckResultWithMessage(WriteControllerInfos(writer, linControllers), "Could not write LIN controllers.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceSendConnectOk();
    }

    return Result::Ok;
}

[[nodiscard]] Result V2::Protocol::SendConnectOk(ChannelWriter& writer,
                                                 uint32_t protocolVersion,
                                                 Mode clientMode,
                                                 SimulationTime stepSize,
                                                 SimulationState simulationState,
                                                 const std::vector<IoSignalContainer>& incomingSignals,
                                                 const std::vector<IoSignalContainer>& outgoingSignals,
                                                 const std::vector<CanControllerContainer>& canControllers,
                                                 const std::vector<EthControllerContainer>& ethControllers,
                                                 const std::vector<LinControllerContainer>& linControllers,
                                                 const std::vector<FrControllerContainer>& frControllers) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceSendConnectOk(protocolVersion,
                                           clientMode,
                                           stepSize,
                                           simulationState,
                                           incomingSignals,
                                           outgoingSignals,
                                           canControllers,
                                           ethControllers,
                                           linControllers,
                                           frControllers);
    }

    constexpr size_t size =
        sizeof(FrameKind) + sizeof(protocolVersion) + sizeof(clientMode) + sizeof(stepSize) + sizeof(simulationState);

    BlockWriter blockWriter;
    CheckResultWithMessage(writer.Reserve(size, blockWriter), "Could not reserve memory for ConnectOk frame.");

    blockWriter.Write(FrameKind::ConnectOk);
    blockWriter.Write(protocolVersion);
    blockWriter.Write(clientMode);
    blockWriter.Write(stepSize);
    blockWriter.Write(simulationState);

    CheckResultWithMessage(WriteIoSignalInfos(writer, incomingSignals), "Could not write incoming signals.");
    CheckResultWithMessage(WriteIoSignalInfos(writer, outgoingSignals), "Could not write outgoing signals.");
    CheckResultWithMessage(V1::Protocol::WriteControllerInfos(writer, canControllers),
                           "Could not write CAN controllers.");
    CheckResultWithMessage(V1::Protocol::WriteControllerInfos(writer, ethControllers),
                           "Could not write ETH controllers.");
    CheckResultWithMessage(V1::Protocol::WriteControllerInfos(writer, linControllers),
                           "Could not write LIN controllers.");
    CheckResultWithMessage(WriteControllerInfos(writer, frControllers), "Could not write FLEXRAY controllers.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceSendConnectOk();
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::ReadStart(ChannelReader& reader, SimulationTime& simulationTime) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceReadStart();
    }

    CheckResultWithMessage(reader.Read(simulationTime), "Could not read simulation time.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceReadStart(simulationTime);
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::SendStart(ChannelWriter& writer, SimulationTime simulationTime) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceSendStart(simulationTime);
    }

    constexpr size_t size = sizeof(FrameKind) + sizeof(simulationTime);

    BlockWriter blockWriter;
    CheckResultWithMessage(writer.Reserve(size, blockWriter), "Could not reserve memory for Start frame.");

    blockWriter.Write(FrameKind::Start);
    blockWriter.Write(simulationTime);

    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceSendStart();
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::ReadStop(ChannelReader& reader, SimulationTime& simulationTime) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceReadStop();
    }

    CheckResultWithMessage(reader.Read(simulationTime), "Could not read simulation time.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceReadStop(simulationTime);
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::SendStop(ChannelWriter& writer, SimulationTime simulationTime) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceSendStop(simulationTime);
    }

    constexpr size_t size = sizeof(FrameKind) + sizeof(simulationTime);

    BlockWriter blockWriter;
    CheckResultWithMessage(writer.Reserve(size, blockWriter), "Could not reserve memory for Stop frame.");

    blockWriter.Write(FrameKind::Stop);
    blockWriter.Write(simulationTime);

    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceSendStop();
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::ReadTerminate(ChannelReader& reader,
                                                 SimulationTime& simulationTime,
                                                 TerminateReason& reason) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceReadTerminate();
    }

    constexpr size_t size = sizeof(simulationTime) + sizeof(reason);

    BlockReader blockReader;
    CheckResultWithMessage(reader.ReadBlock(size, blockReader), "Could not read block for Terminate frame.");

    blockReader.Read(simulationTime);
    blockReader.Read(reason);

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceReadTerminate(simulationTime, reason);
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::SendTerminate(ChannelWriter& writer,
                                                 SimulationTime simulationTime,
                                                 TerminateReason reason) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceSendTerminate(simulationTime, reason);
    }

    constexpr size_t size = sizeof(FrameKind) + sizeof(simulationTime) + sizeof(reason);

    BlockWriter blockWriter;
    CheckResultWithMessage(writer.Reserve(size, blockWriter), "Could not reserve memory for Terminate frame.");

    blockWriter.Write(FrameKind::Terminate);
    blockWriter.Write(simulationTime);
    blockWriter.Write(reason);

    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceSendTerminate();
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::ReadPause(ChannelReader& reader, SimulationTime& simulationTime) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceReadPause();
    }

    CheckResultWithMessage(reader.Read(simulationTime), "Could not read simulation time.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceReadPause(simulationTime);
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::SendPause(ChannelWriter& writer, SimulationTime simulationTime) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceSendPause(simulationTime);
    }

    constexpr size_t size = sizeof(FrameKind) + sizeof(simulationTime);

    BlockWriter blockWriter;
    CheckResultWithMessage(writer.Reserve(size, blockWriter), "Could not reserve memory for Pause frame.");

    blockWriter.Write(FrameKind::Pause);
    blockWriter.Write(simulationTime);

    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceSendPause();
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::ReadContinue(ChannelReader& reader, SimulationTime& simulationTime) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceReadContinue();
    }

    CheckResultWithMessage(reader.Read(simulationTime), "Could not read simulation time.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceReadContinue(simulationTime);
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::SendContinue(ChannelWriter& writer, SimulationTime simulationTime) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceSendContinue(simulationTime);
    }

    constexpr size_t size = sizeof(FrameKind) + sizeof(simulationTime);

    BlockWriter blockWriter;
    CheckResultWithMessage(writer.Reserve(size, blockWriter), "Could not reserve memory for Continue frame.");

    blockWriter.Write(FrameKind::Continue);
    blockWriter.Write(simulationTime);

    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceSendContinue();
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::ReadStep(ChannelReader& reader,
                                            SimulationTime& simulationTime,
                                            const DeserializeFunction& deserializeIoData,
                                            const DeserializeFunction& deserializeBusMessages,
                                            const Callbacks& callbacks) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceReadStep();
    }

    CheckResultWithMessage(reader.Read(simulationTime), "Could not read simulation time.");

    if (callbacks.simulationBeginStepCallback) {
        callbacks.simulationBeginStepCallback(simulationTime);
    }

    CheckResultWithMessage(deserializeIoData(reader, simulationTime, callbacks), "Could not read IO buffer data.");
    CheckResultWithMessage(deserializeBusMessages(reader, simulationTime, callbacks),
                           "Could not read bus buffer data.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceReadStep(simulationTime);
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::SendStep(ChannelWriter& writer,
                                            SimulationTime simulationTime,
                                            const SerializeFunction& serializeIoData,
                                            const SerializeFunction& serializeBusMessages) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceSendStep(simulationTime);
    }

    constexpr size_t size = sizeof(FrameKind) + sizeof(simulationTime);

    BlockWriter blockWriter;
    CheckResultWithMessage(writer.Reserve(size, blockWriter), "Could not reserve memory for Step frame.");

    blockWriter.Write(FrameKind::Step);
    blockWriter.Write(simulationTime);

    CheckResultWithMessage(serializeIoData(writer), "Could not write IO buffer data.");
    CheckResultWithMessage(serializeBusMessages(writer), "Could not write bus buffer data.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceSendStep();
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::ReadStepOk(ChannelReader& reader,
                                              SimulationTime& nextSimulationTime,
                                              Command& command,
                                              const DeserializeFunction& deserializeIoData,
                                              const DeserializeFunction& deserializeBusMessages,
                                              const Callbacks& callbacks) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceReadStepOk();
    }

    constexpr size_t size = sizeof(nextSimulationTime) + sizeof(command);

    BlockReader blockReader;
    CheckResultWithMessage(reader.ReadBlock(size, blockReader), "Could not read block for StepOk frame.");

    blockReader.Read(nextSimulationTime);
    blockReader.Read(command);

    if (callbacks.simulationBeginStepCallback) {
        callbacks.simulationBeginStepCallback(nextSimulationTime);
    }

    CheckResultWithMessage(deserializeIoData(reader, nextSimulationTime, callbacks), "Could not read IO buffer data.");
    CheckResultWithMessage(deserializeBusMessages(reader, nextSimulationTime, callbacks),
                           "Could not read bus buffer data.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceReadStepOk(nextSimulationTime, command);
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::SendStepOk(ChannelWriter& writer,
                                              SimulationTime nextSimulationTime,
                                              Command command,
                                              const SerializeFunction& serializeIoData,
                                              const SerializeFunction& serializeBusMessages) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceSendStepOk(nextSimulationTime, command);
    }

    constexpr size_t size = sizeof(FrameKind) + sizeof(nextSimulationTime) + sizeof(command);

    BlockWriter blockWriter;
    CheckResultWithMessage(writer.Reserve(size, blockWriter), "Could not reserve memory for StepOk frame.");

    blockWriter.Write(FrameKind::StepOk);
    blockWriter.Write(nextSimulationTime);
    blockWriter.Write(command);

    CheckResultWithMessage(serializeIoData(writer), "Could not write IO buffer data.");
    CheckResultWithMessage(serializeBusMessages(writer), "Could not write bus buffer data.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceSendStepOk();
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::ReadSetPort(ChannelReader& reader, std::string& serverName, uint16_t& port) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceReadSetPort();
    }

    CheckResultWithMessage(V1::Protocol::ReadString(reader, serverName), "Could not read server name.");
    CheckResultWithMessage(reader.Read(port), "Could not read port.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceReadSetPort(serverName, port);
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::SendSetPort(ChannelWriter& writer, const std::string& serverName, uint16_t port) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceSendSetPort(serverName, port);
    }

    CheckResultWithMessage(writer.Write(FrameKind::SetPort), "Could not write frame kind.");
    CheckResultWithMessage(WriteString(writer, serverName), "Could not write server name.");
    CheckResultWithMessage(writer.Write(port), "Could not write port.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceSendSetPort();
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::ReadUnsetPort(ChannelReader& reader, std::string& serverName) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceReadUnsetPort();
    }

    CheckResultWithMessage(ReadString(reader, serverName), "Could not read server name.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceReadUnsetPort(serverName);
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::SendUnsetPort(ChannelWriter& writer, const std::string& serverName) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceSendUnsetPort(serverName);
    }

    CheckResultWithMessage(writer.Write(FrameKind::UnsetPort), "Could not write frame kind.");
    CheckResultWithMessage(WriteString(writer, serverName), "Could not write server name.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceSendUnsetPort();
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::ReadGetPort(ChannelReader& reader, std::string& serverName) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceReadGetPort();
    }

    CheckResultWithMessage(ReadString(reader, serverName), "Could not read server name.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceReadGetPort(serverName);
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::SendGetPort(ChannelWriter& writer, const std::string& serverName) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceSendGetPort(serverName);
    }

    CheckResultWithMessage(writer.Write(FrameKind::GetPort), "Could not write frame kind.");
    CheckResultWithMessage(WriteString(writer, serverName), "Could not write server name.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceSendGetPort();
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::ReadGetPortOk(ChannelReader& reader, uint16_t& port) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceReadGetPortOk();
    }

    CheckResultWithMessage(reader.Read(port), "Could not read port.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceReadGetPortOk(port);
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::SendGetPortOk(ChannelWriter& writer, uint16_t port) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceSendGetPortOk(port);
    }

    constexpr size_t size = sizeof(FrameKind) + sizeof(port);

    BlockWriter blockWriter;
    CheckResultWithMessage(writer.Reserve(size, blockWriter), "Could not reserve memory for GetPortOk frame.");

    blockWriter.Write(FrameKind::GetPortOk);
    blockWriter.Write(port);

    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceSendGetPortOk();
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::ReadString(ChannelReader& reader, std::string& string) {
    size_t size{};
    CheckResultWithMessage(ReadSize(reader, size), "Could not read string size.");

    string.resize(size);

    CheckResultWithMessage(reader.Read(string.data(), size), "Could not read string data.");
    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::WriteString(ChannelWriter& writer, const std::string& string) {
    CheckResultWithMessage(WriteSize(writer, string.size()), "Could not write string size.");
    CheckResultWithMessage(writer.Write(string.data(), string.size()), "Could not write string data.");
    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::ReadIoSignalInfo(ChannelReader& reader, IoSignalContainer& signal) {
    BlockReader blockReader;
    CheckResultWithMessage(reader.ReadBlock(IoSignalInfoSize, blockReader),
                           "Could not read block for IoSignalContainer.");

    blockReader.Read(signal.id);
    blockReader.Read(signal.length);
    blockReader.Read(signal.dataType);
    blockReader.Read(signal.sizeKind);

    CheckResultWithMessage(ReadString(reader, signal.name), "Could not read name.");
    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::WriteIoSignalInfo(ChannelWriter& writer, const IoSignalContainer& signal) {
    BlockWriter blockWriter;
    CheckResultWithMessage(writer.Reserve(IoSignalInfoSize, blockWriter),
                           "Could not reserve memory for IoSignalContainer.");

    blockWriter.Write(signal.id);
    blockWriter.Write(signal.length);
    blockWriter.Write(signal.dataType);
    blockWriter.Write(signal.sizeKind);

    CheckResultWithMessage(WriteString(writer, signal.name), "Could not write name.");
    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::ReadIoSignalInfos(ChannelReader& reader, std::vector<IoSignalContainer>& signals) {
    size_t size{};
    CheckResultWithMessage(ReadSize(reader, size), "Could not read signals count.");

    signals.resize(size);

    for (size_t i = 0; i < size; i++) {
        CheckResultWithMessage(ReadIoSignalInfo(reader, signals[i]), "Could not read signal info.");
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::WriteIoSignalInfos(ChannelWriter& writer,
                                                      const std::vector<IoSignalContainer>& signals) {
    CheckResultWithMessage(WriteSize(writer, signals.size()), "Could not write signals count.");

    for (const auto& signal : signals) {
        CheckResultWithMessage(WriteIoSignalInfo(writer, signal), "Could not write signal info.");
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::ReadControllerInfo(ChannelReader& reader, CanControllerContainer& controller) {
    BlockReader blockReader;
    CheckResultWithMessage(reader.ReadBlock(CanControllerSize, blockReader),
                           "Could not read block for CanControllerContainer.");

    blockReader.Read(controller.id);
    blockReader.Read(controller.queueSize);
    blockReader.Read(controller.bitsPerSecond);
    blockReader.Read(controller.flexibleDataRateBitsPerSecond);

    CheckResultWithMessage(ReadString(reader, controller.name), "Could not read name.");
    CheckResultWithMessage(ReadString(reader, controller.channelName), "Could not read channel name.");
    CheckResultWithMessage(ReadString(reader, controller.clusterName), "Could not read cluster name.");
    return Result::Ok;
}

[[nodiscard]] Result
V1::Protocol::WriteControllerInfo(ChannelWriter& writer, const CanControllerContainer& controller) {
    BlockWriter blockWriter;
    CheckResultWithMessage(writer.Reserve(CanControllerSize, blockWriter),
                           "Could not reserve memory for CanControllerContainer.");

    blockWriter.Write(controller.id);
    blockWriter.Write(controller.queueSize);
    blockWriter.Write(controller.bitsPerSecond);
    blockWriter.Write(controller.flexibleDataRateBitsPerSecond);

    CheckResultWithMessage(WriteString(writer, controller.name), "Could not write name.");
    CheckResultWithMessage(WriteString(writer, controller.channelName), "Could not write channel name.");
    CheckResultWithMessage(WriteString(writer, controller.clusterName), "Could not write cluster name.");
    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::ReadControllerInfos(ChannelReader& reader,
                                                       std::vector<CanControllerContainer>& controllers) {
    size_t size{};
    CheckResultWithMessage(ReadSize(reader, size), "Could not read controllers count.");

    controllers.resize(size);

    for (size_t i = 0; i < size; i++) {
        CheckResultWithMessage(ReadControllerInfo(reader, controllers[i]), "Could not read controller.");
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::WriteControllerInfos(ChannelWriter& writer,
                                                        const std::vector<CanControllerContainer>& controllers) {
    CheckResultWithMessage(WriteSize(writer, controllers.size()), "Could not write controllers count.");

    for (const auto& controller : controllers) {
        CheckResultWithMessage(WriteControllerInfo(writer, controller), "Could not write controller.");
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::ReadControllerInfo(ChannelReader& reader, EthControllerContainer& controller) {
    BlockReader blockReader;
    CheckResultWithMessage(reader.ReadBlock(EthControllerSize, blockReader),
                           "Could not read block for EthControllerContainer.");

    blockReader.Read(controller.id);
    blockReader.Read(controller.queueSize);
    blockReader.Read(controller.bitsPerSecond);
    blockReader.Read(controller.macAddress.data(), controller.macAddress.size());

    CheckResultWithMessage(ReadString(reader, controller.name), "Could not read name.");
    CheckResultWithMessage(ReadString(reader, controller.channelName), "Could not read channel name.");
    CheckResultWithMessage(ReadString(reader, controller.clusterName), "Could not read cluster name.");
    return Result::Ok;
}

[[nodiscard]] Result
V1::Protocol::WriteControllerInfo(ChannelWriter& writer, const EthControllerContainer& controller) {
    BlockWriter blockWriter;
    CheckResultWithMessage(writer.Reserve(EthControllerSize, blockWriter),
                           "Could not reserve memory for EthControllerContainer.");

    blockWriter.Write(controller.id);
    blockWriter.Write(controller.queueSize);
    blockWriter.Write(controller.bitsPerSecond);
    blockWriter.Write(controller.macAddress.data(), controller.macAddress.size());

    CheckResultWithMessage(WriteString(writer, controller.name), "Could not write name.");
    CheckResultWithMessage(WriteString(writer, controller.channelName), "Could not write channel name.");
    CheckResultWithMessage(WriteString(writer, controller.clusterName), "Could not write cluster name.");
    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::ReadControllerInfos(ChannelReader& reader,
                                                       std::vector<EthControllerContainer>& controllers) {
    size_t size{};
    CheckResultWithMessage(ReadSize(reader, size), "Could not read controllers count.");

    controllers.resize(size);

    for (size_t i = 0; i < size; i++) {
        CheckResultWithMessage(ReadControllerInfo(reader, controllers[i]), "Could not read controller.");
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::WriteControllerInfos(ChannelWriter& writer,
                                                        const std::vector<EthControllerContainer>& controllers) {
    CheckResultWithMessage(WriteSize(writer, controllers.size()), "Could not write controllers count.");

    for (const auto& controller : controllers) {
        CheckResultWithMessage(WriteControllerInfo(writer, controller), "Could not write controller.");
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::ReadControllerInfo(ChannelReader& reader, LinControllerContainer& controller) {
    BlockReader blockReader;
    CheckResultWithMessage(reader.ReadBlock(LinControllerSize, blockReader),
                           "Could not read block for LinControllerContainer.");

    blockReader.Read(controller.id);
    blockReader.Read(controller.queueSize);
    blockReader.Read(controller.bitsPerSecond);
    blockReader.Read(controller.type);

    CheckResultWithMessage(ReadString(reader, controller.name), "Could not read name.");
    CheckResultWithMessage(ReadString(reader, controller.channelName), "Could not read channel name.");
    CheckResultWithMessage(ReadString(reader, controller.clusterName), "Could not read cluster name.");
    return Result::Ok;
}

[[nodiscard]] Result
V1::Protocol::WriteControllerInfo(ChannelWriter& writer, const LinControllerContainer& controller) {
    BlockWriter blockWriter;
    CheckResultWithMessage(writer.Reserve(LinControllerSize, blockWriter),
                           "Could not reserve memory for LinControllerContainer.");

    blockWriter.Write(controller.id);
    blockWriter.Write(controller.queueSize);
    blockWriter.Write(controller.bitsPerSecond);
    blockWriter.Write(controller.type);

    CheckResultWithMessage(WriteString(writer, controller.name), "Could not write name.");
    CheckResultWithMessage(WriteString(writer, controller.channelName), "Could not write channel name.");
    CheckResultWithMessage(WriteString(writer, controller.clusterName), "Could not write cluster name.");
    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::ReadControllerInfos(ChannelReader& reader,
                                                       std::vector<LinControllerContainer>& controllers) {
    size_t size{};
    CheckResultWithMessage(ReadSize(reader, size), "Could not read controllers count.");

    controllers.resize(size);

    for (size_t i = 0; i < size; i++) {
        CheckResultWithMessage(ReadControllerInfo(reader, controllers[i]), "Could not read controller.");
    }

    return Result::Ok;
}

[[nodiscard]] Result V1::Protocol::WriteControllerInfos(ChannelWriter& writer,
                                                        const std::vector<LinControllerContainer>& controllers) {
    CheckResultWithMessage(WriteSize(writer, controllers.size()), "Could not write controllers count.");

    for (const auto& controller : controllers) {
        CheckResultWithMessage(WriteControllerInfo(writer, controller), "Could not write controller.");
    }

    return Result::Ok;
}

[[nodiscard]] bool V1::Protocol::DoFlexRayOperations() {
    return false;
}

[[nodiscard]] Result V2::Protocol::ReadControllerInfo(ChannelReader& reader, FrControllerContainer& controller) {
    BlockReader blockReader;
    CheckResultWithMessage(reader.ReadBlock(FrControllerSize, blockReader),
                           "Could not read block for FrControllerContainer.");

    blockReader.Read(controller.id);
    blockReader.Read(controller.queueSize);
    blockReader.Read(controller.bitsPerSecond);

    CheckResultWithMessage(ReadString(reader, controller.name), "Could not read name.");
    CheckResultWithMessage(ReadString(reader, controller.channelName), "Could not read channel name.");
    CheckResultWithMessage(ReadString(reader, controller.clusterName), "Could not read cluster name.");
    return Result::Ok;
}

[[nodiscard]] Result V2::Protocol::WriteControllerInfo(ChannelWriter& writer, const FrControllerContainer& controller) {
    BlockWriter blockWriter;
    CheckResultWithMessage(writer.Reserve(FrControllerSize, blockWriter),
                           "Could not reserve memory for FrControllerContainer.");

    blockWriter.Write(controller.id);
    blockWriter.Write(controller.queueSize);
    blockWriter.Write(controller.bitsPerSecond);

    CheckResultWithMessage(WriteString(writer, controller.name), "Could not write name.");
    CheckResultWithMessage(WriteString(writer, controller.channelName), "Could not write channel name.");
    CheckResultWithMessage(WriteString(writer, controller.clusterName), "Could not write cluster name.");
    return Result::Ok;
}

[[nodiscard]] Result V2::Protocol::ReadControllerInfos(ChannelReader& reader,
                                                       std::vector<FrControllerContainer>& controllers) {
    size_t size{};
    CheckResultWithMessage(ReadSize(reader, size), "Could not read controllers count.");

    controllers.resize(size);

    for (size_t i = 0; i < size; i++) {
        CheckResultWithMessage(ReadControllerInfo(reader, controllers[i]), "Could not read controller.");
    }

    return Result::Ok;
}

[[nodiscard]] Result V2::Protocol::WriteControllerInfos(ChannelWriter& writer,
                                                        const std::vector<FrControllerContainer>& controllers) {
    CheckResultWithMessage(WriteSize(writer, controllers.size()), "Could not write controllers count.");

    for (const auto& controller : controllers) {
        CheckResultWithMessage(WriteControllerInfo(writer, controller), "Could not write controller.");
    }

    return Result::Ok;
}

[[nodiscard]] bool V2::Protocol::DoFlexRayOperations() {
    return true;
}

[[nodiscard]] uint32_t V1::Protocol::GetVersion() {
    return CoSimProtocolVersion;
}


FactoryResult MakeProtocol(uint32_t negotiatedVersion) {
    FactoryResult r{};

    if (negotiatedVersion >= V2_VERSION) {
        // V2 bevorzugen, wenn >= 0x20000
        auto p = std::make_shared<V2::Protocol>();
        if (!p) {
            r.error = FactoryError::ConstructionFailed;
            return r;
        }
        r.protocol = std::move(p);
        return r;
    }

    if (negotiatedVersion >= V1_VERSION) {
        auto p = std::make_shared<V1::Protocol>();
        if (!p) {
            r.error = FactoryError::ConstructionFailed;
            return r;
        }
        r.protocol = std::move(p);
        return r;
    }

    r.error = FactoryError::UnsupportedVersion;
    return r;
}
} // namespace DsVeosCoSim::Protocol
