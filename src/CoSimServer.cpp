// Copyright dSPACE GmbH. All rights reserved.

#include "CoSimServer.h"

#include <format>
#include <memory>
#include <thread>

#include "CoSimHelper.h"
#include "CoSimTypes.h"
#include "PortMapper.h"
#include "Protocol.h"
#include "Socket.h"
#include "SocketChannel.h"

namespace DsVeosCoSim {

CoSimServer::~CoSimServer() noexcept {
    Unload();
}

void CoSimServer::Load(const CoSimServerConfig& config) {
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
        _portMapperServer = std::make_unique<PortMapperServer>(_enableRemoteAccess);
    }

    StartAccepting();
}

void CoSimServer::Unload() {
    if (_channel) {
        _channel.reset();
    }

    StopAccepting();

    if (_portMapperServer) {
        _portMapperServer.reset();
    }
}

void CoSimServer::Start(DsVeosCoSim_SimulationTime simulationTime) {
    if (!_channel) {
        if (_isClientOptional) {
            return;
        }

        LogInfo("Waiting for dSPACE VEOS CoSim client to connect to dSPACE VEOS CoSim server '{}' ...", _serverName);

        while (!AcceptChannel()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
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

void CoSimServer::Stop(DsVeosCoSim_SimulationTime simulationTime) {
    if (!_channel) {
        return;
    }

    if (!StopInternal(simulationTime)) {
        CloseConnection();
    }
}

void CoSimServer::Terminate(DsVeosCoSim_SimulationTime simulationTime, DsVeosCoSim_TerminateReason reason) {
    if (!_channel) {
        return;
    }

    if (!TerminateInternal(simulationTime, reason)) {
        CloseConnection();
    }
}

void CoSimServer::Pause(DsVeosCoSim_SimulationTime simulationTime) {
    if (!_channel) {
        return;
    }

    if (!PauseInternal(simulationTime)) {
        CloseConnection();
    }
}

void CoSimServer::Continue(DsVeosCoSim_SimulationTime simulationTime) {
    if (!_channel) {
        return;
    }

    if (!ContinueInternal(simulationTime)) {
        CloseConnection();
    }
}

void CoSimServer::Step(DsVeosCoSim_SimulationTime simulationTime, DsVeosCoSim_SimulationTime& nextSimulationTime) {
    if (!_channel) {
        return;
    }

    Command command{};
    if (!StepInternal(simulationTime, nextSimulationTime, command)) {
        CloseConnection();
        return;
    }

    HandlePendingCommand(command);
}

void CoSimServer::Write(DsVeosCoSim_IoSignalId signalId, uint32_t length, const void* value) const {
    if (!_channel) {
        return;
    }

    _ioBuffer->Write(signalId, length, value);
}

void CoSimServer::Read(DsVeosCoSim_IoSignalId signalId, uint32_t& length, const void** value) const {
    if (!_channel) {
        return;
    }

    _ioBuffer->Read(signalId, length, value);
}

bool CoSimServer::Transmit(const DsVeosCoSim_CanMessage& message) const {
    if (!_channel) {
        return true;
    }

    return _busBuffer->Transmit(message);
}

bool CoSimServer::Transmit(const DsVeosCoSim_EthMessage& message) const {
    if (!_channel) {
        return true;
    }

    return _busBuffer->Transmit(message);
}

bool CoSimServer::Transmit(const DsVeosCoSim_LinMessage& message) const {
    if (!_channel) {
        return true;
    }

    return _busBuffer->Transmit(message);
}

void CoSimServer::BackgroundService() {
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

uint16_t CoSimServer::GetLocalPort() const {
    if (_tcpChannelServer) {
        return _tcpChannelServer->GetLocalPort();
    }

    return 0;
}

bool CoSimServer::StartInternal(DsVeosCoSim_SimulationTime simulationTime) const {
    CheckResultWithMessage(Protocol::SendStart(_channel->GetWriter(), simulationTime), "Could not send start frame.");
    CheckResultWithMessage(WaitForOkFrame(), "Could not receive ok frame.");
    return true;
}

bool CoSimServer::StopInternal(DsVeosCoSim_SimulationTime simulationTime) const {
    CheckResultWithMessage(Protocol::SendStop(_channel->GetWriter(), simulationTime), "Could not send stop frame.");
    CheckResultWithMessage(WaitForOkFrame(), "Could not receive ok frame.");
    return true;
}

bool CoSimServer::TerminateInternal(DsVeosCoSim_SimulationTime simulationTime,
                                    DsVeosCoSim_TerminateReason reason) const {
    CheckResultWithMessage(Protocol::SendTerminate(_channel->GetWriter(), simulationTime, reason),
                           "Could not send terminate frame.");
    CheckResultWithMessage(WaitForOkFrame(), "Could not receive ok frame.");
    return true;
}

bool CoSimServer::PauseInternal(DsVeosCoSim_SimulationTime simulationTime) const {
    CheckResultWithMessage(Protocol::SendPause(_channel->GetWriter(), simulationTime), "Could not send pause frame.");
    CheckResultWithMessage(WaitForOkFrame(), "Could not receive ok frame.");
    return true;
}

bool CoSimServer::ContinueInternal(DsVeosCoSim_SimulationTime simulationTime) const {
    CheckResultWithMessage(Protocol::SendContinue(_channel->GetWriter(), simulationTime),
                           "Could not send continue frame.");
    CheckResultWithMessage(WaitForOkFrame(), "Could not receive ok frame.");
    return true;
}

bool CoSimServer::StepInternal(DsVeosCoSim_SimulationTime simulationTime,
                               DsVeosCoSim_SimulationTime& nextSimulationTime,
                               Command& command) const {
    CheckResultWithMessage(Protocol::SendStep(_channel->GetWriter(), simulationTime, *_ioBuffer, *_busBuffer),
                           "Could not send step frame.");
    CheckResultWithMessage(WaitForStepOkFrame(nextSimulationTime, command), "Could not receive step ok frame");
    return true;
}

void CoSimServer::CloseConnection() {
    LogWarning("dSPACE VEOS CoSim client disconnected.");

    _channel.reset();

    if (!_isClientOptional && _callbacks.simulationStoppedCallback) {
        _callbacks.simulationStoppedCallback(0);
    }

    StartAccepting();
}

bool CoSimServer::Ping(Command& command) const {
    CheckResultWithMessage(Protocol::SendPing(_channel->GetWriter()), "Could not send ping frame.");
    CheckResultWithMessage(WaitForPingOkFrame(command), "Could not receive ping ok frame.");
    return true;
}

void CoSimServer::StartAccepting() {
    uint16_t port{};
    if (!_tcpChannelServer) {
        _tcpChannelServer = std::make_unique<TcpChannelServer>(_localPort, _enableRemoteAccess);
        port = _tcpChannelServer->GetLocalPort();
    }

#ifdef _WIN32
    if (!_localChannelServer) {
        _localChannelServer = std::make_unique<LocalChannelServer>(_serverName);
    }
#else
    if (!_udsChannelServer && Socket::IsUdsSupported()) {
        _udsChannelServer = std::make_unique<UdsChannelServer>(_serverName);
    }
#endif

    if (port != 0) {
        if (_registerAtPortMapper) {
            if (!PortMapper_SetPort(_serverName, port)) {
                LogTrace("Could not set port in port mapper.");
            }
        }

        std::string_view localIpAddress = _enableRemoteAccess ? "0.0.0.0" : "127.0.0.1";
        LogInfo("dSPACE VEOS CoSim server '{}' is listening on {}:{}.", _serverName, localIpAddress, port);
    }
}

void CoSimServer::StopAccepting() {
    if (_registerAtPortMapper) {
        if (!PortMapper_UnsetPort(_serverName)) {
            LogTrace("Could not unset port in port mapper.");
        }
    }

    if (_tcpChannelServer) {
        _tcpChannelServer.reset();
    }

#ifdef _WIN32
    if (_localChannelServer) {
        _localChannelServer.reset();
    }
#else
    if (_udsChannelServer) {
        _udsChannelServer.reset();
    }
#endif
}

bool CoSimServer::AcceptChannel() {
    if (_channel) {
        return true;
    }

#ifdef _WIN32
    if (_localChannelServer) {
        std::optional<LocalChannel> channel = _localChannelServer->TryAccept();
        if (channel) {
            _channel = std::make_unique<LocalChannel>(std::move(*channel));
            _connectionKind = ConnectionKind::Local;
            return true;
        }
    }
#else
    if (_udsChannelServer) {
        std::optional<SocketChannel> channel = _udsChannelServer->TryAccept();
        if (channel) {
            _channel = std::make_unique<SocketChannel>(std::move(*channel));
            _connectionKind = ConnectionKind::Local;
            return true;
        }
    }
#endif

    if (_tcpChannelServer) {
        std::optional<SocketChannel> channel = _tcpChannelServer->TryAccept();
        if (channel) {
            _channel = std::make_unique<SocketChannel>(std::move(*channel));
            _connectionKind = ConnectionKind::Remote;
            return true;
        }
    }

    return false;
}

bool CoSimServer::OnHandleConnect() {
    uint32_t clientProtocolVersion{};
    std::string clientName;
    CheckResultWithMessage(WaitForConnectFrame(clientProtocolVersion, clientName), "Could not receive connect frame.");

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

    std::vector<DsVeosCoSim_IoSignal> incomingSignalsExtern = Convert(_incomingSignals);
    std::vector<DsVeosCoSim_IoSignal> outgoingSignalsExtern = Convert(_outgoingSignals);
    _ioBuffer = std::make_unique<IoBuffer>(CoSimType::Server,
                                           _connectionKind,
                                           _serverName,
                                           incomingSignalsExtern,
                                           outgoingSignalsExtern);

    std::vector<DsVeosCoSim_CanController> canControllersExtern = Convert(_canControllers);
    std::vector<DsVeosCoSim_EthController> ethControllersExtern = Convert(_ethControllers);
    std::vector<DsVeosCoSim_LinController> linControllersExtern = Convert(_linControllers);
    _busBuffer = std::make_unique<BusBuffer>(CoSimType::Server,
                                             _connectionKind,
                                             _serverName,
                                             canControllersExtern,
                                             ethControllersExtern,
                                             linControllersExtern);

    StopAccepting();

    if (_connectionKind == ConnectionKind::Remote) {
        SocketAddress socketAddress = reinterpret_cast<SocketChannel*>(_channel.get())->GetRemoteAddress();
        if (clientName.empty()) {
            LogInfo("dSPACE VEOS CoSim client at {}:{} connected.", socketAddress.ipAddress, socketAddress.port);
        } else {
            LogInfo("dSPACE VEOS CoSim client '{}' at {}:{} connected.",
                    clientName,
                    socketAddress.ipAddress,
                    socketAddress.port);
        }
    } else {
        if (clientName.empty()) {
            LogInfo("Local dSPACE VEOS CoSim client connected.");
        } else {
            LogInfo("Local dSPACE VEOS CoSim client '{}' connected.", clientName);
        }
    }

    return true;
}

bool CoSimServer::WaitForOkFrame() const {
    FrameKind frameKind{};
    CheckResult(Protocol::ReceiveHeader(_channel->GetReader(), frameKind));

    switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
        case FrameKind::Ok:
            return true;
        case FrameKind::Error: {
            std::string errorMessage;
            CheckResultWithMessage(Protocol::ReadError(_channel->GetReader(), errorMessage),
                                   "Could not read error frame.");
            throw CoSimException(errorMessage);
        }
        default:
            throw CoSimException(std::format("Received unexpected frame {}.", ToString(frameKind)));
    }
}

bool CoSimServer::WaitForPingOkFrame(Command& command) const {
    FrameKind frameKind{};
    CheckResult(Protocol::ReceiveHeader(_channel->GetReader(), frameKind));

    switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
        case FrameKind::PingOk:
            CheckResultWithMessage(Protocol::ReadPingOk(_channel->GetReader(), command),
                                   "Could not read ping ok frame.");
            return true;
        default:
            throw CoSimException(std::format("Received unexpected frame {}.", ToString(frameKind)));
    }
}

bool CoSimServer::WaitForConnectFrame(uint32_t& version, std::string& clientName) const {
    FrameKind frameKind{};
    CheckResult(Protocol::ReceiveHeader(_channel->GetReader(), frameKind));

    switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
        case FrameKind::Connect: {
            Mode mode{};
            std::string serverName;
            CheckResultWithMessage(Protocol::ReadConnect(_channel->GetReader(), version, mode, serverName, clientName),
                                   "Could not read connect frame.");
            return true;
        }
        default:
            throw CoSimException(std::format("Received unexpected frame {}.", ToString(frameKind)));
    }
}

bool CoSimServer::WaitForStepOkFrame(DsVeosCoSim_SimulationTime& simulationTime, Command& command) const {
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
            return true;
        case FrameKind::Error: {
            std::string errorMessage;
            CheckResultWithMessage(Protocol::ReadError(_channel->GetReader(), errorMessage),
                                   "Could not read error frame.");
            throw CoSimException(errorMessage);
        }
        default:
            throw CoSimException(std::format("Received unexpected frame {}.", ToString(frameKind)));
    }
}

void CoSimServer::HandlePendingCommand(Command command) const {
    switch (command) {
        case Command::Start:
            _callbacks.simulationStartedCallback({});
            break;
        case Command::Stop:
            _callbacks.simulationStoppedCallback({});
            break;
        case Command::Terminate:
            _callbacks.simulationTerminatedCallback({}, DsVeosCoSim_TerminateReason_Error);
            break;
        case Command::Pause:
            _callbacks.simulationPausedCallback({});
            break;
        case Command::Continue:
            _callbacks.simulationContinuedCallback({});
            break;
        case Command::TerminateFinished:
            _callbacks.simulationTerminatedCallback({}, DsVeosCoSim_TerminateReason_Finished);
            break;
        case Command::None:
        case Command::Step:
        case Command::Ping:
        default:  // NOLINT(clang-diagnostic-covered-switch-default)
            break;
    }
}

}  // namespace DsVeosCoSim
