// Copyright dSPACE GmbH. All rights reserved.

#include "CoSimClient.h"

#include <chrono>
#include <cstring>

#include "Logger.h"
#include "PortMapper.h"
#include "Protocol.h"

using namespace std::chrono;

namespace DsVeosCoSim {

Result CoSimClient::Connect(const ConnectConfig& connectConfig) {
    if (!connectConfig.serverName && (connectConfig.remotePort == 0)) {
        LogError("Either ConnectConfig.serverName or ConnectConfig.remotePort must be set.");
        return Result::InvalidArgument;
    }

    if (_isConnected) {
        return Result::Ok;
    }

    ResetDataFromPreviousConnect();

    if (connectConfig.remoteIpAddress) {
        _remoteIpAddress = std::string(connectConfig.remoteIpAddress);
    } else {
        _remoteIpAddress = "127.0.0.1";
    }

    if (connectConfig.serverName) {
        _serverName = std::string(connectConfig.serverName);
    }

    if (connectConfig.clientName) {
        _clientName = std::string(connectConfig.clientName);
    }

    _remotePort = connectConfig.remotePort;
    if (_remotePort == 0) {
        LogInfo("Obtaining TCP port of dSPACE VEOS CoSim server '" + _serverName + "' at " + _remoteIpAddress + " ...");
        CheckResult(PortMapper_GetPort(_remoteIpAddress, _serverName, _remotePort));
    }

    if (_serverName.empty()) {
        LogInfo("Connecting to dSPACE VEOS CoSim server at " + _remoteIpAddress + ":" + std::to_string(_remotePort) + " ...");
    } else {
        LogInfo("Connecting to dSPACE VEOS CoSim server '" + _serverName + "' at " + _remoteIpAddress + ":" + std::to_string(_remotePort) + " ...");
    }

    CheckResult(ConnectToServer(_remoteIpAddress, _remotePort, connectConfig.localPort, _channel));

    // Co-Sim connect
    CheckResult(SendConnectRequest());
    return ReceiveConnectResponse();
}

void CoSimClient::Disconnect() {
    _isConnected = false;

    _channel.Stop();
}

ConnectionState CoSimClient::GetConnectionState() const {
    if (_isConnected) {
        return ConnectionState::Connected;
    }

    return ConnectionState::Disconnected;
}

void CoSimClient::SetCallbacks(const Callbacks& callbacks) {
    _callbacks = callbacks;

    if (_callbacks.callbacks.canMessageReceivedCallback) {
        _callbacks.canMessageReceivedCallback = [this](SimulationTime simulationTime, const CanController& canController, const CanMessage& message) {
            _callbacks.callbacks.canMessageReceivedCallback(simulationTime,
                                                            reinterpret_cast<const DsVeosCoSim_CanController*>(&canController),
                                                            reinterpret_cast<const DsVeosCoSim_CanMessage*>(&message),
                                                            _callbacks.callbacks.userData);
        };
    }

    if (_callbacks.callbacks.ethMessageReceivedCallback) {
        _callbacks.ethMessageReceivedCallback = [this](SimulationTime simulationTime, const EthController& ethController, const EthMessage& message) {
            _callbacks.callbacks.ethMessageReceivedCallback(simulationTime,
                                                            reinterpret_cast<const DsVeosCoSim_EthController*>(&ethController),
                                                            reinterpret_cast<const DsVeosCoSim_EthMessage*>(&message),
                                                            _callbacks.callbacks.userData);
        };
    }

    if (_callbacks.callbacks.linMessageReceivedCallback) {
        _callbacks.linMessageReceivedCallback = [this](SimulationTime simulationTime, const LinController& linController, const LinMessage& message) {
            _callbacks.callbacks.linMessageReceivedCallback(simulationTime,
                                                            reinterpret_cast<const DsVeosCoSim_LinController*>(&linController),
                                                            reinterpret_cast<const DsVeosCoSim_LinMessage*>(&message),
                                                            _callbacks.callbacks.userData);
        };
    }

    if (_callbacks.callbacks.incomingSignalChangedCallback) {
        _callbacks.incomingSignalChangedCallback = [this](SimulationTime simulationTime, const IoSignal& ioSignal, uint32_t length, const void* value) {
            _callbacks.callbacks.incomingSignalChangedCallback(simulationTime,
                                                               reinterpret_cast<const DsVeosCoSim_IoSignal*>(&ioSignal),
                                                               length,
                                                               value,
                                                               _callbacks.callbacks.userData);
        };
    }

    if (_callbacks.callbacks.simulationStartedCallback) {
        _callbacks.simulationStartedCallback = [this](SimulationTime simulationTime) {
            _callbacks.callbacks.simulationStartedCallback(simulationTime, _callbacks.callbacks.userData);
        };
    }

    if (_callbacks.callbacks.simulationStoppedCallback) {
        _callbacks.simulationStoppedCallback = [this](SimulationTime simulationTime) {
            _callbacks.callbacks.simulationStoppedCallback(simulationTime, _callbacks.callbacks.userData);
        };
    }

    if (_callbacks.callbacks.simulationPausedCallback) {
        _callbacks.simulationPausedCallback = [this](SimulationTime simulationTime) {
            _callbacks.callbacks.simulationPausedCallback(simulationTime, _callbacks.callbacks.userData);
        };
    }

    if (_callbacks.callbacks.simulationContinuedCallback) {
        _callbacks.simulationContinuedCallback = [this](SimulationTime simulationTime) {
            _callbacks.callbacks.simulationContinuedCallback(simulationTime, _callbacks.callbacks.userData);
        };
    }

    if (_callbacks.callbacks.simulationTerminatedCallback) {
        _callbacks.simulationTerminatedCallback = [this](SimulationTime simulationTime, TerminateReason reason) {
            _callbacks.callbacks.simulationTerminatedCallback(simulationTime, static_cast<DsVeosCoSim_TerminateReason>(reason), _callbacks.callbacks.userData);
        };
    }

    if (_callbacks.callbacks.simulationBeginStepCallback) {
        _callbacks.simulationBeginStepCallback = [this](SimulationTime simulationTime) {
            _callbacks.callbacks.simulationBeginStepCallback(simulationTime, _callbacks.callbacks.userData);
        };
    }

    if (_callbacks.callbacks.simulationEndStepCallback) {
        _callbacks.simulationEndStepCallback = [this](SimulationTime simulationTime) {
            _callbacks.callbacks.simulationEndStepCallback(simulationTime, _callbacks.callbacks.userData);
        };
    }
}

Result CoSimClient::RunCallbackBasedCoSimulation(const Callbacks& callbacks) {
    CheckResult(EnsureIsConnected());
    CheckResult(EnsureIsInResponderModeBlocking());

    SetCallbacks(callbacks);

    const Result result = RunCallbackBasedCoSimulationInternal();
    if (result != Result::Ok) {
        CloseConnection();
    }

    return result;
}

Result CoSimClient::StartPollingBasedCoSimulation(const Callbacks& callbacks) {
    CheckResult(EnsureIsConnected());
    CheckResult(EnsureIsInResponderModeNonBlocking());

    SetCallbacks(callbacks);
    return Result::Ok;
}

Result CoSimClient::PollCommand(SimulationTime& simulationTime, Command& command) {
    CheckResult(EnsureIsConnected());
    CheckResult(EnsureIsInResponderModeNonBlocking());

    if (_currentCommand != Command::None) {
        LogError("Call to FinishCommand() for last command is missing.");
        return Result::Error;
    }

    const Result result = PollCommandInternal(simulationTime, command);
    if (result != Result::Ok) {
        CloseConnection();
    }

    return result;
}

Result CoSimClient::FinishCommand() {
    CheckResult(EnsureIsConnected());
    CheckResult(EnsureIsInResponderModeNonBlocking());

    if (_currentCommand == Command::None) {
        LogError("Call to PollCommand(...) is missing.");
        return Result::Error;
    }

    const Result result = FinishCommandInternal();
    if (result != Result::Ok) {
        CloseConnection();
    }

    return result;
}

Result CoSimClient::SetNextSimulationTime(SimulationTime simulationTime) {
    _nextSimulationTime = simulationTime;
    return Result::Ok;
}

Result CoSimClient::GetIncomingSignals(uint32_t* incomingSignalsCount, const DsVeosCoSim_IoSignal** incomingSignals) const {
    CheckResult(EnsureIsConnected());

    *incomingSignalsCount = static_cast<uint32_t>(_incomingSignals2.size());
    *incomingSignals = _incomingSignals2.data();
    return Result::Ok;
}

Result CoSimClient::GetIncomingSignals(std::vector<IoSignal>& incomingSignals) const {
    CheckResult(EnsureIsConnected());

    incomingSignals = _incomingSignals;
    return Result::Ok;
}

Result CoSimClient::GetOutgoingSignals(uint32_t* outgoingSignalsCount, const DsVeosCoSim_IoSignal** outgoingSignals) const {
    CheckResult(EnsureIsConnected());

    *outgoingSignalsCount = static_cast<uint32_t>(_outgoingSignals2.size());
    *outgoingSignals = _outgoingSignals2.data();
    return Result::Ok;
}

Result CoSimClient::GetOutgoingSignals(std::vector<IoSignal>& outgoingSignals) const {
    CheckResult(EnsureIsConnected());

    outgoingSignals = _outgoingSignals;
    return Result::Ok;
}

Result CoSimClient::Read(IoSignalId incomingSignalId, uint32_t& length, void* value) {
    CheckResult(EnsureIsConnected());

    return _ioBuffer.Read(incomingSignalId, length, value);
}

Result CoSimClient::Write(IoSignalId outgoingSignalId, uint32_t length, const void* value) {
    CheckResult(EnsureIsConnected());

    return _ioBuffer.Write(outgoingSignalId, length, value);
}

Result CoSimClient::GetControllers(uint32_t* controllersCount, const DsVeosCoSim_CanController** controllers) const {
    CheckResult(EnsureIsConnected());

    *controllersCount = static_cast<uint32_t>(_canControllers2.size());
    *controllers = _canControllers2.data();
    return Result::Ok;
}

Result CoSimClient::GetControllers(std::vector<CanController>& controllers) const {
    CheckResult(EnsureIsConnected());

    controllers = _canControllers;
    return Result::Ok;
}

Result CoSimClient::Receive(CanMessage& message) {
    CheckResult(EnsureIsConnected());

    return _busBuffer.Receive(message);
}

Result CoSimClient::Transmit(const CanMessage& message) {
    CheckResult(EnsureIsConnected());

    return _busBuffer.Transmit(message);
}

Result CoSimClient::GetControllers(uint32_t* controllersCount, const DsVeosCoSim_EthController** controllers) const {
    CheckResult(EnsureIsConnected());

    *controllersCount = static_cast<uint32_t>(_ethControllers2.size());
    *controllers = _ethControllers2.data();
    return Result::Ok;
}

Result CoSimClient::GetControllers(std::vector<EthController>& controllers) const {
    CheckResult(EnsureIsConnected());

    controllers = _ethControllers;
    return Result::Ok;
}

Result CoSimClient::Receive(EthMessage& message) {
    CheckResult(EnsureIsConnected());

    return _busBuffer.Receive(message);
}

Result CoSimClient::Transmit(const EthMessage& message) {
    CheckResult(EnsureIsConnected());

    return _busBuffer.Transmit(message);
}

Result CoSimClient::GetControllers(uint32_t* controllersCount, const DsVeosCoSim_LinController** controllers) const {
    CheckResult(EnsureIsConnected());

    *controllersCount = static_cast<uint32_t>(_linControllers2.size());
    *controllers = _linControllers2.data();
    return Result::Ok;
}

Result CoSimClient::GetControllers(std::vector<LinController>& controllers) const {
    CheckResult(EnsureIsConnected());

    controllers = _linControllers;
    return Result::Ok;
}

Result CoSimClient::Receive(LinMessage& message) {
    CheckResult(EnsureIsConnected());

    return _busBuffer.Receive(message);
}

Result CoSimClient::Transmit(const LinMessage& message) {
    CheckResult(EnsureIsConnected());

    return _busBuffer.Transmit(message);
}

void CoSimClient::ResetDataFromPreviousConnect() {
    _responderMode = {};
    _currentCommand = {};
    _isConnected = {};
    _currentSimulationTime = {};
    _callbacks = {};
    _channel.Disconnect();
    _incomingSignals.clear();
    _outgoingSignals.clear();
    _incomingSignals2.clear();
    _outgoingSignals2.clear();
    _canControllers.clear();
    _ethControllers.clear();
    _linControllers.clear();
    _canControllers2.clear();
    _ethControllers2.clear();
    _linControllers2.clear();
}

void CoSimClient::CloseConnection() {
    Disconnect();
    _channel.Disconnect();
}

Result CoSimClient::SendConnectRequest() {
    return Protocol::SendConnect(_channel, CoSimProtocolVersion, Mode::None, _serverName, _clientName);
}

Result CoSimClient::ConnectOnAccepted() {
    uint32_t serverProtocolVersion{};
    Mode mode{};
    CheckResult(
        Protocol::ReadAccepted(_channel, serverProtocolVersion, mode, _incomingSignals, _outgoingSignals, _canControllers, _ethControllers, _linControllers));

    _incomingSignals2.reserve(_incomingSignals.size());
    for (const auto& signal : _incomingSignals) {
        _incomingSignals2.push_back({signal.id,
                                     signal.length,
                                     static_cast<DsVeosCoSim_DataType>(signal.dataType),
                                     static_cast<DsVeosCoSim_SizeKind>(signal.sizeKind),
                                     signal.name.c_str()});
    }

    _outgoingSignals2.reserve(_outgoingSignals.size());
    for (const auto& signal : _outgoingSignals) {
        _outgoingSignals2.push_back({signal.id,
                                     signal.length,
                                     static_cast<DsVeosCoSim_DataType>(signal.dataType),
                                     static_cast<DsVeosCoSim_SizeKind>(signal.sizeKind),
                                     signal.name.c_str()});
    }

    _canControllers2.reserve(_canControllers.size());
    for (const auto& controller : _canControllers) {
        _canControllers2.push_back({controller.id,
                                    controller.queueSize,
                                    controller.bitsPerSecond,
                                    controller.flexibleDataRateBitsPerSecond,
                                    controller.name.c_str(),
                                    controller.channelName.c_str(),
                                    controller.clusterName.c_str()});
    }

    _ethControllers2.reserve(_ethControllers.size());
    for (const auto& controller : _ethControllers) {
        DsVeosCoSim_EthController controller2{controller.id,
                                              controller.queueSize,
                                              controller.bitsPerSecond,
                                              {},
                                              {},
                                              controller.name.c_str(),
                                              controller.channelName.c_str(),
                                              controller.clusterName.c_str()};
        memcpy(controller2.macAddress, controller.macAddress.data(), EthAddressLength);
        _ethControllers2.push_back(controller2);
    }

    _linControllers2.reserve(_linControllers.size());
    for (const auto& controller : _linControllers) {
        _linControllers2.push_back({controller.id,
                                    controller.queueSize,
                                    controller.bitsPerSecond,
                                    static_cast<DsVeosCoSim_LinControllerType>(controller.type),
                                    controller.name.c_str(),
                                    controller.channelName.c_str(),
                                    controller.clusterName.c_str()});
    }

    if (_serverName.empty()) {
        LogInfo("Connected to dSPACE VEOS CoSim server at " + _remoteIpAddress + ":" + std::to_string(_remotePort) + ".");
    } else {
        LogInfo("Connected to dSPACE VEOS CoSim server '" + _serverName + "' at " + _remoteIpAddress + ":" + std::to_string(_remotePort) + ".");
    }

    CheckResult(_ioBuffer.Initialize(_incomingSignals, _outgoingSignals));
    CheckResult(_busBuffer.Initialize(_canControllers, _ethControllers, _linControllers));

    _isConnected = true;
    return Result::Ok;
}

Result CoSimClient::ConnectOnDeclined() {
    std::string errorString;
    CheckResult(Protocol::ReadError(_channel, errorString));
    LogError(errorString);
    return Result::Error;
}

Result CoSimClient::ReceiveConnectResponse() {
    FrameKind frameKind = FrameKind::Unknown;
    CheckResult(WaitForNextFrame(frameKind));

    switch (frameKind)  // NOLINT(clang-diagnostic-switch-enum)
    {
        case FrameKind::Accepted:
            return ConnectOnAccepted();
        case FrameKind::Error:
            return ConnectOnDeclined();
        default:
            LogError("Received unexpected frame " + ToString(frameKind) + ".");
            return Result::Error;
    }
}

Result CoSimClient::RunCallbackBasedCoSimulationInternal() {  // NOLINT(readability-function-cognitive-complexity)
    while (_isConnected) {
        FrameKind frameKind = FrameKind::Unknown;
        CheckResult(Protocol::ReceiveHeader(_channel, frameKind));

        switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
            case FrameKind::Step:
                CheckResult(OnStep());
                CheckResult(Protocol::SendStepResponse(_channel, _nextSimulationTime, _ioBuffer, _busBuffer));
                break;
            case FrameKind::Start:
                CheckResult(OnStart());
                CheckResult(Protocol::SendOk(_channel));
                break;
            case FrameKind::Stop:
                CheckResult(OnStop());
                CheckResult(Protocol::SendOk(_channel));
                break;
            case FrameKind::Terminate:
                CheckResult(OnTerminate());
                CheckResult(Protocol::SendOk(_channel));
                break;
            case FrameKind::Pause:
                CheckResult(OnPause());
                CheckResult(Protocol::SendOk(_channel));
                break;
            case FrameKind::Continue:
                CheckResult(OnContinue());
                CheckResult(Protocol::SendOk(_channel));
                break;
            case FrameKind::Ping:
                CheckResult(Protocol::SendOk(_channel));
                break;
            default:
                LogError("Received unexpected frame " + ToString(frameKind) + ".");
                return Result::Error;
        }
    }

    return Result::Disconnected;
}

Result CoSimClient::PollCommandInternal(SimulationTime& simulationTime, Command& command) {  // NOLINT(readability-function-cognitive-complexity)
    simulationTime = _currentSimulationTime;
    command = Command::Terminate;

    FrameKind frameKind = FrameKind::Unknown;
    CheckResult(WaitForNextFrame(frameKind));
    switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
        case FrameKind::Step:
            CheckResult(OnStep());
            _currentCommand = Command::Step;
            break;
        case FrameKind::Start:
            CheckResult(OnStart());
            _currentCommand = Command::Start;
            break;
        case FrameKind::Stop:
            CheckResult(OnStop());
            _currentCommand = Command::Stop;
            break;
        case FrameKind::Terminate:
            CheckResult(OnTerminate());
            _currentCommand = Command::Terminate;
            break;
        case FrameKind::Pause:
            CheckResult(OnPause());
            _currentCommand = Command::Pause;
            break;
        case FrameKind::Continue:
            CheckResult(OnContinue());
            _currentCommand = Command::Continue;
            break;
        case FrameKind::Error: {
            std::string errorMessage;
            CheckResult(Protocol::ReadError(_channel, errorMessage));
            LogError(errorMessage);
            return Result::Error;
        }
        default:
            LogError("Received unexpected frame " + ToString(frameKind) + ".");
            return Result::Error;
    }

    simulationTime = _currentSimulationTime;
    command = _currentCommand;
    return Result::Ok;
}

Result CoSimClient::FinishCommandInternal() {
    switch (_currentCommand) {  // NOLINT(clang-diagnostic-switch, clang-diagnostic-switch-enum)
        case Command::Start:
        case Command::Stop:
        case Command::Terminate:
        case Command::Pause:
        case Command::Continue:
            CheckResult(Protocol::SendOk(_channel));
            break;
        case Command::Step:
            CheckResult(Protocol::SendStepResponse(_channel, _nextSimulationTime, _ioBuffer, _busBuffer));
            break;
        default:
            break;
    }

    _currentCommand = Command::None;

    return Result::Ok;
}

Result CoSimClient::OnStep() {
    CheckResult(Protocol::ReadStep(_channel, _currentSimulationTime, _ioBuffer, _busBuffer, _callbacks));

    if (_callbacks.simulationEndStepCallback) {
        _callbacks.simulationEndStepCallback(_currentSimulationTime);
    }

    return Result::Ok;
}

Result CoSimClient::OnStart() {
    CheckResult(Protocol::ReadStart(_channel, _currentSimulationTime));

    _ioBuffer.ClearData();
    _busBuffer.ClearData();

    if (_callbacks.simulationStartedCallback) {
        _callbacks.simulationStartedCallback(_currentSimulationTime);
    }

    return Result::Ok;
}

Result CoSimClient::OnStop() {
    CheckResult(Protocol::ReadStop(_channel, _currentSimulationTime));

    if (_callbacks.simulationStoppedCallback) {
        _callbacks.simulationStoppedCallback(_currentSimulationTime);
    }

    return Result::Ok;
}

Result CoSimClient::OnTerminate() {
    TerminateReason reason{};
    CheckResult(Protocol::ReadTerminate(_channel, _currentSimulationTime, reason));

    if (_callbacks.simulationTerminatedCallback) {
        _callbacks.simulationTerminatedCallback(_currentSimulationTime, reason);
    }

    return Result::Ok;
}

Result CoSimClient::OnPause() {
    CheckResult(Protocol::ReadPause(_channel, _currentSimulationTime));

    if (_callbacks.simulationPausedCallback) {
        _callbacks.simulationPausedCallback(_currentSimulationTime);
    }

    return Result::Ok;
}

Result CoSimClient::OnContinue() {
    CheckResult(Protocol::ReadContinue(_channel, _currentSimulationTime));

    if (_callbacks.simulationContinuedCallback) {
        _callbacks.simulationContinuedCallback(_currentSimulationTime);
    }

    return Result::Ok;
}

Result CoSimClient::WaitForNextFrame(FrameKind& frameKind) {
    while (true) {
        CheckResult(Protocol::ReceiveHeader(_channel, frameKind));

        if (frameKind != FrameKind::Ping) {
            return Result::Ok;
        }

        CheckResult(Protocol::SendOk(_channel));
    }
}

Result CoSimClient::WaitForOkFrame() {
    FrameKind frameKind = FrameKind::Unknown;
    CheckResult(WaitForNextFrame(frameKind));

    switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
        case FrameKind::Ok:
            return Result::Ok;
        case FrameKind::Error: {
            std::string errorMessage;
            CheckResult(Protocol::ReadError(_channel, errorMessage));
            LogError(errorMessage);
            return Result::Error;
        }
        default:
            LogError("Received unexpected frame " + ToString(frameKind) + ".");
            return Result::Error;
    }
}

Result CoSimClient::EnsureIsConnected() const {
    if (!_isConnected) {
        LogError("Not connected.");
        return Result::Disconnected;
    }

    return Result::Ok;
}

Result CoSimClient::EnsureIsInResponderModeBlocking() {
    switch (_responderMode) {  // NOLINT(clang-diagnostic-switch-enum)
        case ResponderMode::Unknown:
            _responderMode = ResponderMode::Blocking;
            return Result::Ok;
        case ResponderMode::NonBlocking:
            LogError("dSPACE VEOS CoSim is in non-blocking mode. Blocking function call is not allowed.");
            return Result::Error;
        default:
            return Result::Ok;
    }
}

Result CoSimClient::EnsureIsInResponderModeNonBlocking() {
    switch (_responderMode) {  // NOLINT(clang-diagnostic-switch-enum)
        case ResponderMode::Unknown:
            _responderMode = ResponderMode::NonBlocking;
            return Result::Ok;
        case ResponderMode::Blocking:
            LogError("dSPACE VEOS CoSim is in blocking mode. Non-blocking function call is not allowed.");
            return Result::Error;
        default:
            return Result::Ok;
    }
}

}  // namespace DsVeosCoSim
