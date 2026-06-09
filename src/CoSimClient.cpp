// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "CoSimClient.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <fmt/format.h>

#include "BusExchange.hpp"
#include "Channel.hpp"
#include "CoSimTypes.hpp"
#include "Logger.hpp"
#include "OsUtilities.hpp"
#include "PortMapper.hpp"
#include "Protocol.hpp"
#include "Result.hpp"
#include "SignalExchange.hpp"

namespace DsVeosCoSim {

constexpr uint32_t ClientTimeoutInMilliseconds = 1000;

CoSimClient::CoSimClient() {
    _serializeIoData = [this](ChannelWriter& writer) {
        return _signalExchange->Serialize(writer);
    };

    _serializeBusMessages = [this](ChannelWriter& writer) {
        return _busExchange->Serialize(writer);
    };

    _deserializeIoData = [this](ChannelReader& reader, SimulationTime simulationTime, const Callbacks& callbacks) {
        return _signalExchange->Deserialize(reader, simulationTime, callbacks);
    };

    _deserializeBusMessages = [this](ChannelReader& reader, SimulationTime simulationTime, const Callbacks& callbacks) {
        return _busExchange->Deserialize(reader, simulationTime, callbacks);
    };
}

[[nodiscard]] Result CoSimClient::Connect(const ConnectConfig& connectConfig) {
    if (connectConfig.serverName.empty() && (connectConfig.remotePort == 0)) {
        LogError("Either ConnectConfig.serverName or ConnectConfig.remotePort must be set.");
        return CreateInvalidArgument();
    }

    if (_isConnected) {
        return CreateOk();
    }

    ResetDataFromPreviousConnect();

    _remoteIpAddress = connectConfig.remoteIpAddress;
    _serverName = connectConfig.serverName;
    _clientName = connectConfig.clientName;
    _remotePort = connectConfig.remotePort;

    CheckResult(CreateProtocol(ProtocolVersion1, _protocol));

    CheckResult(ConnectInternal());

    // Co-Sim connect
    CheckResult(SendConnectRequest());
    CheckResultWithMessage(ReceiveConnectResponse(), "Could not receive connect response.");
    return CreateOk();
}

void CoSimClient::Disconnect() {
    LogInfo("Disconnecting from dSPACE VEOS CoSim server ...");
    _isConnected = false;

    if (_channel) {
        _channel->Disconnect();
    }
}

[[nodiscard]] ConnectionState CoSimClient::GetConnectionState() const {
    return _isConnected ? ConnectionState::Connected : ConnectionState::Disconnected;
}

[[nodiscard]] Result CoSimClient::GetStepSize(SimulationTime& stepSize) const {
    CheckResult(EnsureIsConnected());

    stepSize = _stepSize;
    return CreateOk();
}

[[nodiscard]] Result CoSimClient::GetCurrentSimulationTime(SimulationTime& simulationTime) const {
    CheckResult(EnsureIsConnected());

    simulationTime = _currentSimulationTime;
    return CreateOk();
}

[[nodiscard]] Result CoSimClient::GetSimulationState(SimulationState& simulationState) const {
    CheckResult(EnsureIsConnected());

    simulationState = _simulationState;
    return CreateOk();
}

[[nodiscard]] Result CoSimClient::RunCallbackBasedCoSimulation(const Callbacks& callbacks) {
    CheckResult(EnsureIsConnected());
    CheckResult(EnsureIsInResponderModeBlocking());

    _callbacks = callbacks;

    SetThreadAffinity(_serverName);

    Result result = RunCallbackBasedCoSimulationInternal();
    if (!IsOk(result)) {
        CloseConnection();
    }

    return result;
}

[[nodiscard]] Result CoSimClient::StartPollingBasedCoSimulation(const Callbacks& callbacks) {
    CheckResult(EnsureIsConnected());
    CheckResult(EnsureIsInResponderModeNonBlocking());

    SetThreadAffinity(_serverName);

    _callbacks = callbacks;
    return CreateOk();
}

[[nodiscard]] Result CoSimClient::PollCommand(SimulationTime& simulationTime, Command& command, uint32_t timeoutInMilliseconds) {
    CheckResult(EnsureIsConnected());
    CheckResult(EnsureIsInResponderModeNonBlocking());

    if (_currentCommand != Command::None) {
        LogError("Call to FinishCommand() for last command is missing.");
        return CreateError();
    }

    Result result = PollCommandInternal(simulationTime, command, timeoutInMilliseconds);
    if (IsOk(result)) {
        return result;
    }

    if (IsTimeout(result)) {
        _currentCommand = Command::None;
        return result;
    }

    CloseConnection();
    return result;
}

[[nodiscard]] Result CoSimClient::FinishCommand() {
    CheckResult(EnsureIsConnected());
    CheckResult(EnsureIsInResponderModeNonBlocking());

    if (_currentCommand == Command::None) {
        LogError("Call to PollCommand(...) is missing.");
        return CreateError();
    }

    Result result = FinishCommandInternal();
    if (!IsOk(result)) {
        CloseConnection();
    }

    return result;
}

[[nodiscard]] Result CoSimClient::SetNextSimulationTime(SimulationTime simulationTime) {
    CheckResult(EnsureIsConnected());

    _nextSimulationTime = simulationTime;
    return CreateOk();
}

[[nodiscard]] Result CoSimClient::GetRoundTripTime(SimulationTime& roundTripTime) const {
    CheckResult(EnsureIsConnected());

    roundTripTime = _roundTripTime;
    return CreateOk();
}

[[nodiscard]] Result CoSimClient::Start() {
    CheckResult(EnsureIsConnected());

    _nextCommand.exchange(Command::Start);
    return CreateOk();
}

[[nodiscard]] Result CoSimClient::Stop() {
    CheckResult(EnsureIsConnected());

    _nextCommand.exchange(Command::Stop);
    return CreateOk();
}

[[nodiscard]] Result CoSimClient::Terminate(TerminateReason terminateReason) {
    CheckResult(EnsureIsConnected());

    switch (terminateReason) {
        case TerminateReason::Finished:
            _nextCommand.exchange(Command::TerminateFinished);
            return CreateOk();
        case TerminateReason::Error:
            _nextCommand.exchange(Command::Terminate);
            return CreateOk();
    }

    LogError("Unknown terminate reason '{}'.", terminateReason);
    return CreateError();
}

[[nodiscard]] Result CoSimClient::Pause() {
    CheckResult(EnsureIsConnected());

    _nextCommand.exchange(Command::Pause);
    return CreateOk();
}

[[nodiscard]] Result CoSimClient::Continue() {
    CheckResult(EnsureIsConnected());

    _nextCommand.exchange(Command::Continue);
    return CreateOk();
}

[[nodiscard]] Result CoSimClient::GetIncomingSignals(uint32_t& incomingSignalsCount, const IoSignal*& incomingSignals) const {
    CheckResult(EnsureIsConnected());

    incomingSignalsCount = static_cast<uint32_t>(_incomingSignalsExtern.size());
    incomingSignals = _incomingSignalsExtern.data();
    return CreateOk();
}

[[nodiscard]] Result CoSimClient::GetOutgoingSignals(uint32_t& outgoingSignalsCount, const IoSignal*& outgoingSignals) const {
    CheckResult(EnsureIsConnected());

    outgoingSignalsCount = static_cast<uint32_t>(_outgoingSignalsExtern.size());
    outgoingSignals = _outgoingSignalsExtern.data();
    return CreateOk();
}

[[nodiscard]] Result CoSimClient::GetIncomingSignals(std::vector<IoSignal>& signals) const {
    CheckResult(EnsureIsConnected());

    signals = _incomingSignalsExtern;
    return CreateOk();
}

[[nodiscard]] Result CoSimClient::GetOutgoingSignals(std::vector<IoSignal>& signals) const {
    CheckResult(EnsureIsConnected());

    signals = _outgoingSignalsExtern;
    return CreateOk();
}

[[nodiscard]] Result CoSimClient::Write(IoSignalId outgoingSignalId, uint32_t length, const void* value) const {
    CheckResult(EnsureIsConnected());

    return _signalExchange->Write(outgoingSignalId, length, value);
}

[[nodiscard]] Result CoSimClient::Read(IoSignalId incomingSignalId, uint32_t& length, void* value) const {
    CheckResult(EnsureIsConnected());

    return _signalExchange->Read(incomingSignalId, length, value);
}

[[nodiscard]] Result CoSimClient::Read(IoSignalId incomingSignalId, uint32_t& length, const void** value) const {
    CheckResult(EnsureIsConnected());

    return _signalExchange->Read(incomingSignalId, length, value);
}

[[nodiscard]] Result CoSimClient::GetCanControllers(uint32_t& controllersCount, const CanController*& controllers) const {
    CheckResult(EnsureIsConnected());

    controllersCount = static_cast<uint32_t>(_canControllersExtern.size());
    controllers = _canControllersExtern.data();
    return CreateOk();
}

[[nodiscard]] Result CoSimClient::GetEthControllers(uint32_t& controllersCount, const EthController*& controllers) const {
    CheckResult(EnsureIsConnected());

    controllersCount = static_cast<uint32_t>(_ethControllersExtern.size());
    controllers = _ethControllersExtern.data();
    return CreateOk();
}

[[nodiscard]] Result CoSimClient::GetLinControllers(uint32_t& controllersCount, const LinController*& controllers) const {
    CheckResult(EnsureIsConnected());

    controllersCount = static_cast<uint32_t>(_linControllersExtern.size());
    controllers = _linControllersExtern.data();
    return CreateOk();
}

[[nodiscard]] Result CoSimClient::GetFrControllers(uint32_t& controllersCount, const FrController*& controllers) const {
    CheckResult(EnsureIsConnected());

    controllersCount = static_cast<uint32_t>(_frControllersExtern.size());
    controllers = _frControllersExtern.data();
    return CreateOk();
}

[[nodiscard]] Result CoSimClient::GetCanControllers(std::vector<CanController>& controllers) const {
    CheckResult(EnsureIsConnected());

    controllers = _canControllersExtern;
    return CreateOk();
}

[[nodiscard]] Result CoSimClient::GetEthControllers(std::vector<EthController>& controllers) const {
    CheckResult(EnsureIsConnected());

    controllers = _ethControllersExtern;
    return CreateOk();
}

[[nodiscard]] Result CoSimClient::GetLinControllers(std::vector<LinController>& controllers) const {
    CheckResult(EnsureIsConnected());

    controllers = _linControllersExtern;
    return CreateOk();
}

[[nodiscard]] Result CoSimClient::GetFrControllers(std::vector<FrController>& controllers) const {
    CheckResult(EnsureIsConnected());

    controllers = _frControllersExtern;
    return CreateOk();
}

[[nodiscard]] Result CoSimClient::Transmit(const CanMessage& message) const {
    CheckResult(EnsureIsConnected());
    CheckResult(CheckCanMessage(message.flags, message.length));

    return _busExchange->Transmit(message);
}

[[nodiscard]] Result CoSimClient::Transmit(const EthMessage& message) const {
    CheckResult(EnsureIsConnected());

    return _busExchange->Transmit(message);
}

[[nodiscard]] Result CoSimClient::Transmit(const LinMessage& message) const {
    CheckResult(EnsureIsConnected());

    return _busExchange->Transmit(message);
}

[[nodiscard]] Result CoSimClient::Transmit(const FrMessage& message) const {
    CheckResult(EnsureIsConnected());

    return _busExchange->Transmit(message);
}

[[nodiscard]] Result CoSimClient::Transmit(const CanMessageContainer& messageContainer) const {
    CheckResult(EnsureIsConnected());
    CheckResult(CheckCanMessage(messageContainer.flags, messageContainer.length));

    return _busExchange->Transmit(messageContainer);
}

[[nodiscard]] Result CoSimClient::Transmit(const EthMessageContainer& messageContainer) const {
    CheckResult(EnsureIsConnected());

    return _busExchange->Transmit(messageContainer);
}

[[nodiscard]] Result CoSimClient::Transmit(const LinMessageContainer& messageContainer) const {
    CheckResult(EnsureIsConnected());

    return _busExchange->Transmit(messageContainer);
}

[[nodiscard]] Result CoSimClient::Transmit(const FrMessageContainer& messageContainer) const {
    CheckResult(EnsureIsConnected());

    return _busExchange->Transmit(messageContainer);
}

[[nodiscard]] Result CoSimClient::Receive(CanMessage& message) const {
    CheckResult(EnsureIsConnected());

    return _busExchange->Receive(message);
}

[[nodiscard]] Result CoSimClient::Receive(EthMessage& message) const {
    CheckResult(EnsureIsConnected());

    return _busExchange->Receive(message);
}

[[nodiscard]] Result CoSimClient::Receive(LinMessage& message) const {
    CheckResult(EnsureIsConnected());

    return _busExchange->Receive(message);
}

[[nodiscard]] Result CoSimClient::Receive(FrMessage& message) const {
    CheckResult(EnsureIsConnected());

    return _busExchange->Receive(message);
}

[[nodiscard]] Result CoSimClient::Receive(CanMessageContainer& messageContainer) const {
    CheckResult(EnsureIsConnected());

    return _busExchange->Receive(messageContainer);
}

[[nodiscard]] Result CoSimClient::Receive(EthMessageContainer& messageContainer) const {
    CheckResult(EnsureIsConnected());

    return _busExchange->Receive(messageContainer);
}

[[nodiscard]] Result CoSimClient::Receive(LinMessageContainer& messageContainer) const {
    CheckResult(EnsureIsConnected());

    return _busExchange->Receive(messageContainer);
}

[[nodiscard]] Result CoSimClient::Receive(FrMessageContainer& messageContainer) const {
    CheckResult(EnsureIsConnected());

    return _busExchange->Receive(messageContainer);
}

void CoSimClient::ResetDataFromPreviousConnect() {
    _responderMode = {};
    _currentCommand = {};
    _isConnected = {};
    _currentSimulationTime = {};
    _nextSimulationTime = {};
    _nextCommand.exchange({});
    _callbacks = {};
    if (_channel) {
        _channel->Disconnect();
    }

    _incomingSignals.clear();
    _outgoingSignals.clear();
    _incomingSignalsExtern.clear();
    _outgoingSignalsExtern.clear();
    _canControllers.clear();
    _ethControllers.clear();
    _linControllers.clear();
    _frControllers.clear();
    _canControllersExtern.clear();
    _ethControllersExtern.clear();
    _linControllersExtern.clear();
    _frControllersExtern.clear();
}

[[nodiscard]] Result CoSimClient::ConnectInternal() {
    if (!_serverName.empty() && _remoteIpAddress.empty() && (_remotePort == 0)) {
        if (IsOk(LocalConnect())) {
            return CreateOk();
        }

        _remoteIpAddress = "127.0.0.1";
    }

    return RemoteConnect();
}

[[nodiscard]] Result CoSimClient::LocalConnect() {
    CheckResultWithMessage(TryConnectToLocalChannel(_serverName, _channel),
                           fmt::format("Could not connect to local dSPACE VEOS CoSim server '{}'.", _serverName));

    _connectionKind = ConnectionKind::Local;
    return CreateOk();
}

[[nodiscard]] Result CoSimClient::RemoteConnect() {
    if (_remotePort == 0) {
        LogInfo("Obtaining TCP port of dSPACE VEOS CoSim server '{}' at {} ...", _serverName, _remoteIpAddress);
        CheckResultWithMessage(PortMapperGetPort(_remoteIpAddress, _serverName, _remotePort), "Could not get port from port mapper.");
    }

    if (_serverName.empty()) {
        LogInfo("Connecting to dSPACE VEOS CoSim server at {}:{} ...", _remoteIpAddress, _remotePort);
    } else {
        LogInfo("Connecting to dSPACE VEOS CoSim server '{}' at {}:{} ...", _serverName, _remoteIpAddress, _remotePort);
    }

    CheckResultWithMessage(TryConnectToTcpChannel(_remoteIpAddress, _remotePort, _localPort, ClientTimeoutInMilliseconds, _channel),
                           "Could not connect to dSPACE VEOS CoSim server.");

    _connectionKind = ConnectionKind::Remote;
    return CreateOk();
}

[[nodiscard]] Result CoSimClient::SendConnectRequest() const {
    CheckResultWithMessage(_protocol->SendConnect(_channel->GetWriter(), ProtocolVersionLatest, {}, _serverName, _clientName), "Could not send connect frame.");
    return CreateOk();
}

[[nodiscard]] Result CoSimClient::OnConnectOk() {
    uint32_t serverProtocolVersion{};
    CheckResultWithMessage(_protocol->ReadConnectOkVersion(_channel->GetReader(), serverProtocolVersion), "Could not read protocol version.");

    if (_protocol->GetVersion() != serverProtocolVersion) {
        CheckResult(CreateProtocol(serverProtocolVersion, _protocol));
    }

    Mode mode{};
    CheckResultWithMessage(_protocol->ReadConnectOk(_channel->GetReader(),
                                                    mode,
                                                    _stepSize,
                                                    _simulationState,
                                                    _incomingSignals,
                                                    _outgoingSignals,
                                                    _canControllers,
                                                    _ethControllers,
                                                    _linControllers,
                                                    _frControllers),
                           "Could not read connect ok frame.");

    _incomingSignalsExtern = Convert(_incomingSignals);
    _outgoingSignalsExtern = Convert(_outgoingSignals);

    _canControllersExtern = Convert(_canControllers);
    _ethControllersExtern = Convert(_ethControllers);
    _linControllersExtern = Convert(_linControllers);
    _frControllersExtern = Convert(_frControllers);

    if (_connectionKind == ConnectionKind::Local) {
        LogInfo("Connected to local dSPACE VEOS CoSim server '{}'.", _serverName);
    } else {
        if (_serverName.empty()) {
            LogInfo("Connected to dSPACE VEOS CoSim server at {}:{}.", _remoteIpAddress, _remotePort);
        } else {
            LogInfo("Connected to dSPACE VEOS CoSim server '{}' at {}:{}.", _serverName, _remoteIpAddress, _remotePort);
        }
    }

    CheckResult(
        CreateSignalExchange(CoSimType::Client, _connectionKind, _serverName, _incomingSignalsExtern, _outgoingSignalsExtern, *_protocol, _signalExchange));

    CheckResult(CreateBusExchange(CoSimType::Client,
                                  _connectionKind,
                                  _serverName,
                                  _canControllersExtern,
                                  _ethControllersExtern,
                                  _linControllersExtern,
                                  _frControllersExtern,
                                  *_protocol,
                                  _busExchange));

    _isConnected = true;
    return CreateOk();
}

[[nodiscard]] Result CoSimClient::OnConnectError() const {
    std::string errorString;
    CheckResultWithMessage(_protocol->ReadError(_channel->GetReader(), errorString), "Could not read error frame.");
    LogError(errorString);
    return CreateError();
}

[[nodiscard]] Result CoSimClient::ReceiveConnectResponse() {
    FrameKind frameKind{};
    CheckResult(_protocol->ReceiveHeader(_channel->GetReader(), frameKind));

    switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
        case FrameKind::ConnectOk:
            CheckResultWithMessage(OnConnectOk(), "Could not handle connect ok.");
            return CreateOk();
        case FrameKind::Error:
            CheckResultWithMessage(OnConnectError(), "Could not handle connect error.");
            return CreateError();
        default:
            return OnUnexpectedFrame(frameKind);
    }
}

[[nodiscard]] Result CoSimClient::RunCallbackBasedCoSimulationInternal() {
    while (_isConnected) {
        FrameKind frameKind{};
        CheckResult(_protocol->ReceiveHeader(_channel->GetReader(), frameKind));

        switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
            case FrameKind::Step:
                CheckResultWithMessage(OnStep(), "Could not handle step.");
                if (!_isConnected) {
                    return CreateNotConnected();
                }

                CheckResult(FinishStep());
                continue;
            case FrameKind::Ping:
                CheckResultWithMessage(OnPing(), "Could not handle ping.");
                CheckResult(FinishPing());
                continue;
            case FrameKind::Start:
                CheckResultWithMessage(OnStart(), "Could not handle start.");
                break;
            case FrameKind::Stop:
                CheckResultWithMessage(OnStop(), "Could not handle stop.");
                break;
            case FrameKind::Terminate:
                CheckResultWithMessage(OnTerminate(), "Could not handle terminate.");
                break;
            case FrameKind::Pause:
                CheckResultWithMessage(OnPause(), "Could not handle pause.");
                break;
            case FrameKind::Continue:
                CheckResultWithMessage(OnContinue(), "Could not handle continue.");
                break;
            default:
                return OnUnexpectedFrame(frameKind);
        }

        if (!_isConnected) {
            return CreateNotConnected();
        }

        CheckResult(FinishCurrentCommand());
    }

    return CreateNotConnected();
}

[[nodiscard]] Result CoSimClient::PollCommandInternal(SimulationTime& simulationTime, Command& command, uint32_t timeoutInMilliseconds) {
    simulationTime = _currentSimulationTime;
    command = Command::Terminate;

    const bool hasTimeout = timeoutInMilliseconds != Infinite;
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutInMilliseconds);

    do {
        uint32_t waitTimeoutInMilliseconds = timeoutInMilliseconds;
        if (hasTimeout) {
            const auto now = std::chrono::steady_clock::now();
            if (now >= deadline) {
                return CreateTimeout();
            }

            waitTimeoutInMilliseconds = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now).count());
        }

        CheckResult(_channel->GetReader().WaitForData(waitTimeoutInMilliseconds));

        FrameKind frameKind{};
        CheckResult(_protocol->ReceiveHeader(_channel->GetReader(), frameKind));
        switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
            case FrameKind::Step:
                CheckResultWithMessage(OnStep(), "Could not handle step.");
                _currentCommand = Command::Step;
                break;
            case FrameKind::Ping:
                CheckResultWithMessage(OnPing(), "Could not handle ping.");
                _currentCommand = Command::Ping;
                CheckResult(FinishPing());
                break;
            case FrameKind::Start:
                CheckResultWithMessage(OnStart(), "Could not handle start.");
                _currentCommand = Command::Start;
                break;
            case FrameKind::Stop:
                CheckResultWithMessage(OnStop(), "Could not handle stop.");
                _currentCommand = Command::Stop;
                break;
            case FrameKind::Terminate:
                CheckResultWithMessage(OnTerminate(), "Could not handle terminate.");
                _currentCommand = Command::Terminate;
                break;
            case FrameKind::Pause:
                CheckResultWithMessage(OnPause(), "Could not handle pause.");
                _currentCommand = Command::Pause;
                break;
            case FrameKind::Continue:
                CheckResultWithMessage(OnContinue(), "Could not handle continue.");
                _currentCommand = Command::Continue;
                break;
            default:
                return OnUnexpectedFrame(frameKind);
        }
    } while (_currentCommand == Command::Ping);

    simulationTime = _currentSimulationTime;
    command = _currentCommand;
    return CreateOk();
}

[[nodiscard]] Result CoSimClient::FinishCommandInternal() {
    switch (_currentCommand) {
        case Command::Step:
            CheckResult(FinishStep());
            break;
        case Command::Start:
        case Command::Stop:
        case Command::Terminate:
        case Command::TerminateFinished:
        case Command::Pause:
        case Command::Continue:
            CheckResult(FinishCurrentCommand());
            break;
        case Command::Ping:
            CheckResult(FinishPing());
            break;
        case Command::None:
            break;
    }

    _currentCommand = Command::None;
    return CreateOk();
}

[[nodiscard]] Result CoSimClient::OnStep() {
    CheckResultWithMessage(_protocol->ReadStep(_channel->GetReader(), _currentSimulationTime, _deserializeIoData, _deserializeBusMessages, _callbacks),
                           "Could not read step frame.");

    if (_callbacks.simulationEndStepCallback) {
        _callbacks.simulationEndStepCallback(_currentSimulationTime);
    }

    return CreateOk();
}

[[nodiscard]] Result CoSimClient::OnStart() {
    CheckResultWithMessage(_protocol->ReadStart(_channel->GetReader(), _currentSimulationTime), "Could not read start frame.");

    _signalExchange->ClearData();
    _busExchange->ClearData();

    if (_callbacks.simulationStartedCallback) {
        _callbacks.simulationStartedCallback(_currentSimulationTime);
    }

    return CreateOk();
}

[[nodiscard]] Result CoSimClient::OnStop() {
    CheckResultWithMessage(_protocol->ReadStop(_channel->GetReader(), _currentSimulationTime), "Could not read stop frame.");

    if (_callbacks.simulationStoppedCallback) {
        _callbacks.simulationStoppedCallback(_currentSimulationTime);
    }

    return CreateOk();
}

[[nodiscard]] Result CoSimClient::OnTerminate() {
    TerminateReason reason{};
    CheckResultWithMessage(_protocol->ReadTerminate(_channel->GetReader(), _currentSimulationTime, reason), "Could not read terminate frame.");

    if (_callbacks.simulationTerminatedCallback) {
        _callbacks.simulationTerminatedCallback(_currentSimulationTime, reason);
    }

    return CreateOk();
}

[[nodiscard]] Result CoSimClient::OnPause() {
    CheckResultWithMessage(_protocol->ReadPause(_channel->GetReader(), _currentSimulationTime), "Could not read pause frame.");

    if (_callbacks.simulationPausedCallback) {
        _callbacks.simulationPausedCallback(_currentSimulationTime);
    }

    return CreateOk();
}

[[nodiscard]] Result CoSimClient::OnContinue() {
    CheckResultWithMessage(_protocol->ReadContinue(_channel->GetReader(), _currentSimulationTime), "Could not read continue frame.");

    if (_callbacks.simulationContinuedCallback) {
        _callbacks.simulationContinuedCallback(_currentSimulationTime);
    }

    return CreateOk();
}

[[nodiscard]] Result CoSimClient::OnPing() {
    CheckResultWithMessage(_protocol->ReadPing(_channel->GetReader(), _roundTripTime), "Could not read ping frame.");
    return CreateOk();
}

[[nodiscard]] Result CoSimClient::FinishStep() {
    Command nextCommand = _nextCommand.exchange({});
    CheckResultWithMessage(_protocol->SendStepOk(_channel->GetWriter(), _nextSimulationTime, nextCommand, _serializeIoData, _serializeBusMessages),
                           "Could not send step ok frame.");
    return CreateOk();
}

[[nodiscard]] Result CoSimClient::FinishPing() {
    Command nextCommand = _nextCommand.exchange({});
    CheckResultWithMessage(_protocol->SendPingOk(_channel->GetWriter(), nextCommand), "Could not send ping ok frame.");
    return CreateOk();
}

[[nodiscard]] Result CoSimClient::FinishCurrentCommand() const {
    CheckResultWithMessage(_protocol->SendOk(_channel->GetWriter()), "Could not send ok frame.");
    return CreateOk();
}

[[nodiscard]] Result CoSimClient::EnsureIsConnected() const {
    if (!_isConnected) {
        LogError("Not connected.");
        return CreateNotConnected();
    }

    return CreateOk();
}

[[nodiscard]] Result CoSimClient::EnsureIsInResponderModeBlocking() {
    switch (_responderMode) {
        case ResponderMode::Blocking:
            // Nothing to do
            return CreateOk();
        case ResponderMode::Unknown:
            _responderMode = ResponderMode::Blocking;
            return CreateOk();
        case ResponderMode::NonBlocking:
            LogError("dSPACE VEOS CoSim is in non-blocking mode. Blocking function call is not allowed.");
            return CreateError();
    }

    LogError("Invalid responder mode.");
    return CreateError();
}

[[nodiscard]] Result CoSimClient::EnsureIsInResponderModeNonBlocking() {
    switch (_responderMode) {
        case ResponderMode::NonBlocking:
            // Nothing to do
            return CreateOk();
        case ResponderMode::Unknown:
            _responderMode = ResponderMode::NonBlocking;
            return CreateOk();
        case ResponderMode::Blocking:
            LogError("dSPACE VEOS CoSim is in blocking mode. Non-blocking function call is not allowed.");
            return CreateError();
    }

    LogError("Invalid responder mode.");
    return CreateError();
}

void CoSimClient::CloseConnection() {
    LogWarning("dSPACE VEOS CoSim server disconnected.");

    _isConnected = false;

    if (_channel) {
        _channel->Disconnect();
    }
}

[[nodiscard]] Result CoSimClient::OnUnexpectedFrame(FrameKind frameKind) {
    LogError("Received unexpected frame '{}'.", frameKind);
    return CreateError();
}

[[nodiscard]] Result CoSimClient::CheckCanMessage(CanMessageFlags flags, uint32_t length) {
    if (length > CanMessageMaxLength) {
        LogError("CAN message data exceeds maximum length.");
        return CreateInvalidArgument();
    }

    if (!HasFlag(flags, CanMessageFlags::FlexibleDataRateFormat)) {
        if (length > 8) {
            LogError("CAN message flags are invalid. A DLC > 8 requires the flexible data rate format flag.");
            return CreateInvalidArgument();
        }

        if (HasFlag(flags, CanMessageFlags::BitRateSwitch)) {
            LogError("CAN message flags are invalid. A bit rate switch flag requires the flexible data rate format flag.");
            return CreateInvalidArgument();
        }
    }

    return CreateOk();
}

}  // namespace DsVeosCoSim
