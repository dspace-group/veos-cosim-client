// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include "BusBuffer.h"
#include "CoSimTypes.h"
#include "Communication.h"
#include "IoBuffer.h"

namespace DsVeosCoSim {

constexpr uint32_t CoSimProtocolVersion = 0x10000U;

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

std::string ToString(FrameKind frameKind);

namespace Protocol {

[[nodiscard]] Result ReceiveHeader(Channel& channel, FrameKind& frameKind);

[[nodiscard]] Result SendOk(Channel& channel);

[[nodiscard]] Result SendError(Channel& channel, std::string_view errorStr);
[[nodiscard]] Result ReadError(Channel& channel, std::string& errorStr);

[[nodiscard]] Result SendPing(Channel& channel);

[[nodiscard]] Result SendPingOk(Channel& channel, Command command);
[[nodiscard]] Result ReadPingOk(Channel& channel, Command& command);

[[nodiscard]] Result SendConnect(Channel& channel, uint32_t protocolVersion, Mode clientMode, std::string_view serverName, std::string_view clientName);
[[nodiscard]] Result ReadConnect(Channel& channel, uint32_t& protocolVersion, Mode& clientMode, std::string& serverName, std::string& clientName);

[[nodiscard]] Result SendConnectOk(Channel& channel,
                                   uint32_t protocolVersion,
                                   Mode clientMode,
                                   SimulationTime stepSize,
                                   SimulationState simulationState,
                                   const std::vector<IoSignal>& incomingSignals,
                                   const std::vector<IoSignal>& outgoingSignals,
                                   const std::vector<CanController>& canControllers,
                                   const std::vector<EthController>& ethControllers,
                                   const std::vector<LinController>& linControllers);
[[nodiscard]] Result ReadConnectOk(Channel& channel,
                                   uint32_t& protocolVersion,
                                   Mode& clientMode,
                                   SimulationTime& stepSize,
                                   SimulationState& simulationState,
                                   std::vector<IoSignal>& incomingSignals,
                                   std::vector<IoSignal>& outgoingSignals,
                                   std::vector<CanController>& canControllers,
                                   std::vector<EthController>& ethControllers,
                                   std::vector<LinController>& linControllers);

[[nodiscard]] Result SendStart(Channel& channel, SimulationTime simulationTime);
[[nodiscard]] Result ReadStart(Channel& channel, SimulationTime& simulationTime);

[[nodiscard]] Result SendStop(Channel& channel, SimulationTime simulationTime);
[[nodiscard]] Result ReadStop(Channel& channel, SimulationTime& simulationTime);

[[nodiscard]] Result SendTerminate(Channel& channel, SimulationTime simulationTime, TerminateReason reason);
[[nodiscard]] Result ReadTerminate(Channel& channel, SimulationTime& simulationTime, TerminateReason& reason);

[[nodiscard]] Result SendPause(Channel& channel, SimulationTime simulationTime);
[[nodiscard]] Result ReadPause(Channel& channel, SimulationTime& simulationTime);

[[nodiscard]] Result SendContinue(Channel& channel, SimulationTime simulationTime);
[[nodiscard]] Result ReadContinue(Channel& channel, SimulationTime& simulationTime);

[[nodiscard]] Result SendStep(Channel& channel, SimulationTime simulationTime, IoBuffer& ioBuffer, BusBuffer& busBuffer);
[[nodiscard]] Result ReadStep(Channel& channel, SimulationTime& simulationTime, IoBuffer& ioBuffer, BusBuffer& busBuffer, const Callbacks& callbacks);

[[nodiscard]] Result SendStepOk(Channel& channel, SimulationTime nextSimulationTime, Command command, IoBuffer& ioBuffer, BusBuffer& busBuffer);
[[nodiscard]] Result ReadStepOk(Channel& channel,
                                SimulationTime& nextSimulationTime,
                                Command& command,
                                IoBuffer& ioBuffer,
                                BusBuffer& busBuffer,
                                const Callbacks& callbacks);

[[nodiscard]] Result SendGetPort(Channel& channel, std::string_view serverName);
[[nodiscard]] Result ReadGetPort(Channel& channel, std::string& serverName);

[[nodiscard]] Result SendGetPortOk(Channel& channel, uint16_t port);
[[nodiscard]] Result ReadGetPortOk(Channel& channel, uint16_t& port);

[[nodiscard]] Result SendSetPort(Channel& channel, std::string_view serverName, uint16_t port);
[[nodiscard]] Result ReadSetPort(Channel& channel, std::string& serverName, uint16_t& port);

[[nodiscard]] Result SendUnsetPort(Channel& channel, std::string_view serverName);
[[nodiscard]] Result ReadUnsetPort(Channel& channel, std::string& serverName);

}  // namespace Protocol

}  // namespace DsVeosCoSim
