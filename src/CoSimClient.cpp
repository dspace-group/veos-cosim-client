// Copyright dSPACE GmbH. All rights reserved.

#include "DsVeosCoSim/CoSimClient.h"

#include <atomic>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "BusBuffer.h"
#include "Channel.h"
#include "CoSimHelper.h"
#include "DsVeosCoSim/CoSimTypes.h"
#include "IoBuffer.h"
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
    ~CoSimClientImpl() noexcept override = default;

    CoSimClientImpl(const CoSimClientImpl&) = delete;
    CoSimClientImpl& operator=(const CoSimClientImpl&) = delete;

    CoSimClientImpl(CoSimClientImpl&&) = delete;
    CoSimClientImpl& operator=(CoSimClientImpl&&) = delete;

    [[nodiscard]] bool Connect(const ConnectConfig& connectConfig) override {
        if (connectConfig.serverName.empty() && (connectConfig.remotePort == 0)) {
            throw std::runtime_error("Either ConnectConfig.serverName or ConnectConfig.remotePort must be set.");
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

    void Disconnect() override {
        _isConnected = false;

        if (_channel) {
            _channel->Disconnect();
        }
    }

    [[nodiscard]] ConnectionState GetConnectionState() const override {
        if (_isConnected) {
            return ConnectionState::Connected;
        }

        return ConnectionState::Disconnected;
    }

    [[nodiscard]] SimulationTime GetStepSize() const override {
        EnsureIsConnected();

        return _stepSize;
    }

    [[nodiscard]] SimulationTime GetCurrentSimulationTime() const override {
        EnsureIsConnected();

        return _currentSimulationTime;
    }

    [[nodiscard]] bool RunCallbackBasedCoSimulation(const Callbacks& callbacks) override {
        EnsureIsConnected();
        EnsureIsInResponderModeBlocking();

        _callbacks = callbacks;

        if (!RunCallbackBasedCoSimulationInternal()) {
            CloseConnection();
            return false;
        }

        return true;
    }

    void StartPollingBasedCoSimulation(const Callbacks& callbacks) override {
        EnsureIsConnected();
        EnsureIsInResponderModeNonBlocking();

        _callbacks = callbacks;
    }

    [[nodiscard]] bool PollCommand(SimulationTime& simulationTime, Command& command, const bool returnOnPing) override {
        EnsureIsConnected();
        EnsureIsInResponderModeNonBlocking();

        if (_currentCommand != Command::None) {
            throw std::runtime_error("Call to FinishCommand() for last command is missing.");
        }

        if (!PollCommandInternal(simulationTime, command, returnOnPing)) {
            CloseConnection();
            return false;
        }

        return true;
    }

    [[nodiscard]] bool FinishCommand() override {
        EnsureIsConnected();
        EnsureIsInResponderModeNonBlocking();

        if (_currentCommand == Command::None) {
            throw std::runtime_error("Call to PollCommand(...) is missing.");
        }

        if (!FinishCommandInternal()) {
            CloseConnection();
            return false;
        }

        return true;
    }

    void SetNextSimulationTime(const SimulationTime simulationTime) override {
        EnsureIsConnected();

        _nextSimulationTime = simulationTime;
    }

    void Start() override {
        EnsureIsConnected();

        _nextCommand.exchange(Command::Start);
    }

    void Stop() override {
        EnsureIsConnected();

        _nextCommand.exchange(Command::Stop);
    }

    void Terminate(const TerminateReason terminateReason) override {
        EnsureIsConnected();

        switch (terminateReason) {
            case TerminateReason::Finished:
                _nextCommand.exchange(Command::TerminateFinished);
                return;
            case TerminateReason::Error:
                _nextCommand.exchange(Command::Terminate);
                return;
        }

        std::string message = "Unknown terminate reason '";
        message.append(ToString(terminateReason));
        message.append("'.");
        throw std::runtime_error(message);
    }

    void Pause() override {
        EnsureIsConnected();

        _nextCommand.exchange(Command::Pause);
    }

    void Continue() override {
        EnsureIsConnected();

        _nextCommand.exchange(Command::Continue);
    }

    void GetIncomingSignals(uint32_t* incomingSignalsCount, const IoSignal** incomingSignals) const override {
        EnsureIsConnected();

        *incomingSignalsCount = static_cast<uint32_t>(_incomingSignalsExtern.size());
        *incomingSignals = _incomingSignalsExtern.data();
    }

    void GetOutgoingSignals(uint32_t* outgoingSignalsCount, const IoSignal** outgoingSignals) const override {
        EnsureIsConnected();

        *outgoingSignalsCount = static_cast<uint32_t>(_outgoingSignalsExtern.size());
        *outgoingSignals = _outgoingSignalsExtern.data();
    }

    [[nodiscard]] const std::vector<IoSignal>& GetIncomingSignals() const override {
        EnsureIsConnected();

        return _incomingSignalsExtern;
    }

    [[nodiscard]] const std::vector<IoSignal>& GetOutgoingSignals() const override {
        EnsureIsConnected();

        return _outgoingSignalsExtern;
    }

    void Write(const IoSignalId outgoingSignalId, const uint32_t length, const void* value) const override {
        EnsureIsConnected();

        _ioBuffer->Write(outgoingSignalId, length, value);
    }

    void Read(const IoSignalId incomingSignalId, uint32_t& length, void* value) const override {
        EnsureIsConnected();

        _ioBuffer->Read(incomingSignalId, length, value);
    }

    void Read(const IoSignalId incomingSignalId, uint32_t& length, const void** value) const override {
        EnsureIsConnected();

        _ioBuffer->Read(incomingSignalId, length, value);
    }

    void GetCanControllers(uint32_t* controllersCount, const CanController** controllers) const override {
        EnsureIsConnected();

        *controllersCount = static_cast<uint32_t>(_canControllersExtern.size());
        *controllers = _canControllersExtern.data();
    }

    void GetEthControllers(uint32_t* controllersCount, const EthController** controllers) const override {
        EnsureIsConnected();

        *controllersCount = static_cast<uint32_t>(_ethControllersExtern.size());
        *controllers = _ethControllersExtern.data();
    }

    void GetLinControllers(uint32_t* controllersCount, const LinController** controllers) const override {
        EnsureIsConnected();

        *controllersCount = static_cast<uint32_t>(_linControllersExtern.size());
        *controllers = _linControllersExtern.data();
    }

    [[nodiscard]] const std::vector<CanController>& GetCanControllers() const override {
        EnsureIsConnected();

        return _canControllersExtern;
    }

    [[nodiscard]] const std::vector<EthController>& GetEthControllers() const override {
        EnsureIsConnected();

        return _ethControllersExtern;
    }

    [[nodiscard]] const std::vector<LinController>& GetLinControllers() const override {
        EnsureIsConnected();

        return _linControllersExtern;
    }

    [[nodiscard]] bool Transmit(const CanMessage& message) const override {
        EnsureIsConnected();

        return _busBuffer->Transmit(message);
    }

    [[nodiscard]] bool Transmit(const EthMessage& message) const override {
        EnsureIsConnected();

        return _busBuffer->Transmit(message);
    }

    [[nodiscard]] bool Transmit(const LinMessage& message) const override {
        EnsureIsConnected();

        return _busBuffer->Transmit(message);
    }

    [[nodiscard]] bool Receive(CanMessage& message) const override {
        EnsureIsConnected();

        return _busBuffer->Receive(message);
    }

    [[nodiscard]] bool Receive(EthMessage& message) const override {
        EnsureIsConnected();

        return _busBuffer->Receive(message);
    }

    [[nodiscard]] bool Receive(LinMessage& message) const override {
        EnsureIsConnected();

        return _busBuffer->Receive(message);
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
        _canControllersExtern.clear();
        _ethControllersExtern.clear();
        _linControllersExtern.clear();
    }

    [[nodiscard]] bool LocalConnect() {
#ifdef _WIN32
        _channel = TryConnectToLocalChannel(_serverName);
#else
        _channel = TryConnectToUdsChannel(_serverName);
#endif
        if (!_channel) {
            std::string message = "Could not connect to local dSPACE VEOS CoSim server '";
            message.append(_serverName);
            message.append("'.");
            LogTrace(message);
            return false;
        }

        _connectionKind = ConnectionKind::Local;
        return true;
    }

    [[nodiscard]] bool RemoteConnect() {
        if (_remotePort == 0) {
            std::string message = "Obtaining TCP port of dSPACE VEOS CoSim server '";
            message.append(_serverName);
            message.append("' at ");
            message.append(_remoteIpAddress);
            message.append(" ...");
            LogInfo(message);
            CheckResultWithMessage(PortMapper_GetPort(_remoteIpAddress, _serverName, _remotePort),
                                   "Could not get port from port mapper.");
        }

        if (_serverName.empty()) {
            std::string message = "Connecting to dSPACE VEOS CoSim server at ";
            message.append(_remoteIpAddress);
            message.append(":");
            message.append(std::to_string(_remotePort));
            message.append(" ...");
            LogInfo(message);
        } else {
            std::string message = "Connecting to dSPACE VEOS CoSim server '";
            message.append(_serverName);
            message.append("' at ");
            message.append(_remoteIpAddress);
            message.append(":");
            message.append(std::to_string(_remotePort));
            message.append(" ...");
            LogInfo(message);
        }

        _channel = TryConnectToTcpChannel(_remoteIpAddress, _remotePort, _localPort, ClientTimeoutInMilliseconds);
        CheckResultWithMessage(_channel, "Could not connect to dSPACE VEOS CoSim server.");

        _connectionKind = ConnectionKind::Remote;

        return true;
    }

    [[nodiscard]] bool SendConnectRequest() const {
        CheckResultWithMessage(
            Protocol::SendConnect(_channel->GetWriter(), CoSimProtocolVersion, {}, _serverName, _clientName),
            "Could not send connect frame.");
        return true;
    }

    [[nodiscard]] bool OnConnectOk() {
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
            std::string message = "Connected to local dSPACE VEOS CoSim server '";
            message.append(_serverName);
            message.append(".");
            LogInfo(message);
        } else {
            if (_serverName.empty()) {
                std::string message = "Connected to dSPACE VEOS CoSim server at ";
                message.append(_remoteIpAddress);
                message.append(":");
                message.append(std::to_string(_remotePort));
                message.append(".");
                LogInfo(message);
            } else {
                std::string message = "Connected to dSPACE VEOS CoSim server '";
                message.append(_serverName);
                message.append("' at ");
                message.append(_remoteIpAddress);
                message.append(":");
                message.append(std::to_string(_remotePort));
                message.append(".");
                LogInfo(message);
            }
        }

        _ioBuffer = CreateIoBuffer(CoSimType::Client,
                                   _connectionKind,
                                   _serverName,
                                   _incomingSignalsExtern,
                                   _outgoingSignalsExtern);

        _busBuffer = CreateBusBuffer(CoSimType::Client,
                                     _connectionKind,
                                     _serverName,
                                     _canControllersExtern,
                                     _ethControllersExtern,
                                     _linControllersExtern);

        _isConnected = true;

        return true;
    }

    [[nodiscard]] bool OnConnectError() const {
        std::string errorString;
        CheckResultWithMessage(Protocol::ReadError(_channel->GetReader(), errorString), "Could not read error frame.");
        return false;
    }

    [[nodiscard]] bool ReceiveConnectResponse() {
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
                std::string message = "Received unexpected frame '";
                message.append(ToString(frameKind));
                message.append("'.");
                throw std::runtime_error(message);
        }
    }

    [[nodiscard]] bool RunCallbackBasedCoSimulationInternal() {
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
                    std::string message = "Received unexpected frame '";
                    message.append(ToString(frameKind));
                    message.append("'.");
                    throw std::runtime_error(message);
            }
        }

        return true;
    }

    [[nodiscard]] bool PollCommandInternal(SimulationTime& simulationTime, Command& command, const bool returnOnPing) {
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
                    std::string message = "Received unexpected frame '";
                    message.append(ToString(frameKind));
                    message.append("'.");
                    throw std::runtime_error(message);
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

    [[nodiscard]] bool FinishCommandInternal() {
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
                CheckResultWithMessage(Protocol::SendStepOk(_channel->GetWriter(),
                                                            _nextSimulationTime,
                                                            nextCommand,
                                                            *_ioBuffer,
                                                            *_busBuffer),
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

    [[nodiscard]] bool OnStep() {
        CheckResultWithMessage(
            Protocol::ReadStep(_channel->GetReader(), _currentSimulationTime, *_ioBuffer, *_busBuffer, _callbacks),
            "Could not read step frame.");

        if (_callbacks.simulationEndStepCallback) {
            _callbacks.simulationEndStepCallback(_currentSimulationTime);
        }

        return true;
    }

    [[nodiscard]] bool OnStart() {
        CheckResultWithMessage(Protocol::ReadStart(_channel->GetReader(), _currentSimulationTime),
                               "Could not read start frame.");

        _ioBuffer->ClearData();
        _busBuffer->ClearData();

        if (_callbacks.simulationStartedCallback) {
            _callbacks.simulationStartedCallback(_currentSimulationTime);
        }

        return true;
    }

    [[nodiscard]] bool OnStop() {
        CheckResultWithMessage(Protocol::ReadStop(_channel->GetReader(), _currentSimulationTime),
                               "Could not read stop frame.");

        if (_callbacks.simulationStoppedCallback) {
            _callbacks.simulationStoppedCallback(_currentSimulationTime);
        }

        return true;
    }

    [[nodiscard]] bool OnTerminate() {
        TerminateReason reason{};
        CheckResultWithMessage(Protocol::ReadTerminate(_channel->GetReader(), _currentSimulationTime, reason),
                               "Could not read terminate frame.");

        if (_callbacks.simulationTerminatedCallback) {
            _callbacks.simulationTerminatedCallback(_currentSimulationTime, reason);
        }

        return true;
    }

    [[nodiscard]] bool OnPause() {
        CheckResultWithMessage(Protocol::ReadPause(_channel->GetReader(), _currentSimulationTime),
                               "Could not read pause frame.");

        if (_callbacks.simulationPausedCallback) {
            _callbacks.simulationPausedCallback(_currentSimulationTime);
        }

        return true;
    }

    [[nodiscard]] bool OnContinue() {
        CheckResultWithMessage(Protocol::ReadContinue(_channel->GetReader(), _currentSimulationTime),
                               "Could not read continue frame.");

        if (_callbacks.simulationContinuedCallback) {
            _callbacks.simulationContinuedCallback(_currentSimulationTime);
        }

        return true;
    }

    void EnsureIsConnected() const {
        if (!_isConnected) {
            throw std::runtime_error("Not connected.");
        }
    }

    void EnsureIsInResponderModeBlocking() {
        switch (_responderMode) {
            case ResponderMode::Unknown:
                _responderMode = ResponderMode::Blocking;
                break;
            case ResponderMode::NonBlocking:
                throw std::runtime_error(
                    "dSPACE VEOS CoSim is in non-blocking mode. Blocking function call is not allowed.");
            case ResponderMode::Blocking:
                // Nothing to do
                break;
        }
    }

    void EnsureIsInResponderModeNonBlocking() {
        switch (_responderMode) {
            case ResponderMode::Unknown:
                _responderMode = ResponderMode::NonBlocking;
                break;
            case ResponderMode::Blocking:
                throw std::runtime_error(
                    "dSPACE VEOS CoSim is in blocking mode. Non-blocking function call is not allowed.");
            case ResponderMode::NonBlocking:
                // Nothing to do
                break;
        }
    }

    void CloseConnection() {
        LogWarning("dSPACE VEOS CoSim server disconnected.");

        _isConnected = false;

        if (_channel) {
            _channel->Disconnect();
        }
    }

    std::unique_ptr<Channel> _channel;
    ConnectionKind _connectionKind = ConnectionKind::Remote;

    bool _isConnected{};
    Callbacks _callbacks{};
    SimulationTime _currentSimulationTime{};
    SimulationTime _nextSimulationTime{};

    SimulationTime _stepSize{};

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
    std::vector<CanController> _canControllersExtern;
    std::vector<EthController> _ethControllersExtern;
    std::vector<LinController> _linControllersExtern;

    std::unique_ptr<IoBuffer> _ioBuffer;
    std::unique_ptr<BusBuffer> _busBuffer;
};

}  // namespace

std::unique_ptr<CoSimClient> CreateClient() {
    return std::make_unique<CoSimClientImpl>();
}

}  // namespace DsVeosCoSim
