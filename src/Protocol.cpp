// Copyright dSPACE GmbH. All rights reserved.

#include "Protocol.h"

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

Result ReadString(Channel& channel, const char*& stringPointer, std::string& string) {
    CheckResult(ReadString(channel, string));
    stringPointer = string.c_str();
    return Result::Ok;
}

Result WriteString(Channel& channel, std::string_view string) {
    const auto size = static_cast<uint32_t>(string.size());
    CheckResult(channel.Write(size));
    return channel.Write(string.data(), size);
}

Result ReadIoSignalInfo(Channel& channel, IoSignalContainer& container) {
    CheckResult(channel.Read(container.signal.id));
    CheckResult(channel.Read(container.signal.length));
    CheckResult(channel.Read(container.signal.dataType));
    CheckResult(channel.Read(container.signal.sizeKind));
    return ReadString(channel, container.signal.name, container.name);
}

Result WriteIoSignalInfo(Channel& channel, const IoSignalContainer& container) {
    CheckResult(channel.Write(container.signal.id));
    CheckResult(channel.Write(container.signal.length));
    CheckResult(channel.Write(container.signal.dataType));
    CheckResult(channel.Write(container.signal.sizeKind));
    return WriteString(channel, container.signal.name);
}

Result ReadIoSignalInfos(Channel& channel, std::vector<IoSignalContainer>& containers) {
    uint32_t containersCount = 0;
    CheckResult(channel.Read(containersCount));
    containers.resize(containersCount);

    for (uint32_t i = 0; i < containersCount; i++) {
        CheckResult(ReadIoSignalInfo(channel, containers[i]));
    }

    return Result::Ok;
}

Result WriteIoSignalInfos(Channel& channel, const std::vector<IoSignalContainer>& containers) {
    CheckResult(channel.Write(static_cast<uint32_t>(containers.size())));
    for (const auto& container : containers) {
        CheckResult(WriteIoSignalInfo(channel, container));
    }

    return Result::Ok;
}

Result ReadControllerInfo(Channel& channel, CanControllerContainer& container) {
    CheckResult(channel.Read(container.controller.id));
    CheckResult(channel.Read(container.controller.queueSize));
    CheckResult(channel.Read(container.controller.bitsPerSecond));
    CheckResult(channel.Read(container.controller.flexibleDataRateBitsPerSecond));
    CheckResult(ReadString(channel, container.controller.name, container.name));
    CheckResult(ReadString(channel, container.controller.channelName, container.channelName));
    return ReadString(channel, container.controller.clusterName, container.clusterName);
}

Result WriteControllerInfo(Channel& channel, const CanControllerContainer& container) {
    CheckResult(channel.Write(container.controller.id));
    CheckResult(channel.Write(container.controller.queueSize));
    CheckResult(channel.Write(container.controller.bitsPerSecond));
    CheckResult(channel.Write(container.controller.flexibleDataRateBitsPerSecond));
    CheckResult(WriteString(channel, container.controller.name));
    CheckResult(WriteString(channel, container.controller.channelName));
    return WriteString(channel, container.controller.clusterName);
}

Result ReadControllerInfos(Channel& channel, std::vector<CanControllerContainer>& containers) {
    uint32_t containersCount = 0;
    CheckResult(channel.Read(containersCount));
    containers.resize(containersCount);

    for (uint32_t i = 0; i < containersCount; i++) {
        CheckResult(ReadControllerInfo(channel, containers[i]));
    }

    return Result::Ok;
}

Result WriteControllerInfos(Channel& channel, const std::vector<CanControllerContainer>& containers) {
    CheckResult(channel.Write(static_cast<uint32_t>(containers.size())));
    for (const auto& container : containers) {
        CheckResult(WriteControllerInfo(channel, container));
    }

    return Result::Ok;
}

Result ReadControllerInfo(Channel& channel, EthControllerContainer& container) {
    CheckResult(channel.Read(container.controller.id));
    CheckResult(channel.Read(container.controller.queueSize));
    CheckResult(channel.Read(container.controller.bitsPerSecond));
    CheckResult(channel.Read(container.controller.macAddress));
    CheckResult(ReadString(channel, container.controller.name, container.name));
    CheckResult(ReadString(channel, container.controller.channelName, container.channelName));
    return ReadString(channel, container.controller.clusterName, container.clusterName);
}

Result WriteControllerInfo(Channel& channel, const EthControllerContainer& container) {
    CheckResult(channel.Write(container.controller.id));
    CheckResult(channel.Write(container.controller.queueSize));
    CheckResult(channel.Write(container.controller.bitsPerSecond));
    CheckResult(channel.Write(container.controller.macAddress));
    CheckResult(WriteString(channel, container.controller.name));
    CheckResult(WriteString(channel, container.controller.channelName));
    return WriteString(channel, container.controller.clusterName);
}

Result ReadControllerInfos(Channel& channel, std::vector<EthControllerContainer>& containers) {
    uint32_t containersCount = 0;
    CheckResult(channel.Read(containersCount));
    containers.resize(containersCount);

    for (uint32_t i = 0; i < containersCount; i++) {
        CheckResult(ReadControllerInfo(channel, containers[i]));
    }

    return Result::Ok;
}

Result WriteControllerInfos(Channel& channel, const std::vector<EthControllerContainer>& containers) {
    CheckResult(channel.Write(static_cast<uint32_t>(containers.size())));
    for (const auto& container : containers) {
        CheckResult(WriteControllerInfo(channel, container));
    }

    return Result::Ok;
}

Result ReadControllerInfo(Channel& channel, LinControllerContainer& container) {
    CheckResult(channel.Read(container.controller.id));
    CheckResult(channel.Read(container.controller.queueSize));
    CheckResult(channel.Read(container.controller.bitsPerSecond));
    CheckResult(channel.Read(container.controller.type));
    CheckResult(ReadString(channel, container.controller.name, container.name));
    CheckResult(ReadString(channel, container.controller.channelName, container.channelName));
    return ReadString(channel, container.controller.clusterName, container.clusterName);
}

Result WriteControllerInfo(Channel& channel, const LinControllerContainer& container) {
    CheckResult(channel.Write(container.controller.id));
    CheckResult(channel.Write(container.controller.queueSize));
    CheckResult(channel.Write(container.controller.bitsPerSecond));
    CheckResult(channel.Write(container.controller.type));
    CheckResult(WriteString(channel, container.controller.name));
    CheckResult(WriteString(channel, container.controller.channelName));
    return WriteString(channel, container.controller.clusterName);
}

Result ReadControllerInfos(Channel& channel, std::vector<LinControllerContainer>& containers) {
    uint32_t containersCount = 0;
    CheckResult(channel.Read(containersCount));
    containers.resize(containersCount);

    for (uint32_t i = 0; i < containersCount; i++) {
        CheckResult(ReadControllerInfo(channel, containers[i]));
    }

    return Result::Ok;
}

Result WriteControllerInfos(Channel& channel, const std::vector<LinControllerContainer>& containers) {
    CheckResult(channel.Write(static_cast<uint32_t>(containers.size())));
    for (const auto& container : containers) {
        CheckResult(WriteControllerInfo(channel, container));
    }

    return Result::Ok;
}

}  // namespace

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
                    const std::vector<IoSignalContainer>& incomingSignals,
                    const std::vector<IoSignalContainer>& outgoingSignals,
                    const std::vector<CanControllerContainer>& canControllers,
                    const std::vector<EthControllerContainer>& ethControllers,
                    const std::vector<LinControllerContainer>& linControllers) {
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
                    std::vector<IoSignalContainer>& incomingSignals,
                    std::vector<IoSignalContainer>& outgoingSignals,
                    std::vector<CanControllerContainer>& canControllers,
                    std::vector<EthControllerContainer>& ethControllers,
                    std::vector<LinControllerContainer>& linControllers) {
    CheckResult(channel.Read(protocolVersion));
    CheckResult(channel.Read(mode));
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

}  // namespace Protocol

}  // namespace DsVeosCoSim
