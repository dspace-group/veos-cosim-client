// Copyright dSPACE GmbH. All rights reserved.

#include "Protocol.h"

#include "CoSimTypes.h"
#include "Communication.h"

namespace DsVeosCoSim {

std::string ToString(FrameKind frameKind) {
    switch (frameKind) {
        case FrameKind::Unknown:
            return "Unknown";
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
        case FrameKind::Accepted:
            return "Accepted";
        case FrameKind::Ping:
            return "Ping";
        case FrameKind::Connect:
            return "Connect";
        case FrameKind::StepResponse:
            return "StepResponse";
        case FrameKind::GetPort:
            return "GetPort";
        case FrameKind::GetPortResponse:
            return "GetPortResponse";
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

Result WriteHeader(Channel& channel, FrameKind frameKind) {
    return channel.Write(frameKind);
}

Result SendOk(Channel& channel) {
    CheckResult(WriteHeader(channel, FrameKind::Ok));
    return channel.EndWrite();
}

Result SendPing(Channel& channel) {
    CheckResult(WriteHeader(channel, FrameKind::Ping));
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

Result SendConnect(Channel& channel, uint32_t protocolVersion, Mode mode, std::string_view serverName, std::string_view clientName) {
    CheckResult(WriteHeader(channel, FrameKind::Connect));
    CheckResult(channel.Write(protocolVersion));
    CheckResult(channel.Write(mode));
    CheckResult(WriteString(channel, serverName));
    CheckResult(WriteString(channel, clientName));
    return channel.EndWrite();
}

Result ReadConnect(Channel& channel, uint32_t& protocolVersion, Mode& mode, std::string& serverName, std::string& clientName) {
    CheckResult(channel.Read(protocolVersion));
    CheckResult(channel.Read(mode));
    CheckResult(ReadString(channel, serverName));
    return ReadString(channel, clientName);
}

Result SendAccepted(Channel& channel,
                    uint32_t protocolVersion,
                    Mode mode,
                    const std::vector<IoSignal>& incomingSignals,
                    const std::vector<IoSignal>& outgoingSignals,
                    const std::vector<CanController>& canControllers,
                    const std::vector<EthController>& ethControllers,
                    const std::vector<LinController>& linControllers) {
    CheckResult(WriteHeader(channel, FrameKind::Accepted));
    CheckResult(channel.Write(protocolVersion));
    CheckResult(channel.Write(mode));
    CheckResult(WriteIoSignalInfos(channel, incomingSignals));
    CheckResult(WriteIoSignalInfos(channel, outgoingSignals));
    CheckResult(WriteControllerInfos(channel, canControllers));
    CheckResult(WriteControllerInfos(channel, ethControllers));
    CheckResult(WriteControllerInfos(channel, linControllers));
    return channel.EndWrite();
}

Result ReadAccepted(Channel& channel,
                    uint32_t& protocolVersion,
                    Mode& mode,
                    std::vector<IoSignal>& incomingSignals,
                    std::vector<std::string>& incomingSignalStrings,
                    std::vector<IoSignal>& outgoingSignals,
                    std::vector<std::string>& outgoingSignalStrings,
                    std::vector<CanController>& canControllers,
                    std::vector<std::string>& canStrings,
                    std::vector<EthController>& ethControllers,
                    std::vector<std::string>& ethStrings,
                    std::vector<LinController>& linControllers,
                    std::vector<std::string>& linStrings) {
    CheckResult(channel.Read(protocolVersion));
    CheckResult(channel.Read(mode));
    CheckResult(ReadIoSignalInfos(channel, incomingSignals, incomingSignalStrings));
    CheckResult(ReadIoSignalInfos(channel, outgoingSignals, outgoingSignalStrings));
    CheckResult(ReadControllerInfos(channel, canControllers, canStrings));
    CheckResult(ReadControllerInfos(channel, ethControllers, ethStrings));
    CheckResult(ReadControllerInfos(channel, linControllers, linStrings));
    return Result::Ok;
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

Result SendStepResponse(Channel& channel, SimulationTime simulationTime, IoBuffer& ioBuffer, BusBuffer& busBuffer) {
    CheckResult(WriteHeader(channel, FrameKind::StepResponse));
    CheckResult(channel.Write(simulationTime));
    CheckResult(ioBuffer.Serialize(channel));
    CheckResult(busBuffer.Serialize(channel));
    return channel.EndWrite();
}

Result ReadStepResponse(Channel& channel, SimulationTime& simulationTime, IoBuffer& ioBuffer, BusBuffer& busBuffer, const Callbacks& callbacks) {
    CheckResult(channel.Read(simulationTime));

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

Result SendGetPortResponse(Channel& channel, uint16_t port) {
    CheckResult(WriteHeader(channel, FrameKind::GetPortResponse));
    CheckResult(channel.Write(port));
    return channel.EndWrite();
}

Result ReadGetPortResponse(Channel& channel, uint16_t& port) {
    return channel.Read(port);
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

Result ReadString(Channel& channel, const char*& string, std::vector<std::string>& strings) {
    std::string tmpString;
    CheckResult(ReadString(channel, tmpString));
    strings.push_back(tmpString);
    string = strings[strings.size() - 1].c_str();
    return Result::Ok;
}

Result ReadIoSignalInfo(Channel& channel, IoSignal& ioSignal, std::vector<std::string>& strings) {
    CheckResult(channel.Read(ioSignal.id));
    CheckResult(channel.Read(ioSignal.length));
    CheckResult(channel.Read(ioSignal.dataType));
    CheckResult(channel.Read(ioSignal.sizeKind));
    CheckResult(ReadString(channel, ioSignal.name, strings));
    return Result::Ok;
}

Result WriteIoSignalInfo(Channel& channel, const IoSignal& ioSignal) {
    CheckResult(channel.Write(ioSignal.id));
    CheckResult(channel.Write(ioSignal.length));
    CheckResult(channel.Write(ioSignal.dataType));
    CheckResult(channel.Write(ioSignal.sizeKind));
    return WriteString(channel, ioSignal.name);
}

Result ReadIoSignalInfos(Channel& channel, std::vector<IoSignal>& ioSignals, std::vector<std::string>& strings) {
    uint32_t ioSignalsCount = 0;
    CheckResult(channel.Read(ioSignalsCount));
    strings.reserve(ioSignalsCount);
    for (uint32_t i = 0; i < ioSignalsCount; i++) {
        IoSignal ioSignal{};
        CheckResult(ReadIoSignalInfo(channel, ioSignal, strings));
        ioSignals.push_back(ioSignal);
    }

    return Result::Ok;
}

Result WriteIoSignalInfos(Channel& channel, const std::vector<IoSignal>& ioSignals) {
    CheckResult(channel.Write(static_cast<uint32_t>(ioSignals.size())));
    for (const auto& ioSignal : ioSignals) {
        CheckResult(WriteIoSignalInfo(channel, ioSignal));
    }

    return Result::Ok;
}

Result ReadControllerInfo(Channel& channel, CanController& controller, std::vector<std::string>& strings) {
    CheckResult(channel.Read(controller.id));
    CheckResult(channel.Read(controller.queueSize));
    CheckResult(channel.Read(controller.bitsPerSecond));
    CheckResult(channel.Read(controller.flexibleDataRateBitsPerSecond));
    CheckResult(ReadString(channel, controller.name, strings));
    CheckResult(ReadString(channel, controller.channelName, strings));
    CheckResult(ReadString(channel, controller.clusterName, strings));
    return Result::Ok;
}

Result WriteControllerInfo(Channel& channel, const CanController& controller) {
    CheckResult(channel.Write(controller.id));
    CheckResult(channel.Write(controller.queueSize));
    CheckResult(channel.Write(controller.bitsPerSecond));
    CheckResult(channel.Write(controller.flexibleDataRateBitsPerSecond));
    CheckResult(WriteString(channel, controller.name));
    CheckResult(WriteString(channel, controller.channelName));
    CheckResult(WriteString(channel, controller.clusterName));
    return Result::Ok;
}

Result ReadControllerInfos(Channel& channel, std::vector<CanController>& controllers, std::vector<std::string>& strings) {
    uint32_t controllersCount = 0;
    CheckResult(channel.Read(controllersCount));

    // There are 3 strings per CAN controller
    strings.reserve(static_cast<size_t>(controllersCount) * 3);
    for (uint32_t i = 0; i < controllersCount; i++) {
        CanController controller{};
        CheckResult(ReadControllerInfo(channel, controller, strings));
        controllers.push_back(controller);
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

Result ReadControllerInfo(Channel& channel, EthController& controller, std::vector<std::string>& strings) {
    CheckResult(channel.Read(controller.id));
    CheckResult(channel.Read(controller.queueSize));
    CheckResult(channel.Read(controller.bitsPerSecond));
    CheckResult(ReadString(channel, controller.macAddress, strings));
    CheckResult(ReadString(channel, controller.name, strings));
    CheckResult(ReadString(channel, controller.channelName, strings));
    CheckResult(ReadString(channel, controller.clusterName, strings));
    return Result::Ok;
}

Result WriteControllerInfo(Channel& channel, const EthController& controller) {
    CheckResult(channel.Write(controller.id));
    CheckResult(channel.Write(controller.queueSize));
    CheckResult(channel.Write(controller.bitsPerSecond));
    CheckResult(WriteString(channel, controller.macAddress));
    CheckResult(WriteString(channel, controller.name));
    CheckResult(WriteString(channel, controller.channelName));
    CheckResult(WriteString(channel, controller.clusterName));
    return Result::Ok;
}

Result ReadControllerInfos(Channel& channel, std::vector<EthController>& controllers, std::vector<std::string>& strings) {
    uint32_t controllersCount = 0;
    CheckResult(channel.Read(controllersCount));

    // There are 4 strings per LIN controller
    strings.reserve(static_cast<size_t>(controllersCount) * 4);
    for (uint32_t i = 0; i < controllersCount; i++) {
        EthController controller{};
        CheckResult(ReadControllerInfo(channel, controller, strings));
        controllers.push_back(controller);
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

Result ReadControllerInfo(Channel& channel, LinController& controller, std::vector<std::string>& strings) {
    CheckResult(channel.Read(controller.id));
    CheckResult(channel.Read(controller.queueSize));
    CheckResult(channel.Read(controller.bitsPerSecond));
    CheckResult(channel.Read(controller.type));
    CheckResult(ReadString(channel, controller.name, strings));
    CheckResult(ReadString(channel, controller.channelName, strings));
    CheckResult(ReadString(channel, controller.clusterName, strings));
    return Result::Ok;
}

Result WriteControllerInfo(Channel& channel, const LinController& controller) {
    CheckResult(channel.Write(controller.id));
    CheckResult(channel.Write(controller.queueSize));
    CheckResult(channel.Write(controller.bitsPerSecond));
    CheckResult(channel.Write(controller.type));
    CheckResult(WriteString(channel, controller.name));
    CheckResult(WriteString(channel, controller.channelName));
    CheckResult(WriteString(channel, controller.clusterName));
    return Result::Ok;
}

Result ReadControllerInfos(Channel& channel, std::vector<LinController>& controllers, std::vector<std::string>& strings) {
    uint32_t controllersCount = 0;
    CheckResult(channel.Read(controllersCount));

    // There are 3 strings per LIN controller
    strings.reserve(static_cast<size_t>(controllersCount) * 3);
    for (uint32_t i = 0; i < controllersCount; i++) {
        LinController controller{};
        CheckResult(ReadControllerInfo(channel, controller, strings));
        controllers.push_back(controller);
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

}  // namespace Protocol

}  // namespace DsVeosCoSim
