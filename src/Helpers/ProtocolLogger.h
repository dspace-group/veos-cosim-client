// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "DsVeosCoSim/CoSimTypes.h"

namespace DsVeosCoSim {

void LogProtocolBeginTrace(const std::string& message);
void LogProtocolEndTrace(const std::string& message);
void LogProtocolDataTrace(const std::string& message);

void LogProtocolBeginTraceReceiveHeader();
void LogProtocolEndTraceReceiveHeader(FrameKind frameKind);

void LogProtocolBeginTraceSendOk();
void LogProtocolEndTraceSendOk();

void LogProtocolBeginTraceSendError(const std::string& errorMessage);
void LogProtocolEndTraceSendError();

void LogProtocolBeginTraceReadError();
void LogProtocolEndTraceReadError(const std::string& errorMessage);

void LogProtocolBeginTraceSendPing(std::chrono::nanoseconds roundTripTime);
void LogProtocolEndTraceSendPing();

void LogProtocolBeginTraceReadPing();
void LogProtocolEndTraceReadPing(std::chrono::nanoseconds roundTripTime);

void LogProtocolBeginTraceSendPingOk(Command command);
void LogProtocolEndTraceSendPingOk();

void LogProtocolBeginTraceReadPingOk();
void LogProtocolEndTraceReadPingOk(Command command);

void LogProtocolBeginTraceSendConnect(uint32_t protocolVersion, Mode clientMode, const std::string& serverName, const std::string& clientName);
void LogProtocolEndTraceSendConnect();

void LogProtocolBeginTraceReadConnect();
void LogProtocolEndTraceReadConnect(uint32_t protocolVersion, Mode clientMode, const std::string& serverName, const std::string& clientName);

void LogProtocolBeginTraceSendConnectOk(uint32_t protocolVersion,
                                        Mode clientMode,
                                        SimulationTime stepSize,
                                        SimulationState simulationState,
                                        const std::vector<IoSignalContainer>& incomingSignals,
                                        const std::vector<IoSignalContainer>& outgoingSignals,
                                        const std::vector<CanControllerContainer>& canControllers,
                                        const std::vector<EthControllerContainer>& ethControllers,
                                        const std::vector<LinControllerContainer>& linControllers,
                                        const std::vector<FrControllerContainer>& frControllers);
void LogProtocolEndTraceSendConnectOk();

void LogProtocolEndTraceReadConnectOkVersion(uint32_t protocolVersion);
void LogProtocolBeginTraceReadConnectOk();
void LogProtocolEndTraceReadConnectOk(Mode clientMode,
                                      SimulationTime stepSize,
                                      SimulationState simulationState,
                                      const std::vector<IoSignalContainer>& incomingSignals,
                                      const std::vector<IoSignalContainer>& outgoingSignals,
                                      const std::vector<CanControllerContainer>& canControllers,
                                      const std::vector<EthControllerContainer>& ethControllers,
                                      const std::vector<LinControllerContainer>& linControllers,
                                      const std::vector<FrControllerContainer>& frControllers);

void LogProtocolBeginTraceSendStart(SimulationTime simulationTime);
void LogProtocolEndTraceSendStart();

void LogProtocolBeginTraceReadStart();
void LogProtocolEndTraceReadStart(SimulationTime simulationTime);

void LogProtocolBeginTraceSendStop(SimulationTime simulationTime);
void LogProtocolEndTraceSendStop();

void LogProtocolBeginTraceReadStop();
void LogProtocolEndTraceReadStop(SimulationTime simulationTime);

void LogProtocolBeginTraceSendTerminate(SimulationTime simulationTime, TerminateReason reason);
void LogProtocolEndTraceSendTerminate();

void LogProtocolBeginTraceReadTerminate();
void LogProtocolEndTraceReadTerminate(SimulationTime simulationTime, TerminateReason reason);

void LogProtocolBeginTraceSendPause(SimulationTime simulationTime);
void LogProtocolEndTraceSendPause();

void LogProtocolBeginTraceReadPause();
void LogProtocolEndTraceReadPause(SimulationTime simulationTime);

void LogProtocolBeginTraceSendContinue(SimulationTime simulationTime);
void LogProtocolEndTraceSendContinue();

void LogProtocolBeginTraceReadContinue();
void LogProtocolEndTraceReadContinue(SimulationTime simulationTime);

void LogProtocolBeginTraceSendStep(SimulationTime simulationTime);
void LogProtocolEndTraceSendStep();

void LogProtocolBeginTraceReadStep();
void LogProtocolEndTraceReadStep(SimulationTime simulationTime);

void LogProtocolBeginTraceSendStepOk(SimulationTime simulationTime, Command command);
void LogProtocolEndTraceSendStepOk();

void LogProtocolBeginTraceReadStepOk();
void LogProtocolEndTraceReadStepOk(SimulationTime simulationTime, Command command);

void LogProtocolBeginTraceSendSetPort(const std::string& serverName, uint16_t port);
void LogProtocolEndTraceSendSetPort();

void LogProtocolBeginTraceReadSetPort();
void LogProtocolEndTraceReadSetPort(const std::string& serverName, uint16_t port);

void LogProtocolBeginTraceSendUnsetPort(const std::string& serverName);
void LogProtocolEndTraceSendUnsetPort();

void LogProtocolBeginTraceReadUnsetPort();
void LogProtocolEndTraceReadUnsetPort(const std::string& serverName);

void LogProtocolBeginTraceSendGetPort(const std::string& serverName);
void LogProtocolEndTraceSendGetPort();

void LogProtocolBeginTraceReadGetPort();
void LogProtocolEndTraceReadGetPort(const std::string& serverName);

void LogProtocolBeginTraceSendGetPortOk(uint16_t port);
void LogProtocolEndTraceSendGetPortOk();

void LogProtocolBeginTraceReadGetPortOk();
void LogProtocolEndTraceReadGetPortOk(uint16_t port);

void LogProtocolDataTraceSignal(IoSignalId signalId, uint32_t length, DataType dataType, const void* data);

}  // namespace DsVeosCoSim
