// Copyright dSPACE GmbH. All rights reserved.

#include "DsVeosCoSim/CoSimServer.h"

#include <chrono>
#include <cstdint>
#include <exception>
#include <memory>
#include <stdexcept>
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
    CoSimServerImpl() noexcept = default;

    ~CoSimServerImpl() noexcept override {
        Unload();
    }

    CoSimServerImpl(const CoSimServerImpl&) = delete;
    CoSimServerImpl& operator=(const CoSimServerImpl&) = delete;

    CoSimServerImpl(CoSimServerImpl&&) = delete;
    CoSimServerImpl& operator=(CoSimServerImpl&&) = delete;

    void Load(const CoSimServerConfig& config) override {
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
        _callbacks.canMessageReceivedCallback = config.canMessageReceivedCallback;
        _callbacks.linMessageReceivedCallback = config.linMessageReceivedCallback;
        _callbacks.ethMessageReceivedCallback = config.ethMessageReceivedCallback;

        if (config.startPortMapper) {
            _portMapperServer = CreatePortMapperServer(_enableRemoteAccess);
        }

        StartAccepting();
    }

    void Unload() noexcept override {
        if (_channel) {
            _channel.reset();
        }

        StopAccepting();

        if (_portMapperServer) {
            _portMapperServer.reset();
        }
    }

    void Start(SimulationTime simulationTime) override {
        if (!_channel) {
            if (_isClientOptional) {
                return;
            }

            std::string message = "Waiting for dSPACE VEOS CoSim client to connect to dSPACE VEOS CoSim server '";
            message.append(_serverName);
            message.append("' ...");
            LogInfo(message);

            while (!AcceptChannel()) {
                std::this_thread::sleep_for(milliseconds(100));
            }

            if (!OnHandleConnect()) {
                CloseConnection();
                return;
            }
        }

        if (!StartInternal(simulationTime)) {
            CloseConnection();
        }
    }

    void Stop(SimulationTime simulationTime) override {
        if (!_channel) {
            return;
        }

        if (!StopInternal(simulationTime)) {
            CloseConnection();
        }
    }

    void Terminate(SimulationTime simulationTime, TerminateReason reason) override {
        if (!_channel) {
            return;
        }

        if (!TerminateInternal(simulationTime, reason)) {
            CloseConnection();
        }
    }

    void Pause(SimulationTime simulationTime) override {
        if (!_channel) {
            return;
        }

        if (!PauseInternal(simulationTime)) {
            CloseConnection();
        }
    }

    void Continue(SimulationTime simulationTime) override {
        if (!_channel) {
            return;
        }

        if (!ContinueInternal(simulationTime)) {
            CloseConnection();
        }
    }

    [[nodiscard]] SimulationTime Step(SimulationTime simulationTime) override {
        if (!_channel) {
            return {};
        }

        Command command{};
        SimulationTime nextSimulationTime{};
        if (!StepInternal(simulationTime, nextSimulationTime, command)) {
            CloseConnection();
            return {};
        }

        HandlePendingCommand(command);
        return nextSimulationTime;
    }

    void Write(IoSignalId signalId, uint32_t length, const void* value) const override {
        if (!_channel) {
            return;
        }

        _ioBuffer->Write(signalId, length, value);
    }

    [[nodiscard]] bool Read(IoSignalId signalId, uint32_t& length, const void** value) const override {
        if (!_channel) {
            return false;
        }

        _ioBuffer->Read(signalId, length, value);
        return true;
    }

    void Transmit(const CanMessage& message) const override {
        if (!_channel) {
            return;
        }

        (void)_busBuffer->Transmit(message);
    }

    void Transmit(const EthMessage& message) const override {
        if (!_channel) {
            return;
        }

        (void)_busBuffer->Transmit(message);
    }

    void Transmit(const LinMessage& message) const override {
        if (!_channel) {
            return;
        }

        (void)_busBuffer->Transmit(message);
    }

    void BackgroundService() override {
        if (!_channel) {
            if (AcceptChannel()) {
                if (!OnHandleConnect()) {
                    CloseConnection();
                    return;
                }
            }

            return;
        }

        Command command{};
        if (!Ping(command)) {
            CloseConnection();
            return;
        }

        HandlePendingCommand(command);
    }

    [[nodiscard]] uint16_t GetLocalPort() const override {
        if (_tcpChannelServer) {
            return _tcpChannelServer->GetLocalPort();
        }

        return {};
    }

private:
    [[nodiscard]] bool StartInternal(SimulationTime simulationTime) const {
        CheckResultWithMessage(Protocol::SendStart(_channel->GetWriter(), simulationTime),
                               "Could not send start frame.");
        CheckResultWithMessage(WaitForOkFrame(), "Could not receive ok frame.");
        return true;
    }

    [[nodiscard]] bool StopInternal(SimulationTime simulationTime) const {
        CheckResultWithMessage(Protocol::SendStop(_channel->GetWriter(), simulationTime), "Could not send stop frame.");
        CheckResultWithMessage(WaitForOkFrame(), "Could not receive ok frame.");
        return true;
    }

    [[nodiscard]] bool TerminateInternal(SimulationTime simulationTime, TerminateReason reason) const {
        CheckResultWithMessage(Protocol::SendTerminate(_channel->GetWriter(), simulationTime, reason),
                               "Could not send terminate frame.");
        CheckResultWithMessage(WaitForOkFrame(), "Could not receive ok frame.");
        return true;
    }

    [[nodiscard]] bool PauseInternal(SimulationTime simulationTime) const {
        CheckResultWithMessage(Protocol::SendPause(_channel->GetWriter(), simulationTime),
                               "Could not send pause frame.");
        CheckResultWithMessage(WaitForOkFrame(), "Could not receive ok frame.");
        return true;
    }

    [[nodiscard]] bool ContinueInternal(SimulationTime simulationTime) const {
        CheckResultWithMessage(Protocol::SendContinue(_channel->GetWriter(), simulationTime),
                               "Could not send continue frame.");
        CheckResultWithMessage(WaitForOkFrame(), "Could not receive ok frame.");
        return true;
    }

    [[nodiscard]] bool StepInternal(SimulationTime simulationTime,
                                    SimulationTime& nextSimulationTime,
                                    Command& command) {
        if (_firstStep) {
            SetThreadAffinity(_serverName);
            _firstStep = false;
        }

        CheckResultWithMessage(Protocol::SendStep(_channel->GetWriter(), simulationTime, *_ioBuffer, *_busBuffer),
                               "Could not send step frame.");
        CheckResultWithMessage(WaitForStepOkFrame(nextSimulationTime, command), "Could not receive step ok frame");
        return true;
    }

    void CloseConnection() {
        LogWarning("dSPACE VEOS CoSim client disconnected.");

        _channel.reset();

        if (!_isClientOptional && _callbacks.simulationStoppedCallback) {
            _callbacks.simulationStoppedCallback(milliseconds(0));
        }

        StartAccepting();
    }

    [[nodiscard]] bool Ping(Command& command) const {
        CheckResultWithMessage(Protocol::SendPing(_channel->GetWriter()), "Could not send ping frame.");
        CheckResultWithMessage(WaitForPingOkFrame(command), "Could not receive ping ok frame.");
        return true;
    }

    void StartAccepting() {
        uint16_t port{};
        if (!_tcpChannelServer) {
            _tcpChannelServer = CreateTcpChannelServer(_localPort, _enableRemoteAccess);
            port = _tcpChannelServer->GetLocalPort();
        }

        if (!_localChannelServer) {
#ifdef _WIN32
            _localChannelServer = CreateLocalChannelServer(_serverName);
#else
            _localChannelServer = CreateUdsChannelServer(_serverName);
#endif
        }

        if (port != 0) {
            if (_registerAtPortMapper) {
                if (!PortMapper_SetPort(_serverName, port)) {
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
    }

    void StopAccepting() noexcept {
        if (_registerAtPortMapper) {
            try {
                if (!PortMapper_UnsetPort(_serverName)) {
                    LogTrace("Could not unset port in port mapper.");
                }
            } catch (const std::exception& e) {
                std::string message = "Could not unset port in port mapper. Reason: ";
                message.append(e.what());
                LogTrace(message);
            }
        }

        if (_tcpChannelServer) {
            _tcpChannelServer.reset();
        }

        if (_localChannelServer) {
            _localChannelServer.reset();
        }
    }

    [[nodiscard]] bool AcceptChannel() {
        if (_channel) {
            return true;
        }

        if (_localChannelServer) {
            _channel = _localChannelServer->TryAccept();
            if (_channel) {
                _connectionKind = ConnectionKind::Local;
                _firstStep = true;
                return true;
            }
        }

        if (_tcpChannelServer) {
            _channel = _tcpChannelServer->TryAccept();
            if (_channel) {
                _connectionKind = ConnectionKind::Remote;
                _firstStep = true;
                return true;
            }
        }

        return false;
    }

    [[nodiscard]] bool OnHandleConnect() {
        uint32_t clientProtocolVersion{};
        std::string clientName;
        CheckResultWithMessage(WaitForConnectFrame(clientProtocolVersion, clientName),
                               "Could not receive connect frame.");

        CheckResultWithMessage(Protocol::SendConnectOk(_channel->GetWriter(),
                                                       CoSimProtocolVersion,
                                                       {},
                                                       _stepSize,
                                                       {},
                                                       _incomingSignals,
                                                       _outgoingSignals,
                                                       _canControllers,
                                                       _ethControllers,
                                                       _linControllers),
                               "Could not send connect ok frame.");

        std::vector<IoSignal> incomingSignalsExtern = Convert(_incomingSignals);
        std::vector<IoSignal> outgoingSignalsExtern = Convert(_outgoingSignals);
        _ioBuffer = CreateIoBuffer(CoSimType::Server,
                                   _connectionKind,
                                   _serverName,
                                   incomingSignalsExtern,
                                   outgoingSignalsExtern);

        std::vector<CanController> canControllersExtern = Convert(_canControllers);
        std::vector<EthController> ethControllersExtern = Convert(_ethControllers);
        std::vector<LinController> linControllersExtern = Convert(_linControllers);
        _busBuffer = CreateBusBuffer(CoSimType::Server,
                                     _connectionKind,
                                     _serverName,
                                     canControllersExtern,
                                     ethControllersExtern,
                                     linControllersExtern);

        StopAccepting();

        if (_connectionKind == ConnectionKind::Remote) {
            std::string remoteAddress = _channel->GetRemoteAddress();
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

        return true;
    }

    [[nodiscard]] bool WaitForOkFrame() const {
        FrameKind frameKind{};
        CheckResult(Protocol::ReceiveHeader(_channel->GetReader(), frameKind));

        switch (frameKind) {
            case FrameKind::Ok:
                return true;
            case FrameKind::Error: {
                std::string errorMessage;
                CheckResultWithMessage(Protocol::ReadError(_channel->GetReader(), errorMessage),
                                       "Could not read error frame.");
                throw std::runtime_error(errorMessage);
            }
            default:
                std::string message = "Received unexpected frame '";
                message.append(ToString(frameKind));
                message.append("'.");
                throw std::runtime_error(message);
        }
    }

    [[nodiscard]] bool WaitForPingOkFrame(Command& command) const {
        FrameKind frameKind{};
        CheckResult(Protocol::ReceiveHeader(_channel->GetReader(), frameKind));

        switch (frameKind) {
            case FrameKind::PingOk:
                CheckResultWithMessage(Protocol::ReadPingOk(_channel->GetReader(), command),
                                       "Could not read ping ok frame.");
                return true;
            default:
                std::string message = "Received unexpected frame '";
                message.append(ToString(frameKind));
                message.append("'.");
                throw std::runtime_error(message);
        }
    }

    [[nodiscard]] bool WaitForConnectFrame(uint32_t& version, std::string& clientName) const {
        FrameKind frameKind{};
        CheckResult(Protocol::ReceiveHeader(_channel->GetReader(), frameKind));

        switch (frameKind) {
            case FrameKind::Connect: {
                Mode mode{};
                std::string serverName;
                CheckResultWithMessage(
                    Protocol::ReadConnect(_channel->GetReader(), version, mode, serverName, clientName),
                    "Could not read connect frame.");
                return true;
            }
            default:
                std::string message = "Received unexpected frame '";
                message.append(ToString(frameKind));
                message.append("'.");
                throw std::runtime_error(message);
        }
    }

    [[nodiscard]] bool WaitForStepOkFrame(SimulationTime& simulationTime, Command& command) const {
        FrameKind frameKind{};
        CheckResult(Protocol::ReceiveHeader(_channel->GetReader(), frameKind));

        switch (frameKind) {
            case FrameKind::StepOk:
                CheckResultWithMessage(Protocol::ReadStepOk(_channel->GetReader(),
                                                            simulationTime,
                                                            command,
                                                            *_ioBuffer,
                                                            *_busBuffer,
                                                            _callbacks),
                                       "Could not receive step ok frame.");
                return true;
            case FrameKind::Error: {
                std::string errorMessage;
                CheckResultWithMessage(Protocol::ReadError(_channel->GetReader(), errorMessage),
                                       "Could not read error frame.");
                throw std::runtime_error(errorMessage);
            }
            default:
                std::string message = "Received unexpected frame '";
                message.append(ToString(frameKind));
                message.append("'.");
                throw std::runtime_error(message);
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

std::unique_ptr<CoSimServer> CreateServer() {
    return std::make_unique<CoSimServerImpl>();
}

}  // namespace DsVeosCoSim
