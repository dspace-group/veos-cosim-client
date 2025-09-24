// Copyright dSPACE GmbH. All rights reserved.

#include "DsVeosCoSim/CoSimServer.h"

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "BusBuffer.h"
#include "Channel.h"
#include "CoSimHelper.h"
#include "DsVeosCoSim/CoSimTypes.h"
#include "IoBuffer.h"
#include "OsUtilities.h"
#include "PortMapper.h"
#include "Protocol.h"

using namespace std::chrono;

namespace DsVeosCoSim {

namespace {

class CoSimServerImpl final : public CoSimServer {
public:
    CoSimServerImpl() = default;

    ~CoSimServerImpl() noexcept override {
        Unload();
    }

    CoSimServerImpl(const CoSimServerImpl&) = delete;
    CoSimServerImpl& operator=(const CoSimServerImpl&) = delete;

    CoSimServerImpl(CoSimServerImpl&&) = delete;
    CoSimServerImpl& operator=(CoSimServerImpl&&) = delete;

    [[nodiscard]] Result Load(const CoSimServerConfig& config) override {
        _enableRemoteAccess = config.enableRemoteAccess;
        _localPort = config.port;
        _serverName = config.serverName;
        _isClientOptional = config.isClientOptional;
        _stepSize = config.stepSize;
        _registerAtPortMapper = config.registerAtPortMapper;
        _incomingSignals = config.incomingSignals;
        _outgoingSignals = config.outgoingSignals;
        _canControllers = config.canControllers;
        _ethControllers = config.ethControllers;
        _linControllers = config.linControllers;

        _callbacks.simulationStartedCallback = config.simulationStartedCallback;
        _callbacks.simulationStoppedCallback = config.simulationStoppedCallback;
        _callbacks.simulationPausedCallback = config.simulationPausedCallback;
        _callbacks.simulationContinuedCallback = config.simulationContinuedCallback;
        _callbacks.simulationTerminatedCallback = config.simulationTerminatedCallback;
        _callbacks.canMessageContainerReceivedCallback = config.canMessageContainerReceivedCallback;
        _callbacks.linMessageContainerReceivedCallback = config.linMessageContainerReceivedCallback;
        _callbacks.ethMessageContainerReceivedCallback = config.ethMessageContainerReceivedCallback;

        if (config.startPortMapper) {
            CheckResult(CreatePortMapperServer(_enableRemoteAccess, _portMapperServer));
        }

        CheckResult(StartAccepting());
        _simulationState = SimulationState::Stopped;
        return Result::Ok;
    }

    void Unload() override {
        if (_channel) {
            _channel.reset();
        }

        StopAccepting();

        if (_portMapperServer) {
            _portMapperServer.reset();
        }

        _simulationState = SimulationState::Unloaded;
    }

    [[nodiscard]] Result Start(SimulationTime simulationTime) override {
        if (!_channel) {
            if (_isClientOptional) {
                return Result::Ok;
            }

            std::string message = "Waiting for dSPACE VEOS CoSim client to connect to dSPACE VEOS CoSim server '";
            message.append(_serverName);
            message.append("' ...");
            LogInfo(message);

            while (!IsOk(AcceptChannel())) {
                std::this_thread::sleep_for(milliseconds(100));
            }

            if (!IsOk(OnHandleConnect())) {
                return CloseConnection();
            }
        }

        if (!IsOk(StartInternal(simulationTime))) {
            return CloseConnection();
        }

        return Result::Ok;
    }

    [[nodiscard]] Result Stop(SimulationTime simulationTime) override {
        if (!_channel) {
            return Result::Ok;
        }

        if (!IsOk(StopInternal(simulationTime))) {
            return CloseConnection();
        }

        return Result::Ok;
    }

    [[nodiscard]] Result Terminate(SimulationTime simulationTime, TerminateReason reason) override {
        if (!_channel) {
            return Result::Ok;
        }

        if (!IsOk(TerminateInternal(simulationTime, reason))) {
            return CloseConnection();
        }

        return Result::Ok;
    }

    [[nodiscard]] Result Pause(SimulationTime simulationTime) override {
        if (!_channel) {
            return Result::Ok;
        }

        if (!IsOk(PauseInternal(simulationTime))) {
            return CloseConnection();
        }

        return Result::Ok;
    }

    [[nodiscard]] Result Continue(SimulationTime simulationTime) override {
        if (!_channel) {
            return Result::Ok;
        }

        if (!IsOk(ContinueInternal(simulationTime))) {
            return CloseConnection();
        }

        return Result::Ok;
    }

    [[nodiscard]] Result Step(SimulationTime simulationTime, SimulationTime& nextSimulationTime) override {
        if (!_channel) {
            return Result::Ok;
        }

        Command command{};
        if (!IsOk(StepInternal(simulationTime, nextSimulationTime, command))) {
            return CloseConnection();
        }

        HandlePendingCommand(command);
        return Result::Ok;
    }

    [[nodiscard]] Result Write(IoSignalId signalId, uint32_t length, const void* value) const override {
        if (!_channel) {
            return Result::Ok;
        }

        return _ioBuffer->Write(signalId, length, value);
    }

    [[nodiscard]] Result Read(IoSignalId signalId,
                              uint32_t& length,
                              const void** value,
                              bool& valueRead) const override {
        if (!_channel) {
            valueRead = false;
            return Result::Ok;
        }

        valueRead = true;
        return _ioBuffer->Read(signalId, length, value);
    }

    [[nodiscard]] Result Transmit(const CanMessage& message) const override {
        if (!_channel) {
            return Result::Ok;
        }

        return _busBuffer->Transmit(message);
    }

    [[nodiscard]] Result Transmit(const EthMessage& message) const override {
        if (!_channel) {
            return Result::Ok;
        }

        return _busBuffer->Transmit(message);
    }

    [[nodiscard]] Result Transmit(const LinMessage& message) const override {
        if (!_channel) {
            return Result::Ok;
        }

        return _busBuffer->Transmit(message);
    }

    [[nodiscard]] Result Transmit(const CanMessageContainer& messageContainer) const override {
        if (!_channel) {
            return Result::Ok;
        }

        return _busBuffer->Transmit(messageContainer);
    }

    [[nodiscard]] Result Transmit(const EthMessageContainer& messageContainer) const override {
        if (!_channel) {
            return Result::Ok;
        }

        return _busBuffer->Transmit(messageContainer);
    }

    [[nodiscard]] Result Transmit(const LinMessageContainer& messageContainer) const override {
        if (!_channel) {
            return Result::Ok;
        }

        return _busBuffer->Transmit(messageContainer);
    }

    [[nodiscard]] Result BackgroundService() override {
        if (!_channel) {
            if (IsOk(AcceptChannel())) {
                if (!IsOk(OnHandleConnect())) {
                    return CloseConnection();
                }
            }

            return Result::Ok;
        }

        Command command{};
        if (!IsOk(Ping(command))) {
            return CloseConnection();
        }

        HandlePendingCommand(command);
        return Result::Ok;
    }

    [[nodiscard]] Result GetLocalPort(uint16_t& localPort) const override {
        if (_tcpChannelServer) {
            localPort = _tcpChannelServer->GetLocalPort();
        }

        return Result::Ok;
    }

private:
    [[nodiscard]] Result StartInternal(SimulationTime simulationTime) {
        CheckResultWithMessage(Protocol::SendStart(_channel->GetWriter(), simulationTime),
                               "Could not send start frame.");
        CheckResultWithMessage(WaitForOkFrame(), "Could not receive ok frame.");
        _simulationState = SimulationState::Running;
        return Result::Ok;
    }

    [[nodiscard]] Result StopInternal(SimulationTime simulationTime) {
        CheckResultWithMessage(Protocol::SendStop(_channel->GetWriter(), simulationTime), "Could not send stop frame.");
        CheckResultWithMessage(WaitForOkFrame(), "Could not receive ok frame.");
        _simulationState = SimulationState::Stopped;
        return Result::Ok;
    }

    [[nodiscard]] Result TerminateInternal(SimulationTime simulationTime, TerminateReason reason) {
        CheckResultWithMessage(Protocol::SendTerminate(_channel->GetWriter(), simulationTime, reason),
                               "Could not send terminate frame.");
        CheckResultWithMessage(WaitForOkFrame(), "Could not receive ok frame.");
        _simulationState = SimulationState::Terminated;
        return Result::Ok;
    }

    [[nodiscard]] Result PauseInternal(SimulationTime simulationTime) {
        CheckResultWithMessage(Protocol::SendPause(_channel->GetWriter(), simulationTime),
                               "Could not send pause frame.");
        CheckResultWithMessage(WaitForOkFrame(), "Could not receive ok frame.");
        _simulationState = SimulationState::Paused;
        return Result::Ok;
    }

    [[nodiscard]] Result ContinueInternal(SimulationTime simulationTime) {
        CheckResultWithMessage(Protocol::SendContinue(_channel->GetWriter(), simulationTime),
                               "Could not send continue frame.");
        CheckResultWithMessage(WaitForOkFrame(), "Could not receive ok frame.");
        _simulationState = SimulationState::Running;
        return Result::Ok;
    }

    [[nodiscard]] Result StepInternal(SimulationTime simulationTime,
                                      SimulationTime& nextSimulationTime,
                                      Command& command) {
        if (_firstStep) {
            SetThreadAffinity(_serverName);
            _firstStep = false;
        }

        CheckResultWithMessage(Protocol::SendStep(_channel->GetWriter(), simulationTime, *_ioBuffer, *_busBuffer),
                               "Could not send step frame.");
        CheckResultWithMessage(WaitForStepOkFrame(nextSimulationTime, command), "Could not receive step ok frame");
        return Result::Ok;
    }

    [[nodiscard]] Result CloseConnection() {
        LogWarning("dSPACE VEOS CoSim client disconnected.");

        _channel.reset();

        if (!_isClientOptional && _callbacks.simulationStoppedCallback) {
            _callbacks.simulationStoppedCallback(milliseconds(0));
        }

        return StartAccepting();
    }

    [[nodiscard]] Result Ping(Command& command) const {
        CheckResultWithMessage(Protocol::SendPing(_channel->GetWriter()), "Could not send ping frame.");
        CheckResultWithMessage(WaitForPingOkFrame(command), "Could not receive ping ok frame.");
        return Result::Ok;
    }

    [[nodiscard]] Result StartAccepting() {
        uint16_t port{};
        if (!_tcpChannelServer) {
            CheckResult(CreateTcpChannelServer(_localPort, _enableRemoteAccess, _tcpChannelServer));
            port = _tcpChannelServer->GetLocalPort();
        }

        if (!_localChannelServer) {
#ifdef _WIN32
            CheckResult(CreateLocalChannelServer(_serverName, _localChannelServer));
#else
            CheckResult(CreateUdsChannelServer(_serverName, _localChannelServer));
#endif
        }

        if (port != 0) {
            if (_registerAtPortMapper) {
                if (!IsOk(PortMapperSetPort(_serverName, port))) {
                    LogTrace("Could not set port in port mapper.");
                }
            }

            std::string message = "dSPACE VEOS CoSim server '";
            message.append(_serverName);
            message.append("' is listening on ");
            message.append(_enableRemoteAccess ? "0.0.0.0" : "127.0.0.1");
            message.append(":");
            message.append(std::to_string(port));
            message.append(".");
            LogInfo(message);
        }

        return Result::Ok;
    }

    void StopAccepting() {
        if (_registerAtPortMapper) {
            (void)PortMapperUnsetPort(_serverName);
        }

        if (_tcpChannelServer) {
            _tcpChannelServer.reset();
        }

        if (_localChannelServer) {
            _localChannelServer.reset();
        }
    }

    [[nodiscard]] Result AcceptChannel() {
        if (_channel) {
            return Result::Ok;
        }

        if (_localChannelServer) {
            CheckResult(_localChannelServer->TryAccept(_channel));
            if (_channel) {
                _connectionKind = ConnectionKind::Local;
                _firstStep = true;
                return Result::Ok;
            }
        }

        if (_tcpChannelServer) {
            CheckResult(_tcpChannelServer->TryAccept(_channel));
            if (_channel) {
                _connectionKind = ConnectionKind::Remote;
                _firstStep = true;
                return Result::Ok;
            }
        }

        return Result::Error;
    }

    [[nodiscard]] Result OnHandleConnect() {
        uint32_t clientProtocolVersion{};
        std::string clientName;
        CheckResultWithMessage(WaitForConnectFrame(clientProtocolVersion, clientName),
                               "Could not receive connect frame.");

        CheckResultWithMessage(Protocol::SendConnectOk(_channel->GetWriter(),
                                                       CoSimProtocolVersion,
                                                       {},
                                                       _stepSize,
                                                       _simulationState,
                                                       _incomingSignals,
                                                       _outgoingSignals,
                                                       _canControllers,
                                                       _ethControllers,
                                                       _linControllers),
                               "Could not send connect ok frame.");

        std::vector<IoSignal> incomingSignalsExtern = Convert(_incomingSignals);
        std::vector<IoSignal> outgoingSignalsExtern = Convert(_outgoingSignals);
        CheckResult(CreateIoBuffer(CoSimType::Server,
                                   _connectionKind,
                                   _serverName,
                                   incomingSignalsExtern,
                                   outgoingSignalsExtern,
                                   _ioBuffer));

        std::vector<CanController> canControllersExtern = Convert(_canControllers);
        std::vector<EthController> ethControllersExtern = Convert(_ethControllers);
        std::vector<LinController> linControllersExtern = Convert(_linControllers);
        CheckResult(CreateBusBuffer(CoSimType::Server,
                                    _connectionKind,
                                    _serverName,
                                    canControllersExtern,
                                    ethControllersExtern,
                                    linControllersExtern,
                                    _busBuffer));

        StopAccepting();

        if (_connectionKind == ConnectionKind::Remote) {
            std::string remoteAddress;
            CheckResult(_channel->GetRemoteAddress(remoteAddress));
            if (clientName.empty()) {
                std::string message = "dSPACE VEOS CoSim client at ";
                message.append(remoteAddress);
                message.append(" connected.");
                LogInfo(message);
            } else {
                std::string message = "dSPACE VEOS CoSim client '";
                message.append(clientName);
                message.append("' at ");
                message.append(remoteAddress);
                message.append(" connected.");
                LogInfo(message);
            }
        } else {
            if (clientName.empty()) {
                LogInfo("Local dSPACE VEOS CoSim client connected.");
            } else {
                std::string message = "Local dSPACE VEOS CoSim client '";
                message.append(clientName);
                message.append("' connected.");
                LogInfo(message);
            }
        }

        return Result::Ok;
    }

    [[nodiscard]] Result WaitForOkFrame() const {
        FrameKind frameKind{};
        CheckResult(Protocol::ReceiveHeader(_channel->GetReader(), frameKind));

        switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
            case FrameKind::Ok:
                return Result::Ok;
            case FrameKind::Error: {
                std::string errorMessage;
                CheckResultWithMessage(Protocol::ReadError(_channel->GetReader(), errorMessage),
                                       "Could not read error frame.");
                LogError(errorMessage);
                return Result::Error;
            }
            default:
                std::string message = "Received unexpected frame '";
                message.append(ToString(frameKind));
                message.append("'.");
                LogError(message);
                return Result::Error;
        }
    }

    [[nodiscard]] Result WaitForPingOkFrame(Command& command) const {
        FrameKind frameKind{};
        CheckResult(Protocol::ReceiveHeader(_channel->GetReader(), frameKind));

        switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
            case FrameKind::PingOk:
                CheckResultWithMessage(Protocol::ReadPingOk(_channel->GetReader(), command),
                                       "Could not read ping ok frame.");
                return Result::Ok;
            default:
                std::string message = "Received unexpected frame '";
                message.append(ToString(frameKind));
                message.append("'.");
                LogError(message);
                return Result::Error;
        }
    }

    [[nodiscard]] Result WaitForConnectFrame(uint32_t& version, std::string& clientName) const {
        FrameKind frameKind{};
        CheckResult(Protocol::ReceiveHeader(_channel->GetReader(), frameKind));

        switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
            case FrameKind::Connect: {
                Mode mode{};
                std::string serverName;
                CheckResultWithMessage(
                    Protocol::ReadConnect(_channel->GetReader(), version, mode, serverName, clientName),
                    "Could not read connect frame.");
                return Result::Ok;
            }
            default:
                std::string message = "Received unexpected frame '";
                message.append(ToString(frameKind));
                message.append("'.");
                LogError(message);
                return Result::Error;
        }
    }

    [[nodiscard]] Result WaitForStepOkFrame(SimulationTime& simulationTime, Command& command) const {
        FrameKind frameKind{};
        CheckResult(Protocol::ReceiveHeader(_channel->GetReader(), frameKind));

        switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
            case FrameKind::StepOk:
                CheckResultWithMessage(Protocol::ReadStepOk(_channel->GetReader(),
                                                            simulationTime,
                                                            command,
                                                            *_ioBuffer,
                                                            *_busBuffer,
                                                            _callbacks),
                                       "Could not receive step ok frame.");
                return Result::Ok;
            case FrameKind::Error: {
                std::string errorMessage;
                CheckResultWithMessage(Protocol::ReadError(_channel->GetReader(), errorMessage),
                                       "Could not read error frame.");
                LogError(errorMessage);
                return Result::Error;
            }
            default:
                std::string message = "Received unexpected frame '";
                message.append(ToString(frameKind));
                message.append("'.");
                LogError(message);
                return Result::Error;
        }
    }

    void HandlePendingCommand(Command command) const {
        switch (command) {
            case Command::Start:
                _callbacks.simulationStartedCallback({});
                break;
            case Command::Stop:
                _callbacks.simulationStoppedCallback({});
                break;
            case Command::Terminate:
                _callbacks.simulationTerminatedCallback({}, TerminateReason::Error);
                break;
            case Command::Pause:
                _callbacks.simulationPausedCallback({});
                break;
            case Command::Continue:
                _callbacks.simulationContinuedCallback({});
                break;
            case Command::TerminateFinished:
                _callbacks.simulationTerminatedCallback({}, TerminateReason::Finished);
                break;
            case Command::None:
            case Command::Step:
            case Command::Ping:
                break;
        }
    }

    std::unique_ptr<Channel> _channel;

    uint16_t _localPort{};
    bool _enableRemoteAccess{};

    std::unique_ptr<PortMapperServer> _portMapperServer;
    std::unique_ptr<ChannelServer> _tcpChannelServer;
    std::unique_ptr<ChannelServer> _localChannelServer;

    ConnectionKind _connectionKind = ConnectionKind::Remote;
    std::string _serverName;
    Callbacks _callbacks{};
    bool _isClientOptional{};
    SimulationTime _stepSize{};
    SimulationState _simulationState{};
    bool _registerAtPortMapper{};

    bool _firstStep{true};

    std::vector<IoSignalContainer> _incomingSignals;
    std::vector<IoSignalContainer> _outgoingSignals;
    std::vector<CanControllerContainer> _canControllers;
    std::vector<EthControllerContainer> _ethControllers;
    std::vector<LinControllerContainer> _linControllers;
    std::unique_ptr<IoBuffer> _ioBuffer;
    std::unique_ptr<BusBuffer> _busBuffer;
};

}  // namespace

[[nodiscard]] Result CreateServer(std::unique_ptr<CoSimServer>& server) {
    server = std::make_unique<CoSimServerImpl>();
    return Result::Ok;
}

}  // namespace DsVeosCoSim
