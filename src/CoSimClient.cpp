// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "DsVeosCoSim/CoSimClient.h"

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "BusBuffer.h"
#include "Channel.h"
#include "DsVeosCoSim/CoSimTypes.h"
#include "IoBuffer.h"
#include "OsUtilities.h"
#include "PortMapper.h"
#include "Protocol.h"

namespace DsVeosCoSim {

namespace {

constexpr uint32_t ClientTimeoutInMilliseconds = 1000;

enum class ResponderMode {
    Unknown,
    Blocking,
    NonBlocking
};

class CoSimClientImpl final : public CoSimClient {
public:
    CoSimClientImpl() = default;
    ~CoSimClientImpl() override = default;

    CoSimClientImpl(const CoSimClientImpl&) = delete;
    CoSimClientImpl& operator=(const CoSimClientImpl&) = delete;

    CoSimClientImpl(CoSimClientImpl&&) = delete;
    CoSimClientImpl& operator=(CoSimClientImpl&&) = delete;

    [[nodiscard]] Result Connect(const ConnectConfig& connectConfig) override {
        if (connectConfig.serverName.empty() && (connectConfig.remotePort == 0)) {
            Logger::Instance().LogError("Either ConnectConfig.serverName or ConnectConfig.remotePort must be set.");
            return Result::InvalidArgument;
        }

        if (_isConnected) {
            return Result::Ok;
        }

        ResetDataFromPreviousConnect();

        _remoteIpAddress = connectConfig.remoteIpAddress;
        _serverName = connectConfig.serverName;
        _clientName = connectConfig.clientName;
        _remotePort = connectConfig.remotePort;

        CheckResult(MakeProtocol(V1_VERSION, _protocol));

        if (!connectConfig.serverName.empty() && _remoteIpAddress.empty() && (connectConfig.remotePort == 0)) {
            if (!IsOk(LocalConnect())) {
                _remoteIpAddress = "127.0.0.1";
                CheckResult(RemoteConnect());
            }
        } else {
            CheckResult(RemoteConnect());
        }

        // Co-Sim connect
        CheckResult(SendConnectRequest());
        CheckResultWithMessage(ReceiveConnectResponse(), "Could not receive connect response.");
        return Result::Ok;
    }

    void Disconnect() override {
        _isConnected = false;

        if (_channel) {
            _channel->Disconnect();
        }
    }

    [[nodiscard]] Result GetConnectionState(ConnectionState& connectionState) const override {
        if (_isConnected) {
            connectionState = ConnectionState::Connected;
        } else {
            connectionState = ConnectionState::Disconnected;
        }

        return Result::Ok;
    }

    [[nodiscard]] Result GetStepSize(SimulationTime& stepSize) const override {
        CheckResult(EnsureIsConnected());

        stepSize = _stepSize;
        return Result::Ok;
    }

    [[nodiscard]] Result GetCurrentSimulationTime(SimulationTime& simulationTime) const override {
        CheckResult(EnsureIsConnected());

        simulationTime = _currentSimulationTime;
        return Result::Ok;
    }

    [[nodiscard]] Result GetSimulationState(SimulationState& simulationState) const override {
        CheckResult(EnsureIsConnected());

        simulationState = _simulationState;
        return Result::Ok;
    }

    [[nodiscard]] Result RunCallbackBasedCoSimulation(const Callbacks& callbacks) override {
        CheckResult(EnsureIsConnected());
        CheckResult(EnsureIsInResponderModeBlocking());

        _callbacks = callbacks;

        SetThreadAffinity(_serverName);

        if (!IsDisconnected(RunCallbackBasedCoSimulationInternal())) {
            CloseConnection();
            return Result::Error;
        }

        return Result::Disconnected;
    }

    [[nodiscard]] Result StartPollingBasedCoSimulation(const Callbacks& callbacks) override {
        CheckResult(EnsureIsConnected());
        CheckResult(EnsureIsInResponderModeNonBlocking());

        SetThreadAffinity(_serverName);

        _callbacks = callbacks;
        return Result::Ok;
    }

    [[nodiscard]] Result PollCommand(SimulationTime& simulationTime, Command& command) override {
        CheckResult(EnsureIsConnected());
        CheckResult(EnsureIsInResponderModeNonBlocking());

        if (_currentCommand != Command::None) {
            Logger::Instance().LogError("Call to FinishCommand() for last command is missing.");
            return Result::Error;
        }

        if (!IsOk(PollCommandInternal(simulationTime, command))) {
            CloseConnection();
            return Result::Error;
        }

        return Result::Ok;
    }

    [[nodiscard]] Result FinishCommand() override {
        CheckResult(EnsureIsConnected());
        CheckResult(EnsureIsInResponderModeNonBlocking());

        if (_currentCommand == Command::None) {
            Logger::Instance().LogError("Call to PollCommand(...) is missing.");
            return Result::Error;
        }

        if (!IsOk(FinishCommandInternal())) {
            CloseConnection();
            return Result::Error;
        }

        return Result::Ok;
    }

    [[nodiscard]] Result SetNextSimulationTime(SimulationTime simulationTime) override {
        CheckResult(EnsureIsConnected());

        _nextSimulationTime = simulationTime;
        return Result::Ok;
    }

    [[nodiscard]] Result Start() override {
        CheckResult(EnsureIsConnected());

        _nextCommand.exchange(Command::Start);
        return Result::Ok;
    }

    [[nodiscard]] Result Stop() override {
        CheckResult(EnsureIsConnected());

        _nextCommand.exchange(Command::Stop);
        return Result::Ok;
    }

    [[nodiscard]] Result Terminate(TerminateReason terminateReason) override {
        CheckResult(EnsureIsConnected());

        switch (terminateReason) {
            case TerminateReason::Finished:
                _nextCommand.exchange(Command::TerminateFinished);
                return Result::Ok;
            case TerminateReason::Error:
                _nextCommand.exchange(Command::Terminate);
                return Result::Ok;
        }

        std::string message = "Unknown terminate reason '";
        message.append(ToString(terminateReason));
        message.append("'.");
        Logger::Instance().LogError(message);
        return Result::Error;
    }

    [[nodiscard]] Result Pause() override {
        CheckResult(EnsureIsConnected());

        _nextCommand.exchange(Command::Pause);
        return Result::Ok;
    }

    [[nodiscard]] Result Continue() override {
        CheckResult(EnsureIsConnected());

        _nextCommand.exchange(Command::Continue);
        return Result::Ok;
    }

    [[nodiscard]] Result GetIncomingSignals(uint32_t& incomingSignalsCount, const IoSignal*& incomingSignals) const override {
        CheckResult(EnsureIsConnected());

        incomingSignalsCount = static_cast<uint32_t>(_incomingSignalsExtern.size());
        incomingSignals = _incomingSignalsExtern.data();
        return Result::Ok;
    }

    [[nodiscard]] Result GetOutgoingSignals(uint32_t& outgoingSignalsCount, const IoSignal*& outgoingSignals) const override {
        CheckResult(EnsureIsConnected());

        outgoingSignalsCount = static_cast<uint32_t>(_outgoingSignalsExtern.size());
        outgoingSignals = _outgoingSignalsExtern.data();
        return Result::Ok;
    }

    [[nodiscard]] Result GetIncomingSignals(std::vector<IoSignal>& signals) const override {
        CheckResult(EnsureIsConnected());

        signals = _incomingSignalsExtern;
        return Result::Ok;
    }

    [[nodiscard]] Result GetOutgoingSignals(std::vector<IoSignal>& signals) const override {
        CheckResult(EnsureIsConnected());

        signals = _outgoingSignalsExtern;
        return Result::Ok;
    }

    [[nodiscard]] Result Write(IoSignalId outgoingSignalId, uint32_t length, const void* value) const override {
        CheckResult(EnsureIsConnected());

        return _ioBuffer->Write(outgoingSignalId, length, value);
    }

    [[nodiscard]] Result Read(IoSignalId incomingSignalId, uint32_t& length, void* value) const override {
        CheckResult(EnsureIsConnected());

        return _ioBuffer->Read(incomingSignalId, length, value);
    }

    [[nodiscard]] Result Read(IoSignalId incomingSignalId, uint32_t& length, const void** value) const override {
        CheckResult(EnsureIsConnected());

        return _ioBuffer->Read(incomingSignalId, length, value);
    }

    [[nodiscard]] Result GetCanControllers(uint32_t& controllersCount, const CanController*& controllers) const override {
        CheckResult(EnsureIsConnected());

        controllersCount = static_cast<uint32_t>(_canControllersExtern.size());
        controllers = _canControllersExtern.data();
        return Result::Ok;
    }

    [[nodiscard]] Result GetEthControllers(uint32_t& controllersCount, const EthController*& controllers) const override {
        CheckResult(EnsureIsConnected());

        controllersCount = static_cast<uint32_t>(_ethControllersExtern.size());
        controllers = _ethControllersExtern.data();
        return Result::Ok;
    }

    [[nodiscard]] Result GetLinControllers(uint32_t& controllersCount, const LinController*& controllers) const override {
        CheckResult(EnsureIsConnected());

        controllersCount = static_cast<uint32_t>(_linControllersExtern.size());
        controllers = _linControllersExtern.data();
        return Result::Ok;
    }

    [[nodiscard]] Result GetFrControllers(uint32_t& controllersCount, const FrController*& controllers) const override {
        CheckResult(EnsureIsConnected());

        controllersCount = static_cast<uint32_t>(_frControllersExtern.size());
        controllers = _frControllersExtern.data();
        return Result::Ok;
    }

    [[nodiscard]] Result GetCanControllers(std::vector<CanController>& controllers) const override {
        CheckResult(EnsureIsConnected());

        controllers = _canControllersExtern;
        return Result::Ok;
    }

    [[nodiscard]] Result GetEthControllers(std::vector<EthController>& controllers) const override {
        CheckResult(EnsureIsConnected());

        controllers = _ethControllersExtern;
        return Result::Ok;
    }

    [[nodiscard]] Result GetLinControllers(std::vector<LinController>& controllers) const override {
        CheckResult(EnsureIsConnected());

        controllers = _linControllersExtern;
        return Result::Ok;
    }

    [[nodiscard]] Result GetFrControllers(std::vector<FrController>& controllers) const override {
        CheckResult(EnsureIsConnected());

        controllers = _frControllersExtern;
        return Result::Ok;
    }

    [[nodiscard]] Result Transmit(const CanMessage& message) const override {
        CheckResult(EnsureIsConnected());
        CheckResult(CheckCanMessage(message.flags, message.length));

        return _busBuffer->Transmit(message);
    }

    [[nodiscard]] Result Transmit(const EthMessage& message) const override {
        CheckResult(EnsureIsConnected());

        return _busBuffer->Transmit(message);
    }

    [[nodiscard]] Result Transmit(const LinMessage& message) const override {
        CheckResult(EnsureIsConnected());

        return _busBuffer->Transmit(message);
    }

    [[nodiscard]] Result Transmit(const FrMessage& message) const override {
        CheckResult(EnsureIsConnected());

        return _busBuffer->Transmit(message);
    }

    [[nodiscard]] Result Transmit(const CanMessageContainer& messageContainer) const override {
        CheckResult(EnsureIsConnected());
        CheckResult(CheckCanMessage(messageContainer.flags, messageContainer.length));

        return _busBuffer->Transmit(messageContainer);
    }

    [[nodiscard]] Result Transmit(const EthMessageContainer& messageContainer) const override {
        CheckResult(EnsureIsConnected());

        return _busBuffer->Transmit(messageContainer);
    }

    [[nodiscard]] Result Transmit(const LinMessageContainer& messageContainer) const override {
        CheckResult(EnsureIsConnected());

        return _busBuffer->Transmit(messageContainer);
    }

    [[nodiscard]] Result Transmit(const FrMessageContainer& messageContainer) const override {
        CheckResult(EnsureIsConnected());

        return _busBuffer->Transmit(messageContainer);
    }

    [[nodiscard]] Result Receive(CanMessage& message) const override {
        CheckResult(EnsureIsConnected());

        return _busBuffer->Receive(message);
    }

    [[nodiscard]] Result Receive(EthMessage& message) const override {
        CheckResult(EnsureIsConnected());

        return _busBuffer->Receive(message);
    }

    [[nodiscard]] Result Receive(LinMessage& message) const override {
        CheckResult(EnsureIsConnected());

        return _busBuffer->Receive(message);
    }

    [[nodiscard]] Result Receive(FrMessage& message) const override {
        CheckResult(EnsureIsConnected());

        return _busBuffer->Receive(message);
    }

    [[nodiscard]] Result Receive(CanMessageContainer& messageContainer) const override {
        CheckResult(EnsureIsConnected());

        return _busBuffer->Receive(messageContainer);
    }

    [[nodiscard]] Result Receive(EthMessageContainer& messageContainer) const override {
        CheckResult(EnsureIsConnected());

        return _busBuffer->Receive(messageContainer);
    }

    [[nodiscard]] Result Receive(LinMessageContainer& messageContainer) const override {
        CheckResult(EnsureIsConnected());

        return _busBuffer->Receive(messageContainer);
    }

    [[nodiscard]] Result Receive(FrMessageContainer& messageContainer) const override {
        CheckResult(EnsureIsConnected());

        return _busBuffer->Receive(messageContainer);
    }

private:
    void ResetDataFromPreviousConnect() {
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

    [[nodiscard]] Result LocalConnect() {
        CheckResult(TryConnectToLocalChannel(_serverName, _channel));
        if (!_channel) {
            std::string message = "Could not connect to local dSPACE VEOS CoSim server '";
            message.append(_serverName);
            message.append("'.");
            Logger::Instance().LogTrace(message);
            return Result::Error;
        }

        _connectionKind = ConnectionKind::Local;
        return Result::Ok;
    }

    [[nodiscard]] Result RemoteConnect() {
        if (_remotePort == 0) {
            std::string message = "Obtaining TCP port of dSPACE VEOS CoSim server '";
            message.append(_serverName);
            message.append("' at ");
            message.append(_remoteIpAddress);
            message.append(" ...");
            Logger::Instance().LogInfo(message);
            CheckResultWithMessage(PortMapperGetPort(_remoteIpAddress, _serverName, _remotePort), "Could not get port from port mapper.");
        }

        if (_serverName.empty()) {
            std::string message = "Connecting to dSPACE VEOS CoSim server at ";
            message.append(_remoteIpAddress);
            message.append(":");
            message.append(std::to_string(_remotePort));
            message.append(" ...");
            Logger::Instance().LogInfo(message);
        } else {
            std::string message = "Connecting to dSPACE VEOS CoSim server '";
            message.append(_serverName);
            message.append("' at ");
            message.append(_remoteIpAddress);
            message.append(":");
            message.append(std::to_string(_remotePort));
            message.append(" ...");
            Logger::Instance().LogInfo(message);
        }

        CheckResult(TryConnectToTcpChannel(_remoteIpAddress, _remotePort, _localPort, ClientTimeoutInMilliseconds, _channel));
        CheckBoolWithMessage(_channel, "Could not connect to dSPACE VEOS CoSim server.");

        _connectionKind = ConnectionKind::Remote;
        return Result::Ok;
    }

    [[nodiscard]] Result SendConnectRequest() {
        CheckResultWithMessage(_protocol->SendConnect(_channel->GetWriter(), DsVeosCoSim::LATEST_VERSION, {}, _serverName, _clientName),
                               "Could not send connect frame.");
        return Result::Ok;
    }

    [[nodiscard]] Result OnConnectOk() {
        uint32_t serverProtocolVersion{};
        CheckResultWithMessage(_protocol->ReadConnectOkVersion(_channel->GetReader(), serverProtocolVersion), "Could not read protocol version.");

        if (_protocol->GetVersion() != serverProtocolVersion) {
            CheckResult(MakeProtocol(serverProtocolVersion, _protocol));
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
            std::string message = "Connected to local dSPACE VEOS CoSim server '";
            message.append(_serverName);
            message.append("'.");
            Logger::Instance().LogInfo(message);
        } else {
            if (_serverName.empty()) {
                std::string message = "Connected to dSPACE VEOS CoSim server at ";
                message.append(_remoteIpAddress);
                message.append(":");
                message.append(std::to_string(_remotePort));
                message.append(".");
                Logger::Instance().LogInfo(message);
            } else {
                std::string message = "Connected to dSPACE VEOS CoSim server '";
                message.append(_serverName);
                message.append("' at ");
                message.append(_remoteIpAddress);
                message.append(":");
                message.append(std::to_string(_remotePort));
                message.append(".");
                Logger::Instance().LogInfo(message);
            }
        }

        CheckResult(CreateIoBuffer(CoSimType::Client, _connectionKind, _serverName, _incomingSignalsExtern, _outgoingSignalsExtern, *_protocol, _ioBuffer));

        CheckResult(CreateBusBuffer(CoSimType::Client,
                                    _connectionKind,
                                    _serverName,
                                    _canControllersExtern,
                                    _ethControllersExtern,
                                    _linControllersExtern,
                                    _frControllersExtern,
                                    *_protocol,
                                    _busBuffer));

        _isConnected = true;
        return Result::Ok;
    }

    [[nodiscard]] Result OnConnectError() const {
        std::string errorString;
        CheckResultWithMessage(_protocol->ReadError(_channel->GetReader(), errorString), "Could not read error frame.");
        return Result::Error;
    }

    [[nodiscard]] Result ReceiveConnectResponse() {
        FrameKind frameKind{};
        CheckResult(_protocol->ReceiveHeader(_channel->GetReader(), frameKind));

        switch (frameKind) {
            // NOLINT(clang-diagnostic-switch-enum)
            case FrameKind::ConnectOk:
                CheckResultWithMessage(OnConnectOk(), "Could not handle connect ok.");
                return Result::Ok;
            case FrameKind::Error:
                CheckResultWithMessage(OnConnectError(), "Could not handle connect error.");
                return Result::Error;
            default:
                return OnUnexpectedFrame(frameKind);
        }
    }

    [[nodiscard]] Result RunCallbackBasedCoSimulationInternal() {
        while (_isConnected) {
            FrameKind frameKind{};
            CheckResult(_protocol->ReceiveHeader(_channel->GetReader(), frameKind));

            switch (frameKind) {
                // NOLINT(clang-diagnostic-switch-enum)
                case FrameKind::Step: {
                    CheckResultWithMessage(OnStep(), "Could not handle step.");
                    if (!_isConnected) {
                        return Result::Disconnected;
                    }

                    CheckResult(FinishStep());
                    break;
                }
                case FrameKind::Start:
                    CheckResultWithMessage(OnStart(), "Could not handle start.");
                    if (!_isConnected) {
                        return Result::Disconnected;
                    }

                    CheckResult(FinishCurrentCommand());
                    break;
                case FrameKind::Stop:
                    CheckResultWithMessage(OnStop(), "Could not handle stop.");
                    if (!_isConnected) {
                        return Result::Disconnected;
                    }

                    CheckResult(FinishCurrentCommand());
                    break;
                case FrameKind::Terminate:
                    CheckResultWithMessage(OnTerminate(), "Could not handle terminate.");
                    if (!_isConnected) {
                        return Result::Disconnected;
                    }

                    CheckResult(FinishCurrentCommand());
                    break;
                case FrameKind::Pause:
                    CheckResultWithMessage(OnPause(), "Could not handle pause.");
                    if (!_isConnected) {
                        return Result::Disconnected;
                    }

                    CheckResult(FinishCurrentCommand());
                    break;
                case FrameKind::Continue:
                    CheckResultWithMessage(OnContinue(), "Could not handle continue.");
                    if (!_isConnected) {
                        return Result::Disconnected;
                    }

                    CheckResult(FinishCurrentCommand());
                    break;
                case FrameKind::Ping:
                    CheckResult(FinishPing());
                    break;
                default:
                    return OnUnexpectedFrame(frameKind);
            }
        }

        return Result::Disconnected;
    }

    [[nodiscard]] Result PollCommandInternal(SimulationTime& simulationTime, Command& command) {
        simulationTime = _currentSimulationTime;
        command = Command::Terminate;

        do {
            FrameKind frameKind{};
            CheckResult(_protocol->ReceiveHeader(_channel->GetReader(), frameKind));
            switch (frameKind) {
                // NOLINT(clang-diagnostic-switch-enum)
                case FrameKind::Step:
                    CheckResultWithMessage(OnStep(), "Could not handle step.");
                    _currentCommand = Command::Step;
                    break;
                case FrameKind::Ping:
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
        return Result::Ok;
    }

    [[nodiscard]] Result FinishCommandInternal() {
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
        return Result::Ok;
    }

    [[nodiscard]] Result OnStep() {
        CheckResultWithMessage(_protocol->ReadStep(_channel->GetReader(), _currentSimulationTime, _deserializeIoData, _deserializeBusMessages, _callbacks),
                               "Could not read step frame.");

        if (_callbacks.simulationEndStepCallback) {
            _callbacks.simulationEndStepCallback(_currentSimulationTime);
        }

        return Result::Ok;
    }

    [[nodiscard]] Result OnStart() {
        CheckResultWithMessage(_protocol->ReadStart(_channel->GetReader(), _currentSimulationTime), "Could not read start frame.");

        _ioBuffer->ClearData();
        _busBuffer->ClearData();

        if (_callbacks.simulationStartedCallback) {
            _callbacks.simulationStartedCallback(_currentSimulationTime);
        }

        return Result::Ok;
    }

    [[nodiscard]] Result OnStop() {
        CheckResultWithMessage(_protocol->ReadStop(_channel->GetReader(), _currentSimulationTime), "Could not read stop frame.");

        if (_callbacks.simulationStoppedCallback) {
            _callbacks.simulationStoppedCallback(_currentSimulationTime);
        }

        return Result::Ok;
    }

    [[nodiscard]] Result OnTerminate() {
        TerminateReason reason{};
        CheckResultWithMessage(_protocol->ReadTerminate(_channel->GetReader(), _currentSimulationTime, reason), "Could not read terminate frame.");

        if (_callbacks.simulationTerminatedCallback) {
            _callbacks.simulationTerminatedCallback(_currentSimulationTime, reason);
        }

        return Result::Ok;
    }

    [[nodiscard]] Result OnPause() {
        CheckResultWithMessage(_protocol->ReadPause(_channel->GetReader(), _currentSimulationTime), "Could not read pause frame.");

        if (_callbacks.simulationPausedCallback) {
            _callbacks.simulationPausedCallback(_currentSimulationTime);
        }

        return Result::Ok;
    }

    [[nodiscard]] Result OnContinue() {
        CheckResultWithMessage(_protocol->ReadContinue(_channel->GetReader(), _currentSimulationTime), "Could not read continue frame.");

        if (_callbacks.simulationContinuedCallback) {
            _callbacks.simulationContinuedCallback(_currentSimulationTime);
        }

        return Result::Ok;
    }

    [[nodiscard]] Result FinishStep() {
        Command nextCommand = _nextCommand.exchange({});
        CheckResultWithMessage(_protocol->SendStepOk(_channel->GetWriter(), _nextSimulationTime, nextCommand, _serializeIoData, _serializeBusMessages),
                               "Could not send step ok frame.");
        return Result::Ok;
    }

    [[nodiscard]] Result FinishPing() {
        Command nextCommand = _nextCommand.exchange({});
        CheckResultWithMessage(_protocol->SendPingOk(_channel->GetWriter(), nextCommand), "Could not send ping ok frame.");
        return Result::Ok;
    }

    [[nodiscard]] Result FinishCurrentCommand() {
        CheckResultWithMessage(_protocol->SendOk(_channel->GetWriter()), "Could not send ok frame.");
        return Result::Ok;
    }

    [[nodiscard]] Result EnsureIsConnected() const {
        if (!_isConnected) {
            Logger::Instance().LogError("Not connected.");
            return Result::Error;
        }

        return Result::Ok;
    }

    [[nodiscard]] Result EnsureIsInResponderModeBlocking() {
        switch (_responderMode) {
            case ResponderMode::Blocking:
                // Nothing to do
                return Result::Ok;
            case ResponderMode::Unknown:
                _responderMode = ResponderMode::Blocking;
                return Result::Ok;
            case ResponderMode::NonBlocking:
                Logger::Instance().LogError("dSPACE VEOS CoSim is in non-blocking mode. Blocking function call is not allowed.");
                return Result::Error;
        }

        Logger::Instance().LogError("Invalid responder mode.");
        return Result::Error;
    }

    [[nodiscard]] Result EnsureIsInResponderModeNonBlocking() {
        switch (_responderMode) {
            case ResponderMode::NonBlocking:
                // Nothing to do
                return Result::Ok;
            case ResponderMode::Unknown:
                _responderMode = ResponderMode::NonBlocking;
                return Result::Ok;
            case ResponderMode::Blocking:
                Logger::Instance().LogError("dSPACE VEOS CoSim is in blocking mode. Non-blocking function call is not allowed.");
                return Result::Error;
        }

        Logger::Instance().LogError("Invalid responder mode.");
        return Result::Error;
    }

    void CloseConnection() {
        Logger::Instance().LogWarning("dSPACE VEOS CoSim server disconnected.");

        _isConnected = false;

        if (_channel) {
            _channel->Disconnect();
        }
    }

    [[nodiscard]] static Result OnUnexpectedFrame(FrameKind frameKind) {
        std::string message = "Received unexpected frame '";
        message.append(ToString(frameKind));
        message.append("'.");
        Logger::Instance().LogError(message);
        return Result::Error;
    }

    [[nodiscard]] static Result CheckCanMessage(CanMessageFlags flags, uint32_t length) {
        if (length > CanMessageMaxLength) {
            Logger::Instance().LogError("CAN message data exceeds maximum length.");
            return Result::InvalidArgument;
        }

        if (!HasFlag(flags, CanMessageFlags::FlexibleDataRateFormat)) {
            if (length > 8) {
                Logger::Instance().LogError("CAN message flags are invalid. A DLC > 8 requires the flexible data rate format flag.");
                return Result::InvalidArgument;
            }

            if (HasFlag(flags, CanMessageFlags::BitRateSwitch)) {
                Logger::Instance().LogError(
                    "CAN message flags are invalid. A bit rate switch flag requires the flexible data rate format "
                    "flag.");
                return Result::InvalidArgument;
            }
        }

        return Result::Ok;
    }

    std::unique_ptr<Channel> _channel;
    ConnectionKind _connectionKind = ConnectionKind::Remote;

    std::unique_ptr<IProtocol> _protocol;

    bool _isConnected{};
    Callbacks _callbacks{};
    SimulationTime _currentSimulationTime{};
    SimulationTime _nextSimulationTime{};

    SimulationTime _stepSize{};

    SimulationState _simulationState{};

    std::string _remoteIpAddress;
    std::string _serverName;
    std::string _clientName;
    uint16_t _remotePort{};
    uint16_t _localPort{};

    ResponderMode _responderMode{};
    Command _currentCommand{};
    std::atomic<Command> _nextCommand{};

    std::vector<IoSignalContainer> _incomingSignals;
    std::vector<IoSignalContainer> _outgoingSignals;
    std::vector<IoSignal> _incomingSignalsExtern;
    std::vector<IoSignal> _outgoingSignalsExtern;

    std::vector<CanControllerContainer> _canControllers;
    std::vector<EthControllerContainer> _ethControllers;
    std::vector<LinControllerContainer> _linControllers;
    std::vector<FrControllerContainer> _frControllers;
    std::vector<CanController> _canControllersExtern;
    std::vector<EthController> _ethControllersExtern;
    std::vector<LinController> _linControllersExtern;
    std::vector<FrController> _frControllersExtern;

    std::unique_ptr<IoBuffer> _ioBuffer;
    std::unique_ptr<BusBuffer> _busBuffer;

    SerializeFunction _serializeIoData = [&](ChannelWriter& writer) {
        return _ioBuffer->Serialize(writer);
    };

    SerializeFunction _serializeBusMessages = [&](ChannelWriter& writer) {
        return _busBuffer->Serialize(writer);
    };

    DeserializeFunction _deserializeIoData = [&](ChannelReader& reader, SimulationTime simulationTime, const Callbacks& callbacks) {
        return _ioBuffer->Deserialize(reader, simulationTime, callbacks);
    };

    DeserializeFunction _deserializeBusMessages = [&](ChannelReader& reader, SimulationTime simulationTime, const Callbacks& callbacks) {
        return _busBuffer->Deserialize(reader, simulationTime, callbacks);
    };
};

}  // namespace

[[nodiscard]] Result CreateClient(std::unique_ptr<CoSimClient>& client) {
    client = std::make_unique<CoSimClientImpl>();
    return Result::Ok;
}

}  // namespace DsVeosCoSim
