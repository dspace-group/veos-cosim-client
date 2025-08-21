// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "DsVeosCoSim/CoSimTypes.h"

namespace DsVeosCoSim {

void LogError(const std::string& message);
void LogWarning(const std::string& message);
void LogInfo(const std::string& message);
void LogTrace(const std::string& message);
void LogSystemError(const std::string& message, int32_t errorCode);
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

void LogProtocolBeginTraceSendPing();
void LogProtocolEndTraceSendPing();

void LogProtocolBeginTraceSendPingOk(Command command);
void LogProtocolEndTraceSendPingOk();

void LogProtocolBeginTraceReadPingOk();
void LogProtocolEndTraceReadPingOk(Command command);

void LogProtocolBeginTraceSendConnect(uint32_t protocolVersion,
                                      Mode clientMode,
                                      const std::string& serverName,
                                      const std::string& clientName);
void LogProtocolEndTraceSendConnect();

void LogProtocolBeginTraceReadConnect();
void LogProtocolEndTraceReadConnect(uint32_t protocolVersion,
                                    Mode clientMode,
                                    const std::string& serverName,
                                    const std::string& clientName);

void LogProtocolBeginTraceSendConnectOk(uint32_t protocolVersion,
                                        Mode clientMode,
                                        SimulationTime stepSize,
                                        SimulationState simulationState,
                                        const std::vector<IoSignalContainer>& incomingSignals,
                                        const std::vector<IoSignalContainer>& outgoingSignals,
                                        const std::vector<CanControllerContainer>& canControllers,
                                        const std::vector<EthControllerContainer>& ethControllers,
                                        const std::vector<LinControllerContainer>& linControllers);
void LogProtocolEndTraceSendConnectOk();

void LogProtocolBeginTraceReadConnectOk();
void LogProtocolEndTraceReadConnectOk(uint32_t protocolVersion,
                                      Mode clientMode,
                                      SimulationTime stepSize,
                                      SimulationState simulationState,
                                      const std::vector<IoSignalContainer>& incomingSignals,
                                      const std::vector<IoSignalContainer>& outgoingSignals,
                                      const std::vector<CanControllerContainer>& canControllers,
                                      const std::vector<EthControllerContainer>& ethControllers,
                                      const std::vector<LinControllerContainer>& linControllers);

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

void LogProtocolDataTraceSignal(IoSignalId id, uint32_t length, DataType dataType, const void* data);

#define CheckResultWithMessage(result, message) \
    do {                                        \
        Result _result_ = (result);             \
        if (!IsOk(_result_)) {                  \
            LogTrace(message);                  \
            return _result_;                    \
        }                                       \
    } while (0)

#define CheckBoolWithMessage(result, message) \
    do {                                      \
        if (!(result)) {                      \
            LogTrace(message);                \
            return Result::Error;             \
        }                                     \
    } while (0)

[[nodiscard]] std::string GetSystemErrorMessage(int32_t errorCode);

}  // namespace DsVeosCoSim
