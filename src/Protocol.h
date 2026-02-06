// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <cstdint>
#include <string>

#include "Channel.h"
#include "DsVeosCoSim/CoSimTypes.h"

namespace DsVeosCoSim {

constexpr uint32_t ProtocolVersion1 = 0x10000;                // NOLINT
constexpr uint32_t ProtocolVersion2 = 0x20000;                // NOLINT
constexpr uint32_t ProtocolVersionLatest = ProtocolVersion2;  // NOLINT

using SerializeFunction = std::function<Result(ChannelWriter& writer)>;
using DeserializeFunction = std::function<Result(ChannelReader& reader, SimulationTime simulationTime, const Callbacks& callbacks)>;

class IProtocol {
public:
    IProtocol() = default;
    IProtocol(const IProtocol&) = default;
    IProtocol(IProtocol&&) = default;
    IProtocol& operator=(const IProtocol&) = default;
    IProtocol& operator=(IProtocol&&) = default;
    virtual ~IProtocol() = default;

    [[nodiscard]] virtual Result ReadSize(ChannelReader& reader, size_t& size) = 0;
    [[nodiscard]] virtual Result WriteSize(ChannelWriter& writer, size_t size) = 0;

    [[nodiscard]] virtual Result ReadLength(ChannelReader& reader, uint32_t& length) = 0;
    [[nodiscard]] virtual Result WriteLength(ChannelWriter& writer, uint32_t length) = 0;

    [[nodiscard]] virtual Result ReadData(ChannelReader& reader, void* data, size_t size) = 0;
    [[nodiscard]] virtual Result WriteData(ChannelWriter& writer, const void* data, size_t size) = 0;

    [[nodiscard]] virtual Result ReadSignalId(ChannelReader& reader, IoSignalId& signalId) = 0;
    [[nodiscard]] virtual Result WriteSignalId(ChannelWriter& writer, IoSignalId signalId) = 0;

    [[nodiscard]] virtual Result ReadMessage(ChannelReader& reader, CanMessageContainer& messageContainer) = 0;
    [[nodiscard]] virtual Result WriteMessage(ChannelWriter& writer, const CanMessageContainer& messageContainer) = 0;

    [[nodiscard]] virtual Result ReadMessage(ChannelReader& reader, EthMessageContainer& messageContainer) = 0;
    [[nodiscard]] virtual Result WriteMessage(ChannelWriter& writer, const EthMessageContainer& messageContainer) = 0;

    [[nodiscard]] virtual Result ReadMessage(ChannelReader& reader, LinMessageContainer& messageContainer) = 0;
    [[nodiscard]] virtual Result WriteMessage(ChannelWriter& writer, const LinMessageContainer& messageContainer) = 0;

    [[nodiscard]] virtual Result ReadMessage(ChannelReader& reader, FrMessageContainer& messageContainer) = 0;
    [[nodiscard]] virtual Result WriteMessage(ChannelWriter& writer, const FrMessageContainer& messageContainer) = 0;

    [[nodiscard]] virtual Result ReceiveHeader(ChannelReader& reader, FrameKind& frameKind) = 0;

    [[nodiscard]] virtual Result SendOk(ChannelWriter& writer) = 0;

    [[nodiscard]] virtual Result ReadError(ChannelReader& reader, std::string& errorMessage) = 0;
    [[nodiscard]] virtual Result SendError(ChannelWriter& writer, const std::string& errorMessage) = 0;

    [[nodiscard]] virtual Result SendPing(ChannelWriter& writer) = 0;

    [[nodiscard]] virtual Result ReadPingOk(ChannelReader& reader, Command& command) = 0;
    [[nodiscard]] virtual Result SendPingOk(ChannelWriter& writer, Command command) = 0;

    [[nodiscard]] virtual Result ReadConnect(ChannelReader& reader,
                                             uint32_t& protocolVersion,
                                             Mode& clientMode,
                                             std::string& serverName,
                                             std::string& clientName) = 0;
    [[nodiscard]] virtual Result SendConnect(ChannelWriter& writer,
                                             uint32_t protocolVersion,
                                             Mode clientMode,
                                             const std::string& serverName,
                                             const std::string& clientName) = 0;

    [[nodiscard]] virtual Result ReadConnectOkVersion(ChannelReader& reader, uint32_t& protocolVersion) = 0;

    [[nodiscard]] virtual Result ReadConnectOk(ChannelReader& reader,
                                               Mode& clientMode,
                                               SimulationTime& stepSize,
                                               SimulationState& simulationState,
                                               std::vector<IoSignalContainer>& incomingSignals,
                                               std::vector<IoSignalContainer>& outgoingSignals,
                                               std::vector<CanControllerContainer>& canControllers,
                                               std::vector<EthControllerContainer>& ethControllers,
                                               std::vector<LinControllerContainer>& linControllers,
                                               std::vector<FrControllerContainer>& frControllers) = 0;

    [[nodiscard]] virtual Result SendConnectOk(ChannelWriter& writer,
                                               uint32_t protocolVersion,
                                               Mode clientMode,
                                               SimulationTime stepSize,
                                               SimulationState simulationState,
                                               const std::vector<IoSignalContainer>& incomingSignals,
                                               const std::vector<IoSignalContainer>& outgoingSignals,
                                               const std::vector<CanControllerContainer>& canControllers,
                                               const std::vector<EthControllerContainer>& ethControllers,
                                               const std::vector<LinControllerContainer>& linControllers,
                                               const std::vector<FrControllerContainer>& frController) = 0;

    [[nodiscard]] virtual Result ReadStart(ChannelReader& reader, SimulationTime& simulationTime) = 0;
    [[nodiscard]] virtual Result SendStart(ChannelWriter& writer, SimulationTime simulationTime) = 0;

    [[nodiscard]] virtual Result ReadStop(ChannelReader& reader, SimulationTime& simulationTime) = 0;
    [[nodiscard]] virtual Result SendStop(ChannelWriter& writer, SimulationTime simulationTime) = 0;

    [[nodiscard]] virtual Result ReadTerminate(ChannelReader& reader, SimulationTime& simulationTime, TerminateReason& reason) = 0;
    [[nodiscard]] virtual Result SendTerminate(ChannelWriter& writer, SimulationTime simulationTime, TerminateReason reason) = 0;

    [[nodiscard]] virtual Result ReadPause(ChannelReader& reader, SimulationTime& simulationTime) = 0;
    [[nodiscard]] virtual Result SendPause(ChannelWriter& writer, SimulationTime simulationTime) = 0;

    [[nodiscard]] virtual Result ReadContinue(ChannelReader& reader, SimulationTime& simulationTime) = 0;
    [[nodiscard]] virtual Result SendContinue(ChannelWriter& writer, SimulationTime simulationTime) = 0;

    [[nodiscard]] virtual Result ReadStep(ChannelReader& reader,
                                          SimulationTime& simulationTime,
                                          const DeserializeFunction& deserializeIoData,
                                          const DeserializeFunction& deserializeBusMessages,
                                          const Callbacks& callbacks) = 0;
    [[nodiscard]] virtual Result SendStep(ChannelWriter& writer,
                                          SimulationTime simulationTime,
                                          const SerializeFunction& serializeIoData,
                                          const SerializeFunction& serializeBusMessages) = 0;

    [[nodiscard]] virtual Result ReadStepOk(ChannelReader& reader,
                                            SimulationTime& nextSimulationTime,
                                            Command& command,
                                            const DeserializeFunction& deserializeIoData,
                                            const DeserializeFunction& deserializeBusMessages,
                                            const Callbacks& callbacks) = 0;
    [[nodiscard]] virtual Result SendStepOk(ChannelWriter& writer,
                                            SimulationTime nextSimulationTime,
                                            Command command,
                                            const SerializeFunction& serializeIoData,
                                            const SerializeFunction& serializeBusMessages) = 0;

    [[nodiscard]] virtual Result ReadGetPort(ChannelReader& reader, std::string& serverName) = 0;
    [[nodiscard]] virtual Result SendGetPort(ChannelWriter& writer, const std::string& serverName) = 0;

    [[nodiscard]] virtual Result ReadGetPortOk(ChannelReader& reader, uint16_t& port) = 0;
    [[nodiscard]] virtual Result SendGetPortOk(ChannelWriter& writer, uint16_t port) = 0;

    [[nodiscard]] virtual Result ReadSetPort(ChannelReader& reader, std::string& serverName, uint16_t& port) = 0;
    [[nodiscard]] virtual Result SendSetPort(ChannelWriter& writer, const std::string& serverName, uint16_t port) = 0;

    [[nodiscard]] virtual Result ReadUnsetPort(ChannelReader& reader, std::string& serverName) = 0;
    [[nodiscard]] virtual Result SendUnsetPort(ChannelWriter& writer, const std::string& serverName) = 0;

    [[nodiscard]] virtual uint32_t GetVersion() = 0;

    [[nodiscard]] virtual bool DoFlexRayOperations() = 0;
};

[[nodiscard]] Result CreateProtocol(uint32_t negotiatedVersion, std::unique_ptr<IProtocol>& protocol);

}  // namespace DsVeosCoSim
