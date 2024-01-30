// Copyright dSPACE GmbH. All rights reserved.

#include "Protocol.h"

#include <bit>

#include "CoSimTypes.h"
#include "Communication.h"

namespace DsVeosCoSim {

namespace {

Result WriteHeader(Channel& channel, FrameKind frameKind) {
    return channel.Write(frameKind);
}

Result ReadString(Channel& channel, std::string& string) {
    uint32_t size = 0;
    CheckResult(channel.Read(size));
    string.resize(size);
    return channel.Read(string.data(), size);
}

Result WriteString(Channel& channel, std::string_view string) {
    const auto size = static_cast<uint32_t>(string.size());
    CheckResult(channel.Write(size));
    return channel.Write(string.data(), size);
}

Result ReadIoSignalInfo(Channel& channel, IoSignal& signal) {
    CheckResult(channel.Read(signal.id));
    CheckResult(channel.Read(signal.length));
    CheckResult(channel.Read(signal.dataType));
    CheckResult(channel.Read(signal.sizeKind));
    return ReadString(channel, signal.name);
}

Result WriteIoSignalInfo(Channel& channel, const IoSignal& signal) {
    CheckResult(channel.Write(signal.id));
    CheckResult(channel.Write(signal.length));
    CheckResult(channel.Write(signal.dataType));
    CheckResult(channel.Write(signal.sizeKind));
    return WriteString(channel, signal.name);
}

Result ReadIoSignalInfos(Channel& channel, std::vector<IoSignal>& signals) {
    uint32_t signalsCount = 0;
    CheckResult(channel.Read(signalsCount));
    signals.resize(signalsCount);

    for (uint32_t i = 0; i < signalsCount; i++) {
        CheckResult(ReadIoSignalInfo(channel, signals[i]));
    }

    return Result::Ok;
}

Result WriteIoSignalInfos(Channel& channel, const std::vector<IoSignal>& signals) {
    CheckResult(channel.Write(static_cast<uint32_t>(signals.size())));
    for (const auto& signal : signals) {
        CheckResult(WriteIoSignalInfo(channel, signal));
    }

    return Result::Ok;
}

Result ReadControllerInfo(Channel& channel, CanController& controller) {
    CheckResult(channel.Read(controller.id));
    CheckResult(channel.Read(controller.queueSize));
    CheckResult(channel.Read(controller.bitsPerSecond));
    CheckResult(channel.Read(controller.flexibleDataRateBitsPerSecond));
    CheckResult(ReadString(channel, controller.name));
    CheckResult(ReadString(channel, controller.channelName));
    return ReadString(channel, controller.clusterName);
}

Result WriteControllerInfo(Channel& channel, const CanController& controller) {
    CheckResult(channel.Write(controller.id));
    CheckResult(channel.Write(controller.queueSize));
    CheckResult(channel.Write(controller.bitsPerSecond));
    CheckResult(channel.Write(controller.flexibleDataRateBitsPerSecond));
    CheckResult(WriteString(channel, controller.name));
    CheckResult(WriteString(channel, controller.channelName));
    return WriteString(channel, controller.clusterName);
}

Result ReadControllerInfos(Channel& channel, std::vector<CanController>& controllers) {
    uint32_t controllersCount = 0;
    CheckResult(channel.Read(controllersCount));
    controllers.resize(controllersCount);

    for (uint32_t i = 0; i < controllersCount; i++) {
        CheckResult(ReadControllerInfo(channel, controllers[i]));
    }

    return Result::Ok;
}

Result WriteControllerInfos(Channel& channel, const std::vector<CanController>& controllers) {
    CheckResult(channel.Write(static_cast<uint32_t>(controllers.size())));
    for (const auto& controller : controllers) {
        CheckResult(WriteControllerInfo(channel, controller));
    }

    return Result::Ok;
}

Result ReadControllerInfo(Channel& channel, EthController& controller) {
    CheckResult(channel.Read(controller.id));
    CheckResult(channel.Read(controller.queueSize));
    CheckResult(channel.Read(controller.bitsPerSecond));
    CheckResult(channel.Read(controller.macAddress));
    CheckResult(ReadString(channel, controller.name));
    CheckResult(ReadString(channel, controller.channelName));
    return ReadString(channel, controller.clusterName);
}

Result WriteControllerInfo(Channel& channel, const EthController& controller) {
    CheckResult(channel.Write(controller.id));
    CheckResult(channel.Write(controller.queueSize));
    CheckResult(channel.Write(controller.bitsPerSecond));
    CheckResult(channel.Write(controller.macAddress));
    CheckResult(WriteString(channel, controller.name));
    CheckResult(WriteString(channel, controller.channelName));
    return WriteString(channel, controller.clusterName);
}

Result ReadControllerInfos(Channel& channel, std::vector<EthController>& controllers) {
    uint32_t controllersCount = 0;
    CheckResult(channel.Read(controllersCount));
    controllers.resize(controllersCount);

    for (uint32_t i = 0; i < controllersCount; i++) {
        CheckResult(ReadControllerInfo(channel, controllers[i]));
    }

    return Result::Ok;
}

Result WriteControllerInfos(Channel& channel, const std::vector<EthController>& controllers) {
    CheckResult(channel.Write(static_cast<uint32_t>(controllers.size())));
    for (const auto& controller : controllers) {
        CheckResult(WriteControllerInfo(channel, controller));
    }

    return Result::Ok;
}

Result ReadControllerInfo(Channel& channel, LinController& controller) {
    CheckResult(channel.Read(controller.id));
    CheckResult(channel.Read(controller.queueSize));
    CheckResult(channel.Read(controller.bitsPerSecond));
    CheckResult(channel.Read(controller.type));
    CheckResult(ReadString(channel, controller.name));
    CheckResult(ReadString(channel, controller.channelName));
    return ReadString(channel, controller.clusterName);
}

Result WriteControllerInfo(Channel& channel, const LinController& controller) {
    CheckResult(channel.Write(controller.id));
    CheckResult(channel.Write(controller.queueSize));
    CheckResult(channel.Write(controller.bitsPerSecond));
    CheckResult(channel.Write(controller.type));
    CheckResult(WriteString(channel, controller.name));
    CheckResult(WriteString(channel, controller.channelName));
    return WriteString(channel, controller.clusterName);
}

Result ReadControllerInfos(Channel& channel, std::vector<LinController>& controllers) {
    uint32_t controllersCount = 0;
    CheckResult(channel.Read(controllersCount));
    controllers.resize(controllersCount);

    for (uint32_t i = 0; i < controllersCount; i++) {
        CheckResult(ReadControllerInfo(channel, controllers[i]));
    }

    return Result::Ok;
}

Result WriteControllerInfos(Channel& channel, const std::vector<LinController>& controllers) {
    CheckResult(channel.Write(static_cast<uint32_t>(controllers.size())));
    for (const auto& controller : controllers) {
        CheckResult(WriteControllerInfo(channel, controller));
    }

    return Result::Ok;
}

}  // namespace

std::string ToString(FrameKind frameKind) {
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
        default:  // NOLINT(clang-diagnostic-covered-switch-default)
            return std::to_string(static_cast<int>(frameKind));
    }
}

namespace Protocol {

Result ReceiveHeader(Channel& channel, FrameKind& frameKind) {
    return channel.Read(frameKind);
}

Result SendOk(Channel& channel) {
    CheckResult(WriteHeader(channel, FrameKind::Ok));
    return channel.EndWrite();
}

Result SendError(Channel& channel, std::string_view errorStr) {
    CheckResult(WriteHeader(channel, FrameKind::Error));
    CheckResult(WriteString(channel, errorStr));
    return channel.EndWrite();
}

Result ReadError(Channel& channel, std::string& errorStr) {
    return ReadString(channel, errorStr);
}

Result SendPing(Channel& channel) {
    CheckResult(WriteHeader(channel, FrameKind::Ping));
    return channel.EndWrite();
}

Result SendPingOk(Channel& channel, Command command) {
    CheckResult(WriteHeader(channel, FrameKind::PingOk));
    CheckResult(channel.Write(command));
    return channel.EndWrite();
}

Result ReadPingOk(Channel& channel, Command& command) {
    return channel.Read(command);
}

Result SendConnect(Channel& channel, uint32_t protocolVersion, Mode clientMode, std::string_view serverName, std::string_view clientName) {
    CheckResult(WriteHeader(channel, FrameKind::Connect));
    CheckResult(channel.Write(protocolVersion));
    CheckResult(channel.Write(clientMode));
    CheckResult(WriteString(channel, serverName));
    CheckResult(WriteString(channel, clientName));
    return channel.EndWrite();
}

Result ReadConnect(Channel& channel, uint32_t& protocolVersion, Mode& clientMode, std::string& serverName, std::string& clientName) {
    CheckResult(channel.Read(protocolVersion));
    CheckResult(channel.Read(clientMode));
    CheckResult(ReadString(channel, serverName));
    return ReadString(channel, clientName);
}

Result SendConnectOk(Channel& channel,
                     uint32_t protocolVersion,
                     Mode clientMode,
                     SimulationTime stepSize,
                     SimulationState simulationState,
                     const std::vector<IoSignal>& incomingSignals,
                     const std::vector<IoSignal>& outgoingSignals,
                     const std::vector<CanController>& canControllers,
                     const std::vector<EthController>& ethControllers,
                     const std::vector<LinController>& linControllers) {
    CheckResult(WriteHeader(channel, FrameKind::ConnectOk));
    CheckResult(channel.Write(protocolVersion));
    CheckResult(channel.Write(clientMode));
    CheckResult(channel.Write(stepSize));
    CheckResult(channel.Write(simulationState));
    CheckResult(WriteIoSignalInfos(channel, incomingSignals));
    CheckResult(WriteIoSignalInfos(channel, outgoingSignals));
    CheckResult(WriteControllerInfos(channel, canControllers));
    CheckResult(WriteControllerInfos(channel, ethControllers));
    CheckResult(WriteControllerInfos(channel, linControllers));
    return channel.EndWrite();
}

Result ReadConnectOk(Channel& channel,
                     uint32_t& protocolVersion,
                     Mode& clientMode,
                     SimulationTime& stepSize,
                     SimulationState& simulationState,
                     std::vector<IoSignal>& incomingSignals,
                     std::vector<IoSignal>& outgoingSignals,
                     std::vector<CanController>& canControllers,
                     std::vector<EthController>& ethControllers,
                     std::vector<LinController>& linControllers) {
    CheckResult(channel.Read(protocolVersion));
    CheckResult(channel.Read(clientMode));
    CheckResult(channel.Read(stepSize));
    CheckResult(channel.Read(simulationState));
    CheckResult(ReadIoSignalInfos(channel, incomingSignals));
    CheckResult(ReadIoSignalInfos(channel, outgoingSignals));
    CheckResult(ReadControllerInfos(channel, canControllers));
    CheckResult(ReadControllerInfos(channel, ethControllers));
    return ReadControllerInfos(channel, linControllers);
}

Result SendStart(Channel& channel, SimulationTime simulationTime) {
    CheckResult(WriteHeader(channel, FrameKind::Start));
    CheckResult(channel.Write(simulationTime));
    return channel.EndWrite();
}

Result ReadStart(Channel& channel, SimulationTime& simulationTime) {
    return channel.Read(simulationTime);
}

Result SendStop(Channel& channel, SimulationTime simulationTime) {
    CheckResult(WriteHeader(channel, FrameKind::Stop));
    CheckResult(channel.Write(simulationTime));
    return channel.EndWrite();
}

Result ReadStop(Channel& channel, SimulationTime& simulationTime) {
    return channel.Read(simulationTime);
}

Result SendTerminate(Channel& channel, SimulationTime simulationTime, TerminateReason reason) {
    CheckResult(WriteHeader(channel, FrameKind::Terminate));
    CheckResult(channel.Write(simulationTime));
    CheckResult(channel.Write(reason));
    return channel.EndWrite();
}

Result ReadTerminate(Channel& channel, SimulationTime& simulationTime, TerminateReason& reason) {
    CheckResult(channel.Read(simulationTime));
    return channel.Read(reason);
}

Result SendPause(Channel& channel, SimulationTime simulationTime) {
    CheckResult(WriteHeader(channel, FrameKind::Pause));
    CheckResult(channel.Write(simulationTime));
    return channel.EndWrite();
}

Result ReadPause(Channel& channel, SimulationTime& simulationTime) {
    return channel.Read(simulationTime);
}

Result SendContinue(Channel& channel, SimulationTime simulationTime) {
    CheckResult(WriteHeader(channel, FrameKind::Continue));
    CheckResult(channel.Write(simulationTime));
    return channel.EndWrite();
}

Result ReadContinue(Channel& channel, SimulationTime& simulationTime) {
    return channel.Read(simulationTime);
}

Result SendStep(Channel& channel, SimulationTime simulationTime, IoBuffer& ioBuffer, BusBuffer& busBuffer) {
    CheckResult(WriteHeader(channel, FrameKind::Step));
    CheckResult(channel.Write(simulationTime));
    CheckResult(ioBuffer.Serialize(channel));
    CheckResult(busBuffer.Serialize(channel));
    return channel.EndWrite();
}

Result ReadStep(Channel& channel, SimulationTime& simulationTime, IoBuffer& ioBuffer, BusBuffer& busBuffer, const Callbacks& callbacks) {
    CheckResult(channel.Read(simulationTime));

    if (callbacks.simulationBeginStepCallback) {
        callbacks.simulationBeginStepCallback(simulationTime);
    }

    CheckResult(ioBuffer.Deserialize(channel, simulationTime, callbacks));
    return busBuffer.Deserialize(channel, simulationTime, callbacks);
}

Result SendStepOk(Channel& channel, SimulationTime simulationTime, Command command, IoBuffer& ioBuffer, BusBuffer& busBuffer) {
    CheckResult(WriteHeader(channel, FrameKind::StepOk));
    CheckResult(channel.Write(simulationTime));
    CheckResult(channel.Write(command));
    CheckResult(ioBuffer.Serialize(channel));
    CheckResult(busBuffer.Serialize(channel));
    return channel.EndWrite();
}

Result ReadStepOk(Channel& channel, SimulationTime& simulationTime, Command& command, IoBuffer& ioBuffer, BusBuffer& busBuffer, const Callbacks& callbacks) {
    CheckResult(channel.Read(simulationTime));
    CheckResult(channel.Read(command));

    if (callbacks.simulationBeginStepCallback) {
        callbacks.simulationBeginStepCallback(simulationTime);
    }

    CheckResult(ioBuffer.Deserialize(channel, simulationTime, callbacks));
    return busBuffer.Deserialize(channel, simulationTime, callbacks);
}

Result SendSetPort(Channel& channel, std::string_view serverName, uint16_t port) {
    CheckResult(WriteHeader(channel, FrameKind::SetPort));
    CheckResult(WriteString(channel, serverName));
    CheckResult(channel.Write(port));
    return channel.EndWrite();
}

Result ReadSetPort(Channel& channel, std::string& serverName, uint16_t& port) {
    CheckResult(ReadString(channel, serverName));
    return channel.Read(port);
}

Result SendUnsetPort(Channel& channel, std::string_view serverName) {
    CheckResult(WriteHeader(channel, FrameKind::UnsetPort));
    CheckResult(WriteString(channel, serverName));
    return channel.EndWrite();
}

Result ReadUnsetPort(Channel& channel, std::string& serverName) {
    return ReadString(channel, serverName);
}

Result SendGetPort(Channel& channel, std::string_view serverName) {
    CheckResult(WriteHeader(channel, FrameKind::GetPort));
    CheckResult(WriteString(channel, serverName));
    return channel.EndWrite();
}

Result ReadGetPort(Channel& channel, std::string& serverName) {
    return ReadString(channel, serverName);
}

Result SendGetPortOk(Channel& channel, uint16_t port) {
    CheckResult(WriteHeader(channel, FrameKind::GetPortOk));
    CheckResult(channel.Write(port));
    return channel.EndWrite();
}

Result ReadGetPortOk(Channel& channel, uint16_t& port) {
    return channel.Read(port);
}

}  // namespace Protocol

static_assert(std::endian::native == std::endian::little, "Only supported on little endian platforms.");

}  // namespace DsVeosCoSim
