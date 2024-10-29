// Copyright dSPACE GmbH. All rights reserved.

#include "Protocol.h"

#include <string>
#include <string_view>

#include "CoSimHelper.h"

namespace DsVeosCoSim {

namespace {

[[nodiscard]] bool WriteHeader(ChannelWriter& writer, FrameKind frameKind) {
    CheckResultWithMessage(writer.Write(frameKind), "Could not write frame header.");
    return true;
}

[[nodiscard]] bool ReadString(ChannelReader& reader, std::string& string) {
    uint32_t size = 0;
    CheckResultWithMessage(reader.Read(size), "Could not read string size.");
    string.resize(size);
    CheckResultWithMessage(reader.Read(string.data(), size), "Could not read string data.");
    return true;
}

[[nodiscard]] bool WriteString(ChannelWriter& writer, std::string_view string) {
    const auto size = static_cast<uint32_t>(string.size());
    CheckResultWithMessage(writer.Write(size), "Could not write string size.");
    CheckResultWithMessage(writer.Write(string.data(), size), "Could not write string data.");
    return true;
}

[[nodiscard]] bool ReadIoSignalInfo(ChannelReader& reader, IoSignal& signal) {
    CheckResultWithMessage(reader.Read(signal.id), "Could not read id.");
    CheckResultWithMessage(reader.Read(signal.length), "Could not read length.");
    CheckResultWithMessage(reader.Read(signal.dataType), "Could not read data type.");
    CheckResultWithMessage(reader.Read(signal.sizeKind), "Could not read size kind.");
    CheckResultWithMessage(ReadString(reader, signal.name), "Could not read name.");
    return true;
}

[[nodiscard]] bool WriteIoSignalInfo(ChannelWriter& writer, const IoSignal& signal) {
    CheckResultWithMessage(writer.Write(signal.id), "Could not write id.");
    CheckResultWithMessage(writer.Write(signal.length), "Could not write length.");
    CheckResultWithMessage(writer.Write(signal.dataType), "Could not write data type.");
    CheckResultWithMessage(writer.Write(signal.sizeKind), "Could not write size kind.");
    CheckResultWithMessage(WriteString(writer, signal.name), "Could not write name.");
    return true;
}

[[nodiscard]] bool ReadIoSignalInfos(ChannelReader& reader, std::vector<IoSignal>& signals) {
    uint32_t signalsCount = 0;
    CheckResultWithMessage(reader.Read(signalsCount), "Could not read signals count.");
    signals.resize(signalsCount);

    for (uint32_t i = 0; i < signalsCount; i++) {
        CheckResultWithMessage(ReadIoSignalInfo(reader, signals[i]), "Could not read signal info.");
    }

    return true;
}

[[nodiscard]] bool WriteIoSignalInfos(ChannelWriter& writer, const std::vector<IoSignal>& signals) {
    auto size = static_cast<uint32_t>(signals.size());
    CheckResultWithMessage(writer.Write(size), "Could not write signals count.");
    for (const auto& signal : signals) {
        CheckResultWithMessage(WriteIoSignalInfo(writer, signal), "Could not write signal info.");
    }

    return true;
}

[[nodiscard]] bool ReadControllerInfo(ChannelReader& reader, CanController& controller) {
    CheckResultWithMessage(reader.Read(controller.id), "Could not read id.");
    CheckResultWithMessage(reader.Read(controller.queueSize), "Could not read queue size.");
    CheckResultWithMessage(reader.Read(controller.bitsPerSecond), "Could not read bits per second.");
    CheckResultWithMessage(reader.Read(controller.flexibleDataRateBitsPerSecond),
                           "Could not read flexible data rate bits per second.");
    CheckResultWithMessage(ReadString(reader, controller.name), "Could not read name.");
    CheckResultWithMessage(ReadString(reader, controller.channelName), "Could not read channel name.");
    CheckResultWithMessage(ReadString(reader, controller.clusterName), "Could not read cluster name.");
    return true;
}

[[nodiscard]] bool WriteControllerInfo(ChannelWriter& writer, const CanController& controller) {
    CheckResultWithMessage(writer.Write(controller.id), "Could not write id.");
    CheckResultWithMessage(writer.Write(controller.queueSize), "Could not write queue size.");
    CheckResultWithMessage(writer.Write(controller.bitsPerSecond), "Could not write bits per second.");
    CheckResultWithMessage(writer.Write(controller.flexibleDataRateBitsPerSecond),
                           "Could not write flexible data rate bits per second.");
    CheckResultWithMessage(WriteString(writer, controller.name), "Could not write name.");
    CheckResultWithMessage(WriteString(writer, controller.channelName), "Could not write channel name.");
    CheckResultWithMessage(WriteString(writer, controller.clusterName), "Could not write cluster name.");
    return true;
}

[[nodiscard]] bool ReadControllerInfos(ChannelReader& reader, std::vector<CanController>& controllers) {
    uint32_t controllersCount = 0;
    CheckResultWithMessage(reader.Read(controllersCount), "Could not read controllers count.");
    controllers.resize(controllersCount);

    for (uint32_t i = 0; i < controllersCount; i++) {
        CheckResultWithMessage(ReadControllerInfo(reader, controllers[i]), "Could not read controller.");
    }

    return true;
}

[[nodiscard]] bool WriteControllerInfos(ChannelWriter& writer, const std::vector<CanController>& controllers) {
    auto size = static_cast<uint32_t>(controllers.size());
    CheckResultWithMessage(writer.Write(size), "Could not write controllers count.");
    for (const auto& controller : controllers) {
        CheckResultWithMessage(WriteControllerInfo(writer, controller), "Could not write controller.");
    }

    return true;
}

[[nodiscard]] bool ReadControllerInfo(ChannelReader& reader, EthController& controller) {
    CheckResultWithMessage(reader.Read(controller.id), "Could not read id.");
    CheckResultWithMessage(reader.Read(controller.queueSize), "Could not read queue size.");
    CheckResultWithMessage(reader.Read(controller.bitsPerSecond), "Could not read bits per second.");
    CheckResultWithMessage(reader.Read(controller.macAddress), "Could not read MAC address.");
    CheckResultWithMessage(ReadString(reader, controller.name), "Could not read name.");
    CheckResultWithMessage(ReadString(reader, controller.channelName), "Could not read channel name.");
    CheckResultWithMessage(ReadString(reader, controller.clusterName), "Could not read cluster name.");
    return true;
}

[[nodiscard]] bool WriteControllerInfo(ChannelWriter& writer, const EthController& controller) {
    CheckResultWithMessage(writer.Write(controller.id), "Could not write id.");
    CheckResultWithMessage(writer.Write(controller.queueSize), "Could not write queue size.");
    CheckResultWithMessage(writer.Write(controller.bitsPerSecond), "Could not write bits per second.");
    CheckResultWithMessage(writer.Write(controller.macAddress), "Could not write MAC address.");
    CheckResultWithMessage(WriteString(writer, controller.name), "Could not write name.");
    CheckResultWithMessage(WriteString(writer, controller.channelName), "Could not write channel name.");
    CheckResultWithMessage(WriteString(writer, controller.clusterName), "Could not write cluster name.");
    return true;
}

[[nodiscard]] bool ReadControllerInfos(ChannelReader& reader, std::vector<EthController>& controllers) {
    uint32_t controllersCount = 0;
    CheckResultWithMessage(reader.Read(controllersCount), "Could not read controllers count.");
    controllers.resize(controllersCount);

    for (uint32_t i = 0; i < controllersCount; i++) {
        CheckResultWithMessage(ReadControllerInfo(reader, controllers[i]), "Could not read controller.");
    }

    return true;
}

[[nodiscard]] bool WriteControllerInfos(ChannelWriter& writer, const std::vector<EthController>& controllers) {
    auto size = static_cast<uint32_t>(controllers.size());
    CheckResultWithMessage(writer.Write(size), "Could not write controllers count.");
    for (const auto& controller : controllers) {
        CheckResultWithMessage(WriteControllerInfo(writer, controller), "Could not write controller.");
    }

    return true;
}

[[nodiscard]] bool ReadControllerInfo(ChannelReader& reader, LinController& controller) {
    CheckResultWithMessage(reader.Read(controller.id), "Could not read id.");
    CheckResultWithMessage(reader.Read(controller.queueSize), "Could not read queue size.");
    CheckResultWithMessage(reader.Read(controller.bitsPerSecond), "Could not read bits per second.");
    CheckResultWithMessage(reader.Read(controller.type), "Could not read type.");
    CheckResultWithMessage(ReadString(reader, controller.name), "Could not read name.");
    CheckResultWithMessage(ReadString(reader, controller.channelName), "Could not read channel name.");
    CheckResultWithMessage(ReadString(reader, controller.clusterName), "Could not read cluster name.");
    return true;
}

[[nodiscard]] bool WriteControllerInfo(ChannelWriter& writer, const LinController& controller) {
    CheckResultWithMessage(writer.Write(controller.id), "Could not write id.");
    CheckResultWithMessage(writer.Write(controller.queueSize), "Could not write queue size.");
    CheckResultWithMessage(writer.Write(controller.bitsPerSecond), "Could not write bits per second.");
    CheckResultWithMessage(writer.Write(controller.type), "Could not write type.");
    CheckResultWithMessage(WriteString(writer, controller.name), "Could not write name.");
    CheckResultWithMessage(WriteString(writer, controller.channelName), "Could not write channel name.");
    CheckResultWithMessage(WriteString(writer, controller.clusterName), "Could not write cluster name.");
    return true;
}

[[nodiscard]] bool ReadControllerInfos(ChannelReader& reader, std::vector<LinController>& controllers) {
    uint32_t controllersCount = 0;
    CheckResultWithMessage(reader.Read(controllersCount), "Could not read controllers count.");
    controllers.resize(controllersCount);

    for (uint32_t i = 0; i < controllersCount; i++) {
        CheckResultWithMessage(ReadControllerInfo(reader, controllers[i]), "Could not read controller.");
    }

    return true;
}

[[nodiscard]] bool WriteControllerInfos(ChannelWriter& writer, const std::vector<LinController>& controllers) {
    auto size = static_cast<uint32_t>(controllers.size());
    CheckResultWithMessage(writer.Write(size), "Could not write controllers count.");
    for (const auto& controller : controllers) {
        CheckResultWithMessage(WriteControllerInfo(writer, controller), "Could not write controller.");
    }

    return true;
}

}  // namespace

namespace Protocol {

[[nodiscard]] bool ReceiveHeader(ChannelReader& reader, FrameKind& frameKind) {
#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolBeginTrace("ReceiveHeader()");
#endif

    CheckResultWithMessage(reader.Read(frameKind), "Could not receive frame header.");

#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolEndTrace("ReceiveHeader(FrameKind: " + ToString(frameKind) + ")");
#endif

    return true;
}

[[nodiscard]] bool SendOk(ChannelWriter& writer) {
#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolBeginTrace("SendOk()");
#endif

    CheckResult(WriteHeader(writer, FrameKind::Ok));
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolEndTrace("SendOk()");
#endif

    return true;
}

[[nodiscard]] bool SendError(ChannelWriter& writer, const std::string& errorMessage) {
#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolBeginTrace("SendError(ErrorMessage: \"" + errorMessage + "\")");
#endif

    CheckResult(WriteHeader(writer, FrameKind::Error));
    CheckResultWithMessage(WriteString(writer, errorMessage), "Could not write error message.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolEndTrace("SendError()");
#endif

    return true;
}

[[nodiscard]] bool ReadError(ChannelReader& reader, std::string& errorMessage) {
#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolBeginTrace("ReadError()");
#endif

    CheckResultWithMessage(ReadString(reader, errorMessage), "Could not read error message.");

#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolEndTrace("ReadError(ErrorMessage: \"" + errorMessage + "\")");
#endif

    return true;
}

[[nodiscard]] bool SendPing(ChannelWriter& writer) {
#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolBeginTrace("SendPing()");
#endif

    CheckResult(WriteHeader(writer, FrameKind::Ping));
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolEndTrace("SendPing()");
#endif

    return true;
}

[[nodiscard]] bool SendPingOk(ChannelWriter& writer, Command command) {
#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolBeginTrace("SendPingOk(Command: " + ToString(command) + ")");
#endif

    CheckResult(WriteHeader(writer, FrameKind::PingOk));
    CheckResultWithMessage(writer.Write(command), "Could not write command.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolEndTrace("SendPingOk()");
#endif

    return true;
}

[[nodiscard]] bool ReadPingOk(ChannelReader& reader, Command& command) {
#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolBeginTrace("ReadPingOk()");
#endif

    CheckResultWithMessage(reader.Read(command), "Could not read command.");

#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolEndTrace("ReadPingOk(Command: " + ToString(command) + ")");
#endif

    return true;
}

[[nodiscard]] bool SendConnect(ChannelWriter& writer,
                               uint32_t protocolVersion,
                               Mode clientMode,
                               const std::string& serverName,
                               const std::string& clientName) {
#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolBeginTrace("SendConnect(ProtocolVersion: " + std::to_string(protocolVersion) +
                          ", ClientMode: " + ToString(clientMode) + ", ServerName: \"" + serverName +
                          "\", ClientName: \"" + clientName + "\")");
#endif

    CheckResult(WriteHeader(writer, FrameKind::Connect));
    CheckResultWithMessage(writer.Write(protocolVersion), "Could not write protocol version.");
    CheckResultWithMessage(writer.Write(clientMode), "Could not write client mode.");
    CheckResultWithMessage(WriteString(writer, serverName), "Could not write server name.");
    CheckResultWithMessage(WriteString(writer, clientName), "Could not write client name.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolEndTrace("SendConnect()");
#endif

    return true;
}

[[nodiscard]] bool ReadConnect(ChannelReader& reader,
                               uint32_t& protocolVersion,
                               Mode& clientMode,
                               std::string& serverName,
                               std::string& clientName) {
#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolBeginTrace("ReadConnect()");
#endif

    CheckResultWithMessage(reader.Read(protocolVersion), "Could not read protocol version.");
    CheckResultWithMessage(reader.Read(clientMode), "Could not read client mode.");
    CheckResultWithMessage(ReadString(reader, serverName), "Could not read server name.");
    CheckResultWithMessage(ReadString(reader, clientName), "Could not read client name.");

#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolEndTrace("ReadConnect(ProtocolVersion: " + std::to_string(protocolVersion) +
                        ", ClientMode: " + ToString(clientMode) + ", ServerName: \"" + serverName +
                        "\", ClientName: \"" + clientName + "\")");
#endif

    return true;
}

[[nodiscard]] bool SendConnectOk(ChannelWriter& writer,
                                 uint32_t protocolVersion,
                                 Mode clientMode,
                                 DsVeosCoSim_SimulationTime stepSize,
                                 SimulationState simulationState,
                                 const std::vector<IoSignal>& incomingSignals,
                                 const std::vector<IoSignal>& outgoingSignals,
                                 const std::vector<CanController>& canControllers,
                                 const std::vector<EthController>& ethControllers,
                                 const std::vector<LinController>& linControllers) {
#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolBeginTrace(
        "SendConnectOk(ProtocolVersion: " + std::to_string(protocolVersion) + ", ClientMode: " + ToString(clientMode) +
        ", StepSize: " + SimulationTimeToString(stepSize) + " s, SimulationState: " + ToString(simulationState) +
        ", IncomingSignals: " + ToString(incomingSignals) + ", OutgoingSignals: " + ToString(outgoingSignals) +
        ", CanControllers: " + ToString(canControllers) + ", EthControllers: " + ToString(ethControllers) +
        ", LinControllers: " + ToString(linControllers) + ")");
#endif

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

#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolEndTrace("SendConnectOk()");
#endif

    return true;
}

[[nodiscard]] bool ReadConnectOk(ChannelReader& reader,
                                 uint32_t& protocolVersion,
                                 Mode& clientMode,
                                 DsVeosCoSim_SimulationTime& stepSize,
                                 SimulationState& simulationState,
                                 std::vector<IoSignal>& incomingSignals,
                                 std::vector<IoSignal>& outgoingSignals,
                                 std::vector<CanController>& canControllers,
                                 std::vector<EthController>& ethControllers,
                                 std::vector<LinController>& linControllers) {
#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolBeginTrace("ReadConnectOk()");
#endif

    CheckResultWithMessage(reader.Read(protocolVersion), "Could not read protocol version.");
    CheckResultWithMessage(reader.Read(clientMode), "Could not read client mode.");
    CheckResultWithMessage(reader.Read(stepSize), "Could not read step size.");
    CheckResultWithMessage(reader.Read(simulationState), "Could not read simulation state.");
    CheckResultWithMessage(ReadIoSignalInfos(reader, incomingSignals), "Could not read incoming signals.");
    CheckResultWithMessage(ReadIoSignalInfos(reader, outgoingSignals), "Could not read outgoing signals.");
    CheckResultWithMessage(ReadControllerInfos(reader, canControllers), "Could not read CAN controllers.");
    CheckResultWithMessage(ReadControllerInfos(reader, ethControllers), "Could not read ETH controllers.");
    CheckResultWithMessage(ReadControllerInfos(reader, linControllers), "Could not read LIN controllers.");

#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolEndTrace(
        "ReadConnectOk(ProtocolVersion: " + std::to_string(protocolVersion) + ", ClientMode: " + ToString(clientMode) +
        ", StepSize: " + SimulationTimeToString(stepSize) + " s, SimulationState: " + ToString(simulationState) +
        ", IncomingSignals: " + ToString(incomingSignals) + ", OutgoingSignals: " + ToString(outgoingSignals) +
        ", CanControllers: " + ToString(canControllers) + ", EthControllers: " + ToString(ethControllers) +
        ", LinControllers: " + ToString(linControllers) + ")");
#endif

    return true;
}

[[nodiscard]] bool SendStart(ChannelWriter& writer, DsVeosCoSim_SimulationTime simulationTime) {
#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolBeginTrace("SendStart(SimulationTime: " + SimulationTimeToString(simulationTime) + " s)");
#endif

    CheckResult(WriteHeader(writer, FrameKind::Start));
    CheckResultWithMessage(writer.Write(simulationTime), "Could not write simulation time.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolEndTrace("SendStart()");
#endif

    return true;
}

[[nodiscard]] bool ReadStart(ChannelReader& reader, DsVeosCoSim_SimulationTime& simulationTime) {
#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolBeginTrace("ReadStart()");
#endif

    CheckResultWithMessage(reader.Read(simulationTime), "Could not read simulation time.");

#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolEndTrace("ReadStart(SimulationTime: " + SimulationTimeToString(simulationTime) + " s)");
#endif

    return true;
}

[[nodiscard]] bool SendStop(ChannelWriter& writer, DsVeosCoSim_SimulationTime simulationTime) {
#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolBeginTrace("SendStop(SimulationTime: " + SimulationTimeToString(simulationTime) + " s)");
#endif

    CheckResult(WriteHeader(writer, FrameKind::Stop));
    CheckResultWithMessage(writer.Write(simulationTime), "Could not write simulation time.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolEndTrace("SendStop()");
#endif

    return true;
}

[[nodiscard]] bool ReadStop(ChannelReader& reader, DsVeosCoSim_SimulationTime& simulationTime) {
#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolBeginTrace("ReadStop()");
#endif

    CheckResultWithMessage(reader.Read(simulationTime), "Could not read simulation time.");

#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolEndTrace("ReadStop(SimulationTime: " + SimulationTimeToString(simulationTime) + " s)");
#endif

    return true;
}

[[nodiscard]] bool SendTerminate(ChannelWriter& writer,
                                 DsVeosCoSim_SimulationTime simulationTime,
                                 DsVeosCoSim_TerminateReason reason) {
#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolBeginTrace("SendTerminate(SimulationTime: " + SimulationTimeToString(simulationTime) +
                          " s, Reason: " + ToString(reason) + ")");
#endif

    CheckResult(WriteHeader(writer, FrameKind::Terminate));
    CheckResultWithMessage(writer.Write(simulationTime), "Could not write simulation time.");
    CheckResultWithMessage(writer.Write(reason), "Could not write reason.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolEndTrace("SendTerminate()");
#endif

    return true;
}

[[nodiscard]] bool ReadTerminate(ChannelReader& reader,
                                 DsVeosCoSim_SimulationTime& simulationTime,
                                 DsVeosCoSim_TerminateReason& reason) {
#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolBeginTrace("ReadTerminate()");
#endif

    CheckResultWithMessage(reader.Read(simulationTime), "Could not read simulation time.");
    CheckResultWithMessage(reader.Read(reason), "Could not read reason.");

#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolEndTrace("ReadTerminate(SimulationTime: " + SimulationTimeToString(simulationTime) +
                        " s, Reason: " + ToString(reason) + ")");
#endif

    return true;
}

[[nodiscard]] bool SendPause(ChannelWriter& writer, DsVeosCoSim_SimulationTime simulationTime) {
#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolBeginTrace("SendPause(SimulationTime: " + SimulationTimeToString(simulationTime) + " s)");
#endif

    CheckResult(WriteHeader(writer, FrameKind::Pause));
    CheckResultWithMessage(writer.Write(simulationTime), "Could not write simulation time.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolEndTrace("SendPause()");
#endif

    return true;
}

[[nodiscard]] bool ReadPause(ChannelReader& reader, DsVeosCoSim_SimulationTime& simulationTime) {
#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolBeginTrace("ReadPause()");
#endif

    CheckResultWithMessage(reader.Read(simulationTime), "Could not read simulation time.");

#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolEndTrace("ReadPause(SimulationTime: " + SimulationTimeToString(simulationTime) + " s)");
#endif

    return true;
}

[[nodiscard]] bool SendContinue(ChannelWriter& writer, DsVeosCoSim_SimulationTime simulationTime) {
#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolBeginTrace("SendContinue(SimulationTime: " + SimulationTimeToString(simulationTime) + " s)");
#endif

    CheckResult(WriteHeader(writer, FrameKind::Continue));
    CheckResultWithMessage(writer.Write(simulationTime), "Could not write simulation time.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolEndTrace("SendContinue()");
#endif

    return true;
}

[[nodiscard]] bool ReadContinue(ChannelReader& reader, DsVeosCoSim_SimulationTime& simulationTime) {
#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolBeginTrace("ReadContinue()");
#endif

    CheckResultWithMessage(reader.Read(simulationTime), "Could not read simulation time.");

#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolEndTrace("SendStep(SimulationTime: " + SimulationTimeToString(simulationTime) + " s)");
#endif

    return true;
}

[[nodiscard]] bool SendStep(ChannelWriter& writer,
                            DsVeosCoSim_SimulationTime simulationTime,
                            const IoBuffer& ioBuffer,
                            const BusBuffer& busBuffer) {
#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolBeginTrace("SendStep(SimulationTime: " + SimulationTimeToString(simulationTime) + " s)");
#endif

    CheckResult(WriteHeader(writer, FrameKind::Step));
    CheckResultWithMessage(writer.Write(simulationTime), "Could not write simulation time.");
    CheckResultWithMessage(ioBuffer.Serialize(writer), "Could not write IO buffer data.");
    CheckResultWithMessage(busBuffer.Serialize(writer), "Could not write bus buffer data.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolEndTrace("SendStep()");
#endif

    return true;
}

[[nodiscard]] bool ReadStep(ChannelReader& reader,
                            DsVeosCoSim_SimulationTime& simulationTime,
                            const IoBuffer& ioBuffer,
                            const BusBuffer& busBuffer,
                            const Callbacks& callbacks) {
#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolBeginTrace("ReadStep()");
#endif

    CheckResultWithMessage(reader.Read(simulationTime), "Could not read simulation time.");

    if (callbacks.simulationBeginStepCallback) {
        callbacks.simulationBeginStepCallback(simulationTime);
    }

    CheckResultWithMessage(ioBuffer.Deserialize(reader, simulationTime, callbacks), "Could not read IO buffer data.");
    CheckResultWithMessage(busBuffer.Deserialize(reader, simulationTime, callbacks), "Could not read bus buffer data.");

#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolEndTrace("ReadStep(SimulationTime: " + SimulationTimeToString(simulationTime) + " s)");
#endif

    return true;
}

[[nodiscard]] bool SendStepOk(ChannelWriter& writer,
                              DsVeosCoSim_SimulationTime nextSimulationTime,
                              Command command,
                              const IoBuffer& ioBuffer,
                              const BusBuffer& busBuffer) {
#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolBeginTrace("SendStepOk(NextSimulationTime: " + SimulationTimeToString(nextSimulationTime) +
                          " s, Command: " + ToString(command) + ")");
#endif

    CheckResult(WriteHeader(writer, FrameKind::StepOk));
    CheckResultWithMessage(writer.Write(nextSimulationTime), "Could not write simulation time.");
    CheckResultWithMessage(writer.Write(command), "Could not write command.");
    CheckResultWithMessage(ioBuffer.Serialize(writer), "Could not write IO buffer data.");
    CheckResultWithMessage(busBuffer.Serialize(writer), "Could not write bus buffer data.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolEndTrace("SendStepOk()");
#endif

    return true;
}

[[nodiscard]] bool ReadStepOk(ChannelReader& reader,
                              DsVeosCoSim_SimulationTime& nextSimulationTime,
                              Command& command,
                              const IoBuffer& ioBuffer,
                              const BusBuffer& busBuffer,
                              const Callbacks& callbacks) {
#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolBeginTrace("ReadStepOk()");
#endif

    CheckResultWithMessage(reader.Read(nextSimulationTime), "Could not read simulation time.");
    CheckResultWithMessage(reader.Read(command), "Could not read command.");

    if (callbacks.simulationBeginStepCallback) {
        callbacks.simulationBeginStepCallback(nextSimulationTime);
    }

    CheckResultWithMessage(ioBuffer.Deserialize(reader, nextSimulationTime, callbacks),
                           "Could not read IO buffer data.");
    CheckResultWithMessage(busBuffer.Deserialize(reader, nextSimulationTime, callbacks),
                           "Could not read bus buffer data.");

#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolEndTrace("ReadStepOk(NextSimulationTime: " + SimulationTimeToString(nextSimulationTime) +
                        " s, Command: " + ToString(command) + ")");
#endif

    return true;
}

[[nodiscard]] bool SendSetPort(ChannelWriter& writer, const std::string& serverName, uint16_t port) {
#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolBeginTrace("SendSetPort(ServerName: \"" + serverName + "\", Port: " + std::to_string(port) + ")");
#endif

    CheckResult(WriteHeader(writer, FrameKind::SetPort));
    CheckResultWithMessage(WriteString(writer, serverName), "Could not write server name.");
    CheckResultWithMessage(writer.Write(port), "Could not write port.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolEndTrace("SendSetPort()");
#endif

    return true;
}

[[nodiscard]] bool ReadSetPort(ChannelReader& reader, std::string& serverName, uint16_t& port) {
#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolBeginTrace("ReadSetPort()");
#endif

    CheckResultWithMessage(ReadString(reader, serverName), "Could not read server name.");
    CheckResultWithMessage(reader.Read(port), "Could not read port.");

#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolEndTrace("ReadSetPort(ServerName: \"" + serverName + "\", Port: " + std::to_string(port) + ")");
#endif

    return true;
}

[[nodiscard]] bool SendUnsetPort(ChannelWriter& writer, const std::string& serverName) {
#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolBeginTrace("SendUnsetPort(ServerName: \"" + serverName + "\")");
#endif

    CheckResult(WriteHeader(writer, FrameKind::UnsetPort));
    CheckResultWithMessage(WriteString(writer, serverName), "Could not write server name.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolEndTrace("SendUnsetPort()");
#endif

    return true;
}

[[nodiscard]] bool ReadUnsetPort(ChannelReader& reader, std::string& serverName) {
#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolBeginTrace("ReadUnsetPort()");
#endif

    CheckResultWithMessage(ReadString(reader, serverName), "Could not read server name.");

#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolEndTrace("ReadUnsetPort(ServerName: \"" + serverName + "\")");
#endif

    return true;
}

[[nodiscard]] bool SendGetPort(ChannelWriter& writer, const std::string& serverName) {
#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolBeginTrace("SendGetPort(ServerName: \"" + serverName + "\")");
#endif

    CheckResult(WriteHeader(writer, FrameKind::GetPort));
    CheckResultWithMessage(WriteString(writer, serverName), "Could not write server name.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolEndTrace("SendGetPort()");
#endif

    return true;
}

[[nodiscard]] bool ReadGetPort(ChannelReader& reader, std::string& serverName) {
#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolBeginTrace("ReadGetPort()");
#endif

    CheckResultWithMessage(ReadString(reader, serverName), "Could not read server name.");

#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolEndTrace("ReadGetPort(ServerName: \"" + serverName + "\")");
#endif

    return true;
}

[[nodiscard]] bool SendGetPortOk(ChannelWriter& writer, uint16_t port) {
#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolBeginTrace("SendGetPortOk(Port: " + std::to_string(port) + ")");
#endif

    CheckResult(WriteHeader(writer, FrameKind::GetPortOk));
    CheckResultWithMessage(writer.Write(port), "Could not write port.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");

#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolEndTrace("SendGetPortOk()");
#endif

    return true;
}

[[nodiscard]] bool ReadGetPortOk(ChannelReader& reader, uint16_t& port) {
#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolBeginTrace("ReadGetPortOk()");
#endif

    CheckResultWithMessage(reader.Read(port), "Could not read port.");

#ifdef DSVEOSCOSIM_ENABLE_TRACING
    LogProtocolEndTrace("ReadGetPortOk(Port: " + std::to_string(port) + ")");
#endif

    return true;
}

}  // namespace Protocol

}  // namespace DsVeosCoSim
