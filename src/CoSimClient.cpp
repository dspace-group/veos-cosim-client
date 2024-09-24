// Copyright dSPACE GmbH. All rights reserved.

#include "CoSimClient.h"

#include <format>
#include <memory>

#include "CoSimHelper.h"
#include "CoSimTypes.h"
#include "DsVeosCoSim/DsVeosCoSim.h"
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

bool CoSimClient::Connect(const ConnectConfig& connectConfig) {
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

DsVeosCoSim_ConnectionState CoSimClient::GetConnectionState() const {
    if (_isConnected) {
        return DsVeosCoSim_ConnectionState_Connected;
    }

    return DsVeosCoSim_ConnectionState_Disconnected;
}

void CoSimClient::SetCallbacks(const Callbacks& callbacks) {
    _callbacks = callbacks;

    if (_callbacks.callbacks.canMessageReceivedCallback) {
        _callbacks.canMessageReceivedCallback = [this](DsVeosCoSim_SimulationTime simulationTime,
                                                       const DsVeosCoSim_CanController& canController,
                                                       const DsVeosCoSim_CanMessage& message) {
            _callbacks.callbacks.canMessageReceivedCallback(simulationTime,
                                                            &canController,
                                                            &message,
                                                            _callbacks.callbacks.userData);
        };
    }

    if (_callbacks.callbacks.ethMessageReceivedCallback) {
        _callbacks.ethMessageReceivedCallback = [this](DsVeosCoSim_SimulationTime simulationTime,
                                                       const DsVeosCoSim_EthController& ethController,
                                                       const DsVeosCoSim_EthMessage& message) {
            _callbacks.callbacks.ethMessageReceivedCallback(simulationTime,
                                                            &ethController,
                                                            &message,
                                                            _callbacks.callbacks.userData);
        };
    }

    if (_callbacks.callbacks.linMessageReceivedCallback) {
        _callbacks.linMessageReceivedCallback = [this](DsVeosCoSim_SimulationTime simulationTime,
                                                       const DsVeosCoSim_LinController& linController,
                                                       const DsVeosCoSim_LinMessage& message) {
            _callbacks.callbacks.linMessageReceivedCallback(simulationTime,
                                                            &linController,
                                                            &message,
                                                            _callbacks.callbacks.userData);
        };
    }

    if (_callbacks.callbacks.incomingSignalChangedCallback) {
        _callbacks.incomingSignalChangedCallback = [this](DsVeosCoSim_SimulationTime simulationTime,
                                                          const DsVeosCoSim_IoSignal& ioSignal,
                                                          uint32_t length,
                                                          const void* value) {
            _callbacks.callbacks.incomingSignalChangedCallback(simulationTime,
                                                               &ioSignal,
                                                               length,
                                                               value,
                                                               _callbacks.callbacks.userData);
        };
    }

    if (_callbacks.callbacks.simulationStartedCallback) {
        _callbacks.simulationStartedCallback = [this](DsVeosCoSim_SimulationTime simulationTime) {
            _callbacks.callbacks.simulationStartedCallback(simulationTime, _callbacks.callbacks.userData);
        };
    }

    if (_callbacks.callbacks.simulationStoppedCallback) {
        _callbacks.simulationStoppedCallback = [this](DsVeosCoSim_SimulationTime simulationTime) {
            _callbacks.callbacks.simulationStoppedCallback(simulationTime, _callbacks.callbacks.userData);
        };
    }

    if (_callbacks.callbacks.simulationPausedCallback) {
        _callbacks.simulationPausedCallback = [this](DsVeosCoSim_SimulationTime simulationTime) {
            _callbacks.callbacks.simulationPausedCallback(simulationTime, _callbacks.callbacks.userData);
        };
    }

    if (_callbacks.callbacks.simulationContinuedCallback) {
        _callbacks.simulationContinuedCallback = [this](DsVeosCoSim_SimulationTime simulationTime) {
            _callbacks.callbacks.simulationContinuedCallback(simulationTime, _callbacks.callbacks.userData);
        };
    }

    if (_callbacks.callbacks.simulationTerminatedCallback) {
        _callbacks.simulationTerminatedCallback = [this](DsVeosCoSim_SimulationTime simulationTime,
                                                         DsVeosCoSim_TerminateReason reason) {
            _callbacks.callbacks.simulationTerminatedCallback(simulationTime,
                                                              static_cast<DsVeosCoSim_TerminateReason>(reason),
                                                              _callbacks.callbacks.userData);
        };
    }

    if (_callbacks.callbacks.simulationBeginStepCallback) {
        _callbacks.simulationBeginStepCallback = [this](DsVeosCoSim_SimulationTime simulationTime) {
            _callbacks.callbacks.simulationBeginStepCallback(simulationTime, _callbacks.callbacks.userData);
        };
    }

    if (_callbacks.callbacks.simulationEndStepCallback) {
        _callbacks.simulationEndStepCallback = [this](DsVeosCoSim_SimulationTime simulationTime) {
            _callbacks.callbacks.simulationEndStepCallback(simulationTime, _callbacks.callbacks.userData);
        };
    }
}

bool CoSimClient::RunCallbackBasedCoSimulation(const Callbacks& callbacks) {
    EnsureIsConnected();
    EnsureIsInResponderModeBlocking();

    SetCallbacks(callbacks);

    if (!RunCallbackBasedCoSimulationInternal()) [[unlikely]] {
        CloseConnection();
        return false;
    }

    return true;
}

void CoSimClient::StartPollingBasedCoSimulation(const Callbacks& callbacks) {
    EnsureIsConnected();
    EnsureIsInResponderModeNonBlocking();

    SetCallbacks(callbacks);
}

bool CoSimClient::PollCommand(DsVeosCoSim_SimulationTime& simulationTime, Command& command, bool returnOnPing) {
    EnsureIsConnected();
    EnsureIsInResponderModeNonBlocking();

    if (_currentCommand != Command::None) {
        throw CoSimException("Call to FinishCommand() for last command is missing.");
    }

    if (!PollCommandInternal(simulationTime, command, returnOnPing)) [[unlikely]] {
        CloseConnection();
        return false;
    }

    return true;
}

bool CoSimClient::FinishCommand() {
    EnsureIsConnected();
    EnsureIsInResponderModeNonBlocking();

    if (_currentCommand == Command::None) {
        throw CoSimException("Call to PollCommand(...) is missing.");
    }

    if (!FinishCommandInternal()) [[unlikely]] {
        CloseConnection();
        return false;
    }

    return true;
}

void CoSimClient::SetNextSimulationTime(DsVeosCoSim_SimulationTime simulationTime) {
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

void CoSimClient::Terminate(DsVeosCoSim_TerminateReason terminateReason) {
    EnsureIsConnected();

    switch (terminateReason) {
        case DsVeosCoSim_TerminateReason_Finished:
            _nextCommand.exchange(Command::TerminateFinished);
            break;
        case DsVeosCoSim_TerminateReason_Error:
            _nextCommand.exchange(Command::Terminate);
            break;
        default:  // NOLINT(clang-diagnostic-covered-switch-default)
            throw CoSimException(std::format("Unknown terminate reason {}.", ToString(terminateReason)));
    }
}

void CoSimClient::Pause() {
    EnsureIsConnected();

    _nextCommand.exchange(Command::Pause);
}

void CoSimClient::Continue() {
    EnsureIsConnected();

    _nextCommand.exchange(Command::Continue);
}

DsVeosCoSim_SimulationTime CoSimClient::GetStepSize() const {
    EnsureIsConnected();

    return _stepSize;
}

void CoSimClient::GetIncomingSignals(uint32_t* incomingSignalsCount,
                                     const DsVeosCoSim_IoSignal** incomingSignals) const {
    EnsureIsConnected();

    *incomingSignalsCount = static_cast<uint32_t>(_incomingSignalsExtern.size());
    *incomingSignals = _incomingSignalsExtern.data();
}

void CoSimClient::GetOutgoingSignals(uint32_t* outgoingSignalsCount,
                                     const DsVeosCoSim_IoSignal** outgoingSignals) const {
    EnsureIsConnected();

    *outgoingSignalsCount = static_cast<uint32_t>(_outgoingSignalsExtern.size());
    *outgoingSignals = _outgoingSignalsExtern.data();
}

std::vector<IoSignal> CoSimClient::GetIncomingSignals() const {
    EnsureIsConnected();

    return _incomingSignals;
}

std::vector<IoSignal> CoSimClient::GetOutgoingSignals() const {
    EnsureIsConnected();

    return _outgoingSignals;
}

void CoSimClient::Write(DsVeosCoSim_IoSignalId outgoingSignalId, uint32_t length, const void* value) const {
    EnsureIsConnected();

    _ioBuffer->Write(outgoingSignalId, length, value);
}

void CoSimClient::Read(DsVeosCoSim_IoSignalId incomingSignalId, uint32_t& length, void* value) const {
    EnsureIsConnected();

    _ioBuffer->Read(incomingSignalId, length, value);
}

void CoSimClient::Read(DsVeosCoSim_IoSignalId incomingSignalId, uint32_t& length, const void** value) const {
    EnsureIsConnected();

    _ioBuffer->Read(incomingSignalId, length, value);
}

void CoSimClient::GetCanControllers(uint32_t* controllersCount, const DsVeosCoSim_CanController** controllers) const {
    EnsureIsConnected();

    *controllersCount = static_cast<uint32_t>(_canControllersExtern.size());
    *controllers = _canControllersExtern.data();
}

void CoSimClient::GetEthControllers(uint32_t* controllersCount, const DsVeosCoSim_EthController** controllers) const {
    EnsureIsConnected();

    *controllersCount = static_cast<uint32_t>(_ethControllersExtern.size());
    *controllers = _ethControllersExtern.data();
}

void CoSimClient::GetLinControllers(uint32_t* controllersCount, const DsVeosCoSim_LinController** controllers) const {
    EnsureIsConnected();

    *controllersCount = static_cast<uint32_t>(_linControllersExtern.size());
    *controllers = _linControllersExtern.data();
}

std::vector<CanController> CoSimClient::GetCanControllers() const {
    EnsureIsConnected();

    return _canControllers;
}

std::vector<EthController> CoSimClient::GetEthControllers() const {
    EnsureIsConnected();

    return _ethControllers;
}

std::vector<LinController> CoSimClient::GetLinControllers() const {
    EnsureIsConnected();

    return _linControllers;
}

bool CoSimClient::Transmit(const DsVeosCoSim_CanMessage& message) const {
    EnsureIsConnected();

    return _busBuffer->Transmit(message);
}

bool CoSimClient::Transmit(const DsVeosCoSim_EthMessage& message) const {
    EnsureIsConnected();

    return _busBuffer->Transmit(message);
}

bool CoSimClient::Transmit(const DsVeosCoSim_LinMessage& message) const {
    EnsureIsConnected();

    return _busBuffer->Transmit(message);
}

bool CoSimClient::Receive(DsVeosCoSim_CanMessage& message) const {
    EnsureIsConnected();

    return _busBuffer->Receive(message);
}

bool CoSimClient::Receive(DsVeosCoSim_EthMessage& message) const {
    EnsureIsConnected();

    return _busBuffer->Receive(message);
}

bool CoSimClient::Receive(DsVeosCoSim_LinMessage& message) const {
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

bool CoSimClient::LocalConnect() {
#ifdef _WIN32
    std::optional<LocalChannel> channel = TryConnectToLocalChannel(_serverName);
    if (!channel) {
        LogTrace("Could not connect to local dSPACE VEOS CoSim server '{}'.", _serverName);
        return false;
    }

    _channel = std::make_unique<LocalChannel>(std::move(*channel));
#else
    std::optional<SocketChannel> channel = TryConnectToUdsChannel(_serverName);
    if (!channel) {
        LogTrace("Could not connect to local dSPACE VEOS CoSim server '{}'.", _serverName);
        return false;
    }

    _channel = std::make_unique<SocketChannel>(std::move(*channel));
#endif

    _connectionKind = ConnectionKind::Local;
    return true;
}

bool CoSimClient::RemoteConnect() {
    if (_remotePort == 0) {
        LogInfo("Obtaining TCP port of dSPACE VEOS CoSim server '{}' at {} ...", _serverName, _remoteIpAddress);
        CheckResultWithMessage(PortMapper_GetPort(_remoteIpAddress, _serverName, _remotePort),
                               "Could not get port from port mapper.");
    }

    if (_serverName.empty()) {
        LogInfo("Connecting to dSPACE VEOS CoSim server at {}:{} ...", _remoteIpAddress, _remotePort);
    } else {
        LogInfo("Connecting to dSPACE VEOS CoSim server '{}' at {}:{} ...", _serverName, _remoteIpAddress, _remotePort);
    }

    std::optional<SocketChannel> channel =
        TryConnectToTcpChannel(_remoteIpAddress, _remotePort, _localPort, ClientTimeoutInMilliseconds);
    CheckResultWithMessage(channel, "Could not connect to dSPACE VEOS CoSim server.");

    _channel = std::make_unique<SocketChannel>(std::move(*channel));
    _connectionKind = ConnectionKind::Remote;

    return true;
}

bool CoSimClient::SendConnectRequest() const {
    CheckResultWithMessage(
        Protocol::SendConnect(_channel->GetWriter(), CoSimProtocolVersion, {}, _serverName, _clientName),
        "Could not send connect frame.");
    return true;
}

bool CoSimClient::OnConnectOk() {
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
        LogInfo("Connected to local dSPACE VEOS CoSim server '{}'.", _serverName);
    } else {
        if (_serverName.empty()) {
            LogInfo("Connected to dSPACE VEOS CoSim server at {}:{}.", _remoteIpAddress, _remotePort);
        } else {
            LogInfo("Connected to dSPACE VEOS CoSim server '{}' at {}:{}.", _serverName, _remoteIpAddress, _remotePort);
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

bool CoSimClient::OnConnectError() const {
    std::string errorString;
    CheckResultWithMessage(Protocol::ReadError(_channel->GetReader(), errorString), "Could not read error frame.");
    throw CoSimException(errorString);
}

bool CoSimClient::ReceiveConnectResponse() {
    FrameKind frameKind{};
    CheckResult(Protocol::ReceiveHeader(_channel->GetReader(), frameKind));

    switch (frameKind)  // NOLINT(clang-diagnostic-switch-enum)
    {
        case FrameKind::ConnectOk:
            CheckResultWithMessage(OnConnectOk(), "Could not handle connect ok.");
            return true;
        case FrameKind::Error:
            CheckResultWithMessage(OnConnectError(), "Could not handle connect error.");
            return true;
        default:
            throw CoSimException(std::format("Received unexpected frame {}.", ToString(frameKind)));
    }
}

bool CoSimClient::RunCallbackBasedCoSimulationInternal() {
    while (_isConnected) {
        FrameKind frameKind{};
        CheckResult(Protocol::ReceiveHeader(_channel->GetReader(), frameKind));

        switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
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
                throw CoSimException(std::format("Received unexpected frame {}.", ToString(frameKind)));
        }
    }

    return true;
}

bool CoSimClient::PollCommandInternal(DsVeosCoSim_SimulationTime& simulationTime, Command& command, bool returnOnPing) {
    simulationTime = _currentSimulationTime;
    command = Command::Terminate;

    while (true) {
        FrameKind frameKind{};
        CheckResult(Protocol::ReceiveHeader(_channel->GetReader(), frameKind));
        switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
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
                throw CoSimException(std::format("Received unexpected frame {}.", ToString(frameKind)));
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

bool CoSimClient::FinishCommandInternal() {
    switch (_currentCommand) {  // NOLINT(clang-diagnostic-switch, clang-diagnostic-switch-enum)
        case Command::Start:
        case Command::Stop:
        case Command::Terminate:
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
        default:
            break;
    }

    _currentCommand = Command::None;

    return true;
}

bool CoSimClient::OnStep() {
    CheckResultWithMessage(
        Protocol::ReadStep(_channel->GetReader(), _currentSimulationTime, *_ioBuffer, *_busBuffer, _callbacks),
        "Could not read step frame.");

    if (_callbacks.simulationEndStepCallback) {
        _callbacks.simulationEndStepCallback(_currentSimulationTime);
    }

    return true;
}

bool CoSimClient::OnStart() {
    CheckResultWithMessage(Protocol::ReadStart(_channel->GetReader(), _currentSimulationTime),
                           "Could not read start frame.");

    _ioBuffer->ClearData();
    _busBuffer->ClearData();

    if (_callbacks.simulationStartedCallback) {
        _callbacks.simulationStartedCallback(_currentSimulationTime);
    }

    return true;
}

bool CoSimClient::OnStop() {
    CheckResultWithMessage(Protocol::ReadStop(_channel->GetReader(), _currentSimulationTime),
                           "Could not read stop frame.");

    if (_callbacks.simulationStoppedCallback) {
        _callbacks.simulationStoppedCallback(_currentSimulationTime);
    }

    return true;
}

bool CoSimClient::OnTerminate() {
    DsVeosCoSim_TerminateReason reason{};
    CheckResultWithMessage(Protocol::ReadTerminate(_channel->GetReader(), _currentSimulationTime, reason),
                           "Could not read terminate frame.");

    if (_callbacks.simulationTerminatedCallback) {
        _callbacks.simulationTerminatedCallback(_currentSimulationTime, reason);
    }

    return true;
}

bool CoSimClient::OnPause() {
    CheckResultWithMessage(Protocol::ReadPause(_channel->GetReader(), _currentSimulationTime),
                           "Could not read pause frame.");

    if (_callbacks.simulationPausedCallback) {
        _callbacks.simulationPausedCallback(_currentSimulationTime);
    }

    return true;
}

bool CoSimClient::OnContinue() {
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
    switch (_responderMode) {  // NOLINT(clang-diagnostic-switch-enum)
        case ResponderMode::Unknown:
            _responderMode = ResponderMode::Blocking;
            break;
        case ResponderMode::NonBlocking:
            throw CoSimException("dSPACE VEOS CoSim is in non-blocking mode. Blocking function call is not allowed.");
        default:
            // Nothing to do
            break;
    }
}

void CoSimClient::EnsureIsInResponderModeNonBlocking() {
    switch (_responderMode) {  // NOLINT(clang-diagnostic-switch-enum)
        case ResponderMode::Unknown:
            _responderMode = ResponderMode::NonBlocking;
            break;
        case ResponderMode::Blocking:
            throw CoSimException("dSPACE VEOS CoSim is in blocking mode. Non-blocking function call is not allowed.");
        default:
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
