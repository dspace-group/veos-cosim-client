// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <string>

#include "BusBuffer.h"
#include "Channel.h"
#include "CoSimTypes.h"
#include "IoBuffer.h"

namespace DsVeosCoSim {

constexpr uint32_t CoSimProtocolVersion = 0x10000U;  // NOLINT

enum class FrameKind {
    Ok = 1,
    Error,

    Connect,
    ConnectOk,

    Ping,
    PingOk,

    Start,
    Stop,
    Terminate,
    Pause,
    Continue,

    Step,
    StepOk,

    GetPort,
    GetPortOk,
    SetPort,
    UnsetPort
};

[[nodiscard]] inline std::string ToString(const FrameKind& frameKind) {
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

[[nodiscard]] bool ReceiveHeader(ChannelReader& reader, FrameKind& frameKind);

[[nodiscard]] bool SendOk(ChannelWriter& writer);

[[nodiscard]] bool SendError(ChannelWriter& writer, const std::string& errorMessage);
[[nodiscard]] bool ReadError(ChannelReader& reader, std::string& errorMessage);

[[nodiscard]] bool SendPing(ChannelWriter& writer);

[[nodiscard]] bool SendPingOk(ChannelWriter& writer, Command command);
[[nodiscard]] bool ReadPingOk(ChannelReader& reader, Command& command);

[[nodiscard]] bool SendConnect(ChannelWriter& writer,
                               uint32_t protocolVersion,
                               Mode clientMode,
                               const std::string& serverName,
                               const std::string& clientName);
[[nodiscard]] bool ReadConnect(ChannelReader& reader,
                               uint32_t& protocolVersion,
                               Mode& clientMode,
                               std::string& serverName,
                               std::string& clientName);

[[nodiscard]] bool SendConnectOk(ChannelWriter& writer,
                                 uint32_t protocolVersion,
                                 Mode clientMode,
                                 SimulationTime stepSize,
                                 SimulationState simulationState,
                                 const std::vector<IoSignalContainer>& incomingSignals,
                                 const std::vector<IoSignalContainer>& outgoingSignals,
                                 const std::vector<CanControllerContainer>& canControllers,
                                 const std::vector<EthControllerContainer>& ethControllers,
                                 const std::vector<LinControllerContainer>& linControllers);
[[nodiscard]] bool ReadConnectOk(ChannelReader& reader,
                                 uint32_t& protocolVersion,
                                 Mode& clientMode,
                                 SimulationTime& stepSize,
                                 SimulationState& simulationState,
                                 std::vector<IoSignalContainer>& incomingSignals,
                                 std::vector<IoSignalContainer>& outgoingSignals,
                                 std::vector<CanControllerContainer>& canControllers,
                                 std::vector<EthControllerContainer>& ethControllers,
                                 std::vector<LinControllerContainer>& linControllers);

[[nodiscard]] bool SendStart(ChannelWriter& writer, SimulationTime simulationTime);
[[nodiscard]] bool ReadStart(ChannelReader& reader, SimulationTime& simulationTime);

[[nodiscard]] bool SendStop(ChannelWriter& writer, SimulationTime simulationTime);
[[nodiscard]] bool ReadStop(ChannelReader& reader, SimulationTime& simulationTime);

[[nodiscard]] bool SendTerminate(ChannelWriter& writer, SimulationTime simulationTime, TerminateReason reason);
[[nodiscard]] bool ReadTerminate(ChannelReader& reader, SimulationTime& simulationTime, TerminateReason& reason);

[[nodiscard]] bool SendPause(ChannelWriter& writer, SimulationTime simulationTime);
[[nodiscard]] bool ReadPause(ChannelReader& reader, SimulationTime& simulationTime);

[[nodiscard]] bool SendContinue(ChannelWriter& writer, SimulationTime simulationTime);
[[nodiscard]] bool ReadContinue(ChannelReader& reader, SimulationTime& simulationTime);

[[nodiscard]] bool SendStep(ChannelWriter& writer,
                            SimulationTime simulationTime,
                            const IoBuffer& ioBuffer,
                            const BusBuffer& busBuffer);
[[nodiscard]] bool ReadStep(ChannelReader& reader,
                            SimulationTime& simulationTime,
                            const IoBuffer& ioBuffer,
                            const BusBuffer& busBuffer,
                            const Callbacks& callbacks);

[[nodiscard]] bool SendStepOk(ChannelWriter& writer,
                              SimulationTime nextSimulationTime,
                              Command command,
                              const IoBuffer& ioBuffer,
                              const BusBuffer& busBuffer);
[[nodiscard]] bool ReadStepOk(ChannelReader& reader,
                              SimulationTime& nextSimulationTime,
                              Command& command,
                              const IoBuffer& ioBuffer,
                              const BusBuffer& busBuffer,
                              const Callbacks& callbacks);

[[nodiscard]] bool SendGetPort(ChannelWriter& writer, const std::string& serverName);
[[nodiscard]] bool ReadGetPort(ChannelReader& reader, std::string& serverName);

[[nodiscard]] bool SendGetPortOk(ChannelWriter& writer, uint16_t port);
[[nodiscard]] bool ReadGetPortOk(ChannelReader& reader, uint16_t& port);

[[nodiscard]] bool SendSetPort(ChannelWriter& writer, const std::string& serverName, uint16_t port);
[[nodiscard]] bool ReadSetPort(ChannelReader& reader, std::string& serverName, uint16_t& port);

[[nodiscard]] bool SendUnsetPort(ChannelWriter& writer, const std::string& serverName);
[[nodiscard]] bool ReadUnsetPort(ChannelReader& reader, std::string& serverName);

}  // namespace Protocol

}  // namespace DsVeosCoSim
