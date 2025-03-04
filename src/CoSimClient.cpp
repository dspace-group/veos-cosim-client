// Copyright dSPACE GmbH. All rights reserved.

#include "CoSimClient.h"

#include <memory>
#include <string>

#include "CoSimHelper.h"
#include "CoSimTypes.h"
#include "PortMapper.h"
#include "Protocol.h"
#include "SocketChannel.h"

#ifdef _WIN32
#include "LocalChannel.h"
#endif

namespace DsVeosCoSim {

namespace {

constexpr uint32_t ClientTimeoutInMilliseconds = 1000;

}  // namespace

[[nodiscard]] bool CoSimClient::Connect(const ConnectConfig& connectConfig) {
    if (connectConfig.serverName.empty() && (connectConfig.remotePort == 0)) {
        throw CoSimException("Either ConnectConfig.serverName or ConnectConfig.remotePort must be set.");
    }

    if (_isConnected) {
        return true;
    }

    ResetDataFromPreviousConnect();

    _remoteIpAddress = connectConfig.remoteIpAddress;
    _serverName = connectConfig.serverName;
    _clientName = connectConfig.clientName;
    _remotePort = connectConfig.remotePort;

    if (!connectConfig.serverName.empty() && _remoteIpAddress.empty() && (connectConfig.remotePort == 0)) {
        if (!LocalConnect()) {
            _remoteIpAddress = "127.0.0.1";
            CheckResult(RemoteConnect());
        }
    } else {
        CheckResult(RemoteConnect());
    }

    // Co-Sim connect
    CheckResult(SendConnectRequest());
    CheckResultWithMessage(ReceiveConnectResponse(), "Could not receive connect response.");
    return true;
}

void CoSimClient::Disconnect() {
    _isConnected = false;

    if (_channel) {
        _channel->Disconnect();
    }
}

[[nodiscard]] ConnectionState CoSimClient::GetConnectionState() const {
    if (_isConnected) {
        return ConnectionState::Connected;
    }

    return ConnectionState::Disconnected;
}

[[nodiscard]] bool CoSimClient::RunCallbackBasedCoSimulation(const Callbacks& callbacks) {
    EnsureIsConnected();
    EnsureIsInResponderModeBlocking();

    _callbacks = callbacks;

    if (!RunCallbackBasedCoSimulationInternal()) {
        CloseConnection();
        return false;
    }

    return true;
}

void CoSimClient::StartPollingBasedCoSimulation(const Callbacks& callbacks) {
    EnsureIsConnected();
    EnsureIsInResponderModeNonBlocking();

    _callbacks = callbacks;
}

[[nodiscard]] bool CoSimClient::PollCommand(SimulationTime& simulationTime, Command& command, const bool returnOnPing) {
    EnsureIsConnected();
    EnsureIsInResponderModeNonBlocking();

    if (_currentCommand != Command::None) {
        throw CoSimException("Call to FinishCommand() for last command is missing.");
    }

    if (!PollCommandInternal(simulationTime, command, returnOnPing)) {
        CloseConnection();
        return false;
    }

    return true;
}

[[nodiscard]] bool CoSimClient::FinishCommand() {
    EnsureIsConnected();
    EnsureIsInResponderModeNonBlocking();

    if (_currentCommand == Command::None) {
        throw CoSimException("Call to PollCommand(...) is missing.");
    }

    if (!FinishCommandInternal()) {
        CloseConnection();
        return false;
    }

    return true;
}

void CoSimClient::SetNextSimulationTime(const SimulationTime simulationTime) {
    EnsureIsConnected();

    _nextSimulationTime = simulationTime;
}

void CoSimClient::Start() {
    EnsureIsConnected();

    _nextCommand.exchange(Command::Start);
}

void CoSimClient::Stop() {
    EnsureIsConnected();

    _nextCommand.exchange(Command::Stop);
}

void CoSimClient::Terminate(const TerminateReason terminateReason) {
    EnsureIsConnected();

    switch (terminateReason) {
        case TerminateReason::Finished:
            _nextCommand.exchange(Command::TerminateFinished);
            return;
        case TerminateReason::Error:
            _nextCommand.exchange(Command::Terminate);
            return;
    }

    throw CoSimException("Unknown terminate reason " + ToString(terminateReason) + ".");
}

void CoSimClient::Pause() {
    EnsureIsConnected();

    _nextCommand.exchange(Command::Pause);
}

void CoSimClient::Continue() {
    EnsureIsConnected();

    _nextCommand.exchange(Command::Continue);
}

[[nodiscard]] SimulationTime CoSimClient::GetStepSize() const {
    EnsureIsConnected();

    return _stepSize;
}

void CoSimClient::GetIncomingSignals(uint32_t* incomingSignalsCount, const IoSignal** incomingSignals) const {
    EnsureIsConnected();

    *incomingSignalsCount = static_cast<uint32_t>(_incomingSignalsExtern.size());
    *incomingSignals = _incomingSignalsExtern.data();
}

void CoSimClient::GetOutgoingSignals(uint32_t* outgoingSignalsCount, const IoSignal** outgoingSignals) const {
    EnsureIsConnected();

    *outgoingSignalsCount = static_cast<uint32_t>(_outgoingSignalsExtern.size());
    *outgoingSignals = _outgoingSignalsExtern.data();
}

[[nodiscard]] std::vector<IoSignal> CoSimClient::GetIncomingSignals() const {
    EnsureIsConnected();

    return _incomingSignalsExtern;
}

[[nodiscard]] std::vector<IoSignal> CoSimClient::GetOutgoingSignals() const {
    EnsureIsConnected();

    return _outgoingSignalsExtern;
}

void CoSimClient::Write(const IoSignalId outgoingSignalId, const uint32_t length, const void* value) const {
    EnsureIsConnected();

    _ioBuffer->Write(outgoingSignalId, length, value);
}

void CoSimClient::Read(const IoSignalId incomingSignalId, uint32_t& length, void* value) const {
    EnsureIsConnected();

    _ioBuffer->Read(incomingSignalId, length, value);
}

void CoSimClient::Read(const IoSignalId incomingSignalId, uint32_t& length, const void** value) const {
    EnsureIsConnected();

    _ioBuffer->Read(incomingSignalId, length, value);
}

void CoSimClient::GetCanControllers(uint32_t* controllersCount, const CanController** controllers) const {
    EnsureIsConnected();

    *controllersCount = static_cast<uint32_t>(_canControllersExtern.size());
    *controllers = _canControllersExtern.data();
}

void CoSimClient::GetEthControllers(uint32_t* controllersCount, const EthController** controllers) const {
    EnsureIsConnected();

    *controllersCount = static_cast<uint32_t>(_ethControllersExtern.size());
    *controllers = _ethControllersExtern.data();
}

void CoSimClient::GetLinControllers(uint32_t* controllersCount, const LinController** controllers) const {
    EnsureIsConnected();

    *controllersCount = static_cast<uint32_t>(_linControllersExtern.size());
    *controllers = _linControllersExtern.data();
}

[[nodiscard]] std::vector<CanController> CoSimClient::GetCanControllers() const {
    EnsureIsConnected();

    return _canControllersExtern;
}

[[nodiscard]] std::vector<EthController> CoSimClient::GetEthControllers() const {
    EnsureIsConnected();

    return _ethControllersExtern;
}

[[nodiscard]] std::vector<LinController> CoSimClient::GetLinControllers() const {
    EnsureIsConnected();

    return _linControllersExtern;
}

[[nodiscard]] bool CoSimClient::Transmit(const CanMessage& message) const {
    EnsureIsConnected();

    return _busBuffer->Transmit(message);
}

[[nodiscard]] bool CoSimClient::Transmit(const EthMessage& message) const {
    EnsureIsConnected();

    return _busBuffer->Transmit(message);
}

[[nodiscard]] bool CoSimClient::Transmit(const LinMessage& message) const {
    EnsureIsConnected();

    return _busBuffer->Transmit(message);
}

[[nodiscard]] bool CoSimClient::Receive(CanMessage& message) const {
    EnsureIsConnected();

    return _busBuffer->Receive(message);
}

[[nodiscard]] bool CoSimClient::Receive(EthMessage& message) const {
    EnsureIsConnected();

    return _busBuffer->Receive(message);
}

[[nodiscard]] bool CoSimClient::Receive(LinMessage& message) const {
    EnsureIsConnected();

    return _busBuffer->Receive(message);
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
    _canControllersExtern.clear();
    _ethControllersExtern.clear();
    _linControllersExtern.clear();
}

[[nodiscard]] bool CoSimClient::LocalConnect() {
#ifdef _WIN32
    std::optional<LocalChannel> channel = TryConnectToLocalChannel(_serverName);
    if (!channel) {
        LogTrace("Could not connect to local dSPACE VEOS CoSim server '" + _serverName + "'.");
        return false;
    }

    _channel = std::make_unique<LocalChannel>(std::move(*channel));
#else
    std::optional<SocketChannel> channel = TryConnectToUdsChannel(_serverName);
    if (!channel) {
        LogTrace("Could not connect to local dSPACE VEOS CoSim server '" + _serverName + "'.");
        return false;
    }

    _channel = std::make_unique<SocketChannel>(std::move(*channel));
#endif

    _connectionKind = ConnectionKind::Local;
    return true;
}

[[nodiscard]] bool CoSimClient::RemoteConnect() {
    if (_remotePort == 0) {
        LogInfo("Obtaining TCP port of dSPACE VEOS CoSim server '" + _serverName + "' at " + _remoteIpAddress + " ...");
        CheckResultWithMessage(PortMapper_GetPort(_remoteIpAddress, _serverName, _remotePort),
                               "Could not get port from port mapper.");
    }

    if (_serverName.empty()) {
        LogInfo("Connecting to dSPACE VEOS CoSim server at " + _remoteIpAddress + ":" + std::to_string(_remotePort) +
                "...");
    } else {
        LogInfo("Connecting to dSPACE VEOS CoSim server '" + _serverName + "' at " + _remoteIpAddress + ":" +
                std::to_string(_remotePort) + "...");
    }

    std::optional<SocketChannel> channel =
        TryConnectToTcpChannel(_remoteIpAddress, _remotePort, _localPort, ClientTimeoutInMilliseconds);
    CheckResultWithMessage(channel, "Could not connect to dSPACE VEOS CoSim server.");

    _channel = std::make_unique<SocketChannel>(std::move(*channel));
    _connectionKind = ConnectionKind::Remote;

    return true;
}

[[nodiscard]] bool CoSimClient::SendConnectRequest() const {
    CheckResultWithMessage(
        Protocol::SendConnect(_channel->GetWriter(), CoSimProtocolVersion, {}, _serverName, _clientName),
        "Could not send connect frame.");
    return true;
}

[[nodiscard]] bool CoSimClient::OnConnectOk() {
    uint32_t serverProtocolVersion{};
    Mode mode{};
    SimulationState simulationState{};
    CheckResultWithMessage(Protocol::ReadConnectOk(_channel->GetReader(),
                                                   serverProtocolVersion,
                                                   mode,
                                                   _stepSize,
                                                   simulationState,
                                                   _incomingSignals,
                                                   _outgoingSignals,
                                                   _canControllers,
                                                   _ethControllers,
                                                   _linControllers),
                           "Could not read connect ok frame.");

    _incomingSignalsExtern = Convert(_incomingSignals);
    _outgoingSignalsExtern = Convert(_outgoingSignals);

    _canControllersExtern = Convert(_canControllers);
    _ethControllersExtern = Convert(_ethControllers);
    _linControllersExtern = Convert(_linControllers);

    if (_connectionKind == ConnectionKind::Local) {
        LogInfo("Connected to local dSPACE VEOS CoSim server '" + _serverName + "'.");
    } else {
        if (_serverName.empty()) {
            LogInfo("Connected to dSPACE VEOS CoSim server at " + _remoteIpAddress + ":" + std::to_string(_remotePort) +
                    ".");
        } else {
            LogInfo("Connected to dSPACE VEOS CoSim server '" + _serverName + "' at " + _remoteIpAddress + ":" +
                    std::to_string(_remotePort) + ".");
        }
    }

    _ioBuffer = std::make_unique<IoBuffer>(CoSimType::Client,
                                           _connectionKind,
                                           _serverName,
                                           _incomingSignalsExtern,
                                           _outgoingSignalsExtern);

    _busBuffer = std::make_unique<BusBuffer>(CoSimType::Client,
                                             _connectionKind,
                                             _serverName,
                                             _canControllersExtern,
                                             _ethControllersExtern,
                                             _linControllersExtern);

    _isConnected = true;

    return true;
}

[[nodiscard]] bool CoSimClient::OnConnectError() const {
    std::string errorString;
    CheckResultWithMessage(Protocol::ReadError(_channel->GetReader(), errorString), "Could not read error frame.");
    return false;
}

[[nodiscard]] bool CoSimClient::ReceiveConnectResponse() {
    FrameKind frameKind{};
    CheckResult(Protocol::ReceiveHeader(_channel->GetReader(), frameKind));

    switch (frameKind) {
        case FrameKind::ConnectOk:
            CheckResultWithMessage(OnConnectOk(), "Could not handle connect ok.");
            return true;
        case FrameKind::Error:
            CheckResultWithMessage(OnConnectError(), "Could not handle connect error.");
            return false;
        default:
            throw CoSimException("Received unexpected frame " + ToString(frameKind) + ".");
    }
}

[[nodiscard]] bool CoSimClient::RunCallbackBasedCoSimulationInternal() {
    while (_isConnected) {
        FrameKind frameKind{};
        CheckResult(Protocol::ReceiveHeader(_channel->GetReader(), frameKind));

        switch (frameKind) {
            case FrameKind::Step: {
                CheckResultWithMessage(OnStep(), "Could not handle step.");
                if (!_isConnected) {
                    return true;
                }

                const Command nextCommand = _nextCommand.exchange({});
                CheckResultWithMessage(Protocol::SendStepOk(_channel->GetWriter(),
                                                            _nextSimulationTime,
                                                            nextCommand,
                                                            *_ioBuffer,
                                                            *_busBuffer),
                                       "Could not send step ok frame.");
                break;
            }
            case FrameKind::Start:
                CheckResultWithMessage(OnStart(), "Could not handle start.");
                if (!_isConnected) {
                    return true;
                }

                CheckResultWithMessage(Protocol::SendOk(_channel->GetWriter()), "Could not send ok frame.");
                break;
            case FrameKind::Stop:
                CheckResultWithMessage(OnStop(), "Could not handle stop.");
                if (!_isConnected) {
                    return true;
                }

                CheckResultWithMessage(Protocol::SendOk(_channel->GetWriter()), "Could not send ok frame.");
                break;
            case FrameKind::Terminate:
                CheckResultWithMessage(OnTerminate(), "Could not handle terminate.");
                if (!_isConnected) {
                    return true;
                }

                CheckResultWithMessage(Protocol::SendOk(_channel->GetWriter()), "Could not send ok frame.");
                break;
            case FrameKind::Pause:
                CheckResultWithMessage(OnPause(), "Could not handle pause.");
                if (!_isConnected) {
                    return true;
                }

                CheckResultWithMessage(Protocol::SendOk(_channel->GetWriter()), "Could not send ok frame.");
                break;
            case FrameKind::Continue:
                CheckResultWithMessage(OnContinue(), "Could not handle continue.");
                if (!_isConnected) {
                    return true;
                }

                CheckResultWithMessage(Protocol::SendOk(_channel->GetWriter()), "Could not send ok frame.");
                break;
            case FrameKind::Ping: {
                const Command nextCommand = _nextCommand.exchange({});
                CheckResultWithMessage(Protocol::SendPingOk(_channel->GetWriter(), nextCommand),
                                       "Could not send ping ok frame.");
                break;
            }
            default:
                throw CoSimException("Received unexpected frame " + ToString(frameKind) + ".");
        }
    }

    return true;
}

[[nodiscard]] bool CoSimClient::PollCommandInternal(SimulationTime& simulationTime,
                                                    Command& command,
                                                    const bool returnOnPing) {
    simulationTime = _currentSimulationTime;
    command = Command::Terminate;

    while (true) {
        FrameKind frameKind{};
        CheckResult(Protocol::ReceiveHeader(_channel->GetReader(), frameKind));
        switch (frameKind) {
            case FrameKind::Step:
                CheckResultWithMessage(OnStep(), "Could not handle step.");
                _currentCommand = Command::Step;
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
            case FrameKind::Ping:
                _currentCommand = Command::Ping;
                break;
            default:
                throw CoSimException("Received unexpected frame " + ToString(frameKind) + ".");
        }

        if (returnOnPing || (_currentCommand != Command::Ping)) {
            break;
        }

        const Command nextCommand = _nextCommand.exchange({});
        CheckResultWithMessage(Protocol::SendPingOk(_channel->GetWriter(), nextCommand),
                               "Could not send ping ok frame.");
    }

    simulationTime = _currentSimulationTime;
    command = _currentCommand;

    return true;
}

[[nodiscard]] bool CoSimClient::FinishCommandInternal() {
    switch (_currentCommand) {
        case Command::Start:
        case Command::Stop:
        case Command::Terminate:
        case Command::TerminateFinished:
        case Command::Pause:
        case Command::Continue:
            CheckResultWithMessage(Protocol::SendOk(_channel->GetWriter()), "Could not send ok frame.");
            break;
        case Command::Step: {
            const Command nextCommand = _nextCommand.exchange({});
            CheckResultWithMessage(
                Protocol::SendStepOk(_channel->GetWriter(), _nextSimulationTime, nextCommand, *_ioBuffer, *_busBuffer),
                "Could not send step ok frame.");
            break;
        }
        case Command::Ping: {
            const Command nextCommand = _nextCommand.exchange({});
            CheckResultWithMessage(Protocol::SendPingOk(_channel->GetWriter(), nextCommand),
                                   "Could not send ping ok frame.");
            break;
        }
        case Command::None:
            break;
    }

    _currentCommand = Command::None;

    return true;
}

[[nodiscard]] bool CoSimClient::OnStep() {
    CheckResultWithMessage(
        Protocol::ReadStep(_channel->GetReader(), _currentSimulationTime, *_ioBuffer, *_busBuffer, _callbacks),
        "Could not read step frame.");

    if (_callbacks.simulationEndStepCallback) {
        _callbacks.simulationEndStepCallback(_currentSimulationTime);
    }

    return true;
}

[[nodiscard]] bool CoSimClient::OnStart() {
    CheckResultWithMessage(Protocol::ReadStart(_channel->GetReader(), _currentSimulationTime),
                           "Could not read start frame.");

    _ioBuffer->ClearData();
    _busBuffer->ClearData();

    if (_callbacks.simulationStartedCallback) {
        _callbacks.simulationStartedCallback(_currentSimulationTime);
    }

    return true;
}

[[nodiscard]] bool CoSimClient::OnStop() {
    CheckResultWithMessage(Protocol::ReadStop(_channel->GetReader(), _currentSimulationTime),
                           "Could not read stop frame.");

    if (_callbacks.simulationStoppedCallback) {
        _callbacks.simulationStoppedCallback(_currentSimulationTime);
    }

    return true;
}

[[nodiscard]] bool CoSimClient::OnTerminate() {
    TerminateReason reason{};
    CheckResultWithMessage(Protocol::ReadTerminate(_channel->GetReader(), _currentSimulationTime, reason),
                           "Could not read terminate frame.");

    if (_callbacks.simulationTerminatedCallback) {
        _callbacks.simulationTerminatedCallback(_currentSimulationTime, reason);
    }

    return true;
}

[[nodiscard]] bool CoSimClient::OnPause() {
    CheckResultWithMessage(Protocol::ReadPause(_channel->GetReader(), _currentSimulationTime),
                           "Could not read pause frame.");

    if (_callbacks.simulationPausedCallback) {
        _callbacks.simulationPausedCallback(_currentSimulationTime);
    }

    return true;
}

[[nodiscard]] bool CoSimClient::OnContinue() {
    CheckResultWithMessage(Protocol::ReadContinue(_channel->GetReader(), _currentSimulationTime),
                           "Could not read continue frame.");

    if (_callbacks.simulationContinuedCallback) {
        _callbacks.simulationContinuedCallback(_currentSimulationTime);
    }

    return true;
}

void CoSimClient::EnsureIsConnected() const {
    if (!_isConnected) {
        throw CoSimException("Not connected.");
    }
}

void CoSimClient::EnsureIsInResponderModeBlocking() {
    switch (_responderMode) {
        case ResponderMode::Unknown:
            _responderMode = ResponderMode::Blocking;
            break;
        case ResponderMode::NonBlocking:
            throw CoSimException("dSPACE VEOS CoSim is in non-blocking mode. Blocking function call is not allowed.");
        case ResponderMode::Blocking:
            // Nothing to do
            break;
    }
}

void CoSimClient::EnsureIsInResponderModeNonBlocking() {
    switch (_responderMode) {
        case ResponderMode::Unknown:
            _responderMode = ResponderMode::NonBlocking;
            break;
        case ResponderMode::Blocking:
            throw CoSimException("dSPACE VEOS CoSim is in blocking mode. Non-blocking function call is not allowed.");
        case ResponderMode::NonBlocking:
            // Nothing to do
            break;
    }
}

void CoSimClient::CloseConnection() {
    LogWarning("dSPACE VEOS CoSim server disconnected.");

    _isConnected = false;

    if (_channel) {
        _channel->Disconnect();
    }
}

}  // namespace DsVeosCoSim
