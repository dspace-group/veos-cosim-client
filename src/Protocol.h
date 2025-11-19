// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <string>

#include "Channel.h"
#include "DsVeosCoSim/CoSimTypes.h"

namespace DsVeosCoSim {

[[maybe_unused]] constexpr uint32_t CoSimProtocolVersion = 0x10000U;

using SerializeFunction = std::function<Result(ChannelWriter& writer)>;
using DeserializeFunction =
    std::function<Result(ChannelReader& reader, SimulationTime simulationTime, const Callbacks& callbacks)>;

namespace Protocol {

[[nodiscard]] Result ReadSize(ChannelReader& reader, size_t& size);
[[nodiscard]] Result WriteSize(ChannelWriter& writer, size_t size);

[[nodiscard]] Result ReadLength(ChannelReader& reader, uint32_t& length);
[[nodiscard]] Result WriteLength(ChannelWriter& writer, uint32_t length);

[[nodiscard]] Result ReadData(ChannelReader& reader, void* data, size_t size);
[[nodiscard]] Result WriteData(ChannelWriter& writer, const void* data, size_t size);

[[nodiscard]] Result ReadSignalId(ChannelReader& reader, IoSignalId& signalId);
[[nodiscard]] Result WriteSignalId(ChannelWriter& writer, IoSignalId signalId);

[[nodiscard]] Result ReadMessage(ChannelReader& reader, CanMessageContainer& messageContainer);
[[nodiscard]] Result WriteMessage(ChannelWriter& writer, const CanMessageContainer& messageContainer);

[[nodiscard]] Result ReadMessage(ChannelReader& reader, EthMessageContainer& messageContainer);
[[nodiscard]] Result WriteMessage(ChannelWriter& writer, const EthMessageContainer& messageContainer);

[[nodiscard]] Result ReadMessage(ChannelReader& reader, LinMessageContainer& messageContainer);
[[nodiscard]] Result WriteMessage(ChannelWriter& writer, const LinMessageContainer& messageContainer);

[[nodiscard]] Result ReadMessage(ChannelReader& reader, FrMessageContainer& messageContainer);
[[nodiscard]] Result WriteMessage(ChannelWriter& writer, const FrMessageContainer& messageContainer);

[[nodiscard]] Result ReceiveHeader(ChannelReader& reader, FrameKind& frameKind);

[[nodiscard]] Result SendOk(ChannelWriter& writer);

[[nodiscard]] Result ReadError(ChannelReader& reader, std::string& errorMessage);
[[nodiscard]] Result SendError(ChannelWriter& writer, const std::string& errorMessage);

[[nodiscard]] Result SendPing(ChannelWriter& writer);

[[nodiscard]] Result ReadPingOk(ChannelReader& reader, Command& command);
[[nodiscard]] Result SendPingOk(ChannelWriter& writer, Command command);

[[nodiscard]] Result ReadConnect(ChannelReader& reader,
                                 uint32_t& protocolVersion,
                                 Mode& clientMode,
                                 std::string& serverName,
                                 std::string& clientName);
[[nodiscard]] Result SendConnect(ChannelWriter& writer,
                                 uint32_t protocolVersion,
                                 Mode clientMode,
                                 const std::string& serverName,
                                 const std::string& clientName);

[[nodiscard]] Result ReadConnectOk(ChannelReader& reader,
                                   uint32_t& protocolVersion,
                                   Mode& clientMode,
                                   SimulationTime& stepSize,
                                   SimulationState& simulationState,
                                   std::vector<IoSignalContainer>& incomingSignals,
                                   std::vector<IoSignalContainer>& outgoingSignals,
                                   std::vector<CanControllerContainer>& canControllers,
                                   std::vector<EthControllerContainer>& ethControllers,
                                   std::vector<LinControllerContainer>& linControllers,
                                   std::vector<FrControllerContainer>& frControllers);
[[nodiscard]] Result SendConnectOk(ChannelWriter& writer,
                                   uint32_t protocolVersion,
                                   Mode clientMode,
                                   SimulationTime stepSize,
                                   SimulationState simulationState,
                                   const std::vector<IoSignalContainer>& incomingSignals,
                                   const std::vector<IoSignalContainer>& outgoingSignals,
                                   const std::vector<CanControllerContainer>& canControllers,
                                   const std::vector<EthControllerContainer>& ethControllers,
                                   const std::vector<LinControllerContainer>& linController,
                                   const std::vector<FrControllerContainer>& frControllers);

[[nodiscard]] Result ReadStart(ChannelReader& reader, SimulationTime& simulationTime);
[[nodiscard]] Result SendStart(ChannelWriter& writer, SimulationTime simulationTime);

[[nodiscard]] Result ReadStop(ChannelReader& reader, SimulationTime& simulationTime);
[[nodiscard]] Result SendStop(ChannelWriter& writer, SimulationTime simulationTime);

[[nodiscard]] Result ReadTerminate(ChannelReader& reader, SimulationTime& simulationTime, TerminateReason& reason);
[[nodiscard]] Result SendTerminate(ChannelWriter& writer, SimulationTime simulationTime, TerminateReason reason);

[[nodiscard]] Result ReadPause(ChannelReader& reader, SimulationTime& simulationTime);
[[nodiscard]] Result SendPause(ChannelWriter& writer, SimulationTime simulationTime);

[[nodiscard]] Result ReadContinue(ChannelReader& reader, SimulationTime& simulationTime);
[[nodiscard]] Result SendContinue(ChannelWriter& writer, SimulationTime simulationTime);

[[nodiscard]] Result ReadStep(ChannelReader& reader,
                              SimulationTime& simulationTime,
                              const DeserializeFunction& deserializeIoData,
                              const DeserializeFunction& deserializeBusMessages,
                              const Callbacks& callbacks);
[[nodiscard]] Result SendStep(ChannelWriter& writer,
                              SimulationTime simulationTime,
                              const SerializeFunction& serializeIoData,
                              const SerializeFunction& serializeBusMessages);

[[nodiscard]] Result ReadStepOk(ChannelReader& reader,
                                SimulationTime& nextSimulationTime,
                                Command& command,
                                const DeserializeFunction& deserializeIoData,
                                const DeserializeFunction& deserializeBusMessages,
                                const Callbacks& callbacks);
[[nodiscard]] Result SendStepOk(ChannelWriter& writer,
                                SimulationTime nextSimulationTime,
                                Command command,
                                const SerializeFunction& serializeIoData,
                                const SerializeFunction& serializeBusMessages);

[[nodiscard]] Result ReadGetPort(ChannelReader& reader, std::string& serverName);
[[nodiscard]] Result SendGetPort(ChannelWriter& writer, const std::string& serverName);

[[nodiscard]] Result ReadGetPortOk(ChannelReader& reader, uint16_t& port);
[[nodiscard]] Result SendGetPortOk(ChannelWriter& writer, uint16_t port);

[[nodiscard]] Result ReadSetPort(ChannelReader& reader, std::string& serverName, uint16_t& port);
[[nodiscard]] Result SendSetPort(ChannelWriter& writer, const std::string& serverName, uint16_t port);

[[nodiscard]] Result ReadUnsetPort(ChannelReader& reader, std::string& serverName);
[[nodiscard]] Result SendUnsetPort(ChannelWriter& writer, const std::string& serverName);

[[nodiscard]] Result ReadString(ChannelReader& reader, std::string& string);
[[nodiscard]] Result WriteString(ChannelWriter& writer, const std::string& string);

[[nodiscard]] Result ReadIoSignalInfo(ChannelReader& reader, IoSignalContainer& signal);
[[nodiscard]] Result WriteIoSignalInfo(ChannelWriter& writer, const IoSignalContainer& signal);

[[nodiscard]] Result ReadIoSignalInfos(ChannelReader& reader, std::vector<IoSignalContainer>& signals);
[[nodiscard]] Result WriteIoSignalInfos(ChannelWriter& writer, const std::vector<IoSignalContainer>& signals);

[[nodiscard]] Result ReadControllerInfo(ChannelReader& reader, CanControllerContainer& controller);
[[nodiscard]] Result WriteControllerInfo(ChannelWriter& writer, const CanControllerContainer& controller);

[[nodiscard]] Result ReadControllerInfos(ChannelReader& reader, std::vector<CanControllerContainer>& controllers);
[[nodiscard]] Result WriteControllerInfos(ChannelWriter& writer,
                                          const std::vector<CanControllerContainer>& controllers);

[[nodiscard]] Result ReadControllerInfo(ChannelReader& reader, EthControllerContainer& controller);
[[nodiscard]] Result WriteControllerInfo(ChannelWriter& writer, const EthControllerContainer& controller);

[[nodiscard]] Result ReadControllerInfos(ChannelReader& reader, std::vector<EthControllerContainer>& controllers);
[[nodiscard]] Result WriteControllerInfos(ChannelWriter& writer,
                                          const std::vector<EthControllerContainer>& controllers);

[[nodiscard]] Result ReadControllerInfo(ChannelReader& reader, LinControllerContainer& controller);
[[nodiscard]] Result WriteControllerInfo(ChannelWriter& writer, const LinControllerContainer& controller);

[[nodiscard]] Result ReadControllerInfos(ChannelReader& reader, std::vector<LinControllerContainer>& controllers);
[[nodiscard]] Result WriteControllerInfos(ChannelWriter& writer,
                                          const std::vector<LinControllerContainer>& controllers);

[[nodiscard]] Result ReadControllerInfo(ChannelReader& reader, FrControllerContainer& controller);
[[nodiscard]] Result WriteControllerInfo(ChannelWriter& writer, const FrControllerContainer& controller);

[[nodiscard]] Result ReadControllerInfos(ChannelReader& reader, std::vector<FrControllerContainer>& controllers);
[[nodiscard]] Result WriteControllerInfos(ChannelWriter& writer,
                                          const std::vector<FrControllerContainer>& controllers);

}  // namespace Protocol

}  // namespace DsVeosCoSim
