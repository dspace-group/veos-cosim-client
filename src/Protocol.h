// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <cstdint>
#include <string>

#include "Channel.h"
#include "DsVeosCoSim/CoSimTypes.h"

namespace DsVeosCoSim {

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

    // V2:
    [[nodiscard]] virtual Result ReadMessage(ChannelReader& reader, FrMessageContainer& messageContainer) = 0;
    [[nodiscard]] virtual Result WriteMessage(ChannelWriter& writer, const FrMessageContainer& messageContainer) = 0;

    [[nodiscard]] virtual bool DoFlexRayOperations() = 0;
};

namespace V1 {

class Protocol : public IProtocol {
public:
    static const uint32_t CoSimProtocolVersion = 0x10000U;
    [[nodiscard]] Result ReadSize(ChannelReader& reader, size_t& size) override;
    [[nodiscard]] Result WriteSize(ChannelWriter& writer, size_t size) override;

    [[nodiscard]] Result ReadLength(ChannelReader& reader, uint32_t& length) override;
    [[nodiscard]] Result WriteLength(ChannelWriter& writer, uint32_t length) override;

    [[nodiscard]] Result ReadData(ChannelReader& reader, void* data, size_t size) override;
    [[nodiscard]] Result WriteData(ChannelWriter& writer, const void* data, size_t size) override;

    [[nodiscard]] Result ReadSignalId(ChannelReader& reader, IoSignalId& signalId) override;
    [[nodiscard]] Result WriteSignalId(ChannelWriter& writer, IoSignalId signalId) override;

    [[nodiscard]] Result ReadMessage(ChannelReader& reader, CanMessageContainer& messageContainer) override;
    [[nodiscard]] Result WriteMessage(ChannelWriter& writer, const CanMessageContainer& messageContainer) override;

    [[nodiscard]] Result ReadMessage(ChannelReader& reader, EthMessageContainer& messageContainer) override;
    [[nodiscard]] Result WriteMessage(ChannelWriter& writer, const EthMessageContainer& messageContainer) override;

    [[nodiscard]] Result ReadMessage(ChannelReader& reader, LinMessageContainer& messageContainer) override;
    [[nodiscard]] Result WriteMessage(ChannelWriter& writer, const LinMessageContainer& messageContainer) override;

    [[nodiscard]] Result ReceiveHeader(ChannelReader& reader, FrameKind& frameKind) override;

    [[nodiscard]] Result SendOk(ChannelWriter& writer) override;

    [[nodiscard]] Result ReadError(ChannelReader& reader, std::string& errorMessage) override;
    [[nodiscard]] Result SendError(ChannelWriter& writer, const std::string& errorMessage) override;

    [[nodiscard]] Result SendPing(ChannelWriter& writer) override;

    [[nodiscard]] Result ReadPingOk(ChannelReader& reader, Command& command) override;
    [[nodiscard]] Result SendPingOk(ChannelWriter& writer, Command command) override;

    [[nodiscard]] Result ReadConnect(ChannelReader& reader,
                                     uint32_t& protocolVersion,
                                     Mode& clientMode,
                                     std::string& serverName,
                                     std::string& clientName) override;
    [[nodiscard]] Result SendConnect(ChannelWriter& writer,
                                     uint32_t protocolVersion,
                                     Mode clientMode,
                                     const std::string& serverName,
                                     const std::string& clientName) override;

    [[nodiscard]] Result ReadConnectOkVersion(ChannelReader& reader, uint32_t& protocolVersion) override;
    [[nodiscard]] Result ReadConnectOk(ChannelReader& reader,
                                       Mode& clientMode,
                                       SimulationTime& stepSize,
                                       SimulationState& simulationState,
                                       std::vector<IoSignalContainer>& incomingSignals,
                                       std::vector<IoSignalContainer>& outgoingSignals,
                                       std::vector<CanControllerContainer>& canControllers,
                                       std::vector<EthControllerContainer>& ethControllers,
                                       std::vector<LinControllerContainer>& linControllers,
                                       std::vector<FrControllerContainer>& frControllers) override;

    [[nodiscard]] Result SendConnectOk(ChannelWriter& writer,
                                       uint32_t protocolVersion,
                                       Mode clientMode,
                                       SimulationTime stepSize,
                                       SimulationState simulationState,
                                       const std::vector<IoSignalContainer>& incomingSignals,
                                       const std::vector<IoSignalContainer>& outgoingSignals,
                                       const std::vector<CanControllerContainer>& canControllers,
                                       const std::vector<EthControllerContainer>& ethControllers,
                                       const std::vector<LinControllerContainer>& linControllers,
                                       const std::vector<FrControllerContainer>& frControllers) override;

    [[nodiscard]] Result ReadStart(ChannelReader& reader, SimulationTime& simulationTime) override;
    [[nodiscard]] Result SendStart(ChannelWriter& writer, SimulationTime simulationTime) override;

    [[nodiscard]] Result ReadStop(ChannelReader& reader, SimulationTime& simulationTime) override;
    [[nodiscard]] Result SendStop(ChannelWriter& writer, SimulationTime simulationTime) override;

    [[nodiscard]] Result ReadTerminate(ChannelReader& reader, SimulationTime& simulationTime, TerminateReason& reason) override;
    [[nodiscard]] Result SendTerminate(ChannelWriter& writer, SimulationTime simulationTime, TerminateReason reason) override;

    [[nodiscard]] Result ReadPause(ChannelReader& reader, SimulationTime& simulationTime) override;
    [[nodiscard]] Result SendPause(ChannelWriter& writer, SimulationTime simulationTime) override;

    [[nodiscard]] Result ReadContinue(ChannelReader& reader, SimulationTime& simulationTime) override;
    [[nodiscard]] Result SendContinue(ChannelWriter& writer, SimulationTime simulationTime) override;

    [[nodiscard]] Result ReadStep(ChannelReader& reader,
                                  SimulationTime& simulationTime,
                                  const DeserializeFunction& deserializeIoData,
                                  const DeserializeFunction& deserializeBusMessages,
                                  const Callbacks& callbacks) override;
    [[nodiscard]] Result SendStep(ChannelWriter& writer,
                                  SimulationTime simulationTime,
                                  const SerializeFunction& serializeIoData,
                                  const SerializeFunction& serializeBusMessages) override;

    [[nodiscard]] Result ReadStepOk(ChannelReader& reader,
                                    SimulationTime& nextSimulationTime,
                                    Command& command,
                                    const DeserializeFunction& deserializeIoData,
                                    const DeserializeFunction& deserializeBusMessages,
                                    const Callbacks& callbacks) override;
    [[nodiscard]] Result SendStepOk(ChannelWriter& writer,
                                    SimulationTime nextSimulationTime,
                                    Command command,
                                    const SerializeFunction& serializeIoData,
                                    const SerializeFunction& serializeBusMessages) override;

    [[nodiscard]] Result ReadGetPort(ChannelReader& reader, std::string& serverName) override;
    [[nodiscard]] Result SendGetPort(ChannelWriter& writer, const std::string& serverName) override;

    [[nodiscard]] Result ReadGetPortOk(ChannelReader& reader, uint16_t& port) override;
    [[nodiscard]] Result SendGetPortOk(ChannelWriter& writer, uint16_t port) override;

    [[nodiscard]] Result ReadSetPort(ChannelReader& reader, std::string& serverName, uint16_t& port) override;
    [[nodiscard]] Result SendSetPort(ChannelWriter& writer, const std::string& serverName, uint16_t port) override;

    [[nodiscard]] Result ReadUnsetPort(ChannelReader& reader, std::string& serverName) override;
    [[nodiscard]] Result SendUnsetPort(ChannelWriter& writer, const std::string& serverName) override;

    [[nodiscard]] Result ReadMessage(ChannelReader& reader, FrMessageContainer& messageContainer) override;
    [[nodiscard]] Result WriteMessage(ChannelWriter& writer, const FrMessageContainer& messageContainer) override;

protected:
    [[nodiscard]] Result ReadString(ChannelReader& reader, std::string& string);
    [[nodiscard]] Result WriteString(ChannelWriter& writer, const std::string& string);

    [[nodiscard]] Result ReadIoSignalInfo(ChannelReader& reader, IoSignalContainer& signal);
    [[nodiscard]] Result WriteIoSignalInfo(ChannelWriter& writer, const IoSignalContainer& signal);

    [[nodiscard]] Result ReadIoSignalInfos(ChannelReader& reader, std::vector<IoSignalContainer>& signals);
    [[nodiscard]] Result WriteIoSignalInfos(ChannelWriter& writer, const std::vector<IoSignalContainer>& signals);

    [[nodiscard]] Result ReadControllerInfo(ChannelReader& reader, CanControllerContainer& controller);
    [[nodiscard]] Result WriteControllerInfo(ChannelWriter& writer, const CanControllerContainer& controller);

    [[nodiscard]] Result ReadControllerInfos(ChannelReader& reader, std::vector<CanControllerContainer>& controllers);
    [[nodiscard]] Result WriteControllerInfos(ChannelWriter& writer, const std::vector<CanControllerContainer>& controllers);

    [[nodiscard]] Result ReadControllerInfo(ChannelReader& reader, EthControllerContainer& controller);
    [[nodiscard]] Result WriteControllerInfo(ChannelWriter& writer, const EthControllerContainer& controller);

    [[nodiscard]] Result ReadControllerInfos(ChannelReader& reader, std::vector<EthControllerContainer>& controllers);
    [[nodiscard]] Result WriteControllerInfos(ChannelWriter& writer, const std::vector<EthControllerContainer>& controllers);

    [[nodiscard]] Result ReadControllerInfo(ChannelReader& reader, LinControllerContainer& controller);
    [[nodiscard]] Result WriteControllerInfo(ChannelWriter& writer, const LinControllerContainer& controller);

    [[nodiscard]] Result ReadControllerInfos(ChannelReader& reader, std::vector<LinControllerContainer>& controllers);
    [[nodiscard]] Result WriteControllerInfos(ChannelWriter& writer, const std::vector<LinControllerContainer>& controllers);

    [[nodiscard]] uint32_t GetVersion() override;

    [[nodiscard]] bool DoFlexRayOperations() override;
};

}  // namespace V1

namespace V2 {

class Protocol : public DsVeosCoSim::V1::Protocol {
public:
    static const uint32_t CoSimProtocolVersion = 0x20000U;

    [[nodiscard]] Result ReadConnectOk(ChannelReader& reader,
                                       Mode& clientMode,
                                       SimulationTime& stepSize,
                                       SimulationState& simulationState,
                                       std::vector<IoSignalContainer>& incomingSignals,
                                       std::vector<IoSignalContainer>& outgoingSignals,
                                       std::vector<CanControllerContainer>& canControllers,
                                       std::vector<EthControllerContainer>& ethControllers,
                                       std::vector<LinControllerContainer>& linControllers,
                                       std::vector<FrControllerContainer>& frControllers) override;

    [[nodiscard]] Result SendConnectOk(ChannelWriter& writer,
                                       uint32_t protocolVersion,
                                       Mode clientMode,
                                       SimulationTime stepSize,
                                       SimulationState simulationState,
                                       const std::vector<IoSignalContainer>& incomingSignals,
                                       const std::vector<IoSignalContainer>& outgoingSignals,
                                       const std::vector<CanControllerContainer>& canControllers,
                                       const std::vector<EthControllerContainer>& ethControllers,
                                       const std::vector<LinControllerContainer>& linControllers,
                                       const std::vector<FrControllerContainer>& frControllers) override;

    [[nodiscard]] Result ReadMessage(ChannelReader& reader, FrMessageContainer& messageContainer) override;
    [[nodiscard]] Result WriteMessage(ChannelWriter& writer, const FrMessageContainer& messageContainer) override;

    [[nodiscard]] bool DoFlexRayOperations() override;

protected:
    [[nodiscard]] Result ReadControllerInfo(ChannelReader& reader, FrControllerContainer& controller);
    [[nodiscard]] Result WriteControllerInfo(ChannelWriter& writer, const FrControllerContainer& controller);

    [[nodiscard]] Result ReadControllerInfos(ChannelReader& reader, std::vector<FrControllerContainer>& controllers);
    [[nodiscard]] Result WriteControllerInfos(ChannelWriter& writer, const std::vector<FrControllerContainer>& controllers);
};

}  // namespace V2

constexpr uint32_t V1_VERSION = V1::Protocol::CoSimProtocolVersion;  // 0x10000U
constexpr uint32_t V2_VERSION = V2::Protocol::CoSimProtocolVersion;  // 0x20000U
constexpr uint32_t LATEST_VERSION = V2_VERSION;

[[nodiscard]] Result MakeProtocol(uint32_t negotiatedVersion, std::unique_ptr<IProtocol>& protocol);

}  // namespace DsVeosCoSim
