// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "ProtocolLogger.h"

#include <cstdint>
#include <string>
#include <vector>

#include "DsVeosCoSim/CoSimTypes.h"

namespace DsVeosCoSim {

void LogProtocolBeginTrace(const std::string& message) {
    std::string traceMessage = "PROT BEGIN ";
    traceMessage.append(message);
    Logger::Instance().LogTrace(traceMessage);
}

void LogProtocolEndTrace(const std::string& message) {
    std::string traceMessage = "PROT END   ";
    traceMessage.append(message);
    Logger::Instance().LogTrace(traceMessage);
}

void LogProtocolDataTrace(const std::string& message) {
    std::string traceMessage = "PROT DATA  ";
    traceMessage.append(message);
    Logger::Instance().LogTrace(traceMessage);
}

void LogProtocolBeginTraceReceiveHeader() {
    LogProtocolBeginTrace("ReceiveHeader()");
}

void LogProtocolEndTraceReceiveHeader(FrameKind frameKind) {
    std::string str = "ReceiveHeader(FrameKind: ";
    str.append(format_as(frameKind));
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

void LogProtocolBeginTraceSendPing(std::chrono::nanoseconds roundTripTime) {
    std::string str = "SendPing(RoundTripTime: ";
    str.append(format_as(roundTripTime));
    str.append(")");
    LogProtocolBeginTrace(str);
}

void LogProtocolEndTraceSendPing() {
    LogProtocolEndTrace("SendPing()");
}

void LogProtocolBeginTraceReadPing() {
    LogProtocolBeginTrace("ReadPing()");
}

void LogProtocolEndTraceReadPing(std::chrono::nanoseconds roundTripTime) {
    std::string str = "ReadPing(RoundTripTime: ";
    str.append(format_as(roundTripTime));
    str.append(")");
    LogProtocolEndTrace(str);
}

void LogProtocolBeginTraceSendPingOk(Command command) {
    std::string str = "SendPingOk(Command: ";
    str.append(format_as(command));
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
    str.append(format_as(command));
    str.append(")");
    LogProtocolEndTrace(str);
}

void LogProtocolBeginTraceSendConnect(uint32_t protocolVersion, Mode clientMode, const std::string& serverName, const std::string& clientName) {
    std::string str = "SendConnect(ProtocolVersion: ";
    str.append(std::to_string(protocolVersion));
    str.append(", ClientMode: ");
    str.append(format_as(clientMode));
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

void LogProtocolEndTraceReadConnect(uint32_t protocolVersion, Mode clientMode, const std::string& serverName, const std::string& clientName) {
    std::string str = "ReadConnect(ProtocolVersion: ";
    str.append(std::to_string(protocolVersion));
    str.append(", ClientMode: ");
    str.append(format_as(clientMode));
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
                                        const std::vector<LinControllerContainer>& linControllers,
                                        const std::vector<FrControllerContainer>& frControllers) {
    std::string str = "SendConnectOk(ProtocolVersion: ";
    str.append(std::to_string(protocolVersion));
    str.append(", ClientMode: ");
    str.append(format_as(clientMode));
    str.append(", StepSize: ");
    str.append(format_as(stepSize));
    str.append(" s, SimulationState: ");
    str.append(format_as(simulationState));
    str.append(", IncomingSignals: ");
    str.append(format_as(incomingSignals));
    str.append(", OutgoingSignals: ");
    str.append(format_as(outgoingSignals));
    str.append(", CanControllers: ");
    str.append(format_as(canControllers));
    str.append(", EthControllers: ");
    str.append(format_as(ethControllers));
    str.append(", LinControllers: ");
    str.append(format_as(linControllers));
    str.append(", FrControllers: ");
    str.append(format_as(frControllers));
    str.append(")");
    LogProtocolBeginTrace(str);
}

void LogProtocolEndTraceSendConnectOk() {
    LogProtocolEndTrace("SendConnectOk()");
}

void LogProtocolBeginTraceReadConnectOk() {
    LogProtocolBeginTrace("ReadConnectOk()");
}

void LogProtocolEndTraceReadConnectOkVersion(uint32_t protocolVersion) {
    std::string str = "ReadConnectOk(ProtocolVersion: ";
    str.append(std::to_string(protocolVersion));
    str.append(")");
    LogProtocolEndTrace(str);
}

void LogProtocolEndTraceReadConnectOk(Mode clientMode,
                                      SimulationTime stepSize,
                                      SimulationState simulationState,
                                      const std::vector<IoSignalContainer>& incomingSignals,
                                      const std::vector<IoSignalContainer>& outgoingSignals,
                                      const std::vector<CanControllerContainer>& canControllers,
                                      const std::vector<EthControllerContainer>& ethControllers,
                                      const std::vector<LinControllerContainer>& linControllers,
                                      const std::vector<FrControllerContainer>& frControllers) {
    std::string str = "ClientMode: ";
    str.append(format_as(clientMode));
    str.append(", StepSize: ");
    str.append(format_as(stepSize));
    str.append(" s, SimulationState: ");
    str.append(format_as(simulationState));
    str.append(", IncomingSignals: ");
    str.append(format_as(incomingSignals));
    str.append(", OutgoingSignals: ");
    str.append(format_as(outgoingSignals));
    str.append(", CanControllers: ");
    str.append(format_as(canControllers));
    str.append(", EthControllers: ");
    str.append(format_as(ethControllers));
    str.append(", LinControllers: ");
    str.append(format_as(linControllers));
    str.append(", FrControllers: ");
    str.append(format_as(frControllers));
    str.append(")");
    LogProtocolEndTrace(str);
}

void LogProtocolBeginTraceSendStart(SimulationTime simulationTime) {
    std::string str = "SendStart(SimulationTime: ";
    str.append(format_as(simulationTime));
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
    str.append(format_as(simulationTime));
    str.append(" s)");
    LogProtocolEndTrace(str);
}

void LogProtocolBeginTraceSendStop(SimulationTime simulationTime) {
    std::string str = "SendStop(SimulationTime: ";
    str.append(format_as(simulationTime));
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
    str.append(format_as(simulationTime));
    str.append(" s)");
    LogProtocolEndTrace(str);
}

void LogProtocolBeginTraceSendTerminate(SimulationTime simulationTime, TerminateReason reason) {
    std::string str = "SendTerminate(SimulationTime: ";
    str.append(format_as(simulationTime));
    str.append(" s, Reason: ");
    str.append(format_as(reason));
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
    str.append(format_as(simulationTime));
    str.append(" s, Reason: ");
    str.append(format_as(reason));
    str.append(")");
    LogProtocolEndTrace(str);
}

void LogProtocolBeginTraceSendPause(SimulationTime simulationTime) {
    std::string str = "SendPause(SimulationTime: ";
    str.append(format_as(simulationTime));
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
    str.append(format_as(simulationTime));
    str.append(" s)");
    LogProtocolEndTrace(str);
}

void LogProtocolBeginTraceSendContinue(SimulationTime simulationTime) {
    std::string str = "SendContinue(SimulationTime: ";
    str.append(format_as(simulationTime));
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
    str.append(format_as(simulationTime));
    str.append(" s)");
    LogProtocolEndTrace(str);
}

void LogProtocolBeginTraceSendStep(SimulationTime simulationTime) {
    std::string str = "SendStep(SimulationTime: ";
    str.append(format_as(simulationTime));
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
    str.append(format_as(simulationTime));
    str.append(" s)");
    LogProtocolEndTrace(str);
}

void LogProtocolBeginTraceSendStepOk(SimulationTime simulationTime, Command command) {
    std::string str = "SendStepOk(NextSimulationTime: ";
    str.append(format_as(simulationTime));
    str.append(" s, Command: ");
    str.append(format_as(command));
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
    str.append(format_as(simulationTime));
    str.append(" s, Command: ");
    str.append(format_as(command));
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
    message.append(format_as(signalId));
    message.append(", Length: ");
    message.append(std::to_string(length));
    message.append(", Data: ");
    message.append(ValueToString(dataType, length, data));
    message.append(" }");
    LogProtocolDataTrace(message);
}

}  // namespace DsVeosCoSim
