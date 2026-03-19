// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "Protocol.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <fmt/format.h>

#include "Channel.hpp"
#include "CoSimTypes.hpp"
#include "Environment.hpp"

namespace DsVeosCoSim {

namespace {

constexpr size_t IoSignalInfoSize = sizeof(IoSignalId) + sizeof(uint32_t) + sizeof(DataType) + sizeof(SizeKind);
constexpr size_t CanControllerSize = sizeof(BusControllerId) + sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint64_t);
constexpr size_t EthControllerSize = sizeof(BusControllerId) + sizeof(uint32_t) + sizeof(uint64_t) + EthAddressLength;
constexpr size_t LinControllerSize = sizeof(BusControllerId) + sizeof(uint32_t) + sizeof(uint64_t) + sizeof(LinControllerType);
constexpr size_t FrControllerSize = sizeof(BusControllerId) + sizeof(uint32_t) + sizeof(uint64_t);
constexpr size_t CanMessageSize = sizeof(SimulationTime) + sizeof(BusControllerId) + sizeof(BusMessageId) + sizeof(CanMessageFlags) + sizeof(uint32_t);
constexpr size_t EthMessageSize = sizeof(SimulationTime) + sizeof(BusControllerId) + sizeof(EthMessageFlags) + sizeof(uint32_t);
constexpr size_t LinMessageSize = sizeof(SimulationTime) + sizeof(BusControllerId) + sizeof(BusMessageId) + sizeof(LinMessageFlags) + sizeof(uint32_t);
constexpr size_t FrMessageSize = sizeof(SimulationTime) + sizeof(BusControllerId) + sizeof(BusMessageId) + sizeof(FrMessageFlags) + sizeof(uint32_t);

}  // namespace

class ProtocolV1 : public IProtocol {
public:
    [[nodiscard]] Result ReadSize(ChannelReader& reader, size_t& size) override {
        uint32_t intSize{};
        CheckResultWithMessage(reader.Read(intSize), "Could not read size.");
        size = static_cast<size_t>(intSize);
        return Result::Ok;
    }

    [[nodiscard]] Result WriteSize(ChannelWriter& writer, size_t size) override {
        auto intSize = static_cast<uint32_t>(size);
        CheckResultWithMessage(writer.Write(intSize), "Could not write size.");
        return Result::Ok;
    }

    [[nodiscard]] Result ReadLength(ChannelReader& reader, uint32_t& length) override {
        CheckResultWithMessage(reader.Read(length), "Could not read length.");
        return Result::Ok;
    }

    [[nodiscard]] Result WriteLength(ChannelWriter& writer, uint32_t length) override {
        CheckResultWithMessage(writer.Write(length), "Could not write length.");
        return Result::Ok;
    }

    [[nodiscard]] Result ReadData(ChannelReader& reader, void* data, size_t size) override {
        CheckResultWithMessage(reader.Read(data, size), "Could not read data.");
        return Result::Ok;
    }

    [[nodiscard]] Result WriteData(ChannelWriter& writer, const void* data, size_t size) override {
        CheckResultWithMessage(writer.Write(data, size), "Could not write data.");
        return Result::Ok;
    }

    [[nodiscard]] Result ReadSignalId(ChannelReader& reader, IoSignalId& signalId) override {
        CheckResultWithMessage(reader.Read(signalId), "Could not read signal id.");
        return Result::Ok;
    }

    [[nodiscard]] Result WriteSignalId(ChannelWriter& writer, IoSignalId signalId) override {
        CheckResultWithMessage(writer.Write(signalId), "Could not write signal id.");
        return Result::Ok;
    }

    [[nodiscard]] Result ReadMessage(ChannelReader& reader, CanMessageContainer& messageContainer) override {
        BlockReader blockReader;
        CheckResultWithMessage(reader.ReadBlock(CanMessageSize, blockReader), "Could not read block for CanMessageContainer.");

        blockReader.Read(messageContainer.timestamp);
        blockReader.Read(messageContainer.controllerId);
        blockReader.Read(messageContainer.id);
        blockReader.Read(messageContainer.flags);
        blockReader.Read(messageContainer.length);
        blockReader.EndRead();

        CheckResult(messageContainer.Check());

        CheckResultWithMessage(reader.Read(messageContainer.data.data(), messageContainer.length), "Could not read data.");
        return Result::Ok;
    }

    [[nodiscard]] Result WriteMessage(ChannelWriter& writer, const CanMessageContainer& messageContainer) override {
        BlockWriter blockWriter;
        CheckResultWithMessage(writer.Reserve(CanMessageSize + messageContainer.length, blockWriter), "Could not reserve memory for CanMessageContainer.");

        blockWriter.Write(messageContainer.timestamp);
        blockWriter.Write(messageContainer.controllerId);
        blockWriter.Write(messageContainer.id);
        blockWriter.Write(messageContainer.flags);
        blockWriter.Write(messageContainer.length);
        blockWriter.Write(messageContainer.data.data(), messageContainer.length);
        blockWriter.EndWrite();
        return Result::Ok;
    }

    [[nodiscard]] Result ReadMessage(ChannelReader& reader, EthMessageContainer& messageContainer) override {
        BlockReader blockReader;
        CheckResultWithMessage(reader.ReadBlock(EthMessageSize, blockReader), "Could not read block for EthMessageContainer.");

        blockReader.Read(messageContainer.timestamp);
        blockReader.Read(messageContainer.controllerId);
        blockReader.Read(messageContainer.flags);
        blockReader.Read(messageContainer.length);
        blockReader.EndRead();

        CheckResult(messageContainer.Check());

        CheckResultWithMessage(reader.Read(messageContainer.data.data(), messageContainer.length), "Could not read data.");
        return Result::Ok;
    }

    [[nodiscard]] Result WriteMessage(ChannelWriter& writer, const EthMessageContainer& messageContainer) override {
        BlockWriter blockWriter;
        CheckResultWithMessage(writer.Reserve(EthMessageSize + messageContainer.length, blockWriter), "Could not reserve memory for EthMessageContainer.");

        blockWriter.Write(messageContainer.timestamp);
        blockWriter.Write(messageContainer.controllerId);
        blockWriter.Write(messageContainer.flags);
        blockWriter.Write(messageContainer.length);
        blockWriter.Write(messageContainer.data.data(), messageContainer.length);
        blockWriter.EndWrite();
        return Result::Ok;
    }

    [[nodiscard]] Result ReadMessage(ChannelReader& reader, LinMessageContainer& messageContainer) override {
        BlockReader blockReader;
        CheckResultWithMessage(reader.ReadBlock(LinMessageSize, blockReader), "Could not read block for LinMessageContainer.");

        blockReader.Read(messageContainer.timestamp);
        blockReader.Read(messageContainer.controllerId);
        blockReader.Read(messageContainer.id);
        blockReader.Read(messageContainer.flags);
        blockReader.Read(messageContainer.length);
        blockReader.EndRead();

        CheckResult(messageContainer.Check());

        CheckResultWithMessage(reader.Read(messageContainer.data.data(), messageContainer.length), "Could not read data.");
        return Result::Ok;
    }

    [[nodiscard]] Result WriteMessage(ChannelWriter& writer, const LinMessageContainer& messageContainer) override {
        BlockWriter blockWriter;
        CheckResultWithMessage(writer.Reserve(LinMessageSize + messageContainer.length, blockWriter), "Could not reserve memory for LinMessageContainer.");

        blockWriter.Write(messageContainer.timestamp);
        blockWriter.Write(messageContainer.controllerId);
        blockWriter.Write(messageContainer.id);
        blockWriter.Write(messageContainer.flags);
        blockWriter.Write(messageContainer.length);
        blockWriter.Write(messageContainer.data.data(), messageContainer.length);
        blockWriter.EndWrite();
        return Result::Ok;
    }

    [[nodiscard]] Result ReadMessage([[maybe_unused]] ChannelReader& reader, [[maybe_unused]] FrMessageContainer& messageContainer) override {
        // V1 does not have this functionality
        return Result::Ok;
    }

    [[nodiscard]] Result WriteMessage([[maybe_unused]] ChannelWriter& writer, [[maybe_unused]] const FrMessageContainer& messageContainer) override {
        // V1 does not have this functionality
        return Result::Ok;
    }

    [[nodiscard]] Result ReceiveHeader(ChannelReader& reader, FrameKind& frameKind) override {
        if (IsProtocolHeaderTracingEnabled()) {
            Logger::Instance().LogProtBegin("ReceiveHeader()");
        }

        CheckResultWithMessage(reader.Read(frameKind), "Could not receive frame kind.");

        if (IsProtocolHeaderTracingEnabled()) {
            Logger::Instance().LogProtEnd("ReceiveHeader(FrameKind: {})", frameKind);
        }

        return Result::Ok;
    }

    [[nodiscard]] Result ReadOk(ChannelReader& reader) override {
        reader.EndRead();

        return Result::Ok;
    }

    [[nodiscard]] Result SendOk(ChannelWriter& writer) override {
        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtBegin("SendOk()");
        }

        CheckResultWithMessage(writer.Write(FrameKind::Ok), "Could not write frame kind.");
        CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtEnd("SendOk()");
        }

        return Result::Ok;
    }

    [[nodiscard]] Result ReadError(ChannelReader& reader, std::string& errorMessage) override {
        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtBegin("ReadError()");
        }

        CheckResultWithMessage(ReadString(reader, errorMessage), "Could not read error message.");
        reader.EndRead();

        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtEnd(R"(ReadError(ErrorMessage: "{}"))", errorMessage);
        }

        return Result::Ok;
    }

    [[nodiscard]] Result SendError(ChannelWriter& writer, const std::string& errorMessage) override {
        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtBegin(R"(SendError(ErrorMessage: "{}"))", errorMessage);
        }

        CheckResultWithMessage(writer.Write(FrameKind::Error), "Could not write frame kind.");
        CheckResultWithMessage(WriteString(writer, errorMessage), "Could not write error message.");
        CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtEnd("SendError()");
        }

        return Result::Ok;
    }

    [[nodiscard]] Result ReadPing([[maybe_unused]] ChannelReader& reader, [[maybe_unused]] std::chrono::nanoseconds& roundTripTime) override {
        return Result::Ok;
    }

    [[nodiscard]] Result SendPing(ChannelWriter& writer, std::chrono::nanoseconds roundTripTime) override {
        if (IsProtocolPingTracingEnabled()) {
            Logger::Instance().LogProtBegin("SendPing(RoundTripTime: {} s)", SimulationTimeToString(roundTripTime));
        }

        CheckResultWithMessage(writer.Write(FrameKind::Ping), "Could not write frame kind.");
        CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

        if (IsProtocolPingTracingEnabled()) {
            Logger::Instance().LogProtEnd("SendPing()");
        }

        return Result::Ok;
    }

    [[nodiscard]] Result ReadPingOk(ChannelReader& reader, Command& command) override {
        if (IsProtocolPingTracingEnabled()) {
            Logger::Instance().LogProtBegin("ReadPingOk()");
        }

        CheckResultWithMessage(reader.Read(command), "Could not read command.");
        reader.EndRead();

        if (IsProtocolPingTracingEnabled()) {
            Logger::Instance().LogProtEnd("ReadPingOk(Command: {})", command);
        }

        return Result::Ok;
    }

    [[nodiscard]] Result SendPingOk(ChannelWriter& writer, Command command) override {
        if (IsProtocolPingTracingEnabled()) {
            Logger::Instance().LogProtBegin("SendPingOk(Command: {})", command);
        }

        constexpr size_t size = sizeof(FrameKind) + sizeof(command);

        BlockWriter blockWriter;
        CheckResultWithMessage(writer.Reserve(size, blockWriter), "Could not reserve memory for PingOk frame.");

        blockWriter.Write(FrameKind::PingOk);
        blockWriter.Write(command);
        blockWriter.EndWrite();

        CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

        if (IsProtocolPingTracingEnabled()) {
            Logger::Instance().LogProtEnd("SendPingOk()");
        }

        return Result::Ok;
    }

    [[nodiscard]] Result ReadConnect(ChannelReader& reader,
                                     uint32_t& protocolVersion,
                                     Mode& clientMode,
                                     std::string& serverName,
                                     std::string& clientName) override {
        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtBegin("ReadConnect()");
        }

        constexpr size_t size = sizeof(protocolVersion) + sizeof(clientMode);

        BlockReader blockReader;
        CheckResultWithMessage(reader.ReadBlock(size, blockReader), "Could not read block for Connect frame.");

        blockReader.Read(protocolVersion);
        blockReader.Read(clientMode);
        blockReader.EndRead();

        CheckResultWithMessage(ReadString(reader, serverName), "Could not read server name.");
        CheckResultWithMessage(ReadString(reader, clientName), "Could not read client name.");
        reader.EndRead();

        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtEnd(R"(ReadConnect(ProtocolVersion: {}, ClientMode: {}, ServerName: "{}", ClientName: "{}"))",
                                          protocolVersion,
                                          clientMode,
                                          serverName,
                                          clientName);
        }

        return Result::Ok;
    }

    [[nodiscard]] Result SendConnect(ChannelWriter& writer,
                                     uint32_t protocolVersion,
                                     Mode clientMode,
                                     const std::string& serverName,
                                     const std::string& clientName) override {
        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtBegin(R"(SendConnect(ProtocolVersion: {}, ClientMode: {}, ServerName: "{}", ClientName: "{}"))",
                                            protocolVersion,
                                            clientMode,
                                            serverName,
                                            clientName);
        }

        constexpr size_t size = sizeof(FrameKind) + sizeof(protocolVersion) + sizeof(clientMode);

        BlockWriter blockWriter;
        CheckResultWithMessage(writer.Reserve(size, blockWriter), "Could not reserve memory for Connect frame.");

        blockWriter.Write(FrameKind::Connect);
        blockWriter.Write(protocolVersion);
        blockWriter.Write(clientMode);
        blockWriter.EndWrite();

        CheckResultWithMessage(WriteString(writer, serverName), "Could not write server name.");
        CheckResultWithMessage(WriteString(writer, clientName), "Could not write client name.");
        CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtEnd("SendConnect()");
        }

        return Result::Ok;
    }

    [[nodiscard]] Result ReadConnectOkVersion(ChannelReader& reader, uint32_t& protocolVersion) override {
        CheckResultWithMessage(reader.Read(protocolVersion), "Could not read protocolVersion block for ConnectOkVersion frame.");

        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtEnd("ReadConnectOk(ProtocolVersion: {})", protocolVersion);
        }

        return Result::Ok;
    }

    [[nodiscard]] Result ReadConnectOk(ChannelReader& reader,
                                       Mode& clientMode,
                                       SimulationTime& stepSize,
                                       SimulationState& simulationState,
                                       std::vector<IoSignalContainer>& incomingSignals,
                                       std::vector<IoSignalContainer>& outgoingSignals,
                                       std::vector<CanControllerContainer>& canControllers,
                                       std::vector<EthControllerContainer>& ethControllers,
                                       std::vector<LinControllerContainer>& linControllers,
                                       [[maybe_unused]] std::vector<FrControllerContainer>& frControllers) override {
        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtBegin("ReadConnectOk()");
        }

        constexpr size_t size = sizeof(clientMode) + sizeof(stepSize) + sizeof(simulationState);

        BlockReader blockReader;
        CheckResultWithMessage(reader.ReadBlock(size, blockReader), "Could not read block for ConnectOk frame.");

        blockReader.Read(clientMode);
        blockReader.Read(stepSize);
        blockReader.Read(simulationState);
        blockReader.EndRead();

        CheckResultWithMessage(ReadIoSignalInfos(reader, incomingSignals), "Could not read incoming signals.");
        CheckResultWithMessage(ReadIoSignalInfos(reader, outgoingSignals), "Could not read outgoing signals.");
        CheckResultWithMessage(ReadControllerInfos(reader, canControllers), "Could not read CAN controllers.");
        CheckResultWithMessage(ReadControllerInfos(reader, ethControllers), "Could not read Ethernet controllers.");
        CheckResultWithMessage(ReadControllerInfos(reader, linControllers), "Could not read LIN controllers.");
        reader.EndRead();

        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtEnd(
                "ClientMode: {}, StepSize: {} s, SimulationState: {}, IncomingSignals: {}, OutgoingSignals: {}, CanControllers: {}, EthControllers: {}, "
                "LinControllers: {})",
                clientMode,
                SimulationTimeToString(stepSize),
                simulationState,
                incomingSignals,
                outgoingSignals,
                canControllers,
                ethControllers,
                linControllers);
        }

        return Result::Ok;
    }

    [[nodiscard]] Result SendConnectOk(ChannelWriter& writer,
                                       uint32_t protocolVersion,
                                       Mode clientMode,
                                       SimulationTime stepSize,
                                       SimulationState simulationState,
                                       const std::vector<IoSignalContainer>& incomingSignals,
                                       const std::vector<IoSignalContainer>& outgoingSignals,
                                       const std::vector<CanControllerContainer>& canControllers,
                                       const std::vector<EthControllerContainer>& ethControllers,
                                       const std::vector<LinControllerContainer>& linControllers,
                                       [[maybe_unused]] const std::vector<FrControllerContainer>& frControllers) override {
        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtBegin(
                "SendConnectOk(ProtocolVersion: {}, ClientMode: {}, StepSize: {} s, SimulationState: {}, IncomingSignals: {}, OutgoingSignals: {}, "
                "CanControllers: {}, EthControllers: {}, LinControllers: {})",
                protocolVersion,
                clientMode,
                SimulationTimeToString(stepSize),
                simulationState,
                incomingSignals,
                outgoingSignals,
                canControllers,
                ethControllers,
                linControllers);
        }
        constexpr size_t size = sizeof(FrameKind) + sizeof(protocolVersion) + sizeof(clientMode) + sizeof(stepSize) + sizeof(simulationState);

        BlockWriter blockWriter;
        CheckResultWithMessage(writer.Reserve(size, blockWriter), "Could not reserve memory for ConnectOk frame.");

        blockWriter.Write(FrameKind::ConnectOk);
        blockWriter.Write(protocolVersion);
        blockWriter.Write(clientMode);
        blockWriter.Write(stepSize);
        blockWriter.Write(simulationState);
        blockWriter.EndWrite();

        CheckResultWithMessage(WriteIoSignalInfos(writer, incomingSignals), "Could not write incoming signals.");
        CheckResultWithMessage(WriteIoSignalInfos(writer, outgoingSignals), "Could not write outgoing signals.");
        CheckResultWithMessage(WriteControllerInfos(writer, canControllers), "Could not write CAN controllers.");
        CheckResultWithMessage(WriteControllerInfos(writer, ethControllers), "Could not write Ethernet controllers.");
        CheckResultWithMessage(WriteControllerInfos(writer, linControllers), "Could not write LIN controllers.");
        CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtEnd("SendConnectOk()");
        }

        return Result::Ok;
    }

    [[nodiscard]] Result ReadStart(ChannelReader& reader, SimulationTime& simulationTime) override {
        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtBegin("ReadStart()");
        }

        CheckResultWithMessage(reader.Read(simulationTime), "Could not read simulation time.");
        reader.EndRead();

        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtEnd("ReadStart(SimulationTime: {} s)", SimulationTimeToString(simulationTime));
        }

        return Result::Ok;
    }

    [[nodiscard]] Result SendStart(ChannelWriter& writer, SimulationTime simulationTime) override {
        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtBegin("SendStart(SimulationTime: {} s)", SimulationTimeToString(simulationTime));
        }

        constexpr size_t size = sizeof(FrameKind) + sizeof(simulationTime);

        BlockWriter blockWriter;
        CheckResultWithMessage(writer.Reserve(size, blockWriter), "Could not reserve memory for Start frame.");

        blockWriter.Write(FrameKind::Start);
        blockWriter.Write(simulationTime);
        blockWriter.EndWrite();

        CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtEnd("SendStart()");
        }

        return Result::Ok;
    }

    [[nodiscard]] Result ReadStop(ChannelReader& reader, SimulationTime& simulationTime) override {
        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtBegin("ReadStop()");
        }

        CheckResultWithMessage(reader.Read(simulationTime), "Could not read simulation time.");
        reader.EndRead();

        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtEnd("ReadStop(SimulationTime: {} s)", SimulationTimeToString(simulationTime));
        }

        return Result::Ok;
    }

    [[nodiscard]] Result SendStop(ChannelWriter& writer, SimulationTime simulationTime) override {
        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtBegin("SendStop(SimulationTime: {} s)", SimulationTimeToString(simulationTime));
        }

        constexpr size_t size = sizeof(FrameKind) + sizeof(simulationTime);

        BlockWriter blockWriter;
        CheckResultWithMessage(writer.Reserve(size, blockWriter), "Could not reserve memory for Stop frame.");

        blockWriter.Write(FrameKind::Stop);
        blockWriter.Write(simulationTime);
        blockWriter.EndWrite();

        CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtEnd("SendStop()");
        }

        return Result::Ok;
    }

    [[nodiscard]] Result ReadTerminate(ChannelReader& reader, SimulationTime& simulationTime, TerminateReason& reason) override {
        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtBegin("ReadTerminate()");
        }

        constexpr size_t size = sizeof(simulationTime) + sizeof(reason);

        BlockReader blockReader;
        CheckResultWithMessage(reader.ReadBlock(size, blockReader), "Could not read block for Terminate frame.");

        blockReader.Read(simulationTime);
        blockReader.Read(reason);
        blockReader.EndRead();

        reader.EndRead();

        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtEnd("ReadTerminate(SimulationTime: {} s, Reason: {})", SimulationTimeToString(simulationTime), reason);
        }

        return Result::Ok;
    }

    [[nodiscard]] Result SendTerminate(ChannelWriter& writer, SimulationTime simulationTime, TerminateReason reason) override {
        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtBegin("SendTerminate(SimulationTime: {} s, Reason: {})", SimulationTimeToString(simulationTime), reason);
        }

        constexpr size_t size = sizeof(FrameKind) + sizeof(simulationTime) + sizeof(reason);

        BlockWriter blockWriter;
        CheckResultWithMessage(writer.Reserve(size, blockWriter), "Could not reserve memory for Terminate frame.");

        blockWriter.Write(FrameKind::Terminate);
        blockWriter.Write(simulationTime);
        blockWriter.Write(reason);
        blockWriter.EndWrite();

        CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtEnd("SendTerminate()");
        }

        return Result::Ok;
    }

    [[nodiscard]] Result ReadPause(ChannelReader& reader, SimulationTime& simulationTime) override {
        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtBegin("ReadPause()");
        }

        CheckResultWithMessage(reader.Read(simulationTime), "Could not read simulation time.");
        reader.EndRead();

        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtEnd("ReadPause(SimulationTime: {} s)", SimulationTimeToString(simulationTime));
        }

        return Result::Ok;
    }

    [[nodiscard]] Result SendPause(ChannelWriter& writer, SimulationTime simulationTime) override {
        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtBegin("SendPause(SimulationTime: {} s)", SimulationTimeToString(simulationTime));
        }

        constexpr size_t size = sizeof(FrameKind) + sizeof(simulationTime);

        BlockWriter blockWriter;
        CheckResultWithMessage(writer.Reserve(size, blockWriter), "Could not reserve memory for Pause frame.");

        blockWriter.Write(FrameKind::Pause);
        blockWriter.Write(simulationTime);
        blockWriter.EndWrite();

        CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtEnd("SendPause()");
        }

        return Result::Ok;
    }

    [[nodiscard]] Result ReadContinue(ChannelReader& reader, SimulationTime& simulationTime) override {
        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtBegin("ReadContinue()");
        }

        CheckResultWithMessage(reader.Read(simulationTime), "Could not read simulation time.");
        reader.EndRead();

        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtEnd("ReadContinue(SimulationTime: {} s)", SimulationTimeToString(simulationTime));
        }

        return Result::Ok;
    }

    [[nodiscard]] Result SendContinue(ChannelWriter& writer, SimulationTime simulationTime) override {
        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtBegin("SendContinue(SimulationTime: {} s)", SimulationTimeToString(simulationTime));
        }

        constexpr size_t size = sizeof(FrameKind) + sizeof(simulationTime);

        BlockWriter blockWriter;
        CheckResultWithMessage(writer.Reserve(size, blockWriter), "Could not reserve memory for Continue frame.");

        blockWriter.Write(FrameKind::Continue);
        blockWriter.Write(simulationTime);
        blockWriter.EndWrite();

        CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtEnd("SendContinue()");
        }

        return Result::Ok;
    }

    [[nodiscard]] Result ReadStep(ChannelReader& reader,
                                  SimulationTime& simulationTime,
                                  const DeserializeFunction& deserializeIoData,
                                  const DeserializeFunction& deserializeBusMessages,
                                  const Callbacks& callbacks) override {
        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtBegin("ReadStep()");
        }

        CheckResultWithMessage(reader.Read(simulationTime), "Could not read simulation time.");

        if (callbacks.simulationBeginStepCallback) {
            callbacks.simulationBeginStepCallback(simulationTime);
        }

        CheckResultWithMessage(deserializeIoData(reader, simulationTime, callbacks), "Could not read IO buffer data.");
        CheckResultWithMessage(deserializeBusMessages(reader, simulationTime, callbacks), "Could not read bus buffer data.");
        reader.EndRead();

        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtEnd("ReadStep(SimulationTime: {} s)", SimulationTimeToString(simulationTime));
        }

        return Result::Ok;
    }

    [[nodiscard]] Result SendStep(ChannelWriter& writer,
                                  SimulationTime simulationTime,
                                  const SerializeFunction& serializeIoData,
                                  const SerializeFunction& serializeBusMessages) override {
        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtBegin("SendStep(SimulationTime: {} s)", SimulationTimeToString(simulationTime));
        }

        constexpr size_t size = sizeof(FrameKind) + sizeof(simulationTime);

        BlockWriter blockWriter;
        CheckResultWithMessage(writer.Reserve(size, blockWriter), "Could not reserve memory for Step frame.");

        blockWriter.Write(FrameKind::Step);
        blockWriter.Write(simulationTime);
        blockWriter.EndWrite();

        CheckResultWithMessage(serializeIoData(writer), "Could not write IO buffer data.");
        CheckResultWithMessage(serializeBusMessages(writer), "Could not write bus buffer data.");
        CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtEnd("SendStep()");
        }

        return Result::Ok;
    }

    [[nodiscard]] Result ReadStepOk(ChannelReader& reader,
                                    SimulationTime& nextSimulationTime,
                                    Command& command,
                                    const DeserializeFunction& deserializeIoData,
                                    const DeserializeFunction& deserializeBusMessages,
                                    const Callbacks& callbacks) override {
        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtBegin("ReadStepOk()");
        }

        constexpr size_t size = sizeof(nextSimulationTime) + sizeof(command);

        BlockReader blockReader;
        CheckResultWithMessage(reader.ReadBlock(size, blockReader), "Could not read block for StepOk frame.");

        blockReader.Read(nextSimulationTime);
        blockReader.Read(command);
        blockReader.EndRead();

        if (callbacks.simulationBeginStepCallback) {
            callbacks.simulationBeginStepCallback(nextSimulationTime);
        }

        CheckResultWithMessage(deserializeIoData(reader, nextSimulationTime, callbacks), "Could not read IO buffer data.");
        CheckResultWithMessage(deserializeBusMessages(reader, nextSimulationTime, callbacks), "Could not read bus buffer data.");
        reader.EndRead();

        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtEnd("ReadStepOk(NextSimulationTime: {} s, Command: {})", SimulationTimeToString(nextSimulationTime), command);
        }

        return Result::Ok;
    }

    [[nodiscard]] Result SendStepOk(ChannelWriter& writer,
                                    SimulationTime nextSimulationTime,
                                    Command command,
                                    const SerializeFunction& serializeIoData,
                                    const SerializeFunction& serializeBusMessages) override {
        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtBegin("SendStepOk(NextSimulationTime: {} s, Command: {})", SimulationTimeToString(nextSimulationTime), command);
        }

        constexpr size_t size = sizeof(FrameKind) + sizeof(nextSimulationTime) + sizeof(command);

        BlockWriter blockWriter;
        CheckResultWithMessage(writer.Reserve(size, blockWriter), "Could not reserve memory for StepOk frame.");

        blockWriter.Write(FrameKind::StepOk);
        blockWriter.Write(nextSimulationTime);
        blockWriter.Write(command);
        blockWriter.EndWrite();

        CheckResultWithMessage(serializeIoData(writer), "Could not write IO buffer data.");
        CheckResultWithMessage(serializeBusMessages(writer), "Could not write bus buffer data.");
        CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtEnd("SendStepOk()");
        }

        return Result::Ok;
    }

    [[nodiscard]] Result ReadSetPort(ChannelReader& reader, std::string& serverName, uint16_t& port) override {
        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtBegin("ReadSetPort()");
        }

        CheckResultWithMessage(ReadString(reader, serverName), "Could not read server name.");
        CheckResultWithMessage(reader.Read(port), "Could not read port.");
        reader.EndRead();

        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtEnd(R"(ReadSetPort(ServerName: "{}", Port: {}))", serverName, port);
        }

        return Result::Ok;
    }

    [[nodiscard]] Result SendSetPort(ChannelWriter& writer, const std::string& serverName, uint16_t port) override {
        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtBegin(R"(SendSetPort(ServerName: "{}", Port: {}))", serverName, port);
        }

        CheckResultWithMessage(writer.Write(FrameKind::SetPort), "Could not write frame kind.");
        CheckResultWithMessage(WriteString(writer, serverName), "Could not write server name.");
        CheckResultWithMessage(writer.Write(port), "Could not write port.");
        CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtEnd("SendSetPort()");
        }

        return Result::Ok;
    }

    [[nodiscard]] Result ReadUnsetPort(ChannelReader& reader, std::string& serverName) override {
        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtBegin("ReadUnsetPort()");
        }

        CheckResultWithMessage(ReadString(reader, serverName), "Could not read server name.");
        reader.EndRead();

        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtEnd(R"(ReadUnsetPort(ServerName: "{}"))", serverName);
        }

        return Result::Ok;
    }

    [[nodiscard]] Result SendUnsetPort(ChannelWriter& writer, const std::string& serverName) override {
        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtBegin(R"(SendUnsetPort(ServerName: "{}"))", serverName);
        }

        CheckResultWithMessage(writer.Write(FrameKind::UnsetPort), "Could not write frame kind.");
        CheckResultWithMessage(WriteString(writer, serverName), "Could not write server name.");
        CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtEnd("SendUnsetPort()");
        }

        return Result::Ok;
    }

    [[nodiscard]] Result ReadGetPort(ChannelReader& reader, std::string& serverName) override {
        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtBegin("ReadGetPort()");
        }

        CheckResultWithMessage(ReadString(reader, serverName), "Could not read server name.");
        reader.EndRead();

        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtEnd(R"(ReadGetPort(ServerName: "{}"))", serverName);
        }

        return Result::Ok;
    }

    [[nodiscard]] Result SendGetPort(ChannelWriter& writer, const std::string& serverName) override {
        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtBegin(R"(SendGetPort(ServerName: "{}"))", serverName);
        }

        CheckResultWithMessage(writer.Write(FrameKind::GetPort), "Could not write frame kind.");
        CheckResultWithMessage(WriteString(writer, serverName), "Could not write server name.");
        CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtEnd("SendGetPort()");
        }

        return Result::Ok;
    }

    [[nodiscard]] Result ReadGetPortOk(ChannelReader& reader, uint16_t& port) override {
        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtBegin("ReadGetPortOk()");
        }

        CheckResultWithMessage(reader.Read(port), "Could not read port.");
        reader.EndRead();

        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtEnd("ReadGetPortOk(Port: {})", port);
        }

        return Result::Ok;
    }

    [[nodiscard]] Result SendGetPortOk(ChannelWriter& writer, uint16_t port) override {
        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtBegin("SendGetPortOk(Port: {})", port);
        }

        constexpr size_t size = sizeof(FrameKind) + sizeof(port);

        BlockWriter blockWriter;
        CheckResultWithMessage(writer.Reserve(size, blockWriter), "Could not reserve memory for GetPortOk frame.");

        blockWriter.Write(FrameKind::GetPortOk);
        blockWriter.Write(port);
        blockWriter.EndWrite();

        CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtEnd("SendGetPortOk()");
        }

        return Result::Ok;
    }

    [[nodiscard]] uint32_t GetVersion() override {
        return ProtocolVersion1;
    }

    [[nodiscard]] bool DoFlexRayOperations() override {
        return false;
    }

protected:
    [[nodiscard]] Result ReadString(ChannelReader& reader, std::string& string) {
        size_t size{};
        CheckResultWithMessage(ReadSize(reader, size), "Could not read string size.");

        string.resize(size);

        CheckResultWithMessage(reader.Read(string.data(), size), "Could not read string data.");
        return Result::Ok;
    }

    [[nodiscard]] Result WriteString(ChannelWriter& writer, const std::string& string) {
        CheckResultWithMessage(WriteSize(writer, string.size()), "Could not write string size.");
        CheckResultWithMessage(writer.Write(string.data(), string.size()), "Could not write string data.");
        return Result::Ok;
    }

    [[nodiscard]] Result ReadIoSignalInfo(ChannelReader& reader, IoSignalContainer& signal) {
        BlockReader blockReader;
        CheckResultWithMessage(reader.ReadBlock(IoSignalInfoSize, blockReader), "Could not read block for IoSignalContainer.");

        blockReader.Read(signal.id);
        blockReader.Read(signal.length);
        blockReader.Read(signal.dataType);
        blockReader.Read(signal.sizeKind);
        blockReader.EndRead();

        CheckResultWithMessage(ReadString(reader, signal.name), "Could not read name.");
        return Result::Ok;
    }

    [[nodiscard]] Result WriteIoSignalInfo(ChannelWriter& writer, const IoSignalContainer& signal) {
        BlockWriter blockWriter;
        CheckResultWithMessage(writer.Reserve(IoSignalInfoSize, blockWriter), "Could not reserve memory for IoSignalContainer.");

        blockWriter.Write(signal.id);
        blockWriter.Write(signal.length);
        blockWriter.Write(signal.dataType);
        blockWriter.Write(signal.sizeKind);
        blockWriter.EndWrite();

        CheckResultWithMessage(WriteString(writer, signal.name), "Could not write name.");
        return Result::Ok;
    }

    [[nodiscard]] Result ReadIoSignalInfos(ChannelReader& reader, std::vector<IoSignalContainer>& signals) {
        size_t size{};
        CheckResultWithMessage(ReadSize(reader, size), "Could not read signals count.");

        signals.resize(size);

        for (size_t i = 0; i < size; i++) {
            CheckResultWithMessage(ReadIoSignalInfo(reader, signals[i]), "Could not read signal info.");
        }

        return Result::Ok;
    }

    [[nodiscard]] Result WriteIoSignalInfos(ChannelWriter& writer, const std::vector<IoSignalContainer>& signals) {
        CheckResultWithMessage(WriteSize(writer, signals.size()), "Could not write signals count.");

        for (const auto& signal : signals) {
            CheckResultWithMessage(WriteIoSignalInfo(writer, signal), "Could not write signal info.");
        }

        return Result::Ok;
    }

    [[nodiscard]] Result ReadControllerInfo(ChannelReader& reader, CanControllerContainer& controller) {
        BlockReader blockReader;
        CheckResultWithMessage(reader.ReadBlock(CanControllerSize, blockReader), "Could not read block for CanControllerContainer.");

        blockReader.Read(controller.id);
        blockReader.Read(controller.queueSize);
        blockReader.Read(controller.bitsPerSecond);
        blockReader.Read(controller.flexibleDataRateBitsPerSecond);
        blockReader.EndRead();

        CheckResultWithMessage(ReadString(reader, controller.name), "Could not read name.");
        CheckResultWithMessage(ReadString(reader, controller.channelName), "Could not read channel name.");
        CheckResultWithMessage(ReadString(reader, controller.clusterName), "Could not read cluster name.");
        return Result::Ok;
    }

    [[nodiscard]] Result WriteControllerInfo(ChannelWriter& writer, const CanControllerContainer& controller) {
        BlockWriter blockWriter;
        CheckResultWithMessage(writer.Reserve(CanControllerSize, blockWriter), "Could not reserve memory for CanControllerContainer.");

        blockWriter.Write(controller.id);
        blockWriter.Write(controller.queueSize);
        blockWriter.Write(controller.bitsPerSecond);
        blockWriter.Write(controller.flexibleDataRateBitsPerSecond);
        blockWriter.EndWrite();

        CheckResultWithMessage(WriteString(writer, controller.name), "Could not write name.");
        CheckResultWithMessage(WriteString(writer, controller.channelName), "Could not write channel name.");
        CheckResultWithMessage(WriteString(writer, controller.clusterName), "Could not write cluster name.");
        return Result::Ok;
    }

    [[nodiscard]] Result ReadControllerInfos(ChannelReader& reader, std::vector<CanControllerContainer>& controllers) {
        size_t size{};
        CheckResultWithMessage(ReadSize(reader, size), "Could not read controllers count.");

        controllers.resize(size);

        for (size_t i = 0; i < size; i++) {
            CheckResultWithMessage(ReadControllerInfo(reader, controllers[i]), "Could not read controller.");
        }

        return Result::Ok;
    }

    [[nodiscard]] Result WriteControllerInfos(ChannelWriter& writer, const std::vector<CanControllerContainer>& controllers) {
        CheckResultWithMessage(WriteSize(writer, controllers.size()), "Could not write controllers count.");

        for (const auto& controller : controllers) {
            CheckResultWithMessage(WriteControllerInfo(writer, controller), "Could not write controller.");
        }

        return Result::Ok;
    }

    [[nodiscard]] Result ReadControllerInfo(ChannelReader& reader, EthControllerContainer& controller) {
        BlockReader blockReader;
        CheckResultWithMessage(reader.ReadBlock(EthControllerSize, blockReader), "Could not read block for EthControllerContainer.");

        blockReader.Read(controller.id);
        blockReader.Read(controller.queueSize);
        blockReader.Read(controller.bitsPerSecond);
        blockReader.Read(controller.macAddress.data(), controller.macAddress.size());
        blockReader.EndRead();

        CheckResultWithMessage(ReadString(reader, controller.name), "Could not read name.");
        CheckResultWithMessage(ReadString(reader, controller.channelName), "Could not read channel name.");
        CheckResultWithMessage(ReadString(reader, controller.clusterName), "Could not read cluster name.");
        return Result::Ok;
    }

    [[nodiscard]] Result WriteControllerInfo(ChannelWriter& writer, const EthControllerContainer& controller) {
        BlockWriter blockWriter;
        CheckResultWithMessage(writer.Reserve(EthControllerSize, blockWriter), "Could not reserve memory for EthControllerContainer.");

        blockWriter.Write(controller.id);
        blockWriter.Write(controller.queueSize);
        blockWriter.Write(controller.bitsPerSecond);
        blockWriter.Write(controller.macAddress.data(), controller.macAddress.size());
        blockWriter.EndWrite();

        CheckResultWithMessage(WriteString(writer, controller.name), "Could not write name.");
        CheckResultWithMessage(WriteString(writer, controller.channelName), "Could not write channel name.");
        CheckResultWithMessage(WriteString(writer, controller.clusterName), "Could not write cluster name.");
        return Result::Ok;
    }

    [[nodiscard]] Result ReadControllerInfos(ChannelReader& reader, std::vector<EthControllerContainer>& controllers) {
        size_t size{};
        CheckResultWithMessage(ReadSize(reader, size), "Could not read controllers count.");

        controllers.resize(size);

        for (size_t i = 0; i < size; i++) {
            CheckResultWithMessage(ReadControllerInfo(reader, controllers[i]), "Could not read controller.");
        }

        return Result::Ok;
    }

    [[nodiscard]] Result WriteControllerInfos(ChannelWriter& writer, const std::vector<EthControllerContainer>& controllers) {
        CheckResultWithMessage(WriteSize(writer, controllers.size()), "Could not write controllers count.");

        for (const auto& controller : controllers) {
            CheckResultWithMessage(WriteControllerInfo(writer, controller), "Could not write controller.");
        }

        return Result::Ok;
    }

    [[nodiscard]] Result ReadControllerInfo(ChannelReader& reader, LinControllerContainer& controller) {
        BlockReader blockReader;
        CheckResultWithMessage(reader.ReadBlock(LinControllerSize, blockReader), "Could not read block for LinControllerContainer.");

        blockReader.Read(controller.id);
        blockReader.Read(controller.queueSize);
        blockReader.Read(controller.bitsPerSecond);
        blockReader.Read(controller.type);
        blockReader.EndRead();

        CheckResultWithMessage(ReadString(reader, controller.name), "Could not read name.");
        CheckResultWithMessage(ReadString(reader, controller.channelName), "Could not read channel name.");
        CheckResultWithMessage(ReadString(reader, controller.clusterName), "Could not read cluster name.");
        return Result::Ok;
    }

    [[nodiscard]] Result WriteControllerInfo(ChannelWriter& writer, const LinControllerContainer& controller) {
        BlockWriter blockWriter;
        CheckResultWithMessage(writer.Reserve(LinControllerSize, blockWriter), "Could not reserve memory for LinControllerContainer.");

        blockWriter.Write(controller.id);
        blockWriter.Write(controller.queueSize);
        blockWriter.Write(controller.bitsPerSecond);
        blockWriter.Write(controller.type);
        blockWriter.EndWrite();

        CheckResultWithMessage(WriteString(writer, controller.name), "Could not write name.");
        CheckResultWithMessage(WriteString(writer, controller.channelName), "Could not write channel name.");
        CheckResultWithMessage(WriteString(writer, controller.clusterName), "Could not write cluster name.");
        return Result::Ok;
    }

    [[nodiscard]] Result ReadControllerInfos(ChannelReader& reader, std::vector<LinControllerContainer>& controllers) {
        size_t size{};
        CheckResultWithMessage(ReadSize(reader, size), "Could not read controllers count.");

        controllers.resize(size);

        for (size_t i = 0; i < size; i++) {
            CheckResultWithMessage(ReadControllerInfo(reader, controllers[i]), "Could not read controller.");
        }

        return Result::Ok;
    }

    [[nodiscard]] Result WriteControllerInfos(ChannelWriter& writer, const std::vector<LinControllerContainer>& controllers) {
        CheckResultWithMessage(WriteSize(writer, controllers.size()), "Could not write controllers count.");

        for (const auto& controller : controllers) {
            CheckResultWithMessage(WriteControllerInfo(writer, controller), "Could not write controller.");
        }

        return Result::Ok;
    }
};

class ProtocolV2 : public ProtocolV1 {
public:
    [[nodiscard]] Result ReadConnectOk(ChannelReader& reader,
                                       Mode& clientMode,
                                       SimulationTime& stepSize,
                                       SimulationState& simulationState,
                                       std::vector<IoSignalContainer>& incomingSignals,
                                       std::vector<IoSignalContainer>& outgoingSignals,
                                       std::vector<CanControllerContainer>& canControllers,
                                       std::vector<EthControllerContainer>& ethControllers,
                                       std::vector<LinControllerContainer>& linControllers,
                                       std::vector<FrControllerContainer>& frControllers) override {
        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtBegin("ReadConnectOk()");
        }

        constexpr size_t size = sizeof(clientMode) + sizeof(stepSize) + sizeof(simulationState);

        BlockReader blockReader;
        CheckResultWithMessage(reader.ReadBlock(size, blockReader), "Could not read block for ConnectOk frame.");

        blockReader.Read(clientMode);
        blockReader.Read(stepSize);
        blockReader.Read(simulationState);
        blockReader.EndRead();

        CheckResultWithMessage(ReadIoSignalInfos(reader, incomingSignals), "Could not read incoming signals.");
        CheckResultWithMessage(ReadIoSignalInfos(reader, outgoingSignals), "Could not read outgoing signals.");
        CheckResultWithMessage(ProtocolV1::ReadControllerInfos(reader, canControllers), "Could not read CAN controllers.");
        CheckResultWithMessage(ProtocolV1::ReadControllerInfos(reader, ethControllers), "Could not read Ethernet controllers.");
        CheckResultWithMessage(ProtocolV1::ReadControllerInfos(reader, linControllers), "Could not read LIN controllers.");
        CheckResultWithMessage(ReadControllerInfos(reader, frControllers), "Could not read FlexRay controllers.");
        reader.EndRead();

        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtEnd(
                "ClientMode: {}, StepSize: {} s, SimulationState: {}, IncomingSignals: {}, OutgoingSignals: {}, CanControllers: {}, EthControllers: {}, "
                "LinControllers: {}, FrControllers: {})",
                clientMode,
                SimulationTimeToString(stepSize),
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

    [[nodiscard]] Result SendConnectOk(ChannelWriter& writer,
                                       uint32_t protocolVersion,
                                       Mode clientMode,
                                       SimulationTime stepSize,
                                       SimulationState simulationState,
                                       const std::vector<IoSignalContainer>& incomingSignals,
                                       const std::vector<IoSignalContainer>& outgoingSignals,
                                       const std::vector<CanControllerContainer>& canControllers,
                                       const std::vector<EthControllerContainer>& ethControllers,
                                       const std::vector<LinControllerContainer>& linControllers,
                                       const std::vector<FrControllerContainer>& frControllers) override {
        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtBegin(
                "SendConnectOk(ProtocolVersion: {}, ClientMode: {}, StepSize: {} s, SimulationState: {}, IncomingSignals: {}, OutgoingSignals: {}, "
                "CanControllers: {}, EthControllers: {}, LinControllers: {}, FrControllers: {})",
                protocolVersion,
                clientMode,
                SimulationTimeToString(stepSize),
                simulationState,
                incomingSignals,
                outgoingSignals,
                canControllers,
                ethControllers,
                linControllers,
                frControllers);
        }

        constexpr size_t size = sizeof(FrameKind) + sizeof(protocolVersion) + sizeof(clientMode) + sizeof(stepSize) + sizeof(simulationState);

        BlockWriter blockWriter;
        CheckResultWithMessage(writer.Reserve(size, blockWriter), "Could not reserve memory for ConnectOk frame.");

        blockWriter.Write(FrameKind::ConnectOk);
        blockWriter.Write(protocolVersion);
        blockWriter.Write(clientMode);
        blockWriter.Write(stepSize);
        blockWriter.Write(simulationState);
        blockWriter.EndWrite();

        CheckResultWithMessage(WriteIoSignalInfos(writer, incomingSignals), "Could not write incoming signals.");
        CheckResultWithMessage(WriteIoSignalInfos(writer, outgoingSignals), "Could not write outgoing signals.");
        CheckResultWithMessage(ProtocolV1::WriteControllerInfos(writer, canControllers), "Could not write CAN controllers.");
        CheckResultWithMessage(ProtocolV1::WriteControllerInfos(writer, ethControllers), "Could not write Ethernet controllers.");
        CheckResultWithMessage(ProtocolV1::WriteControllerInfos(writer, linControllers), "Could not write LIN controllers.");
        CheckResultWithMessage(WriteControllerInfos(writer, frControllers), "Could not write FlexRay controllers.");
        CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

        if (IsProtocolTracingEnabled()) {
            Logger::Instance().LogProtEnd("SendConnectOk()");
        }

        return Result::Ok;
    }

    [[nodiscard]] Result ReadPing(ChannelReader& reader, std::chrono::nanoseconds& roundTripTime) override {
        if (IsProtocolPingTracingEnabled()) {
            Logger::Instance().LogProtBegin("ReadPing()");
        }

        CheckResultWithMessage(reader.Read(roundTripTime), "Could not read round trip time.");
        reader.EndRead();

        if (IsProtocolPingTracingEnabled()) {
            Logger::Instance().LogProtEnd("ReadPing(RoundTripTime: {} s)", SimulationTimeToString(roundTripTime));
        }

        return Result::Ok;
    }

    [[nodiscard]] Result SendPing(ChannelWriter& writer, std::chrono::nanoseconds roundTripTime) override {
        if (IsProtocolPingTracingEnabled()) {
            Logger::Instance().LogProtBegin("SendPing(RoundTripTime: {} s)", SimulationTimeToString(roundTripTime));
        }

        constexpr size_t size = sizeof(FrameKind) + sizeof(roundTripTime);

        BlockWriter blockWriter;
        CheckResultWithMessage(writer.Reserve(size, blockWriter), "Could not reserve memory for Ping frame.");
        blockWriter.Write(FrameKind::Ping);
        blockWriter.Write(roundTripTime);
        blockWriter.EndWrite();

        CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

        if (IsProtocolPingTracingEnabled()) {
            Logger::Instance().LogProtEnd("SendPing()");
        }

        return Result::Ok;
    }

    [[nodiscard]] uint32_t GetVersion() override {
        return ProtocolVersion2;
    }

    [[nodiscard]] bool DoFlexRayOperations() override {
        return true;
    }

    [[nodiscard]] Result ReadControllerInfo(ChannelReader& reader, FrControllerContainer& controller) {
        BlockReader blockReader;
        CheckResultWithMessage(reader.ReadBlock(FrControllerSize, blockReader), "Could not read block for FrControllerContainer.");

        blockReader.Read(controller.id);
        blockReader.Read(controller.queueSize);
        blockReader.Read(controller.bitsPerSecond);
        blockReader.EndRead();

        CheckResultWithMessage(ReadString(reader, controller.name), "Could not read name.");
        CheckResultWithMessage(ReadString(reader, controller.channelName), "Could not read channel name.");
        CheckResultWithMessage(ReadString(reader, controller.clusterName), "Could not read cluster name.");
        return Result::Ok;
    }

    [[nodiscard]] Result WriteControllerInfo(ChannelWriter& writer, const FrControllerContainer& controller) {
        BlockWriter blockWriter;
        CheckResultWithMessage(writer.Reserve(FrControllerSize, blockWriter), "Could not reserve memory for FrControllerContainer.");

        blockWriter.Write(controller.id);
        blockWriter.Write(controller.queueSize);
        blockWriter.Write(controller.bitsPerSecond);
        blockWriter.EndWrite();

        CheckResultWithMessage(WriteString(writer, controller.name), "Could not write name.");
        CheckResultWithMessage(WriteString(writer, controller.channelName), "Could not write channel name.");
        CheckResultWithMessage(WriteString(writer, controller.clusterName), "Could not write cluster name.");
        return Result::Ok;
    }

    [[nodiscard]] Result ReadControllerInfos(ChannelReader& reader, std::vector<FrControllerContainer>& controllers) {
        size_t size{};
        CheckResultWithMessage(ReadSize(reader, size), "Could not read controllers count.");

        controllers.resize(size);

        for (size_t i = 0; i < size; i++) {
            CheckResultWithMessage(ReadControllerInfo(reader, controllers[i]), "Could not read controller.");
        }

        return Result::Ok;
    }

    [[nodiscard]] Result WriteControllerInfos(ChannelWriter& writer, const std::vector<FrControllerContainer>& controllers) {
        CheckResultWithMessage(WriteSize(writer, controllers.size()), "Could not write controllers count.");

        for (const auto& controller : controllers) {
            CheckResultWithMessage(WriteControllerInfo(writer, controller), "Could not write controller.");
        }

        return Result::Ok;
    }

    [[nodiscard]] Result ReadMessage(ChannelReader& reader, FrMessageContainer& messageContainer) override {
        BlockReader blockReader;
        CheckResultWithMessage(reader.ReadBlock(FrMessageSize, blockReader), "Could not read block for FrMessageContainer.");

        blockReader.Read(messageContainer.timestamp);
        blockReader.Read(messageContainer.controllerId);
        blockReader.Read(messageContainer.id);
        blockReader.Read(messageContainer.flags);
        blockReader.Read(messageContainer.length);
        blockReader.EndRead();

        CheckResult(messageContainer.Check());

        CheckResultWithMessage(reader.Read(messageContainer.data.data(), messageContainer.length), "Could not read data.");
        return Result::Ok;
    }

    [[nodiscard]] Result WriteMessage(ChannelWriter& writer, const FrMessageContainer& messageContainer) override {
        BlockWriter blockWriter;
        CheckResultWithMessage(writer.Reserve(FrMessageSize + messageContainer.length, blockWriter), "Could not reserve memory for FrMessageContainer.");

        blockWriter.Write(messageContainer.timestamp);
        blockWriter.Write(messageContainer.controllerId);
        blockWriter.Write(messageContainer.id);
        blockWriter.Write(messageContainer.flags);
        blockWriter.Write(messageContainer.length);
        blockWriter.Write(messageContainer.data.data(), messageContainer.length);
        blockWriter.EndWrite();
        return Result::Ok;
    }
};

[[nodiscard]] Result CreateProtocol(uint32_t negotiatedVersion, std::unique_ptr<IProtocol>& protocol) {
    if (negotiatedVersion >= ProtocolVersion2) {
        protocol = std::make_unique<ProtocolV2>();
        return Result::Ok;
    }

    if (negotiatedVersion >= ProtocolVersion1) {
        protocol = std::make_unique<ProtocolV1>();
        return Result::Ok;
    }

    Logger::Instance().LogError("Unsupported protocol version: {}.", negotiatedVersion);
    return Result::Error;
}

}  // namespace DsVeosCoSim
