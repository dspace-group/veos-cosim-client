// Copyright dSPACE GmbH. All rights reserved.

#include "CoSimHelper.h"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#if _WIN32
#include "OsUtilities.h"
#else
#include <system_error>
#endif

#include "DsVeosCoSim/CoSimTypes.h"

namespace DsVeosCoSim {

namespace {

LogCallback LogCallbackHandler;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

}  // namespace

void SetLogCallback(LogCallback logCallback) {
    LogCallbackHandler = std::move(logCallback);
}

void LogError(const std::string& message) {
    if (auto logCallback = LogCallbackHandler; logCallback) {
        logCallback(Severity::Error, message);
    }
}

void LogWarning(const std::string& message) {
    if (auto logCallback = LogCallbackHandler; logCallback) {
        logCallback(Severity::Warning, message);
    }
}

void LogInfo(const std::string& message) {
    if (auto logCallback = LogCallbackHandler; logCallback) {
        logCallback(Severity::Info, message);
    }
}

void LogTrace(const std::string& message) {
    if (auto logCallback = LogCallbackHandler; logCallback) {
        logCallback(Severity::Trace, message);
    }
}

void LogSystemError(const std::string& message, int32_t errorCode) {
    if (auto logCallback = LogCallbackHandler; logCallback) {
        std::string fullMessage(message);
        fullMessage.append(" ");
        fullMessage.append(GetSystemErrorMessage(errorCode));
        logCallback(Severity::Error, fullMessage);
    }
}

void LogProtocolBeginTrace(const std::string& message) {
    std::string traceMessage = "PROT BEGIN ";
    traceMessage.append(message);
    LogTrace(traceMessage);
}

void LogProtocolEndTrace(const std::string& message) {
    std::string traceMessage = "PROT END   ";
    traceMessage.append(message);
    LogTrace(traceMessage);
}

void LogProtocolDataTrace(const std::string& message) {
    std::string traceMessage = "PROT DATA  ";
    traceMessage.append(message);
    LogTrace(traceMessage);
}

void LogProtocolBeginTraceReceiveHeader() {
    LogProtocolBeginTrace("ReceiveHeader()");
}

void LogProtocolEndTraceReceiveHeader(FrameKind frameKind) {
    std::string str = "ReceiveHeader(FrameKind: ";
    str.append(ToString(frameKind));
    str.append(")");
    LogProtocolEndTrace(str);
}

void LogProtocolBeginTraceSendOk() {
    LogProtocolBeginTrace("SendOk()");
}

void LogProtocolEndTraceSendOk() {
    LogProtocolEndTrace("SendOk()");
}

void LogProtocolBeginTraceSendError(const std::string& errorMessage) {
    std::string str = "SendError(ErrorMessage: \"";
    str.append(errorMessage);
    str.append("\")");
    LogProtocolBeginTrace(str);
}

void LogProtocolEndTraceSendError() {
    LogProtocolEndTrace("SendError()");
}

void LogProtocolBeginTraceReadError() {
    LogProtocolBeginTrace("ReadError()");
}

void LogProtocolEndTraceReadError(const std::string& errorMessage) {
    std::string str = "ReadError(ErrorMessage: \"";
    str.append(errorMessage);
    str.append("\")");
    LogProtocolEndTrace(str);
}

void LogProtocolBeginTraceSendPing() {
    LogProtocolBeginTrace("SendPing()");
}

void LogProtocolEndTraceSendPing() {
    LogProtocolEndTrace("SendPing()");
}

void LogProtocolBeginTraceSendPingOk(Command command) {
    std::string str = "SendPingOk(Command: ";
    str.append(ToString(command));
    str.append(")");
    LogProtocolBeginTrace(str);
}

void LogProtocolEndTraceSendPingOk() {
    LogProtocolEndTrace("SendPingOk()");
}

void LogProtocolBeginTraceReadPingOk() {
    LogProtocolBeginTrace("ReadPingOk()");
}

void LogProtocolEndTraceReadPingOk(Command command) {
    std::string str = "ReadPingOk(Command: ";
    str.append(ToString(command));
    str.append(")");
    LogProtocolEndTrace(str);
}

void LogProtocolBeginTraceSendConnect(uint32_t protocolVersion,
                                      Mode clientMode,
                                      const std::string& serverName,
                                      const std::string& clientName) {
    std::string str = "SendConnect(ProtocolVersion: ";
    str.append(std::to_string(protocolVersion));
    str.append(", ClientMode: ");
    str.append(ToString(clientMode));
    str.append(", ServerName: \"");
    str.append(serverName);
    str.append("\", ClientName: \"");
    str.append(clientName);
    str.append("\")");
    LogProtocolBeginTrace(str);
}

void LogProtocolEndTraceSendConnect() {
    LogProtocolEndTrace("SendConnect()");
}

void LogProtocolBeginTraceReadConnect() {
    LogProtocolBeginTrace("ReadConnect()");
}

void LogProtocolEndTraceReadConnect(uint32_t protocolVersion,
                                    Mode clientMode,
                                    const std::string& serverName,
                                    const std::string& clientName) {
    std::string str = "ReadConnect(ProtocolVersion: ";
    str.append(std::to_string(protocolVersion));
    str.append(", ClientMode: ");
    str.append(ToString(clientMode));
    str.append(", ServerName: \"");
    str.append(serverName);
    str.append("\", ClientName: \"");
    str.append(clientName);
    str.append("\")");
    LogProtocolEndTrace(str);
}

void LogProtocolBeginTraceSendConnectOk(uint32_t protocolVersion,
                                        Mode clientMode,
                                        SimulationTime stepSize,
                                        SimulationState simulationState,
                                        const std::vector<IoSignalContainer>& incomingSignals,
                                        const std::vector<IoSignalContainer>& outgoingSignals,
                                        const std::vector<CanControllerContainer>& canControllers,
                                        const std::vector<EthControllerContainer>& ethControllers,
                                        const std::vector<LinControllerContainer>& linControllers) {
    std::string str = "SendConnectOk(ProtocolVersion: ";
    str.append(std::to_string(protocolVersion));
    str.append(", ClientMode: ");
    str.append(ToString(clientMode));
    str.append(", StepSize: ");
    str.append(SimulationTimeToString(stepSize));
    str.append(" s, SimulationState: ");
    str.append(ToString(simulationState));
    str.append(", IncomingSignals: ");
    str.append(ToString(incomingSignals));
    str.append(", OutgoingSignals: ");
    str.append(ToString(outgoingSignals));
    str.append(", CanControllers: ");
    str.append(ToString(canControllers));
    str.append(", EthControllers: ");
    str.append(ToString(ethControllers));
    str.append(", LinControllers: ");
    str.append(ToString(linControllers));
    str.append(")");
    LogProtocolBeginTrace(str);
}

void LogProtocolEndTraceSendConnectOk() {
    LogProtocolEndTrace("SendConnectOk()");
}

void LogProtocolBeginTraceReadConnectOk() {
    LogProtocolBeginTrace("ReadConnectOk()");
}

void LogProtocolEndTraceReadConnectOk(uint32_t protocolVersion,
                                      Mode clientMode,
                                      SimulationTime stepSize,
                                      SimulationState simulationState,
                                      const std::vector<IoSignalContainer>& incomingSignals,
                                      const std::vector<IoSignalContainer>& outgoingSignals,
                                      const std::vector<CanControllerContainer>& canControllers,
                                      const std::vector<EthControllerContainer>& ethControllers,
                                      const std::vector<LinControllerContainer>& linControllers) {
    std::string str = "ReadConnectOk(ProtocolVersion: ";
    str.append(std::to_string(protocolVersion));
    str.append(", ClientMode: ");
    str.append(ToString(clientMode));
    str.append(", StepSize: ");
    str.append(SimulationTimeToString(stepSize));
    str.append(" s, SimulationState: ");
    str.append(ToString(simulationState));
    str.append(", IncomingSignals: ");
    str.append(ToString(incomingSignals));
    str.append(", OutgoingSignals: ");
    str.append(ToString(outgoingSignals));
    str.append(", CanControllers: ");
    str.append(ToString(canControllers));
    str.append(", EthControllers: ");
    str.append(ToString(ethControllers));
    str.append(", LinControllers: ");
    str.append(ToString(linControllers));
    str.append(")");
    LogProtocolEndTrace(str);
}

void LogProtocolBeginTraceSendStart(SimulationTime simulationTime) {
    std::string str = "SendStart(SimulationTime: ";
    str.append(SimulationTimeToString(simulationTime));
    str.append(" s)");
    LogProtocolBeginTrace(str);
}

void LogProtocolEndTraceSendStart() {
    LogProtocolEndTrace("SendStart()");
}

void LogProtocolBeginTraceReadStart() {
    LogProtocolBeginTrace("ReadStart()");
}

void LogProtocolEndTraceReadStart(SimulationTime simulationTime) {
    std::string str = "ReadStart(SimulationTime: ";
    str.append(SimulationTimeToString(simulationTime));
    str.append(" s)");
    LogProtocolEndTrace(str);
}

void LogProtocolBeginTraceSendStop(SimulationTime simulationTime) {
    std::string str = "SendStop(SimulationTime: ";
    str.append(SimulationTimeToString(simulationTime));
    str.append(" s)");
    LogProtocolBeginTrace(str);
}

void LogProtocolEndTraceSendStop() {
    LogProtocolEndTrace("SendStop()");
}

void LogProtocolBeginTraceReadStop() {
    LogProtocolBeginTrace("ReadStop()");
}

void LogProtocolEndTraceReadStop(SimulationTime simulationTime) {
    std::string str = "ReadStop(SimulationTime: ";
    str.append(SimulationTimeToString(simulationTime));
    str.append(" s)");
    LogProtocolEndTrace(str);
}

void LogProtocolBeginTraceSendTerminate(SimulationTime simulationTime, TerminateReason reason) {
    std::string str = "SendTerminate(SimulationTime: ";
    str.append(SimulationTimeToString(simulationTime));
    str.append(" s, Reason: ");
    str.append(ToString(reason));
    str.append(")");
    LogProtocolBeginTrace(str);
}

void LogProtocolEndTraceSendTerminate() {
    LogProtocolEndTrace("SendTerminate()");
}

void LogProtocolBeginTraceReadTerminate() {
    LogProtocolBeginTrace("ReadTerminate()");
}

void LogProtocolEndTraceReadTerminate(SimulationTime simulationTime, TerminateReason reason) {
    std::string str = "ReadTerminate(SimulationTime: ";
    str.append(SimulationTimeToString(simulationTime));
    str.append(" s, Reason: ");
    str.append(ToString(reason));
    str.append(")");
    LogProtocolEndTrace(str);
}

void LogProtocolBeginTraceSendPause(SimulationTime simulationTime) {
    std::string str = "SendPause(SimulationTime: ";
    str.append(SimulationTimeToString(simulationTime));
    str.append(" s)");
    LogProtocolBeginTrace(str);
}

void LogProtocolEndTraceSendPause() {
    LogProtocolEndTrace("SendPause()");
}

void LogProtocolBeginTraceReadPause() {
    LogProtocolBeginTrace("ReadPause()");
}

void LogProtocolEndTraceReadPause(SimulationTime simulationTime) {
    std::string str = "ReadPause(SimulationTime: ";
    str.append(SimulationTimeToString(simulationTime));
    str.append(" s)");
    LogProtocolEndTrace(str);
}

void LogProtocolBeginTraceSendContinue(SimulationTime simulationTime) {
    std::string str = "SendContinue(SimulationTime: ";
    str.append(SimulationTimeToString(simulationTime));
    str.append(" s)");
    LogProtocolBeginTrace(str);
}

void LogProtocolEndTraceSendContinue() {
    LogProtocolEndTrace("SendContinue()");
}

void LogProtocolBeginTraceReadContinue() {
    LogProtocolBeginTrace("ReadContinue()");
}

void LogProtocolEndTraceReadContinue(SimulationTime simulationTime) {
    std::string str = "ReadContinue(SimulationTime: ";
    str.append(SimulationTimeToString(simulationTime));
    str.append(" s)");
    LogProtocolEndTrace(str);
}

void LogProtocolBeginTraceSendStep(SimulationTime simulationTime) {
    std::string str = "SendStep(SimulationTime: ";
    str.append(SimulationTimeToString(simulationTime));
    str.append(" s)");
    LogProtocolBeginTrace(str);
}

void LogProtocolEndTraceSendStep() {
    LogProtocolEndTrace("SendStep()");
}

void LogProtocolBeginTraceReadStep() {
    LogProtocolBeginTrace("ReadStep()");
}

void LogProtocolEndTraceReadStep(SimulationTime simulationTime) {
    std::string str = "ReadStep(SimulationTime: ";
    str.append(SimulationTimeToString(simulationTime));
    str.append(" s)");
    LogProtocolEndTrace(str);
}

void LogProtocolBeginTraceSendStepOk(SimulationTime simulationTime, Command command) {
    std::string str = "SendStepOk(NextSimulationTime: ";
    str.append(SimulationTimeToString(simulationTime));
    str.append(" s, Command: ");
    str.append(ToString(command));
    str.append(")");
    LogProtocolBeginTrace(str);
}

void LogProtocolEndTraceSendStepOk() {
    LogProtocolEndTrace("SendStepOk()");
}

void LogProtocolBeginTraceReadStepOk() {
    LogProtocolBeginTrace("ReadStepOk()");
}

void LogProtocolEndTraceReadStepOk(SimulationTime simulationTime, Command command) {
    std::string str = "ReadStepOk(NextSimulationTime: ";
    str.append(SimulationTimeToString(simulationTime));
    str.append(" s, Command: ");
    str.append(ToString(command));
    str.append(")");
    LogProtocolEndTrace(str);
}

void LogProtocolBeginTraceSendSetPort(const std::string& serverName, uint16_t port) {
    std::string str = "SendSetPort(ServerName: \"";
    str.append(serverName);
    str.append("\", Port: ");
    str.append(std::to_string(port));
    str.append(")");
    LogProtocolBeginTrace(str);
}

void LogProtocolEndTraceSendSetPort() {
    LogProtocolEndTrace("SendSetPort()");
}

void LogProtocolBeginTraceReadSetPort() {
    LogProtocolBeginTrace("ReadSetPort()");
}

void LogProtocolEndTraceReadSetPort(const std::string& serverName, uint16_t port) {
    std::string str = "ReadSetPort(ServerName: \"";
    str.append(serverName);
    str.append("\", Port: ");
    str.append(std::to_string(port));
    str.append(")");
    LogProtocolEndTrace(str);
}

void LogProtocolBeginTraceSendUnsetPort(const std::string& serverName) {
    std::string str = "SendUnsetPort(ServerName: \"";
    str.append(serverName);
    str.append("\")");
    LogProtocolBeginTrace(str);
}

void LogProtocolEndTraceSendUnsetPort() {
    LogProtocolEndTrace("SendUnsetPort()");
}

void LogProtocolBeginTraceReadUnsetPort() {
    LogProtocolBeginTrace("ReadUnsetPort()");
}

void LogProtocolEndTraceReadUnsetPort(const std::string& serverName) {
    std::string str = "ReadUnsetPort(ServerName: \"";
    str.append(serverName);
    str.append("\")");
    LogProtocolEndTrace(str);
}

void LogProtocolBeginTraceSendGetPort(const std::string& serverName) {
    std::string str = "SendGetPort(ServerName: \"";
    str.append(serverName);
    str.append("\")");
    LogProtocolBeginTrace(str);
}

void LogProtocolEndTraceSendGetPort() {
    LogProtocolEndTrace("SendGetPort()");
}

void LogProtocolBeginTraceReadGetPort() {
    LogProtocolBeginTrace("ReadGetPort()");
}

void LogProtocolEndTraceReadGetPort(const std::string& serverName) {
    std::string str = "ReadGetPort(ServerName: \"";
    str.append(serverName);
    str.append("\")");
    LogProtocolEndTrace(str);
}

void LogProtocolBeginTraceSendGetPortOk(uint16_t port) {
    std::string str = "SendGetPortOk(Port: ";
    str.append(std::to_string(port));
    str.append(")");
    LogProtocolBeginTrace(str);
}

void LogProtocolEndTraceSendGetPortOk() {
    LogProtocolEndTrace("SendGetPortOk()");
}

void LogProtocolBeginTraceReadGetPortOk() {
    LogProtocolBeginTrace("ReadGetPortOk()");
}

void LogProtocolEndTraceReadGetPortOk(uint16_t port) {
    std::string str = "ReadGetPortOk(Port: ";
    str.append(std::to_string(port));
    str.append(")");
    LogProtocolEndTrace(str);
}

void LogProtocolDataTraceSignal(IoSignalId signalId, uint32_t length, DataType dataType, const void* data) {
    std::string message = "Signal { Id: ";
    message.append(ToString(signalId));
    message.append(", Length: ");
    message.append(std::to_string(length));
    message.append(", Data: ");
    message.append(ValueToString(dataType, length, data));
    message.append(" }");
    LogProtocolDataTrace(message);
}

[[nodiscard]] std::string GetSystemErrorMessage(int32_t errorCode) {
    std::string message = "Error code: ";
    message.append(std::to_string(errorCode));
    message.append(". ");

#if _WIN32
    message.append(GetEnglishErrorMessage(errorCode));
#else
    message.append(std::system_category().message(errorCode));
#endif

    return message;
}

}  // namespace DsVeosCoSim
