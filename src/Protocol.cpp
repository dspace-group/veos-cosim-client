// Copyright dSPACE GmbH. All rights reserved.

#include "Protocol.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "BusBuffer.h"
#include "Channel.h"
#include "CoSimHelper.h"
#include "DsVeosCoSim/CoSimTypes.h"
#include "Environment.h"
#include "IoBuffer.h"

namespace DsVeosCoSim {

namespace {

[[nodiscard]] Result WriteHeader(ChannelWriter& writer, FrameKind frameKind) {
    CheckResultWithMessage(writer.Write(frameKind), "Could not write frame header.");
    return Result::Ok;
}

[[nodiscard]] Result ReadString(ChannelReader& reader, std::string& string) {
    uint32_t size = 0;
    CheckResultWithMessage(reader.Read(size), "Could not read string size.");
    string.resize(size);
    CheckResultWithMessage(reader.Read(string.data(), size), "Could not read string data.");
    return Result::Ok;
}

[[nodiscard]] Result WriteString(ChannelWriter& writer, std::string_view string) {
    auto size = static_cast<uint32_t>(string.size());
    CheckResultWithMessage(writer.Write(size), "Could not write string size.");
    CheckResultWithMessage(writer.Write(string.data(), size), "Could not write string data.");
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
    uint32_t signalsCount = 0;
    CheckResultWithMessage(reader.Read(signalsCount), "Could not read signals count.");
    signals.resize(signalsCount);

    for (uint32_t i = 0; i < signalsCount; i++) {
        CheckResultWithMessage(ReadIoSignalInfo(reader, signals[i]), "Could not read signal info.");
    }

    return Result::Ok;
}

[[nodiscard]] Result WriteIoSignalInfos(ChannelWriter& writer, const std::vector<IoSignalContainer>& signals) {
    auto size = static_cast<uint32_t>(signals.size());
    CheckResultWithMessage(writer.Write(size), "Could not write signals count.");
    for (const auto& signal : signals) {
        CheckResultWithMessage(WriteIoSignalInfo(writer, signal), "Could not write signal info.");
    }

    return Result::Ok;
}

[[nodiscard]] Result ReadControllerInfo(ChannelReader& reader, CanControllerContainer& controller) {
    CheckResultWithMessage(reader.Read(controller.id), "Could not read id.");
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
    uint32_t controllersCount = 0;
    CheckResultWithMessage(reader.Read(controllersCount), "Could not read controllers count.");
    controllers.resize(controllersCount);

    for (uint32_t i = 0; i < controllersCount; i++) {
        CheckResultWithMessage(ReadControllerInfo(reader, controllers[i]), "Could not read controller.");
    }

    return Result::Ok;
}

[[nodiscard]] Result WriteControllerInfos(ChannelWriter& writer,
                                          const std::vector<CanControllerContainer>& controllers) {
    auto size = static_cast<uint32_t>(controllers.size());
    CheckResultWithMessage(writer.Write(size), "Could not write controllers count.");
    for (const auto& controller : controllers) {
        CheckResultWithMessage(WriteControllerInfo(writer, controller), "Could not write controller.");
    }

    return Result::Ok;
}

[[nodiscard]] Result ReadControllerInfo(ChannelReader& reader, EthControllerContainer& controller) {
    CheckResultWithMessage(reader.Read(controller.id), "Could not read id.");
    CheckResultWithMessage(reader.Read(controller.queueSize), "Could not read queue size.");
    CheckResultWithMessage(reader.Read(controller.bitsPerSecond), "Could not read bits per second.");
    CheckResultWithMessage(reader.Read(controller.macAddress), "Could not read MAC address.");
    CheckResultWithMessage(ReadString(reader, controller.name), "Could not read name.");
    CheckResultWithMessage(ReadString(reader, controller.channelName), "Could not read channel name.");
    CheckResultWithMessage(ReadString(reader, controller.clusterName), "Could not read cluster name.");
    return Result::Ok;
}

[[nodiscard]] Result WriteControllerInfo(ChannelWriter& writer, const EthControllerContainer& controller) {
    CheckResultWithMessage(writer.Write(controller.id), "Could not write id.");
    CheckResultWithMessage(writer.Write(controller.queueSize), "Could not write queue size.");
    CheckResultWithMessage(writer.Write(controller.bitsPerSecond), "Could not write bits per second.");
    CheckResultWithMessage(writer.Write(controller.macAddress), "Could not write MAC address.");
    CheckResultWithMessage(WriteString(writer, controller.name), "Could not write name.");
    CheckResultWithMessage(WriteString(writer, controller.channelName), "Could not write channel name.");
    CheckResultWithMessage(WriteString(writer, controller.clusterName), "Could not write cluster name.");
    return Result::Ok;
}

[[nodiscard]] Result ReadControllerInfos(ChannelReader& reader, std::vector<EthControllerContainer>& controllers) {
    uint32_t controllersCount = 0;
    CheckResultWithMessage(reader.Read(controllersCount), "Could not read controllers count.");
    controllers.resize(controllersCount);

    for (uint32_t i = 0; i < controllersCount; i++) {
        CheckResultWithMessage(ReadControllerInfo(reader, controllers[i]), "Could not read controller.");
    }

    return Result::Ok;
}

[[nodiscard]] Result WriteControllerInfos(ChannelWriter& writer,
                                          const std::vector<EthControllerContainer>& controllers) {
    auto size = static_cast<uint32_t>(controllers.size());
    CheckResultWithMessage(writer.Write(size), "Could not write controllers count.");
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
    uint32_t controllersCount = 0;
    CheckResultWithMessage(reader.Read(controllersCount), "Could not read controllers count.");
    controllers.resize(controllersCount);

    for (uint32_t i = 0; i < controllersCount; i++) {
        CheckResultWithMessage(ReadControllerInfo(reader, controllers[i]), "Could not read controller.");
    }

    return Result::Ok;
}

[[nodiscard]] Result WriteControllerInfos(ChannelWriter& writer,
                                          const std::vector<LinControllerContainer>& controllers) {
    auto size = static_cast<uint32_t>(controllers.size());
    CheckResultWithMessage(writer.Write(size), "Could not write controllers count.");
    for (const auto& controller : controllers) {
        CheckResultWithMessage(WriteControllerInfo(writer, controller), "Could not write controller.");
    }

    return Result::Ok;
}

}  // namespace

[[nodiscard]] std::string_view ToString(FrameKind frameKind) {
    switch (frameKind) {
        case FrameKind::Ping:
            return "Ping";
        case FrameKind::PingOk:
            return "PingOk";
        case FrameKind::Ok:
            return "Ok";
        case FrameKind::Error:
            return "Error";
        case FrameKind::Start:
            return "Start";
        case FrameKind::Stop:
            return "Stop";
        case FrameKind::Terminate:
            return "Terminate";
        case FrameKind::Pause:
            return "Pause";
        case FrameKind::Continue:
            return "Continue";
        case FrameKind::Step:
            return "Step";
        case FrameKind::StepOk:
            return "StepOk";
        case FrameKind::Connect:
            return "Connect";
        case FrameKind::ConnectOk:
            return "ConnectOk";
        case FrameKind::GetPort:
            return "GetPort";
        case FrameKind::GetPortOk:
            return "GetPortOk";
        case FrameKind::SetPort:
            return "SetPort";
        case FrameKind::UnsetPort:
            return "UnsetPort";
    }

    return "<Invalid FrameKind>";
}

namespace Protocol {

[[nodiscard]] Result ReceiveHeader(ChannelReader& reader, FrameKind& frameKind) {
    if (IsProtocolHeaderTracingEnabled()) {
        LogProtocolBeginTrace("ReceiveHeader()");
    }

    CheckResultWithMessage(reader.Read(frameKind), "Could not receive frame header.");

    if (IsProtocolHeaderTracingEnabled()) {
        std::string str = "ReceiveHeader(FrameKind: ";
        str.append(ToString(frameKind));
        str.append(")");
        LogProtocolEndTrace(str);
    }

    return Result::Ok;
}

[[nodiscard]] Result SendOk(ChannelWriter& writer) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTrace("SendOk()");
    }

    CheckResult(WriteHeader(writer, FrameKind::Ok));
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTrace("SendOk()");
    }

    return Result::Ok;
}

[[nodiscard]] Result SendError(ChannelWriter& writer, std::string_view errorMessage) {
    if (IsProtocolTracingEnabled()) {
        std::string str = "SendError(ErrorMessage: \"";
        str.append(errorMessage);
        str.append("\")");
        LogProtocolBeginTrace(str);
    }

    CheckResult(WriteHeader(writer, FrameKind::Error));
    CheckResultWithMessage(WriteString(writer, errorMessage), "Could not write error message.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTrace("SendError()");
    }

    return Result::Ok;
}

[[nodiscard]] Result ReadError(ChannelReader& reader, std::string& errorMessage) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTrace("ReadError()");
    }

    CheckResultWithMessage(ReadString(reader, errorMessage), "Could not read error message.");

    if (IsProtocolTracingEnabled()) {
        std::string str = "ReadError(ErrorMessage: \"";
        str.append(errorMessage);
        str.append("\")");
        LogProtocolEndTrace(str);
    }

    return Result::Ok;
}

[[nodiscard]] Result SendPing(ChannelWriter& writer) {
    if (IsProtocolPingTracingEnabled()) {
        LogProtocolBeginTrace("SendPing()");
    }

    CheckResult(WriteHeader(writer, FrameKind::Ping));
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolPingTracingEnabled()) {
        LogProtocolEndTrace("SendPing()");
    }

    return Result::Ok;
}

[[nodiscard]] Result SendPingOk(ChannelWriter& writer, Command command) {
    if (IsProtocolPingTracingEnabled()) {
        std::string str = "SendPingOk(Command: ";
        str.append(ToString(command));
        str.append(")");
        LogProtocolBeginTrace(str);
    }

    CheckResult(WriteHeader(writer, FrameKind::PingOk));
    CheckResultWithMessage(writer.Write(command), "Could not write command.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolPingTracingEnabled()) {
        LogProtocolEndTrace("SendPingOk()");
    }

    return Result::Ok;
}

[[nodiscard]] Result ReadPingOk(ChannelReader& reader, Command& command) {
    if (IsProtocolPingTracingEnabled()) {
        LogProtocolBeginTrace("ReadPingOk()");
    }

    CheckResultWithMessage(reader.Read(command), "Could not read command.");

    if (IsProtocolPingTracingEnabled()) {
        std::string str = "ReadPingOk(Command: ";
        str.append(ToString(command));
        str.append(")");
        LogProtocolEndTrace(str);
    }

    return Result::Ok;
}

[[nodiscard]] Result SendConnect(ChannelWriter& writer,
                                 uint32_t protocolVersion,
                                 Mode clientMode,
                                 std::string_view serverName,
                                 std::string_view clientName) {
    if (IsProtocolTracingEnabled()) {
        std::string str = "SendConnect(ProtocolVersion: ";
        str.append(std::to_string(protocolVersion));
        str.append(", ClientMode: ");
        str.append(ToString(clientMode));
        str.append(", ServerName: \"");
        str.append(serverName);
        str.append("\", ClientName: \"");
        str.append(clientName);
        str.append("\")");
        LogProtocolBeginTrace(str);
    }

    CheckResult(WriteHeader(writer, FrameKind::Connect));
    CheckResultWithMessage(writer.Write(protocolVersion), "Could not write protocol version.");
    CheckResultWithMessage(writer.Write(clientMode), "Could not write client mode.");
    CheckResultWithMessage(WriteString(writer, serverName), "Could not write server name.");
    CheckResultWithMessage(WriteString(writer, clientName), "Could not write client name.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTrace("SendConnect()");
    }

    return Result::Ok;
}

[[nodiscard]] Result ReadConnect(ChannelReader& reader,
                                 uint32_t& protocolVersion,
                                 Mode& clientMode,
                                 std::string& serverName,
                                 std::string& clientName) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTrace("ReadConnect()");
    }

    CheckResultWithMessage(reader.Read(protocolVersion), "Could not read protocol version.");
    CheckResultWithMessage(reader.Read(clientMode), "Could not read client mode.");
    CheckResultWithMessage(ReadString(reader, serverName), "Could not read server name.");
    CheckResultWithMessage(ReadString(reader, clientName), "Could not read client name.");

    if (IsProtocolTracingEnabled()) {
        std::string str = "ReadConnect(ProtocolVersion: ";
        str.append(std::to_string(protocolVersion));
        str.append(", ClientMode: ");
        str.append(ToString(clientMode));
        str.append(", ServerName: \"");
        str.append(serverName);
        str.append("\", ClientName: \"");
        str.append(clientName);
        str.append("\")");
        LogProtocolEndTrace(str);
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
        std::string str = "SendConnectOk(ProtocolVersion: ";
        str.append(std::to_string(protocolVersion));
        str.append(", ClientMode: ");
        str.append(ToString(clientMode));
        str.append(", StepSize: ");
        str.append(SimulationTimeToString(stepSize));
        str.append(" s, SimulationState: ");
        str.append(ToString(simulationState));
        str.append(", IncomingSignals: ");
        str.append(ToString(incomingSignals));
        str.append(", OutgoingSignals: ");
        str.append(ToString(outgoingSignals));
        str.append(", CanControllers: ");
        str.append(ToString(canControllers));
        str.append(", EthControllers: ");
        str.append(ToString(ethControllers));
        str.append(", LinControllers: ");
        str.append(ToString(linControllers));
        str.append(")");
        LogProtocolBeginTrace(str);
    }

    CheckResult(WriteHeader(writer, FrameKind::ConnectOk));
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
        LogProtocolEndTrace("SendConnectOk()");
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
        LogProtocolBeginTrace("ReadConnectOk()");
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
        std::string str = "ReadConnectOk(ProtocolVersion: ";
        str.append(std::to_string(protocolVersion));
        str.append(", ClientMode: ");
        str.append(ToString(clientMode));
        str.append(", StepSize: ");
        str.append(SimulationTimeToString(stepSize));
        str.append(" s, SimulationState: ");
        str.append(ToString(simulationState));
        str.append(", IncomingSignals: ");
        str.append(ToString(incomingSignals));
        str.append(", OutgoingSignals: ");
        str.append(ToString(outgoingSignals));
        str.append(", CanControllers: ");
        str.append(ToString(canControllers));
        str.append(", EthControllers: ");
        str.append(ToString(ethControllers));
        str.append(", LinControllers: ");
        str.append(ToString(linControllers));
        str.append(")");
        LogProtocolEndTrace(str);
    }

    return Result::Ok;
}

[[nodiscard]] Result SendStart(ChannelWriter& writer, SimulationTime simulationTime) {
    if (IsProtocolTracingEnabled()) {
        std::string str = "SendStart(SimulationTime: ";
        str.append(SimulationTimeToString(simulationTime));
        str.append(" s)");
        LogProtocolBeginTrace(str);
    }

    CheckResult(WriteHeader(writer, FrameKind::Start));
    CheckResultWithMessage(writer.Write(simulationTime), "Could not write simulation time.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTrace("SendStart()");
    }

    return Result::Ok;
}

[[nodiscard]] Result ReadStart(ChannelReader& reader, SimulationTime& simulationTime) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTrace("ReadStart()");
    }

    CheckResultWithMessage(reader.Read(simulationTime), "Could not read simulation time.");

    if (IsProtocolTracingEnabled()) {
        std::string str = "ReadStart(SimulationTime: ";
        str.append(SimulationTimeToString(simulationTime));
        str.append(" s)");
        LogProtocolEndTrace(str);
    }

    return Result::Ok;
}

[[nodiscard]] Result SendStop(ChannelWriter& writer, SimulationTime simulationTime) {
    if (IsProtocolTracingEnabled()) {
        std::string str = "SendStop(SimulationTime: ";
        str.append(SimulationTimeToString(simulationTime));
        str.append(" s)");
        LogProtocolBeginTrace(str);
    }

    CheckResult(WriteHeader(writer, FrameKind::Stop));
    CheckResultWithMessage(writer.Write(simulationTime), "Could not write simulation time.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTrace("SendStop()");
    }

    return Result::Ok;
}

[[nodiscard]] Result ReadStop(ChannelReader& reader, SimulationTime& simulationTime) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTrace("ReadStop()");
    }

    CheckResultWithMessage(reader.Read(simulationTime), "Could not read simulation time.");

    if (IsProtocolTracingEnabled()) {
        std::string str = "ReadStop(SimulationTime: ";
        str.append(SimulationTimeToString(simulationTime));
        str.append(" s)");
        LogProtocolEndTrace(str);
    }

    return Result::Ok;
}

[[nodiscard]] Result SendTerminate(ChannelWriter& writer, SimulationTime simulationTime, TerminateReason reason) {
    if (IsProtocolTracingEnabled()) {
        std::string str = "SendTerminate(SimulationTime: ";
        str.append(SimulationTimeToString(simulationTime));
        str.append(" s, Reason: ");
        str.append(ToString(reason));
        str.append(")");
        LogProtocolBeginTrace(str);
    }

    CheckResult(WriteHeader(writer, FrameKind::Terminate));
    CheckResultWithMessage(writer.Write(simulationTime), "Could not write simulation time.");
    CheckResultWithMessage(writer.Write(reason), "Could not write reason.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTrace("SendTerminate()");
    }

    return Result::Ok;
}

[[nodiscard]] Result ReadTerminate(ChannelReader& reader, SimulationTime& simulationTime, TerminateReason& reason) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTrace("ReadTerminate()");
    }

    CheckResultWithMessage(reader.Read(simulationTime), "Could not read simulation time.");
    CheckResultWithMessage(reader.Read(reason), "Could not read reason.");

    if (IsProtocolTracingEnabled()) {
        std::string str = "ReadTerminate(SimulationTime: ";
        str.append(SimulationTimeToString(simulationTime));
        str.append(" s, Reason: ");
        str.append(ToString(reason));
        str.append(")");
        LogProtocolEndTrace(str);
    }

    return Result::Ok;
}

[[nodiscard]] Result SendPause(ChannelWriter& writer, SimulationTime simulationTime) {
    if (IsProtocolTracingEnabled()) {
        std::string str = "SendPause(SimulationTime: ";
        str.append(SimulationTimeToString(simulationTime));
        str.append(" s)");
        LogProtocolBeginTrace(str);
    }

    CheckResult(WriteHeader(writer, FrameKind::Pause));
    CheckResultWithMessage(writer.Write(simulationTime), "Could not write simulation time.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTrace("SendPause()");
    }

    return Result::Ok;
}

[[nodiscard]] Result ReadPause(ChannelReader& reader, SimulationTime& simulationTime) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTrace("ReadPause()");
    }

    CheckResultWithMessage(reader.Read(simulationTime), "Could not read simulation time.");

    if (IsProtocolTracingEnabled()) {
        std::string str = "ReadPause(SimulationTime: ";
        str.append(SimulationTimeToString(simulationTime));
        str.append(" s)");
        LogProtocolEndTrace(str);
    }

    return Result::Ok;
}

[[nodiscard]] Result SendContinue(ChannelWriter& writer, SimulationTime simulationTime) {
    if (IsProtocolTracingEnabled()) {
        std::string str = "SendContinue(SimulationTime: ";
        str.append(SimulationTimeToString(simulationTime));
        str.append(" s)");
        LogProtocolBeginTrace(str);
    }

    CheckResult(WriteHeader(writer, FrameKind::Continue));
    CheckResultWithMessage(writer.Write(simulationTime), "Could not write simulation time.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTrace("SendContinue()");
    }

    return Result::Ok;
}

[[nodiscard]] Result ReadContinue(ChannelReader& reader, SimulationTime& simulationTime) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTrace("ReadContinue()");
    }

    CheckResultWithMessage(reader.Read(simulationTime), "Could not read simulation time.");

    if (IsProtocolTracingEnabled()) {
        std::string str = "ReadContinue(SimulationTime: ";
        str.append(SimulationTimeToString(simulationTime));
        str.append(" s)");
        LogProtocolEndTrace(str);
    }

    return Result::Ok;
}

[[nodiscard]] Result SendStep(ChannelWriter& writer,
                              SimulationTime simulationTime,
                              const IoBuffer& ioBuffer,
                              const BusBuffer& busBuffer) {
    if (IsProtocolTracingEnabled()) {
        std::string str = "SendStep(SimulationTime: ";
        str.append(SimulationTimeToString(simulationTime));
        str.append(")");
        LogProtocolBeginTrace(str);
    }

    CheckResult(WriteHeader(writer, FrameKind::Step));
    CheckResultWithMessage(writer.Write(simulationTime), "Could not write simulation time.");
    CheckResultWithMessage(ioBuffer.Serialize(writer), "Could not write IO buffer data.");
    CheckResultWithMessage(busBuffer.Serialize(writer), "Could not write bus buffer data.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTrace("SendStep()");
    }

    return Result::Ok;
}

[[nodiscard]] Result ReadStep(ChannelReader& reader,
                              SimulationTime& simulationTime,
                              const IoBuffer& ioBuffer,
                              const BusBuffer& busBuffer,
                              const Callbacks& callbacks) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTrace("ReadStep()");
    }

    CheckResultWithMessage(reader.Read(simulationTime), "Could not read simulation time.");

    if (callbacks.simulationBeginStepCallback) {
        callbacks.simulationBeginStepCallback(simulationTime);
    }

    CheckResultWithMessage(ioBuffer.Deserialize(reader, simulationTime, callbacks), "Could not read IO buffer data.");
    CheckResultWithMessage(busBuffer.Deserialize(reader, simulationTime, callbacks), "Could not read bus buffer data.");

    if (IsProtocolTracingEnabled()) {
        std::string str = "ReadStep(SimulationTime: ";
        str.append(SimulationTimeToString(simulationTime));
        str.append(" s)");
        LogProtocolEndTrace(str);
    }

    return Result::Ok;
}

[[nodiscard]] Result SendStepOk(ChannelWriter& writer,
                                SimulationTime nextSimulationTime,
                                Command command,
                                const IoBuffer& ioBuffer,
                                const BusBuffer& busBuffer) {
    if (IsProtocolTracingEnabled()) {
        std::string str = "SendStepOk(NextSimulationTime: ";
        str.append(SimulationTimeToString(nextSimulationTime));
        str.append(" s, Command: ");
        str.append(ToString(command));
        str.append(")");
        LogProtocolBeginTrace(str);
    }

    CheckResult(WriteHeader(writer, FrameKind::StepOk));
    CheckResultWithMessage(writer.Write(nextSimulationTime), "Could not write simulation time.");
    CheckResultWithMessage(writer.Write(command), "Could not write command.");
    CheckResultWithMessage(ioBuffer.Serialize(writer), "Could not write IO buffer data.");
    CheckResultWithMessage(busBuffer.Serialize(writer), "Could not write bus buffer data.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTrace("SendStepOk()");
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
        LogProtocolBeginTrace("ReadStepOk()");
    }

    CheckResultWithMessage(reader.Read(nextSimulationTime), "Could not read simulation time.");
    CheckResultWithMessage(reader.Read(command), "Could not read command.");

    if (callbacks.simulationBeginStepCallback) {
        callbacks.simulationBeginStepCallback(nextSimulationTime);
    }

    CheckResultWithMessage(ioBuffer.Deserialize(reader, nextSimulationTime, callbacks),
                           "Could not read IO buffer data.");
    CheckResultWithMessage(busBuffer.Deserialize(reader, nextSimulationTime, callbacks),
                           "Could not read bus buffer data.");

    if (IsProtocolTracingEnabled()) {
        std::string str = "ReadStepOk(NextSimulationTime: ";
        str.append(SimulationTimeToString(nextSimulationTime));
        str.append(" s, Command: ");
        str.append(ToString(command));
        str.append(")");
        LogProtocolEndTrace(str);
    }

    return Result::Ok;
}

[[nodiscard]] Result SendSetPort(ChannelWriter& writer, std::string_view serverName, uint16_t port) {
    if (IsProtocolTracingEnabled()) {
        std::string str = "SendSetPort(ServerName: \"";
        str.append(serverName);
        str.append("\", Port: ");
        str.append(std::to_string(port));
        str.append(")");
        LogProtocolBeginTrace(str);
    }

    CheckResult(WriteHeader(writer, FrameKind::SetPort));
    CheckResultWithMessage(WriteString(writer, serverName), "Could not write server name.");
    CheckResultWithMessage(writer.Write(port), "Could not write port.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTrace("SendSetPort()");
    }

    return Result::Ok;
}

[[nodiscard]] Result ReadSetPort(ChannelReader& reader, std::string& serverName, uint16_t& port) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTrace("ReadSetPort()");
    }

    CheckResultWithMessage(ReadString(reader, serverName), "Could not read server name.");
    CheckResultWithMessage(reader.Read(port), "Could not read port.");

    if (IsProtocolTracingEnabled()) {
        std::string str = "ReadSetPort(ServerName: \"";
        str.append(serverName);
        str.append("\", Port: ");
        str.append(std::to_string(port));
        str.append(")");
        LogProtocolEndTrace(str);
    }

    return Result::Ok;
}

[[nodiscard]] Result SendUnsetPort(ChannelWriter& writer, std::string_view serverName) {
    if (IsProtocolTracingEnabled()) {
        std::string str = "SendUnsetPort(ServerName: \"";
        str.append(serverName);
        str.append("\")");
        LogProtocolBeginTrace(str);
    }

    CheckResult(WriteHeader(writer, FrameKind::UnsetPort));
    CheckResultWithMessage(WriteString(writer, serverName), "Could not write server name.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTrace("SendUnsetPort()");
    }

    return Result::Ok;
}

[[nodiscard]] Result ReadUnsetPort(ChannelReader& reader, std::string& serverName) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTrace("ReadUnsetPort()");
    }

    CheckResultWithMessage(ReadString(reader, serverName), "Could not read server name.");

    if (IsProtocolTracingEnabled()) {
        std::string str = "ReadUnsetPort(ServerName: \"";
        str.append(serverName);
        str.append("\")");
        LogProtocolEndTrace(str);
    }

    return Result::Ok;
}

[[nodiscard]] Result SendGetPort(ChannelWriter& writer, std::string_view serverName) {
    if (IsProtocolTracingEnabled()) {
        std::string str = "SendGetPort(ServerName: \"";
        str.append(serverName);
        str.append("\")");
        LogProtocolBeginTrace(str);
    }

    CheckResult(WriteHeader(writer, FrameKind::GetPort));
    CheckResultWithMessage(WriteString(writer, serverName), "Could not write server name.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTrace("SendGetPort()");
    }

    return Result::Ok;
}

[[nodiscard]] Result ReadGetPort(ChannelReader& reader, std::string& serverName) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTrace("ReadGetPort()");
    }

    CheckResultWithMessage(ReadString(reader, serverName), "Could not read server name.");

    if (IsProtocolTracingEnabled()) {
        std::string str = "ReadGetPort(ServerName: \"";
        str.append(serverName);
        str.append("\")");
        LogProtocolEndTrace(str);
    }

    return Result::Ok;
}

[[nodiscard]] Result SendGetPortOk(ChannelWriter& writer, uint16_t port) {
    if (IsProtocolTracingEnabled()) {
        std::string str = "SendGetPortOk(Port: ";
        str.append(std::to_string(port));
        str.append(")");
        LogProtocolBeginTrace(str);
    }

    CheckResult(WriteHeader(writer, FrameKind::GetPortOk));
    CheckResultWithMessage(writer.Write(port), "Could not write port.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

    if (IsProtocolTracingEnabled()) {
        LogProtocolEndTrace("SendGetPortOk()");
    }

    return Result::Ok;
}

[[nodiscard]] Result ReadGetPortOk(ChannelReader& reader, uint16_t& port) {
    if (IsProtocolTracingEnabled()) {
        LogProtocolBeginTrace("ReadGetPortOk()");
    }

    CheckResultWithMessage(reader.Read(port), "Could not read port.");

    if (IsProtocolTracingEnabled()) {
        std::string str = "ReadGetPortOk(Port: ";
        str.append(std::to_string(port));
        str.append(")");
        LogProtocolEndTrace(str);
    }

    return Result::Ok;
}

}  // namespace Protocol

}  // namespace DsVeosCoSim
