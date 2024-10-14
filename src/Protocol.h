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
                                 DsVeosCoSim_SimulationTime stepSize,
                                 SimulationState simulationState,
                                 const std::vector<IoSignal>& incomingSignals,
                                 const std::vector<IoSignal>& outgoingSignals,
                                 const std::vector<CanController>& canControllers,
                                 const std::vector<EthController>& ethControllers,
                                 const std::vector<LinController>& linControllers);
[[nodiscard]] bool ReadConnectOk(ChannelReader& reader,
                                 uint32_t& protocolVersion,
                                 Mode& clientMode,
                                 DsVeosCoSim_SimulationTime& stepSize,
                                 SimulationState& simulationState,
                                 std::vector<IoSignal>& incomingSignals,
                                 std::vector<IoSignal>& outgoingSignals,
                                 std::vector<CanController>& canControllers,
                                 std::vector<EthController>& ethControllers,
                                 std::vector<LinController>& linControllers);

[[nodiscard]] bool SendStart(ChannelWriter& writer, DsVeosCoSim_SimulationTime simulationTime);
[[nodiscard]] bool ReadStart(ChannelReader& reader, DsVeosCoSim_SimulationTime& simulationTime);

[[nodiscard]] bool SendStop(ChannelWriter& writer, DsVeosCoSim_SimulationTime simulationTime);
[[nodiscard]] bool ReadStop(ChannelReader& reader, DsVeosCoSim_SimulationTime& simulationTime);

[[nodiscard]] bool SendTerminate(ChannelWriter& writer,
                                 DsVeosCoSim_SimulationTime simulationTime,
                                 DsVeosCoSim_TerminateReason reason);
[[nodiscard]] bool ReadTerminate(ChannelReader& reader,
                                 DsVeosCoSim_SimulationTime& simulationTime,
                                 DsVeosCoSim_TerminateReason& reason);

[[nodiscard]] bool SendPause(ChannelWriter& writer, DsVeosCoSim_SimulationTime simulationTime);
[[nodiscard]] bool ReadPause(ChannelReader& reader, DsVeosCoSim_SimulationTime& simulationTime);

[[nodiscard]] bool SendContinue(ChannelWriter& writer, DsVeosCoSim_SimulationTime simulationTime);
[[nodiscard]] bool ReadContinue(ChannelReader& reader, DsVeosCoSim_SimulationTime& simulationTime);

[[nodiscard]] bool SendStep(ChannelWriter& writer,
                            DsVeosCoSim_SimulationTime simulationTime,
                            const IoBuffer& ioBuffer,
                            const BusBuffer& busBuffer);
[[nodiscard]] bool ReadStep(ChannelReader& reader,
                            DsVeosCoSim_SimulationTime& simulationTime,
                            const IoBuffer& ioBuffer,
                            const BusBuffer& busBuffer,
                            const Callbacks& callbacks);

[[nodiscard]] bool SendStepOk(ChannelWriter& writer,
                              DsVeosCoSim_SimulationTime nextSimulationTime,
                              Command command,
                              const IoBuffer& ioBuffer,
                              const BusBuffer& busBuffer);
[[nodiscard]] bool ReadStepOk(ChannelReader& reader,
                              DsVeosCoSim_SimulationTime& nextSimulationTime,
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
