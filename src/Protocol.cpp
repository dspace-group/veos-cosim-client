// Copyright dSPACE GmbH. All rights reserved.

#include "Protocol.h"

#include <bit>

#include "Result.h"

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

bool ReceiveHeader(ChannelReader& reader, FrameKind& frameKind) {
    CheckResultWithMessage(reader.Read(frameKind), "Could not receive frame header.");
    return true;
}

bool SendOk(ChannelWriter& writer) {
    CheckResult(WriteHeader(writer, FrameKind::Ok));
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");
    return true;
}

bool SendError(ChannelWriter& writer, std::string_view errorStr) {
    CheckResult(WriteHeader(writer, FrameKind::Error));
    CheckResultWithMessage(WriteString(writer, errorStr), "Could not write error message.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");
    return true;
}

bool ReadError(ChannelReader& reader, std::string& errorStr) {
    CheckResultWithMessage(ReadString(reader, errorStr), "Could not read error message.");
    return true;
}

bool SendPing(ChannelWriter& writer) {
    CheckResult(WriteHeader(writer, FrameKind::Ping));
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");
    return true;
}

bool SendPingOk(ChannelWriter& writer, Command command) {
    CheckResult(WriteHeader(writer, FrameKind::PingOk));
    CheckResultWithMessage(writer.Write(command), "Could not write command.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");
    return true;
}

bool ReadPingOk(ChannelReader& reader, Command& command) {
    CheckResultWithMessage(reader.Read(command), "Could not read command.");
    return true;
}

bool SendConnect(ChannelWriter& writer,
                 uint32_t protocolVersion,
                 Mode clientMode,
                 std::string_view serverName,
                 std::string_view clientName) {
    CheckResult(WriteHeader(writer, FrameKind::Connect));
    CheckResultWithMessage(writer.Write(protocolVersion), "Could not write protocol version.");
    CheckResultWithMessage(writer.Write(clientMode), "Could not write client mode.");
    CheckResultWithMessage(WriteString(writer, serverName), "Could not write server name.");
    CheckResultWithMessage(WriteString(writer, clientName), "Could not write client name.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");
    return true;
}

bool ReadConnect(ChannelReader& reader,
                 uint32_t& protocolVersion,
                 Mode& clientMode,
                 std::string& serverName,
                 std::string& clientName) {
    CheckResultWithMessage(reader.Read(protocolVersion), "Could not read protocol version.");
    CheckResultWithMessage(reader.Read(clientMode), "Could not read client mode.");
    CheckResultWithMessage(ReadString(reader, serverName), "Could not read server name.");
    CheckResultWithMessage(ReadString(reader, clientName), "Could not read client name.");
    return true;
}

bool SendConnectOk(ChannelWriter& writer,
                   uint32_t protocolVersion,
                   Mode clientMode,
                   SimulationTime stepSize,
                   SimulationState simulationState,
                   const std::vector<IoSignal>& incomingSignals,
                   const std::vector<IoSignal>& outgoingSignals,
                   const std::vector<CanController>& canControllers,
                   const std::vector<EthController>& ethControllers,
                   const std::vector<LinController>& linControllers) {
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
    return true;
}

bool ReadConnectOk(ChannelReader& reader,
                   uint32_t& protocolVersion,
                   Mode& clientMode,
                   SimulationTime& stepSize,
                   SimulationState& simulationState,
                   std::vector<IoSignal>& incomingSignals,
                   std::vector<IoSignal>& outgoingSignals,
                   std::vector<CanController>& canControllers,
                   std::vector<EthController>& ethControllers,
                   std::vector<LinController>& linControllers) {
    CheckResultWithMessage(reader.Read(protocolVersion), "Could not read protocol version.");
    CheckResultWithMessage(reader.Read(clientMode), "Could not read client mode.");
    CheckResultWithMessage(reader.Read(stepSize), "Could not read step size.");
    CheckResultWithMessage(reader.Read(simulationState), "Could not read simulation state.");
    CheckResultWithMessage(ReadIoSignalInfos(reader, incomingSignals), "Could not read incoming signals.");
    CheckResultWithMessage(ReadIoSignalInfos(reader, outgoingSignals), "Could not read outgoing signals.");
    CheckResultWithMessage(ReadControllerInfos(reader, canControllers), "Could not read CAN controllers.");
    CheckResultWithMessage(ReadControllerInfos(reader, ethControllers), "Could not read ETH controllers.");
    CheckResultWithMessage(ReadControllerInfos(reader, linControllers), "Could not read LIN controllers.");
    return true;
}

bool SendStart(ChannelWriter& writer, SimulationTime simulationTime) {
    CheckResult(WriteHeader(writer, FrameKind::Start));
    CheckResultWithMessage(writer.Write(simulationTime), "Could not write simulation time.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");
    return true;
}

bool ReadStart(ChannelReader& reader, SimulationTime& simulationTime) {
    CheckResultWithMessage(reader.Read(simulationTime), "Could not read simulation time.");
    return true;
}

bool SendStop(ChannelWriter& writer, SimulationTime simulationTime) {
    CheckResult(WriteHeader(writer, FrameKind::Stop));
    CheckResultWithMessage(writer.Write(simulationTime), "Could not write simulation time.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");
    return true;
}

bool ReadStop(ChannelReader& reader, SimulationTime& simulationTime) {
    CheckResultWithMessage(reader.Read(simulationTime), "Could not read simulation time.");
    return true;
}

bool SendTerminate(ChannelWriter& writer, SimulationTime simulationTime, TerminateReason reason) {
    CheckResult(WriteHeader(writer, FrameKind::Terminate));
    CheckResultWithMessage(writer.Write(simulationTime), "Could not write simulation time.");
    CheckResultWithMessage(writer.Write(reason), "Could not write reason.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");
    return true;
}

bool ReadTerminate(ChannelReader& reader, SimulationTime& simulationTime, TerminateReason& reason) {
    CheckResultWithMessage(reader.Read(simulationTime), "Could not read simulation time.");
    CheckResultWithMessage(reader.Read(reason), "Could not read reason.");
    return true;
}

bool SendPause(ChannelWriter& writer, SimulationTime simulationTime) {
    CheckResult(WriteHeader(writer, FrameKind::Pause));
    CheckResultWithMessage(writer.Write(simulationTime), "Could not write simulation time.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");
    return true;
}

bool ReadPause(ChannelReader& reader, SimulationTime& simulationTime) {
    CheckResultWithMessage(reader.Read(simulationTime), "Could not read simulation time.");
    return true;
}

bool SendContinue(ChannelWriter& writer, SimulationTime simulationTime) {
    CheckResult(WriteHeader(writer, FrameKind::Continue));
    CheckResultWithMessage(writer.Write(simulationTime), "Could not write simulation time.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");
    return true;
}

bool ReadContinue(ChannelReader& reader, SimulationTime& simulationTime) {
    CheckResultWithMessage(reader.Read(simulationTime), "Could not read simulation time.");
    return true;
}

bool SendStep(ChannelWriter& writer, SimulationTime simulationTime, IoBuffer& ioBuffer, BusBuffer& busBuffer) {
    CheckResult(WriteHeader(writer, FrameKind::Step));
    CheckResultWithMessage(writer.Write(simulationTime), "Could not write simulation time.");
    CheckResultWithMessage(ioBuffer.Serialize(writer), "Could not write IO buffer data.");
    CheckResultWithMessage(busBuffer.Serialize(writer), "Could not write bus buffer data.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");
    return true;
}

bool ReadStep(ChannelReader& reader,
              SimulationTime& simulationTime,
              IoBuffer& ioBuffer,
              BusBuffer& busBuffer,
              const Callbacks& callbacks) {
    CheckResultWithMessage(reader.Read(simulationTime), "Could not read simulation time.");

    if (callbacks.simulationBeginStepCallback) {
        callbacks.simulationBeginStepCallback(simulationTime);
    }

    CheckResultWithMessage(ioBuffer.Deserialize(reader, simulationTime, callbacks), "Could not read IO buffer data.");
    CheckResultWithMessage(busBuffer.Deserialize(reader, simulationTime, callbacks), "Could not read bus buffer data.");
    return true;
}

bool SendStepOk(ChannelWriter& writer,
                SimulationTime simulationTime,
                Command command,
                IoBuffer& ioBuffer,
                BusBuffer& busBuffer) {
    CheckResult(WriteHeader(writer, FrameKind::StepOk));
    CheckResultWithMessage(writer.Write(simulationTime), "Could not write simulation time.");
    CheckResultWithMessage(writer.Write(command), "Could not write command.");
    CheckResultWithMessage(ioBuffer.Serialize(writer), "Could not write IO buffer data.");
    CheckResultWithMessage(busBuffer.Serialize(writer), "Could not write bus buffer data.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");
    return true;
}

bool ReadStepOk(ChannelReader& reader,
                SimulationTime& simulationTime,
                Command& command,
                IoBuffer& ioBuffer,
                BusBuffer& busBuffer,
                const Callbacks& callbacks) {
    CheckResultWithMessage(reader.Read(simulationTime), "Could not read simulation time.");
    CheckResultWithMessage(reader.Read(command), "Could not read command.");

    if (callbacks.simulationBeginStepCallback) {
        callbacks.simulationBeginStepCallback(simulationTime);
    }

    CheckResultWithMessage(ioBuffer.Deserialize(reader, simulationTime, callbacks), "Could not read IO buffer data.");
    CheckResultWithMessage(busBuffer.Deserialize(reader, simulationTime, callbacks), "Could not read bus buffer data.");
    return true;
}

bool SendSetPort(ChannelWriter& writer, std::string_view serverName, uint16_t port) {
    CheckResult(WriteHeader(writer, FrameKind::SetPort));
    CheckResultWithMessage(WriteString(writer, serverName), "Could not write server name.");
    CheckResultWithMessage(writer.Write(port), "Could not write port.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");
    return true;
}

bool ReadSetPort(ChannelReader& reader, std::string& serverName, uint16_t& port) {
    CheckResultWithMessage(ReadString(reader, serverName), "Could not read server name.");
    CheckResultWithMessage(reader.Read(port), "Could not read port.");
    return true;
}

bool SendUnsetPort(ChannelWriter& writer, std::string_view serverName) {
    CheckResult(WriteHeader(writer, FrameKind::UnsetPort));
    CheckResultWithMessage(WriteString(writer, serverName), "Could not write server name.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");
    return true;
}

bool ReadUnsetPort(ChannelReader& reader, std::string& serverName) {
    CheckResultWithMessage(ReadString(reader, serverName), "Could not read server name.");
    return true;
}

bool SendGetPort(ChannelWriter& writer, std::string_view serverName) {
    CheckResult(WriteHeader(writer, FrameKind::GetPort));
    CheckResultWithMessage(WriteString(writer, serverName), "Could not write server name.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");
    return true;
}

bool ReadGetPort(ChannelReader& reader, std::string& serverName) {
    CheckResultWithMessage(ReadString(reader, serverName), "Could not read server name.");
    return true;
}

bool SendGetPortOk(ChannelWriter& writer, uint16_t port) {
    CheckResult(WriteHeader(writer, FrameKind::GetPortOk));
    CheckResultWithMessage(writer.Write(port), "Could not write port.");
    CheckResultWithMessage(writer.EndWrite(), "Could not finish frame.");
    return true;
}

bool ReadGetPortOk(ChannelReader& reader, uint16_t& port) {
    CheckResultWithMessage(reader.Read(port), "Could not read port.");
    return true;
}

}  // namespace Protocol

static_assert(std::endian::native == std::endian::little, "Only supported on little endian platforms.");

}  // namespace DsVeosCoSim
