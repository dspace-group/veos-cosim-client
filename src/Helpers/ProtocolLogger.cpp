// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "ProtocolLogger.hpp"

#include <cstdint>
#include <string>
#include <string_view>  // IWYU pragma: keep
#include <vector>

#include "CoSimTypes.hpp"
#include "Format.hpp"
#include "Logger.hpp"

namespace DsVeosCoSim {

namespace {

void LogProtocolBeginTrace(std::string_view message) {
    Logger::Instance().LogTrace(Format("PROT BEGIN {}", message));
}

void LogProtocolEndTrace(std::string_view message) {
    Logger::Instance().LogTrace(Format("PROT END   {}", message));
}

}  // namespace

void LogProtocolDataTrace(std::string_view message) {
    Logger::Instance().LogTrace(Format("PROT DATA  {}", message));
}

void LogProtocolBeginTraceReceiveHeader() {
    LogProtocolBeginTrace("ReceiveHeader()");
}

void LogProtocolEndTraceReceiveHeader(FrameKind frameKind) {
    LogProtocolEndTrace(Format("ReceiveHeader(FrameKind: {})", frameKind));
}

void LogProtocolBeginTraceSendOk() {
    LogProtocolBeginTrace("SendOk()");
}

void LogProtocolEndTraceSendOk() {
    LogProtocolEndTrace("SendOk()");
}

void LogProtocolBeginTraceSendError(std::string_view errorMessage) {
    LogProtocolBeginTrace(Format(R"(SendError(ErrorMessage: "{}"))", errorMessage));
}

void LogProtocolEndTraceSendError() {
    LogProtocolEndTrace("SendError()");
}

void LogProtocolBeginTraceReadError() {
    LogProtocolBeginTrace("ReadError()");
}

void LogProtocolEndTraceReadError(std::string_view errorMessage) {
    LogProtocolEndTrace(Format(R"(ReadError(ErrorMessage: "{}"))", errorMessage));
}

void LogProtocolBeginTraceSendPing(SimulationTime roundTripTime) {
    LogProtocolBeginTrace(Format("SendPing(RoundTripTime: {})", roundTripTime));
}

void LogProtocolEndTraceSendPing() {
    LogProtocolEndTrace("SendPing()");
}

void LogProtocolBeginTraceReadPing() {
    LogProtocolBeginTrace("ReadPing()");
}

void LogProtocolEndTraceReadPing(SimulationTime roundTripTime) {
    LogProtocolEndTrace(Format("ReadPing(RoundTripTime: {})", roundTripTime));
}

void LogProtocolBeginTraceSendPingOk(Command command) {
    LogProtocolBeginTrace(Format("SendPingOk(Command: {})", command));
}

void LogProtocolEndTraceSendPingOk() {
    LogProtocolEndTrace("SendPingOk()");
}

void LogProtocolBeginTraceReadPingOk() {
    LogProtocolBeginTrace("ReadPingOk()");
}

void LogProtocolEndTraceReadPingOk(Command command) {
    LogProtocolEndTrace(Format("ReadPingOk(Command: {})", command));
}

void LogProtocolBeginTraceSendConnect(uint32_t protocolVersion, Mode clientMode, const std::string& serverName, const std::string& clientName) {
    LogProtocolBeginTrace(
        Format(R"(SendConnect(ProtocolVersion: {}, ClientMode: {}, ServerName: "{}", ClientName: "{}"))", protocolVersion, clientMode, serverName, clientName));
}

void LogProtocolEndTraceSendConnect() {
    LogProtocolEndTrace("SendConnect()");
}

void LogProtocolBeginTraceReadConnect() {
    LogProtocolBeginTrace("ReadConnect()");
}

void LogProtocolEndTraceReadConnect(uint32_t protocolVersion, Mode clientMode, const std::string& serverName, const std::string& clientName) {
    LogProtocolEndTrace(
        Format(R"(ReadConnect(ProtocolVersion: {}, ClientMode: {}, ServerName: "{}", ClientName: "{}"))", protocolVersion, clientMode, serverName, clientName));
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
    LogProtocolBeginTrace(
        Format("SendConnectOk(ProtocolVersion: {}, ClientMode: {}, StepSize: {} s, SimulationState: {}, IncomingSignals: {}, OutgoingSignals: {}, "
               "CanControllers: {}, EthControllers: {}, LinControllers: {}, FrControllers: {})",
               protocolVersion,
               clientMode,
               stepSize,
               simulationState,
               incomingSignals,
               outgoingSignals,
               canControllers,
               ethControllers,
               linControllers,
               frControllers));
}

void LogProtocolEndTraceSendConnectOk() {
    LogProtocolEndTrace("SendConnectOk()");
}

void LogProtocolBeginTraceReadConnectOk() {
    LogProtocolBeginTrace("ReadConnectOk()");
}

void LogProtocolEndTraceReadConnectOkVersion(uint32_t protocolVersion) {
    LogProtocolEndTrace(Format("ReadConnectOk(ProtocolVersion: {})", protocolVersion));
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
    LogProtocolEndTrace(
        Format("ReadConnectOk(ClientMode: {}, StepSize: {} s, SimulationState: {}, IncomingSignals: {}, OutgoingSignals: {}, CanControllers: {}, "
               "EthControllers: {}, LinControllers: {}, FrControllers: {})",
               clientMode,
               stepSize,
               simulationState,
               incomingSignals,
               outgoingSignals,
               canControllers,
               ethControllers,
               linControllers,
               frControllers));
}

void LogProtocolBeginTraceSendStart(SimulationTime simulationTime) {
    LogProtocolBeginTrace(Format("SendStart(SimulationTime: {} s)", simulationTime));
}

void LogProtocolEndTraceSendStart() {
    LogProtocolEndTrace("SendStart()");
}

void LogProtocolBeginTraceReadStart() {
    LogProtocolBeginTrace("ReadStart()");
}

void LogProtocolEndTraceReadStart(SimulationTime simulationTime) {
    LogProtocolEndTrace(Format("ReadStart(SimulationTime: {} s)", simulationTime));
}

void LogProtocolBeginTraceSendStop(SimulationTime simulationTime) {
    LogProtocolBeginTrace(Format("SendStop(SimulationTime: {} s)", simulationTime));
}

void LogProtocolEndTraceSendStop() {
    LogProtocolEndTrace("SendStop()");
}

void LogProtocolBeginTraceReadStop() {
    LogProtocolBeginTrace("ReadStop()");
}

void LogProtocolEndTraceReadStop(SimulationTime simulationTime) {
    LogProtocolEndTrace(Format("ReadStop(SimulationTime: {} s)", simulationTime));
}

void LogProtocolBeginTraceSendTerminate(SimulationTime simulationTime, TerminateReason reason) {
    LogProtocolBeginTrace(Format("SendTerminate(SimulationTime: {} s, Reason: {})", simulationTime, reason));
}

void LogProtocolEndTraceSendTerminate() {
    LogProtocolEndTrace("SendTerminate()");
}

void LogProtocolBeginTraceReadTerminate() {
    LogProtocolBeginTrace("ReadTerminate()");
}

void LogProtocolEndTraceReadTerminate(SimulationTime simulationTime, TerminateReason reason) {
    LogProtocolEndTrace(Format("ReadTerminate(SimulationTime: {} s, Reason: {})", simulationTime, reason));
}

void LogProtocolBeginTraceSendPause(SimulationTime simulationTime) {
    LogProtocolBeginTrace(Format("SendPause(SimulationTime: {} s)", simulationTime));
}

void LogProtocolEndTraceSendPause() {
    LogProtocolEndTrace("SendPause()");
}

void LogProtocolBeginTraceReadPause() {
    LogProtocolBeginTrace("ReadPause()");
}

void LogProtocolEndTraceReadPause(SimulationTime simulationTime) {
    LogProtocolEndTrace(Format("ReadPause(SimulationTime: {} s)", simulationTime));
}

void LogProtocolBeginTraceSendContinue(SimulationTime simulationTime) {
    LogProtocolBeginTrace(Format("SendContinue(SimulationTime: {} s)", simulationTime));
}

void LogProtocolEndTraceSendContinue() {
    LogProtocolEndTrace("SendContinue()");
}

void LogProtocolBeginTraceReadContinue() {
    LogProtocolBeginTrace("ReadContinue()");
}

void LogProtocolEndTraceReadContinue(SimulationTime simulationTime) {
    LogProtocolEndTrace(Format("ReadContinue(SimulationTime: {} s)", simulationTime));
}

void LogProtocolBeginTraceSendStep(SimulationTime simulationTime) {
    LogProtocolBeginTrace(Format("SendStep(SimulationTime: {} s)", simulationTime));
}

void LogProtocolEndTraceSendStep() {
    LogProtocolEndTrace("SendStep()");
}

void LogProtocolBeginTraceReadStep() {
    LogProtocolBeginTrace("ReadStep()");
}

void LogProtocolEndTraceReadStep(SimulationTime simulationTime) {
    LogProtocolEndTrace(Format("ReadStep(SimulationTime: {} s)", simulationTime));
}

void LogProtocolBeginTraceSendStepOk(SimulationTime simulationTime, Command command) {
    LogProtocolBeginTrace(Format("SendStepOk(NextSimulationTime: {} s, Command: {})", simulationTime, command));
}

void LogProtocolEndTraceSendStepOk() {
    LogProtocolEndTrace("SendStepOk()");
}

void LogProtocolBeginTraceReadStepOk() {
    LogProtocolBeginTrace("ReadStepOk()");
}

void LogProtocolEndTraceReadStepOk(SimulationTime simulationTime, Command command) {
    LogProtocolEndTrace(Format("ReadStepOk(NextSimulationTime: {} s, Command: {})", simulationTime, command));
}

void LogProtocolBeginTraceSendSetPort(const std::string& serverName, uint16_t port) {
    LogProtocolBeginTrace(Format(R"(SendSetPort(ServerName: "{}", Port: {}))", serverName, port));
}

void LogProtocolEndTraceSendSetPort() {
    LogProtocolEndTrace("SendSetPort()");
}

void LogProtocolBeginTraceReadSetPort() {
    LogProtocolBeginTrace("ReadSetPort()");
}

void LogProtocolEndTraceReadSetPort(const std::string& serverName, uint16_t port) {
    LogProtocolEndTrace(Format(R"(ReadSetPort(ServerName: "{}", Port: {}))", serverName, port));
}

void LogProtocolBeginTraceSendUnsetPort(const std::string& serverName) {
    LogProtocolBeginTrace(Format(R"(SendUnsetPort(ServerName: "{}"))", serverName));
}

void LogProtocolEndTraceSendUnsetPort() {
    LogProtocolEndTrace("SendUnsetPort()");
}

void LogProtocolBeginTraceReadUnsetPort() {
    LogProtocolBeginTrace("ReadUnsetPort()");
}

void LogProtocolEndTraceReadUnsetPort(const std::string& serverName) {
    LogProtocolEndTrace(Format(R"(ReadUnsetPort(ServerName: "{}"))", serverName));
}

void LogProtocolBeginTraceSendGetPort(const std::string& serverName) {
    LogProtocolBeginTrace(Format(R"(SendGetPort(ServerName: "{}"))", serverName));
}

void LogProtocolEndTraceSendGetPort() {
    LogProtocolEndTrace("SendGetPort()");
}

void LogProtocolBeginTraceReadGetPort() {
    LogProtocolBeginTrace("ReadGetPort()");
}

void LogProtocolEndTraceReadGetPort(const std::string& serverName) {
    LogProtocolEndTrace(Format(R"(ReadGetPort(ServerName: "{}"))", serverName));
}

void LogProtocolBeginTraceSendGetPortOk(uint16_t port) {
    LogProtocolBeginTrace(Format("SendGetPortOk(Port: {}", port));
}

void LogProtocolEndTraceSendGetPortOk() {
    LogProtocolEndTrace("SendGetPortOk()");
}

void LogProtocolBeginTraceReadGetPortOk() {
    LogProtocolBeginTrace("ReadGetPortOk()");
}

void LogProtocolEndTraceReadGetPortOk(uint16_t port) {
    LogProtocolEndTrace(Format("ReadGetPortOk(Port: {})", port));
}

void LogProtocolDataTraceSignal(IoSignalId signalId, uint32_t length, DataType dataType, const void* data) {
    LogProtocolDataTrace(Format("Signal { Id: {}, Length: {}, Data: {} }", signalId, length, ValueToString(dataType, length, data)));
}

}  // namespace DsVeosCoSim
