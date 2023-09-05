// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include "BusBuffer.h"
#include "CoSimTypes.h"
#include "Communication.h"
#include "IoBuffer.h"

namespace DsVeosCoSim {

constexpr uint32_t CoSimProtocolVersion = 0x10000u;

enum class FrameKind : uint32_t {
    Unknown,

    // Both directions
    Ok,
    Error,
    Start,
    Stop,
    Terminate,
    Pause,
    Continue,
    Step,

    // Server -> client
    Accepted,
    Ping,

    // Client -> server
    Connect,
    StepResponse,

    // Port mapper
    GetPort,
    GetPortResponse,
    SetPort,
    UnsetPort
};

enum class Mode : uint32_t {
    None,
    Commander,
    Responder
};

std::string ToString(FrameKind frameKind);

namespace Protocol {

[[nodiscard]] Result ReceiveHeader(Channel& channel, FrameKind& frameKind);

[[nodiscard]] Result SendOk(Channel& channel);

[[nodiscard]] Result SendPing(Channel& channel);

[[nodiscard]] Result SendError(Channel& channel, std::string_view errorStr);
[[nodiscard]] Result ReadError(Channel& channel, std::string& errorStr);

[[nodiscard]] Result SendConnect(Channel& channel, uint32_t protocolVersion, Mode mode, std::string_view serverName, std::string_view clientName);
[[nodiscard]] Result ReadConnect(Channel& channel, uint32_t& protocolVersion, Mode& mode, std::string& serverName, std::string& clientName);

[[nodiscard]] Result SendAccepted(Channel& channel,
                                  uint32_t protocolVersion,
                                  Mode mode,
                                  const std::vector<IoSignal>& incomingSignals,
                                  const std::vector<IoSignal>& outgoingSignals,
                                  const std::vector<CanController>& canControllers,
                                  const std::vector<EthController>& ethControllers,
                                  const std::vector<LinController>& linControllers);
[[nodiscard]] Result ReadAccepted(Channel& channel,
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
                                  std::vector<std::string>& linStrings);

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

[[nodiscard]] Result SendStepResponse(Channel& channel, SimulationTime simulationTime, IoBuffer& ioBuffer, BusBuffer& busBuffer);
[[nodiscard]] Result ReadStepResponse(Channel& channel, SimulationTime& simulationTime, IoBuffer& ioBuffer, BusBuffer& busBuffer, const Callbacks& callbacks);

[[nodiscard]] Result SendGetPort(Channel& channel, std::string_view serverName);
[[nodiscard]] Result ReadGetPort(Channel& channel, std::string& serverName);

[[nodiscard]] Result SendGetPortResponse(Channel& channel, uint16_t port);
[[nodiscard]] Result ReadGetPortResponse(Channel& channel, uint16_t& port);

[[nodiscard]] Result SendSetPort(Channel& channel, std::string_view serverName, uint16_t port);
[[nodiscard]] Result ReadSetPort(Channel& channel, std::string& serverName, uint16_t& port);

[[nodiscard]] Result SendUnsetPort(Channel& channel, std::string_view serverName);
[[nodiscard]] Result ReadUnsetPort(Channel& channel, std::string& serverName);

[[nodiscard]] Result WriteHeader(Channel& channel, FrameKind frameKind);

[[nodiscard]] Result ReadString(Channel& channel, std::string& string);
[[nodiscard]] Result WriteString(Channel& channel, std::string_view string);

[[nodiscard]] Result ReadString(Channel& channel, const char*& string, std::vector<std::string>& strings);

[[nodiscard]] Result ReadIoSignalInfo(Channel& channel, IoSignal& ioSignal, std::vector<std::string>& strings);
[[nodiscard]] Result WriteIoSignalInfo(Channel& channel, const IoSignal& ioSignal);

[[nodiscard]] Result ReadIoSignalInfos(Channel& channel, std::vector<IoSignal>& ioSignals, std::vector<std::string>& strings);
[[nodiscard]] Result WriteIoSignalInfos(Channel& channel, const std::vector<IoSignal>& ioSignals);

[[nodiscard]] Result ReadControllerInfo(Channel& channel, CanController& controller, std::vector<std::string>& strings);
[[nodiscard]] Result WriteControllerInfo(Channel& channel, const CanController& controller);

[[nodiscard]] Result ReadControllerInfos(Channel& channel, std::vector<CanController>& controllers, std::vector<std::string>& strings);
[[nodiscard]] Result WriteControllerInfos(Channel& channel, const std::vector<CanController>& controllers);

[[nodiscard]] Result ReadControllerInfo(Channel& channel, EthController& controller, std::vector<std::string>& strings);
[[nodiscard]] Result WriteControllerInfo(Channel& channel, const EthController& controller);

[[nodiscard]] Result ReadControllerInfos(Channel& channel, std::vector<EthController>& controllers, std::vector<std::string>& strings);
[[nodiscard]] Result WriteControllerInfos(Channel& channel, const std::vector<EthController>& controllers);

[[nodiscard]] Result ReadControllerInfo(Channel& channel, LinController& controller, std::vector<std::string>& strings);
[[nodiscard]] Result WriteControllerInfo(Channel& channel, const LinController& controller);

[[nodiscard]] Result ReadControllerInfos(Channel& channel, std::vector<LinController>& controllers, std::vector<std::string>& strings);
[[nodiscard]] Result WriteControllerInfos(Channel& channel, const std::vector<LinController>& controllers);

}  // namespace Protocol

}  // namespace DsVeosCoSim
