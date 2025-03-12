// Copyright dSPACE GmbH. All rights reserved.

#include "DsVeosCoSim/CoSimServer.h"

#include <chrono>
#include <cstdint>
#include <exception>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "BusBuffer.h"
#include "Channel.h"
#include "CoSimHelper.h"
#include "DsVeosCoSim/CoSimTypes.h"
#include "IoBuffer.h"
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

        SetLogCallback(config.logCallback);

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

    void Start(const SimulationTime simulationTime) override {
        if (!_channel) {
            if (_isClientOptional) {
                return;
            }

            LogInfo("Waiting for dSPACE VEOS CoSim client to connect to dSPACE VEOS CoSim server '" + _serverName +
                    "' ...");

            while (!AcceptChannel()) {
                std::this_thread::sleep_for(milliseconds(1));
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

    void Stop(const SimulationTime simulationTime) override {
        if (!_channel) {
            return;
        }

        if (!StopInternal(simulationTime)) {
            CloseConnection();
        }
    }

    void Terminate(const SimulationTime simulationTime, const TerminateReason reason) override {
        if (!_channel) {
            return;
        }

        if (!TerminateInternal(simulationTime, reason)) {
            CloseConnection();
        }
    }

    void Pause(const SimulationTime simulationTime) override {
        if (!_channel) {
            return;
        }

        if (!PauseInternal(simulationTime)) {
            CloseConnection();
        }
    }

    void Continue(const SimulationTime simulationTime) override {
        if (!_channel) {
            return;
        }

        if (!ContinueInternal(simulationTime)) {
            CloseConnection();
        }
    }

    SimulationTime Step(const SimulationTime simulationTime) override {
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

    void Write(const IoSignalId signalId, const uint32_t length, const void* value) const override {
        if (!_channel) {
            return;
        }

        _ioBuffer->Write(signalId, length, value);
    }

    void Read(const IoSignalId signalId, uint32_t& length, const void** value) const override {
        if (!_channel) {
            return;
        }

        _ioBuffer->Read(signalId, length, value);
    }

    [[nodiscard]] bool Transmit(const CanMessage& message) const override {
        if (!_channel) {
            return true;
        }

        return _busBuffer->Transmit(message);
    }

    [[nodiscard]] bool Transmit(const EthMessage& message) const override {
        if (!_channel) {
            return true;
        }

        return _busBuffer->Transmit(message);
    }

    [[nodiscard]] bool Transmit(const LinMessage& message) const override {
        if (!_channel) {
            return true;
        }

        return _busBuffer->Transmit(message);
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

        return 0;
    }

private:
    [[nodiscard]] bool StartInternal(const SimulationTime simulationTime) const {
        CheckResultWithMessage(Protocol::SendStart(_channel->GetWriter(), simulationTime),
                               "Could not send start frame.");
        CheckResultWithMessage(WaitForOkFrame(), "Could not receive ok frame.");
        return true;
    }

    [[nodiscard]] bool StopInternal(const SimulationTime simulationTime) const {
        CheckResultWithMessage(Protocol::SendStop(_channel->GetWriter(), simulationTime), "Could not send stop frame.");
        CheckResultWithMessage(WaitForOkFrame(), "Could not receive ok frame.");
        return true;
    }

    [[nodiscard]] bool TerminateInternal(const SimulationTime simulationTime, const TerminateReason reason) const {
        CheckResultWithMessage(Protocol::SendTerminate(_channel->GetWriter(), simulationTime, reason),
                               "Could not send terminate frame.");
        CheckResultWithMessage(WaitForOkFrame(), "Could not receive ok frame.");
        return true;
    }

    [[nodiscard]] bool PauseInternal(const SimulationTime simulationTime) const {
        CheckResultWithMessage(Protocol::SendPause(_channel->GetWriter(), simulationTime),
                               "Could not send pause frame.");
        CheckResultWithMessage(WaitForOkFrame(), "Could not receive ok frame.");
        return true;
    }

    [[nodiscard]] bool ContinueInternal(const SimulationTime simulationTime) const {
        CheckResultWithMessage(Protocol::SendContinue(_channel->GetWriter(), simulationTime),
                               "Could not send continue frame.");
        CheckResultWithMessage(WaitForOkFrame(), "Could not receive ok frame.");
        return true;
    }

    [[nodiscard]] bool StepInternal(const SimulationTime simulationTime,
                                    SimulationTime& nextSimulationTime,
                                    Command& command) const {
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

            const std::string localIpAddress = _enableRemoteAccess ? "0.0.0.0" : "127.0.0.1";
            LogInfo("dSPACE VEOS CoSim server '" + _serverName + "' is listening on " + localIpAddress + ":" +
                    std::to_string(port) + ".");
        }
    }

    void StopAccepting() noexcept {
        if (_registerAtPortMapper) {
            try {
                if (!PortMapper_UnsetPort(_serverName)) {
                    LogTrace("Could not unset port in port mapper.");
                }
            } catch (const std::exception& e) {
                LogTrace("Could not unset port in port mapper. Reason: " + std::string(e.what()));
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
                return true;
            }
        }

        if (_tcpChannelServer) {
            _channel = _tcpChannelServer->TryAccept();
            if (_channel) {
                _connectionKind = ConnectionKind::Remote;
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

        const std::vector<IoSignal> incomingSignalsExtern = Convert(_incomingSignals);
        const std::vector<IoSignal> outgoingSignalsExtern = Convert(_outgoingSignals);
        _ioBuffer = CreateIoBuffer(CoSimType::Server,
                                   _connectionKind,
                                   _serverName,
                                   incomingSignalsExtern,
                                   outgoingSignalsExtern);

        const std::vector<CanController> canControllersExtern = Convert(_canControllers);
        const std::vector<EthController> ethControllersExtern = Convert(_ethControllers);
        const std::vector<LinController> linControllersExtern = Convert(_linControllers);
        _busBuffer = CreateBusBuffer(CoSimType::Server,
                                     _connectionKind,
                                     _serverName,
                                     canControllersExtern,
                                     ethControllersExtern,
                                     linControllersExtern);

        StopAccepting();

        if (_connectionKind == ConnectionKind::Remote) {
            const std::string remoteAddress = _channel->GetRemoteAddress();
            if (clientName.empty()) {
                LogInfo("dSPACE VEOS CoSim client at " + remoteAddress + " connected.");
            } else {
                LogInfo("dSPACE VEOS CoSim client '" + clientName + "' at " + remoteAddress + " connected.");
            }
        } else {
            if (clientName.empty()) {
                LogInfo("Local dSPACE VEOS CoSim client connected.");
            } else {
                LogInfo("Local dSPACE VEOS CoSim client '" + clientName + "' connected.");
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
                throw CoSimException(errorMessage);
            }
            default:
                throw CoSimException("Received unexpected frame " + ToString(frameKind) + ".");
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
                throw CoSimException("Received unexpected frame " + ToString(frameKind) + ".");
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
                throw CoSimException("Received unexpected frame " + ToString(frameKind) + ".");
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
                throw CoSimException(errorMessage);
            }
            default:
                throw CoSimException("Received unexpected frame " + ToString(frameKind) + ".");
        }
    }

    void HandlePendingCommand(const Command command) const {
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
