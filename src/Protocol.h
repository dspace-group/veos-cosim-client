// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <string>

#include "BusBuffer.h"
#include "Channel.h"
#include "DsVeosCoSim/CoSimTypes.h"
#include "IoBuffer.h"

namespace DsVeosCoSim {

[[maybe_unused]] constexpr uint32_t CoSimProtocolVersion = 0x10000U;

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

[[nodiscard]] const char* ToString(FrameKind frameKind);

namespace Protocol {

[[nodiscard]] Result ReceiveHeader(ChannelReader& reader, FrameKind& frameKind);

[[nodiscard]] Result SendOk(ChannelWriter& writer);

[[nodiscard]] Result SendError(ChannelWriter& writer, const std::string& errorMessage);
[[nodiscard]] Result ReadError(ChannelReader& reader, std::string& errorMessage);

[[nodiscard]] Result SendPing(ChannelWriter& writer);

[[nodiscard]] Result SendPingOk(ChannelWriter& writer, Command command);
[[nodiscard]] Result ReadPingOk(ChannelReader& reader, Command& command);

[[nodiscard]] Result SendConnect(ChannelWriter& writer,
                                 uint32_t protocolVersion,
                                 Mode clientMode,
                                 const std::string& serverName,
                                 const std::string& clientName);
[[nodiscard]] Result ReadConnect(ChannelReader& reader,
                                 uint32_t& protocolVersion,
                                 Mode& clientMode,
                                 std::string& serverName,
                                 std::string& clientName);

[[nodiscard]] Result SendConnectOk(ChannelWriter& writer,
                                   uint32_t protocolVersion,
                                   Mode clientMode,
                                   SimulationTime stepSize,
                                   SimulationState simulationState,
                                   const std::vector<IoSignalContainer>& incomingSignals,
                                   const std::vector<IoSignalContainer>& outgoingSignals,
                                   const std::vector<CanControllerContainer>& canControllers,
                                   const std::vector<EthControllerContainer>& ethControllers,
                                   const std::vector<LinControllerContainer>& linControllers);
[[nodiscard]] Result ReadConnectOk(ChannelReader& reader,
                                   uint32_t& protocolVersion,
                                   Mode& clientMode,
                                   SimulationTime& stepSize,
                                   SimulationState& simulationState,
                                   std::vector<IoSignalContainer>& incomingSignals,
                                   std::vector<IoSignalContainer>& outgoingSignals,
                                   std::vector<CanControllerContainer>& canControllers,
                                   std::vector<EthControllerContainer>& ethControllers,
                                   std::vector<LinControllerContainer>& linControllers);

[[nodiscard]] Result SendStart(ChannelWriter& writer, SimulationTime simulationTime);
[[nodiscard]] Result ReadStart(ChannelReader& reader, SimulationTime& simulationTime);

[[nodiscard]] Result SendStop(ChannelWriter& writer, SimulationTime simulationTime);
[[nodiscard]] Result ReadStop(ChannelReader& reader, SimulationTime& simulationTime);

[[nodiscard]] Result SendTerminate(ChannelWriter& writer, SimulationTime simulationTime, TerminateReason reason);
[[nodiscard]] Result ReadTerminate(ChannelReader& reader, SimulationTime& simulationTime, TerminateReason& reason);

[[nodiscard]] Result SendPause(ChannelWriter& writer, SimulationTime simulationTime);
[[nodiscard]] Result ReadPause(ChannelReader& reader, SimulationTime& simulationTime);

[[nodiscard]] Result SendContinue(ChannelWriter& writer, SimulationTime simulationTime);
[[nodiscard]] Result ReadContinue(ChannelReader& reader, SimulationTime& simulationTime);

[[nodiscard]] Result SendStep(ChannelWriter& writer,
                              SimulationTime simulationTime,
                              const IoBuffer& ioBuffer,
                              const BusBuffer& busBuffer);
[[nodiscard]] Result ReadStep(ChannelReader& reader,
                              SimulationTime& simulationTime,
                              const IoBuffer& ioBuffer,
                              const BusBuffer& busBuffer,
                              const Callbacks& callbacks);

[[nodiscard]] Result SendStepOk(ChannelWriter& writer,
                                SimulationTime nextSimulationTime,
                                Command command,
                                const IoBuffer& ioBuffer,
                                const BusBuffer& busBuffer);
[[nodiscard]] Result ReadStepOk(ChannelReader& reader,
                                SimulationTime& nextSimulationTime,
                                Command& command,
                                const IoBuffer& ioBuffer,
                                const BusBuffer& busBuffer,
                                const Callbacks& callbacks);

[[nodiscard]] Result SendGetPort(ChannelWriter& writer, const std::string& serverName);
[[nodiscard]] Result ReadGetPort(ChannelReader& reader, std::string& serverName);

[[nodiscard]] Result SendGetPortOk(ChannelWriter& writer, uint16_t port);
[[nodiscard]] Result ReadGetPortOk(ChannelReader& reader, uint16_t& port);

[[nodiscard]] Result SendSetPort(ChannelWriter& writer, const std::string& serverName, uint16_t port);
[[nodiscard]] Result ReadSetPort(ChannelReader& reader, std::string& serverName, uint16_t& port);

[[nodiscard]] Result SendUnsetPort(ChannelWriter& writer, const std::string& serverName);
[[nodiscard]] Result ReadUnsetPort(ChannelReader& reader, std::string& serverName);

}  // namespace Protocol

}  // namespace DsVeosCoSim
