// Copyright dSPACE GmbH. All rights reserved.

#include "Protocol.h"

#include <cstdint>
#include <string>
#include <vector>

#include "BusBuffer.h"
#include "Channel.h"
#include "CoSimHelper.h"
#include "DsVeosCoSim/CoSimTypes.h"
#include "Environment.h"
#include "IoBuffer.h"

namespace DsVeosCoSim::Protocol {

namespace {

[[nodiscard]] Result ReadString(ChannelReader& reader, std::string& string) {
    size_t size{};
    CheckResult(ReadSize(reader, size));

    string.resize(size);

    CheckResultWithMessage(reader.Read(string.data(), size), "Could not read string data.");
    return Result::Ok;
}

[[nodiscard]] Result WriteString(ChannelWriter& writer, const std::string& string) {
    CheckResult(WriteSize(writer, string.size()));
    CheckResultWithMessage(writer.Write(string.data(), string.size()), "Could not write string data.");
    return Result::Ok;
}

[[nodiscard]] Result ReadIoSignalInfo(ChannelReader& reader, IoSignalContainer& signal) {
    CheckResultWithMessage(reader.Read(signal.id), "Could not read id.");
    CheckResultWithMessage(reader.Read(signal.length), "Could not read length.");
    CheckResultWithMessage(reader.Read(signal.dataType), "Could not read data type.");
    CheckResultWithMessage(reader.Read(signal.sizeKind), "Could not read size kind.");
    CheckResultWithMessage(ReadString(reader, signal.name), "Could not read name.");
    return Result::Ok;
}

[[nodiscard]] Result WriteIoSignalInfo(ChannelWriter& writer, const IoSignalContainer& signal) {
    CheckResultWithMessage(writer.Write(signal.id), "Could not write id.");
    CheckResultWithMessage(writer.Write(signal.length), "Could not write length.");
    CheckResultWithMessage(writer.Write(signal.dataType), "Could not write data type.");
    CheckResultWithMessage(writer.Write(signal.sizeKind), "Could not write size kind.");
    CheckResultWithMessage(WriteString(writer, signal.name), "Could not write name.");
    return Result::Ok;
}

[[nodiscard]] Result ReadIoSignalInfos(ChannelReader& reader, std::vector<IoSignalContainer>& signals) {
    size_t size{};
    CheckResult(ReadSize(reader, size));

    signals.resize(size);

    for (size_t i = 0; i < size; i++) {
        CheckResultWithMessage(ReadIoSignalInfo(reader, signals[i]), "Could not read signal info.");
    }

    return Result::Ok;
}

[[nodiscard]] Result WriteIoSignalInfos(ChannelWriter& writer, const std::vector<IoSignalContainer>& signals) {
    CheckResult(WriteSize(writer, signals.size()));

    for (const auto& signal : signals) {
        CheckResultWithMessage(WriteIoSignalInfo(writer, signal), "Could not write signal info.");
    }

    return Result::Ok;
}

[[nodiscard]] Result ReadControllerInfo(ChannelReader& reader, CanControllerContainer& controller) {
    CheckResultWithMessage(reader.Read(controller.id), "Could not read controller id.");
    CheckResultWithMessage(reader.Read(controller.queueSize), "Could not read queue size.");
    CheckResultWithMessage(reader.Read(controller.bitsPerSecond), "Could not read bits per second.");
    CheckResultWithMessage(reader.Read(controller.flexibleDataRateBitsPerSecond),
                           "Could not read flexible data rate bits per second.");
    CheckResultWithMessage(ReadString(reader, controller.name), "Could not read name.");
    CheckResultWithMessage(ReadString(reader, controller.channelName), "Could not read channel name.");
    CheckResultWithMessage(ReadString(reader, controller.clusterName), "Could not read cluster name.");
    return Result::Ok;
}

[[nodiscard]] Result WriteControllerInfo(ChannelWriter& writer, const CanControllerContainer& controller) {
    CheckResultWithMessage(writer.Write(controller.id), "Could not write id.");
    CheckResultWithMessage(writer.Write(controller.queueSize), "Could not write queue size.");
    CheckResultWithMessage(writer.Write(controller.bitsPerSecond), "Could not write bits per second.");
    CheckResultWithMessage(writer.Write(controller.flexibleDataRateBitsPerSecond),
                           "Could not write flexible data rate bits per second.");
    CheckResultWithMessage(WriteString(writer, controller.name), "Could not write name.");
    CheckResultWithMessage(WriteString(writer, controller.channelName), "Could not write channel name.");
    CheckResultWithMessage(WriteString(writer, controller.clusterName), "Could not write cluster name.");
    return Result::Ok;
}

[[nodiscard]] Result ReadControllerInfos(ChannelReader& reader, std::vector<CanControllerContainer>& controllers) {
    size_t size{};
    CheckResult(ReadSize(reader, size));

    controllers.resize(size);

    for (size_t i = 0; i < size; i++) {
        CheckResultWithMessage(ReadControllerInfo(reader, controllers[i]), "Could not read controller.");
    }

    return Result::Ok;
}

[[nodiscard]] Result WriteControllerInfos(ChannelWriter& writer,
                                          const std::vector<CanControllerContainer>& controllers) {
    CheckResult(WriteSize(writer, controllers.size()));

    for (const auto& controller : controllers) {
        CheckResultWithMessage(WriteControllerInfo(writer, controller), "Could not write controller.");
    }

    return Result::Ok;
}

[[nodiscard]] Result ReadControllerInfo(ChannelReader& reader, EthControllerContainer& controller) {
    CheckResultWithMessage(reader.Read(controller.id), "Could not read id.");
    CheckResultWithMessage(reader.Read(controller.queueSize), "Could not read queue size.");
    CheckResultWithMessage(reader.Read(controller.bitsPerSecond), "Could not read bits per second.");
    CheckResultWithMessage(reader.Read(controller.macAddress.data(), sizeof(controller.macAddress)),
                           "Could not read mac address.");
    CheckResultWithMessage(ReadString(reader, controller.name), "Could not read name.");
    CheckResultWithMessage(ReadString(reader, controller.channelName), "Could not read channel name.");
    CheckResultWithMessage(ReadString(reader, controller.clusterName), "Could not read cluster name.");
    return Result::Ok;
}

[[nodiscard]] Result WriteControllerInfo(ChannelWriter& writer, const EthControllerContainer& controller) {
    CheckResultWithMessage(writer.Write(controller.id), "Could not write id.");
    CheckResultWithMessage(writer.Write(controller.queueSize), "Could not write queue size.");
    CheckResultWithMessage(writer.Write(controller.bitsPerSecond), "Could not write bits per second.");
    CheckResultWithMessage(writer.Write(controller.macAddress.data(), sizeof(controller.macAddress)),
                           "Could not write mac address.");
    CheckResultWithMessage(WriteString(writer, controller.name), "Could not write name.");
    CheckResultWithMessage(WriteString(writer, controller.channelName), "Could not write channel name.");
    CheckResultWithMessage(WriteString(writer, controller.clusterName), "Could not write cluster name.");
    return Result::Ok;
}

[[nodiscard]] Result ReadControllerInfos(ChannelReader& reader, std::vector<EthControllerContainer>& controllers) {
    size_t size{};
    CheckResult(ReadSize(reader, size));

    controllers.resize(size);

    for (size_t i = 0; i < size; i++) {
        CheckResultWithMessage(ReadControllerInfo(reader, controllers[i]), "Could not read controller.");
    }

    return Result::Ok;
}

[[nodiscard]] Result WriteControllerInfos(ChannelWriter& writer,
                                          const std::vector<EthControllerContainer>& controllers) {
    CheckResult(WriteSize(writer, controllers.size()));

    for (const auto& controller : controllers) {
        CheckResultWithMessage(WriteControllerInfo(writer, controller), "Could not write controller.");
    }

    return Result::Ok;
}

[[nodiscard]] Result ReadControllerInfo(ChannelReader& reader, LinControllerContainer& controller) {
    CheckResultWithMessage(reader.Read(controller.id), "Could not read id.");
    CheckResultWithMessage(reader.Read(controller.queueSize), "Could not read queue size.");
    CheckResultWithMessage(reader.Read(controller.bitsPerSecond), "Could not read bits per second.");
    CheckResultWithMessage(reader.Read(controller.type), "Could not read type.");
    CheckResultWithMessage(ReadString(reader, controller.name), "Could not read name.");
    CheckResultWithMessage(ReadString(reader, controller.channelName), "Could not read channel name.");
    CheckResultWithMessage(ReadString(reader, controller.clusterName), "Could not read cluster name.");
    return Result::Ok;
}

[[nodiscard]] Result WriteControllerInfo(ChannelWriter& writer, const LinControllerContainer& controller) {
    CheckResultWithMessage(writer.Write(controller.id), "Could not write id.");
    CheckResultWithMessage(writer.Write(controller.queueSize), "Could not write queue size.");
    CheckResultWithMessage(writer.Write(controller.bitsPerSecond), "Could not write bits per second.");
    CheckResultWithMessage(writer.Write(controller.type), "Could not write type.");
    CheckResultWithMessage(WriteString(writer, controller.name), "Could not write name.");
    CheckResultWithMessage(WriteString(writer, controller.channelName), "Could not write channel name.");
    CheckResultWithMessage(WriteString(writer, controller.clusterName), "Could not write cluster name.");
    return Result::Ok;
}

[[nodiscard]] Result ReadControllerInfos(ChannelReader& reader, std::vector<LinControllerContainer>& controllers) {
    size_t size{};
    CheckResult(ReadSize(reader, size));

    controllers.resize(size);

    for (size_t i = 0; i < size; i++) {
        CheckResultWithMessage(ReadControllerInfo(reader, controllers[i]), "Could not read controller.");
    }

    return Result::Ok;
}

[[nodiscard]] Result WriteControllerInfos(ChannelWriter& writer,
                                          const std::vector<LinControllerContainer>& controllers) {
    CheckResult(WriteSize(writer, controllers.size()));

    for (const auto& controller : controllers) {
        CheckResultWithMessage(WriteControllerInfo(writer, controller), "Could not write controller.");
    }

    return Result::Ok;
}

}  // namespace

[[nodiscard]] Result ReadSize(ChannelReader& reader, size_t& size) {
    uint32_t intSize{};
    CheckResultWithMessage(reader.Read(intSize), "Could not read size.");
    size = static_cast<size_t>(intSize);
    return Result::Ok;
}

[[nodiscard]] Result WriteSize(ChannelWriter& writer, size_t size) {
    auto intSize = static_cast<uint32_t>(size);
    CheckResultWithMessage(writer.Write(intSize), "Could not write size.");
    return Result::Ok;
}

[[nodiscard]] Result ReadLength(ChannelReader& reader, uint32_t& length) {
    CheckResultWithMessage(reader.Read(length), "Could not read length.");
    return Result::Ok;
}

[[nodiscard]] Result WriteLength(ChannelWriter& writer, uint32_t length) {
    CheckResultWithMessage(writer.Write(length), "Could not write length.");
    return Result::Ok;
}

[[nodiscard]] Result ReadSignalId(ChannelReader& reader, IoSignalId& signalId) {
    CheckResultWithMessage(reader.Read(signalId), "Could not read signal id.");
    return Result::Ok;
}

[[nodiscard]] Result WriteSignalId(ChannelWriter& writer, IoSignalId signalId) {
    CheckResultWithMessage(writer.Write(signalId), "Could not write signal id.");
    return Result::Ok;
}

[[nodiscard]] Result WriteMessage(ChannelWriter& writer, const CanMessageContainer& messageContainer) {
    CheckResultWithMessage(writer.Write(messageContainer.timestamp), "Could not write timestamp.");
    CheckResultWithMessage(writer.Write(messageContainer.controllerId), "Could not write controller id.");
    CheckResultWithMessage(writer.Write(messageContainer.id), "Could not write id.");
    CheckResultWithMessage(writer.Write(messageContainer.flags), "Could not write flags.");
    CheckResultWithMessage(writer.Write(messageContainer.length), "Could not write length.");
    CheckResultWithMessage(writer.Write(messageContainer.data.data(), messageContainer.length),
                           "Could not write data.");
    return Result::Ok;
}

[[nodiscard]] Result ReadMessage(ChannelReader& reader, CanMessageContainer& messageContainer) {
    CheckResultWithMessage(reader.Read(messageContainer.timestamp), "Could not read timestamp.");
    CheckResultWithMessage(reader.Read(messageContainer.controllerId), "Could not read controller id.");
    CheckResultWithMessage(reader.Read(messageContainer.id), "Could not read id.");
    CheckResultWithMessage(reader.Read(messageContainer.flags), "Could not read flags.");
    CheckResultWithMessage(reader.Read(messageContainer.length), "Could not read length.");

    CheckResult(messageContainer.Check());

    CheckResultWithMessage(reader.Read(messageContainer.data.data(), messageContainer.length), "Could not read data.");
    return Result::Ok;
}

[[nodiscard]] Result WriteMessage(ChannelWriter& writer, const EthMessageContainer& messageContainer) {
    CheckResultWithMessage(writer.Write(messageContainer.timestamp), "Could not write timestamp.");
    CheckResultWithMessage(writer.Write(messageContainer.controllerId), "Could not write controller id.");
    CheckResultWithMessage(writer.Write(messageContainer.flags), "Could not write flags.");
    CheckResultWithMessage(writer.Write(messageContainer.length), "Could not write length.");
    CheckResultWithMessage(writer.Write(messageContainer.data.data(), messageContainer.length),
                           "Could not write data.");
    return Result::Ok;
}

[[nodiscard]] Result ReadMessage(ChannelReader& reader, EthMessageContainer& messageContainer) {
    CheckResultWithMessage(reader.Read(messageContainer.timestamp), "Could not read timestamp.");
    CheckResultWithMessage(reader.Read(messageContainer.controllerId), "Could not read controller id.");
    CheckResultWithMessage(reader.Read(messageContainer.flags), "Could not read flags.");
    CheckResultWithMessage(reader.Read(messageContainer.length), "Could not read length.");

    CheckResult(messageContainer.Check());

    CheckResultWithMessage(reader.Read(messageContainer.data.data(), messageContainer.length), "Could not read data.");
    return Result::Ok;
}

[[nodiscard]] Result WriteMessage(ChannelWriter& writer, const LinMessageContainer& messageContainer) {
    CheckResultWithMessage(writer.Write(messageContainer.timestamp), "Could not write timestamp.");
    CheckResultWithMessage(writer.Write(messageContainer.controllerId), "Could not write controller id.");
    CheckResultWithMessage(writer.Write(messageContainer.id), "Could not write id.");
    CheckResultWithMessage(writer.Write(messageContainer.flags), "Could not write flags.");
    CheckResultWithMessage(writer.Write(messageContainer.length), "Could not write length.");
    CheckResultWithMessage(writer.Write(messageContainer.data.data(), messageContainer.length),
                           "Could not write data.");
    return Result::Ok;
}

[[nodiscard]] Result ReadMessage(ChannelReader& reader, LinMessageContainer& messageContainer) {
    CheckResultWithMessage(reader.Read(messageContainer.timestamp), "Could not read timestamp.");
    CheckResultWithMessage(reader.Read(messageContainer.controllerId), "Could not read controller id.");
    CheckResultWithMessage(reader.Read(messageContainer.id), "Could not read id.");
    CheckResultWithMessage(reader.Read(messageContainer.flags), "Could not read flags.");
    CheckResultWithMessage(reader.Read(messageContainer.length), "Could not read length.");

    CheckResult(messageContainer.Check());

    CheckResultWithMessage(reader.Read(messageContainer.data.data(), messageContainer.length), "Could not read data.");
    return Result::Ok;
}

[[nodiscard]] Result ReceiveHeader(ChannelReader& reader, FrameKind& frameKind) {
    if (IsProtocolHeaderTracingEnabled()) {
        LogProtocolBeginTraceReceiveHeader();
    }

    CheckResultWithMessage(reader.Read(frameKind), "Could not receive frame kind.");

    if (IsProtocolHeaderTracingEnabled()) {
        LogProtocolEndTraceReceiveHeader(frameKind);
    }

    return Result::Ok;
}

[[nodiscard]] Result SendOk(ChannelWriter& writer) {
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

[[nodiscard]] Result SendError(ChannelWriter& writer, const std::string& errorMessage) {
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

[[nodiscard]] Result ReadError(ChannelReader& reader, std::string& errorMessage) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceReadError();
    }

    CheckResultWithMessage(ReadString(reader, errorMessage), "Could not read error message.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceReadError(errorMessage);
    }

    return Result::Ok;
}

[[nodiscard]] Result SendPing(ChannelWriter& writer) {
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

[[nodiscard]] Result SendPingOk(ChannelWriter& writer, Command command) {
    if (IsProtocolPingTracingEnabled()) {
        LogProtocolBeginTraceSendPingOk(command);
    }

    CheckResultWithMessage(writer.Write(FrameKind::PingOk), "Could not write frame kind.");
    CheckResultWithMessage(writer.Write(command), "Could not write command.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolPingTracingEnabled()) {
        LogProtocolEndTraceSendPingOk();
    }

    return Result::Ok;
}

[[nodiscard]] Result ReadPingOk(ChannelReader& reader, Command& command) {
    if (IsProtocolPingTracingEnabled()) {
        LogProtocolBeginTraceReadPingOk();
    }

    CheckResultWithMessage(reader.Read(command), "Could not read command.");

    if (IsProtocolPingTracingEnabled()) {
        LogProtocolEndTraceReadPingOk(command);
    }

    return Result::Ok;
}

[[nodiscard]] Result SendConnect(ChannelWriter& writer,
                                 uint32_t protocolVersion,
                                 Mode clientMode,
                                 const std::string& serverName,
                                 const std::string& clientName) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceSendConnect(protocolVersion, clientMode, serverName, clientName);
    }

    CheckResultWithMessage(writer.Write(FrameKind::Connect), "Could not write frame kind.");
    CheckResultWithMessage(writer.Write(protocolVersion), "Could not write protocol version.");
    CheckResultWithMessage(writer.Write(clientMode), "Could not write client mode.");
    CheckResultWithMessage(WriteString(writer, serverName), "Could not write server name.");
    CheckResultWithMessage(WriteString(writer, clientName), "Could not write client name.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceSendConnect();
    }

    return Result::Ok;
}

[[nodiscard]] Result ReadConnect(ChannelReader& reader,
                                 uint32_t& protocolVersion,
                                 Mode& clientMode,
                                 std::string& serverName,
                                 std::string& clientName) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceReadConnect();
    }

    CheckResultWithMessage(reader.Read(protocolVersion), "Could not read protocol version.");
    CheckResultWithMessage(reader.Read(clientMode), "Could not read client mode.");
    CheckResultWithMessage(ReadString(reader, serverName), "Could not read server name.");
    CheckResultWithMessage(ReadString(reader, clientName), "Could not read client name.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceReadConnect(protocolVersion, clientMode, serverName, clientName);
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
                                   const std::vector<LinControllerContainer>& linControllers) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceSendConnectOk(protocolVersion,
                                           clientMode,
                                           stepSize,
                                           simulationState,
                                           incomingSignals,
                                           outgoingSignals,
                                           canControllers,
                                           ethControllers,
                                           linControllers);
    }

    CheckResultWithMessage(writer.Write(FrameKind::ConnectOk), "Could not write frame kind.");
    CheckResultWithMessage(writer.Write(protocolVersion), "Could not write protocol version.");
    CheckResultWithMessage(writer.Write(clientMode), "Could not write client mode.");
    CheckResultWithMessage(writer.Write(stepSize), "Could not write step size.");
    CheckResultWithMessage(writer.Write(simulationState), "Could not write simulation state.");
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

[[nodiscard]] Result ReadConnectOk(ChannelReader& reader,
                                   uint32_t& protocolVersion,
                                   Mode& clientMode,
                                   SimulationTime& stepSize,
                                   SimulationState& simulationState,
                                   std::vector<IoSignalContainer>& incomingSignals,
                                   std::vector<IoSignalContainer>& outgoingSignals,
                                   std::vector<CanControllerContainer>& canControllers,
                                   std::vector<EthControllerContainer>& ethControllers,
                                   std::vector<LinControllerContainer>& linControllers) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceReadConnectOk();
    }

    CheckResultWithMessage(reader.Read(protocolVersion), "Could not read protocol version.");
    CheckResultWithMessage(reader.Read(clientMode), "Could not read client mode.");
    CheckResultWithMessage(reader.Read(stepSize), "Could not read step size.");
    CheckResultWithMessage(reader.Read(simulationState), "Could not read simulation state.");
    CheckResultWithMessage(ReadIoSignalInfos(reader, incomingSignals), "Could not read incoming signals.");
    CheckResultWithMessage(ReadIoSignalInfos(reader, outgoingSignals), "Could not read outgoing signals.");
    CheckResultWithMessage(ReadControllerInfos(reader, canControllers), "Could not read CAN controllers.");
    CheckResultWithMessage(ReadControllerInfos(reader, ethControllers), "Could not read ETH controllers.");
    CheckResultWithMessage(ReadControllerInfos(reader, linControllers), "Could not read LIN controllers.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceReadConnectOk(protocolVersion,
                                         clientMode,
                                         stepSize,
                                         simulationState,
                                         incomingSignals,
                                         outgoingSignals,
                                         canControllers,
                                         ethControllers,
                                         linControllers);
    }

    return Result::Ok;
}

[[nodiscard]] Result SendStart(ChannelWriter& writer, SimulationTime simulationTime) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceSendStart(simulationTime);
    }

    CheckResultWithMessage(writer.Write(FrameKind::Start), "Could not write frame kind.");
    CheckResultWithMessage(writer.Write(simulationTime), "Could not write simulation time.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceSendStart();
    }

    return Result::Ok;
}

[[nodiscard]] Result ReadStart(ChannelReader& reader, SimulationTime& simulationTime) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceReadStart();
    }

    CheckResult(reader.Read(simulationTime));

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceReadStart(simulationTime);
    }

    return Result::Ok;
}

[[nodiscard]] Result SendStop(ChannelWriter& writer, SimulationTime simulationTime) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceSendStop(simulationTime);
    }

    CheckResultWithMessage(writer.Write(FrameKind::Stop), "Could not write frame kind.");
    CheckResultWithMessage(writer.Write(simulationTime), "Could not write simulation time.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceSendStop();
    }

    return Result::Ok;
}

[[nodiscard]] Result ReadStop(ChannelReader& reader, SimulationTime& simulationTime) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceReadStop();
    }

    CheckResultWithMessage(reader.Read(simulationTime), "Could not read simulation time.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceReadStop(simulationTime);
    }

    return Result::Ok;
}

[[nodiscard]] Result SendTerminate(ChannelWriter& writer, SimulationTime simulationTime, TerminateReason reason) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceSendTerminate(simulationTime, reason);
    }

    CheckResultWithMessage(writer.Write(FrameKind::Terminate), "Could not write frame kind.");
    CheckResultWithMessage(writer.Write(simulationTime), "Could not write simulation time.");
    CheckResultWithMessage(writer.Write(reason), "Could not write terminate reason.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceSendTerminate();
    }

    return Result::Ok;
}

[[nodiscard]] Result ReadTerminate(ChannelReader& reader, SimulationTime& simulationTime, TerminateReason& reason) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceReadTerminate();
    }

    CheckResultWithMessage(reader.Read(simulationTime), "Could not read simulation time.");
    CheckResultWithMessage(reader.Read(reason), "Could not read terminate reason.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceReadTerminate(simulationTime, reason);
    }

    return Result::Ok;
}

[[nodiscard]] Result SendPause(ChannelWriter& writer, SimulationTime simulationTime) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceSendPause(simulationTime);
    }

    CheckResultWithMessage(writer.Write(FrameKind::Pause), "Could not write frame kind.");
    CheckResultWithMessage(writer.Write(simulationTime), "Could not write simulation time.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceSendPause();
    }

    return Result::Ok;
}

[[nodiscard]] Result ReadPause(ChannelReader& reader, SimulationTime& simulationTime) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceReadPause();
    }

    CheckResultWithMessage(reader.Read(simulationTime), "Could not read simulation time.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceReadPause(simulationTime);
    }

    return Result::Ok;
}

[[nodiscard]] Result SendContinue(ChannelWriter& writer, SimulationTime simulationTime) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceSendContinue(simulationTime);
    }

    CheckResultWithMessage(writer.Write(FrameKind::Continue), "Could not write frame kind.");
    CheckResultWithMessage(writer.Write(simulationTime), "Could not write simulation time.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceSendContinue();
    }

    return Result::Ok;
}

[[nodiscard]] Result ReadContinue(ChannelReader& reader, SimulationTime& simulationTime) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceReadContinue();
    }

    CheckResultWithMessage(reader.Read(simulationTime), "Could not read simulation time.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceReadContinue(simulationTime);
    }

    return Result::Ok;
}

[[nodiscard]] Result SendStep(ChannelWriter& writer,
                              SimulationTime simulationTime,
                              const IoBuffer& ioBuffer,
                              const BusBuffer& busBuffer) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceSendStep(simulationTime);
    }

    CheckResultWithMessage(writer.Write(FrameKind::Step), "Could not write frame kind.");
    CheckResultWithMessage(writer.Write(simulationTime), "Could not write simulation time.");
    CheckResultWithMessage(ioBuffer.Serialize(writer), "Could not write IO buffer data.");
    CheckResultWithMessage(busBuffer.Serialize(writer), "Could not write bus buffer data.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceSendStep();
    }

    return Result::Ok;
}

[[nodiscard]] Result ReadStep(ChannelReader& reader,
                              SimulationTime& simulationTime,
                              const IoBuffer& ioBuffer,
                              const BusBuffer& busBuffer,
                              const Callbacks& callbacks) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceReadStep();
    }

    CheckResultWithMessage(reader.Read(simulationTime), "Could not read simulation time.");

    if (callbacks.simulationBeginStepCallback) {
        callbacks.simulationBeginStepCallback(simulationTime);
    }

    CheckResultWithMessage(ioBuffer.Deserialize(reader, simulationTime, callbacks), "Could not read IO buffer data.");
    CheckResultWithMessage(busBuffer.Deserialize(reader, simulationTime, callbacks), "Could not read bus buffer data.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceReadStep(simulationTime);
    }

    return Result::Ok;
}

[[nodiscard]] Result SendStepOk(ChannelWriter& writer,
                                SimulationTime nextSimulationTime,
                                Command command,
                                const IoBuffer& ioBuffer,
                                const BusBuffer& busBuffer) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceSendStepOk(nextSimulationTime, command);
    }

    CheckResultWithMessage(writer.Write(FrameKind::StepOk), "Could not write frame kind.");
    CheckResultWithMessage(writer.Write(nextSimulationTime), "Could not write next simulation time.");
    CheckResultWithMessage(writer.Write(command), "Could not write command.");
    CheckResultWithMessage(ioBuffer.Serialize(writer), "Could not write IO buffer data.");
    CheckResultWithMessage(busBuffer.Serialize(writer), "Could not write bus buffer data.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceSendStepOk();
    }

    return Result::Ok;
}

[[nodiscard]] Result ReadStepOk(ChannelReader& reader,
                                SimulationTime& nextSimulationTime,
                                Command& command,
                                const IoBuffer& ioBuffer,
                                const BusBuffer& busBuffer,
                                const Callbacks& callbacks) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceReadStepOk();
    }

    CheckResultWithMessage(reader.Read(nextSimulationTime), "Could not read next simulation time.");
    CheckResultWithMessage(reader.Read(command), "Could not read command.");

    if (callbacks.simulationBeginStepCallback) {
        callbacks.simulationBeginStepCallback(nextSimulationTime);
    }

    CheckResultWithMessage(ioBuffer.Deserialize(reader, nextSimulationTime, callbacks),
                           "Could not read IO buffer data.");
    CheckResultWithMessage(busBuffer.Deserialize(reader, nextSimulationTime, callbacks),
                           "Could not read bus buffer data.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceReadStepOk(nextSimulationTime, command);
    }

    return Result::Ok;
}

[[nodiscard]] Result SendSetPort(ChannelWriter& writer, const std::string& serverName, uint16_t port) {
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

[[nodiscard]] Result ReadSetPort(ChannelReader& reader, std::string& serverName, uint16_t& port) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceReadSetPort();
    }

    CheckResultWithMessage(ReadString(reader, serverName), "Could not read server name.");
    CheckResultWithMessage(reader.Read(port), "Could not read port.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceReadSetPort(serverName, port);
    }

    return Result::Ok;
}

[[nodiscard]] Result SendUnsetPort(ChannelWriter& writer, const std::string& serverName) {
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

[[nodiscard]] Result ReadUnsetPort(ChannelReader& reader, std::string& serverName) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceReadUnsetPort();
    }

    CheckResultWithMessage(ReadString(reader, serverName), "Could not read server name.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceReadUnsetPort(serverName);
    }

    return Result::Ok;
}

[[nodiscard]] Result SendGetPort(ChannelWriter& writer, const std::string& serverName) {
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

[[nodiscard]] Result ReadGetPort(ChannelReader& reader, std::string& serverName) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceReadGetPort();
    }

    CheckResultWithMessage(ReadString(reader, serverName), "Could not read server name.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceReadGetPort(serverName);
    }

    return Result::Ok;
}

[[nodiscard]] Result SendGetPortOk(ChannelWriter& writer, uint16_t port) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceSendGetPortOk(port);
    }

    CheckResultWithMessage(writer.Write(FrameKind::GetPortOk), "Could not write frame kind.");
    CheckResultWithMessage(writer.Write(port), "Could not write port.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceSendGetPortOk();
    }

    return Result::Ok;
}

[[nodiscard]] Result ReadGetPortOk(ChannelReader& reader, uint16_t& port) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTraceReadGetPortOk();
    }

    CheckResultWithMessage(reader.Read(port), "Could not read port.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTraceReadGetPortOk(port);
    }

    return Result::Ok;
}

}  // namespace DsVeosCoSim::Protocol
