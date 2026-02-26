// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "ProtocolLogger.hpp"

#include <cstdint>
#include <string>
#include <string_view>  // IWYU pragma: keep
#include <vector>

#include "CoSimTypes.hpp"
#include "Logger.hpp"

namespace DsVeosCoSim {

namespace {

constexpr std::string_view ProtocolBegin = "PROT BEGIN";
constexpr std::string_view ProtocolEnd = "PROT END  ";
constexpr std::string_view ProtocolData = "PROT DATA ";

}  // namespace

void LogProtocolDataTrace(std::string_view message) {
    LogTrace("{} {}", ProtocolData, message);
}

void LogProtocolBeginTraceReceiveHeader() {
    LogTrace("{} ReceiveHeader()", ProtocolBegin);
}

void LogProtocolEndTraceReceiveHeader(FrameKind frameKind) {
    LogTrace("{} ReceiveHeader(FrameKind: {})", ProtocolEnd, frameKind);
}

void LogProtocolBeginTraceSendOk() {
    LogTrace("{} SendOk()", ProtocolBegin);
}

void LogProtocolEndTraceSendOk() {
    LogTrace("{} SendOk()", ProtocolEnd);
}

void LogProtocolBeginTraceSendError(std::string_view errorMessage) {
    LogTrace(R"({} SendError(ErrorMessage: "{}"))", ProtocolBegin, errorMessage);
}

void LogProtocolEndTraceSendError() {
    LogTrace("{} SendError()", ProtocolEnd);
}

void LogProtocolBeginTraceReadError() {
    LogTrace("{} ReadError()", ProtocolBegin);
}

void LogProtocolEndTraceReadError(std::string_view errorMessage) {
    LogTrace(R"({} ReadError(ErrorMessage: "{}"))", ProtocolEnd, errorMessage);
}

void LogProtocolBeginTraceSendPing(SimulationTime roundTripTime) {
    LogTrace("{} SendPing(RoundTripTime: {})", ProtocolBegin, roundTripTime);
}

void LogProtocolEndTraceSendPing() {
    LogTrace("{} SendPing()", ProtocolEnd);
}

void LogProtocolBeginTraceReadPing() {
    LogTrace("{} ReadPing()", ProtocolBegin);
}

void LogProtocolEndTraceReadPing(SimulationTime roundTripTime) {
    LogTrace("{} ReadPing(RoundTripTime: {})", ProtocolEnd, roundTripTime);
}

void LogProtocolBeginTraceSendPingOk(Command command) {
    LogTrace("{} SendPingOk(Command: {})", ProtocolBegin, command);
}

void LogProtocolEndTraceSendPingOk() {
    LogTrace("{} SendPingOk()", ProtocolEnd);
}

void LogProtocolBeginTraceReadPingOk() {
    LogTrace("{} ReadPingOk()", ProtocolBegin);
}

void LogProtocolEndTraceReadPingOk(Command command) {
    LogTrace("{} ReadPingOk(Command: {})", ProtocolEnd, command);
}

void LogProtocolBeginTraceSendConnect(uint32_t protocolVersion, Mode clientMode, const std::string& serverName, const std::string& clientName) {
    LogTrace(R"({} SendConnect(ProtocolVersion: {}, ClientMode: {}, ServerName: "{}", ClientName: "{}"))",
             ProtocolBegin,
             protocolVersion,
             clientMode,
             serverName,
             clientName);
}

void LogProtocolEndTraceSendConnect() {
    LogTrace("{} SendConnect()", ProtocolEnd);
}

void LogProtocolBeginTraceReadConnect() {
    LogTrace("{} ReadConnect()", ProtocolBegin);
}

void LogProtocolEndTraceReadConnect(uint32_t protocolVersion, Mode clientMode, const std::string& serverName, const std::string& clientName) {
    LogTrace(R"({} ReadConnect(ProtocolVersion: {}, ClientMode: {}, ServerName: "{}", ClientName: "{}"))",
             ProtocolEnd,
             protocolVersion,
             clientMode,
             serverName,
             clientName);
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
    LogTrace(
        "{} SendConnectOk(ProtocolVersion: {}, ClientMode: {}, StepSize: {} s, SimulationState: {}, IncomingSignals: {}, OutgoingSignals: {}, CanControllers: "
        "{}, EthControllers: {}, LinControllers: {}, FrControllers: {})",
        ProtocolBegin,
        protocolVersion,
        clientMode,
        stepSize,
        simulationState,
        incomingSignals,
        outgoingSignals,
        canControllers,
        ethControllers,
        linControllers,
        frControllers);
}

void LogProtocolEndTraceSendConnectOk() {
    LogTrace("{} SendConnectOk()", ProtocolEnd);
}

void LogProtocolBeginTraceReadConnectOk() {
    LogTrace("{} ReadConnectOk()", ProtocolBegin);
}

void LogProtocolEndTraceReadConnectOkVersion(uint32_t protocolVersion) {
    LogTrace("{} ReadConnectOk(ProtocolVersion: {})", ProtocolEnd, protocolVersion);
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
    LogTrace(
        "{} ReadConnectOk(ClientMode: {}, StepSize: {} s, SimulationState: {}, IncomingSignals: {}, OutgoingSignals: {}, CanControllers: {}, EthControllers: "
        "{}, LinControllers: {}, FrControllers: {})",
        ProtocolEnd,
        clientMode,
        stepSize,
        simulationState,
        incomingSignals,
        outgoingSignals,
        canControllers,
        ethControllers,
        linControllers,
        frControllers);
}

void LogProtocolBeginTraceSendStart(SimulationTime simulationTime) {
    LogTrace("{} SendStart(SimulationTime: {} s)", ProtocolBegin, simulationTime);
}

void LogProtocolEndTraceSendStart() {
    LogTrace("{} SendStart()", ProtocolEnd);
}

void LogProtocolBeginTraceReadStart() {
    LogTrace("{} ReadStart()", ProtocolBegin);
}

void LogProtocolEndTraceReadStart(SimulationTime simulationTime) {
    LogTrace("{} ReadStart(SimulationTime: {} s)", ProtocolEnd, simulationTime);
}

void LogProtocolBeginTraceSendStop(SimulationTime simulationTime) {
    LogTrace("{} SendStop(SimulationTime: {} s)", ProtocolBegin, simulationTime);
}

void LogProtocolEndTraceSendStop() {
    LogTrace("{} SendStop()", ProtocolEnd);
}

void LogProtocolBeginTraceReadStop() {
    LogTrace("{} ReadStop()", ProtocolBegin);
}

void LogProtocolEndTraceReadStop(SimulationTime simulationTime) {
    LogTrace("{} ReadStop(SimulationTime: {} s)", ProtocolEnd, simulationTime);
}

void LogProtocolBeginTraceSendTerminate(SimulationTime simulationTime, TerminateReason reason) {
    LogTrace("{} SendTerminate(SimulationTime: {} s, Reason: {})", ProtocolBegin, simulationTime, reason);
}

void LogProtocolEndTraceSendTerminate() {
    LogTrace("{} SendTerminate()", ProtocolEnd);
}

void LogProtocolBeginTraceReadTerminate() {
    LogTrace("{} ReadTerminate()", ProtocolBegin);
}

void LogProtocolEndTraceReadTerminate(SimulationTime simulationTime, TerminateReason reason) {
    LogTrace("{} ReadTerminate(SimulationTime: {} s, Reason: {})", ProtocolEnd, simulationTime, reason);
}

void LogProtocolBeginTraceSendPause(SimulationTime simulationTime) {
    LogTrace("{} SendPause(SimulationTime: {} s)", ProtocolBegin, simulationTime);
}

void LogProtocolEndTraceSendPause() {
    LogTrace("{} SendPause()", ProtocolEnd);
}

void LogProtocolBeginTraceReadPause() {
    LogTrace("{} ReadPause()", ProtocolBegin);
}

void LogProtocolEndTraceReadPause(SimulationTime simulationTime) {
    LogTrace("{} ReadPause(SimulationTime: {} s)", ProtocolEnd, simulationTime);
}

void LogProtocolBeginTraceSendContinue(SimulationTime simulationTime) {
    LogTrace("{} SendContinue(SimulationTime: {} s)", ProtocolBegin, simulationTime);
}

void LogProtocolEndTraceSendContinue() {
    LogTrace("{} SendContinue()", ProtocolEnd);
}

void LogProtocolBeginTraceReadContinue() {
    LogTrace("{} ReadContinue()", ProtocolBegin);
}

void LogProtocolEndTraceReadContinue(SimulationTime simulationTime) {
    LogTrace("{} ReadContinue(SimulationTime: {} s)", ProtocolEnd, simulationTime);
}

void LogProtocolBeginTraceSendStep(SimulationTime simulationTime) {
    LogTrace("{} SendStep(SimulationTime: {} s)", ProtocolBegin, simulationTime);
}

void LogProtocolEndTraceSendStep() {
    LogTrace("{} SendStep()", ProtocolEnd);
}

void LogProtocolBeginTraceReadStep() {
    LogTrace("{} ReadStep()", ProtocolBegin);
}

void LogProtocolEndTraceReadStep(SimulationTime simulationTime) {
    LogTrace("{} ReadStep(SimulationTime: {} s)", ProtocolEnd, simulationTime);
}

void LogProtocolBeginTraceSendStepOk(SimulationTime simulationTime, Command command) {
    LogTrace("{} SendStepOk(NextSimulationTime: {} s, Command: {})", ProtocolBegin, simulationTime, command);
}

void LogProtocolEndTraceSendStepOk() {
    LogTrace("{} SendStepOk()", ProtocolEnd);
}

void LogProtocolBeginTraceReadStepOk() {
    LogTrace("{} ReadStepOk()", ProtocolBegin);
}

void LogProtocolEndTraceReadStepOk(SimulationTime simulationTime, Command command) {
    LogTrace("{} ReadStepOk(NextSimulationTime: {} s, Command: {})", ProtocolEnd, simulationTime, command);
}

void LogProtocolBeginTraceSendSetPort(const std::string& serverName, uint16_t port) {
    LogTrace(R"({} SendSetPort(ServerName: "{}", Port: {}))", ProtocolBegin, serverName, port);
}

void LogProtocolEndTraceSendSetPort() {
    LogTrace("{} SendSetPort()", ProtocolEnd);
}

void LogProtocolBeginTraceReadSetPort() {
    LogTrace("{} ReadSetPort()", ProtocolBegin);
}

void LogProtocolEndTraceReadSetPort(const std::string& serverName, uint16_t port) {
    LogTrace(R"({} ReadSetPort(ServerName: "{}", Port: {}))", ProtocolEnd, serverName, port);
}

void LogProtocolBeginTraceSendUnsetPort(const std::string& serverName) {
    LogTrace(R"({} SendUnsetPort(ServerName: "{}"))", ProtocolBegin, serverName);
}

void LogProtocolEndTraceSendUnsetPort() {
    LogTrace("{} SendUnsetPort()", ProtocolEnd);
}

void LogProtocolBeginTraceReadUnsetPort() {
    LogTrace("{} ReadUnsetPort()", ProtocolBegin);
}

void LogProtocolEndTraceReadUnsetPort(const std::string& serverName) {
    LogTrace(R"({} ReadUnsetPort(ServerName: "{}"))", ProtocolEnd, serverName);
}

void LogProtocolBeginTraceSendGetPort(const std::string& serverName) {
    LogTrace(R"({} SendGetPort(ServerName: "{}"))", ProtocolBegin, serverName);
}

void LogProtocolEndTraceSendGetPort() {
    LogTrace("{} SendGetPort()", ProtocolEnd);
}

void LogProtocolBeginTraceReadGetPort() {
    LogTrace("{} ReadGetPort()", ProtocolBegin);
}

void LogProtocolEndTraceReadGetPort(const std::string& serverName) {
    LogTrace(R"({} ReadGetPort(ServerName: "{}"))", ProtocolEnd, serverName);
}

void LogProtocolBeginTraceSendGetPortOk(uint16_t port) {
    LogTrace("{} SendGetPortOk(Port: {}", ProtocolBegin, port);
}

void LogProtocolEndTraceSendGetPortOk() {
    LogTrace("{} SendGetPortOk()", ProtocolEnd);
}

void LogProtocolBeginTraceReadGetPortOk() {
    LogTrace("{} ReadGetPortOk()", ProtocolBegin);
}

void LogProtocolEndTraceReadGetPortOk(uint16_t port) {
    LogTrace("{} ReadGetPortOk(Port: {})", ProtocolEnd, port);
}

void LogProtocolDataTraceSignal(IoSignalId signalId, uint32_t length, DataType dataType, const void* data) {
    LogTrace("{} Signal {{ Id: {}, Length: {}, Data: {} }}", ProtocolData, signalId, length, ValueToString(dataType, length, data));
}

}  // namespace DsVeosCoSim
