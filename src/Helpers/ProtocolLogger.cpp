// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "ProtocolLogger.h"

#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

#include "DsVeosCoSim/CoSimTypes.h"

namespace DsVeosCoSim {

void LogProtocolBeginTrace(const std::string& message) {
    std::ostringstream oss;
    oss << "PROT BEGIN " << message;
    Logger::Instance().LogTrace(oss.str());
}

void LogProtocolEndTrace(const std::string& message) {
    std::ostringstream oss;
    oss << "PROT END   " << message;
    Logger::Instance().LogTrace(oss.str());
}

void LogProtocolDataTrace(const std::string& message) {
    std::ostringstream oss;
    oss << "PROT DATA  " << message;
    Logger::Instance().LogTrace(oss.str());
}

void LogProtocolBeginTraceReceiveHeader() {
    LogProtocolBeginTrace("ReceiveHeader()");
}

void LogProtocolEndTraceReceiveHeader(FrameKind frameKind) {
    std::ostringstream oss;
    oss << "ReceiveHeader(FrameKind: " << frameKind << ')';
    LogProtocolEndTrace(oss.str());
}

void LogProtocolBeginTraceSendOk() {
    LogProtocolBeginTrace("SendOk()");
}

void LogProtocolEndTraceSendOk() {
    LogProtocolEndTrace("SendOk()");
}

void LogProtocolBeginTraceSendError(const std::string& errorMessage) {
    std::ostringstream oss;
    oss << "SendError(ErrorMessage: \"" << errorMessage << "\")";
    LogProtocolBeginTrace(oss.str());
}

void LogProtocolEndTraceSendError() {
    LogProtocolEndTrace("SendError()");
}

void LogProtocolBeginTraceReadError() {
    LogProtocolBeginTrace("ReadError()");
}

void LogProtocolEndTraceReadError(const std::string& errorMessage) {
    std::ostringstream oss;
    oss << "ReadError(ErrorMessage: \"" << errorMessage << "\")";
    LogProtocolEndTrace(oss.str());
}

void LogProtocolBeginTraceSendPing(std::chrono::nanoseconds roundTripTime) {
    std::ostringstream oss;
    oss << "SendPing(RoundTripTime: " << roundTripTime << ')';
    LogProtocolBeginTrace(oss.str());
}

void LogProtocolEndTraceSendPing() {
    LogProtocolEndTrace("SendPing()");
}

void LogProtocolBeginTraceReadPing() {
    LogProtocolBeginTrace("ReadPing()");
}

void LogProtocolEndTraceReadPing(std::chrono::nanoseconds roundTripTime) {
    std::ostringstream oss;
    oss << "ReadPing(RoundTripTime: " << roundTripTime << ')';
    LogProtocolEndTrace(oss.str());
}

void LogProtocolBeginTraceSendPingOk(Command command) {
    std::ostringstream oss;
    oss << "SendPingOk(Command: " << command << ')';
    LogProtocolBeginTrace(oss.str());
}

void LogProtocolEndTraceSendPingOk() {
    LogProtocolEndTrace("SendPingOk()");
}

void LogProtocolBeginTraceReadPingOk() {
    LogProtocolBeginTrace("ReadPingOk()");
}

void LogProtocolEndTraceReadPingOk(Command command) {
    std::ostringstream oss;
    oss << "ReadPingOk(Command: " << command << ')';
    LogProtocolEndTrace(oss.str());
}

void LogProtocolBeginTraceSendConnect(uint32_t protocolVersion, Mode clientMode, const std::string& serverName, const std::string& clientName) {
    std::ostringstream oss;
    oss << "SendConnect(ProtocolVersion: " << protocolVersion << ", ClientMode: " << clientMode << ", ServerName: \"" << serverName << "\", ClientName: \""
        << clientName << "\")";
    LogProtocolBeginTrace(oss.str());
}

void LogProtocolEndTraceSendConnect() {
    LogProtocolEndTrace("SendConnect()");
}

void LogProtocolBeginTraceReadConnect() {
    LogProtocolBeginTrace("ReadConnect()");
}

void LogProtocolEndTraceReadConnect(uint32_t protocolVersion, Mode clientMode, const std::string& serverName, const std::string& clientName) {
    std::ostringstream oss;
    oss << "ReadConnect(ProtocolVersion: " << protocolVersion << ", ClientMode: " << clientMode << ", ServerName: \"" << serverName << "\", ClientName: \""
        << clientName << "\")";
    LogProtocolEndTrace(oss.str());
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
    std::ostringstream oss;
    oss << "SendConnectOk(ProtocolVersion: " << protocolVersion << ", ClientMode: " << clientMode << ", StepSize: " << stepSize
        << " s, SimulationState: " << simulationState << ", IncomingSignals: " << incomingSignals << ", OutgoingSignals: " << outgoingSignals
        << ", CanControllers: " << canControllers << ", EthControllers: " << ethControllers << ", LinControllers: " << linControllers
        << ", FrControllers: " << frControllers << ')';
    LogProtocolBeginTrace(oss.str());
}

void LogProtocolEndTraceSendConnectOk() {
    LogProtocolEndTrace("SendConnectOk()");
}

void LogProtocolBeginTraceReadConnectOk() {
    LogProtocolBeginTrace("ReadConnectOk()");
}

void LogProtocolEndTraceReadConnectOkVersion(uint32_t protocolVersion) {
    std::ostringstream oss;
    oss << "ReadConnectOk(ProtocolVersion: " << protocolVersion << ')';
    LogProtocolEndTrace(oss.str());
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
    std::ostringstream oss;
    oss << "ClientMode: " << clientMode << ", StepSize: " << stepSize << " s, SimulationState: " << simulationState << ", IncomingSignals: " << incomingSignals
        << ", OutgoingSignals: " << outgoingSignals << ", CanControllers: " << canControllers << ", EthControllers: " << ethControllers
        << ", LinControllers: " << linControllers << ", FrControllers: " << frControllers << ')';
    LogProtocolEndTrace(oss.str());
}

void LogProtocolBeginTraceSendStart(SimulationTime simulationTime) {
    std::ostringstream oss;
    oss << "SendStart(SimulationTime: " << simulationTime << " s)";
    LogProtocolBeginTrace(oss.str());
}

void LogProtocolEndTraceSendStart() {
    LogProtocolEndTrace("SendStart()");
}

void LogProtocolBeginTraceReadStart() {
    LogProtocolBeginTrace("ReadStart()");
}

void LogProtocolEndTraceReadStart(SimulationTime simulationTime) {
    std::ostringstream oss;
    oss << "ReadStart(SimulationTime: " << simulationTime << " s)";
    LogProtocolEndTrace(oss.str());
}

void LogProtocolBeginTraceSendStop(SimulationTime simulationTime) {
    std::ostringstream oss;
    oss << "SendStop(SimulationTime: " << simulationTime << " s)";
    LogProtocolBeginTrace(oss.str());
}

void LogProtocolEndTraceSendStop() {
    LogProtocolEndTrace("SendStop()");
}

void LogProtocolBeginTraceReadStop() {
    LogProtocolBeginTrace("ReadStop()");
}

void LogProtocolEndTraceReadStop(SimulationTime simulationTime) {
    std::ostringstream oss;
    oss << "ReadStop(SimulationTime: " << simulationTime << " s)";
    LogProtocolEndTrace(oss.str());
}

void LogProtocolBeginTraceSendTerminate(SimulationTime simulationTime, TerminateReason reason) {
    std::ostringstream oss;
    oss << "SendTerminate(SimulationTime: " << simulationTime << " s, Reason: " << reason << ')';
    LogProtocolBeginTrace(oss.str());
}

void LogProtocolEndTraceSendTerminate() {
    LogProtocolEndTrace("SendTerminate()");
}

void LogProtocolBeginTraceReadTerminate() {
    LogProtocolBeginTrace("ReadTerminate()");
}

void LogProtocolEndTraceReadTerminate(SimulationTime simulationTime, TerminateReason reason) {
    std::ostringstream oss;
    oss << "ReadTerminate(SimulationTime: " << simulationTime << " s, Reason: " << reason << ')';
    LogProtocolEndTrace(oss.str());
}

void LogProtocolBeginTraceSendPause(SimulationTime simulationTime) {
    std::ostringstream oss;
    oss << "SendPause(SimulationTime: " << simulationTime << " s)";
    LogProtocolBeginTrace(oss.str());
}

void LogProtocolEndTraceSendPause() {
    LogProtocolEndTrace("SendPause()");
}

void LogProtocolBeginTraceReadPause() {
    LogProtocolBeginTrace("ReadPause()");
}

void LogProtocolEndTraceReadPause(SimulationTime simulationTime) {
    std::ostringstream oss;
    oss << "ReadPause(SimulationTime: " << simulationTime << " s)";
    LogProtocolEndTrace(oss.str());
}

void LogProtocolBeginTraceSendContinue(SimulationTime simulationTime) {
    std::ostringstream oss;
    oss << "SendContinue(SimulationTime: " << simulationTime << " s)";
    LogProtocolBeginTrace(oss.str());
}

void LogProtocolEndTraceSendContinue() {
    LogProtocolEndTrace("SendContinue()");
}

void LogProtocolBeginTraceReadContinue() {
    LogProtocolBeginTrace("ReadContinue()");
}

void LogProtocolEndTraceReadContinue(SimulationTime simulationTime) {
    std::ostringstream oss;
    oss << "ReadContinue(SimulationTime: " << simulationTime << " s)";
    LogProtocolEndTrace(oss.str());
}

void LogProtocolBeginTraceSendStep(SimulationTime simulationTime) {
    std::ostringstream oss;
    oss << "SendStep(SimulationTime: " << simulationTime << " s)";
    LogProtocolBeginTrace(oss.str());
}

void LogProtocolEndTraceSendStep() {
    LogProtocolEndTrace("SendStep()");
}

void LogProtocolBeginTraceReadStep() {
    LogProtocolBeginTrace("ReadStep()");
}

void LogProtocolEndTraceReadStep(SimulationTime simulationTime) {
    std::ostringstream oss;
    oss << "ReadStep(SimulationTime: " << simulationTime << " s)";
    LogProtocolEndTrace(oss.str());
}

void LogProtocolBeginTraceSendStepOk(SimulationTime simulationTime, Command command) {
    std::ostringstream oss;
    oss << "SendStepOk(NextSimulationTime: " << simulationTime << " s, Command: " << command << ')';
    LogProtocolBeginTrace(oss.str());
}

void LogProtocolEndTraceSendStepOk() {
    LogProtocolEndTrace("SendStepOk()");
}

void LogProtocolBeginTraceReadStepOk() {
    LogProtocolBeginTrace("ReadStepOk()");
}

void LogProtocolEndTraceReadStepOk(SimulationTime simulationTime, Command command) {
    std::ostringstream oss;
    oss << "ReadStepOk(NextSimulationTime: " << simulationTime << " s, Command: " << command << ')';
    LogProtocolEndTrace(oss.str());
}

void LogProtocolBeginTraceSendSetPort(const std::string& serverName, uint16_t port) {
    std::ostringstream oss;
    oss << "SendSetPort(ServerName: \"" << serverName << "\", Port: " << port << ')';
    LogProtocolBeginTrace(oss.str());
}

void LogProtocolEndTraceSendSetPort() {
    LogProtocolEndTrace("SendSetPort()");
}

void LogProtocolBeginTraceReadSetPort() {
    LogProtocolBeginTrace("ReadSetPort()");
}

void LogProtocolEndTraceReadSetPort(const std::string& serverName, uint16_t port) {
    std::ostringstream oss;
    oss << "ReadSetPort(ServerName: \"" << serverName << "\", Port: " << port << ')';
    LogProtocolEndTrace(oss.str());
}

void LogProtocolBeginTraceSendUnsetPort(const std::string& serverName) {
    std::ostringstream oss;
    oss << "SendUnsetPort(ServerName: \"" << serverName << "\")";
    LogProtocolBeginTrace(oss.str());
}

void LogProtocolEndTraceSendUnsetPort() {
    LogProtocolEndTrace("SendUnsetPort()");
}

void LogProtocolBeginTraceReadUnsetPort() {
    LogProtocolBeginTrace("ReadUnsetPort()");
}

void LogProtocolEndTraceReadUnsetPort(const std::string& serverName) {
    std::ostringstream oss;
    oss << "ReadUnsetPort(ServerName: \"" << serverName << "\")";
    LogProtocolEndTrace(oss.str());
}

void LogProtocolBeginTraceSendGetPort(const std::string& serverName) {
    std::ostringstream oss;
    oss << "SendGetPort(ServerName: \"" << serverName << "\")";
    LogProtocolBeginTrace(oss.str());
}

void LogProtocolEndTraceSendGetPort() {
    LogProtocolEndTrace("SendGetPort()");
}

void LogProtocolBeginTraceReadGetPort() {
    LogProtocolBeginTrace("ReadGetPort()");
}

void LogProtocolEndTraceReadGetPort(const std::string& serverName) {
    std::ostringstream oss;
    oss << "ReadGetPort(ServerName: \"" << serverName << "\")";
    LogProtocolEndTrace(oss.str());
}

void LogProtocolBeginTraceSendGetPortOk(uint16_t port) {
    std::ostringstream oss;
    oss << "SendGetPortOk(Port: " << port << ')';
    LogProtocolBeginTrace(oss.str());
}

void LogProtocolEndTraceSendGetPortOk() {
    LogProtocolEndTrace("SendGetPortOk()");
}

void LogProtocolBeginTraceReadGetPortOk() {
    LogProtocolBeginTrace("ReadGetPortOk()");
}

void LogProtocolEndTraceReadGetPortOk(uint16_t port) {
    std::ostringstream oss;
    oss << "ReadGetPortOk(Port: " << port << ')';
    LogProtocolEndTrace(oss.str());
}

void LogProtocolDataTraceSignal(IoSignalId signalId, uint32_t length, DataType dataType, const void* data) {
    std::ostringstream oss;
    oss << "Signal { Id: " << signalId << ", Length: " << length << ", Data: " << ValueToString(dataType, length, data) << " }";
    LogProtocolDataTrace(oss.str());
}

}  // namespace DsVeosCoSim
