// Copyright dSPACE GmbH. All rights reserved.

#include "CoSimServer.h"

#include <chrono>
#include <memory>
#include <string>
#include <thread>

#include "CoSimHelper.h"
#include "CoSimTypes.h"
#include "PortMapper.h"
#include "Protocol.h"
#include "Socket.h"
#include "SocketChannel.h"

using namespace std::chrono;

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

void CoSimServer::Start(const SimulationTime simulationTime) {
    if (!_channel) {
        if (_isClientOptional) {
            return;
        }

        LogInfo("Waiting for dSPACE VEOS CoSim client to connect to dSPACE VEOS CoSim server '" + _serverName +
                "' ...");

        while (!AcceptChannel()) {
            std::this_thread::sleep_for(1ms);
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

void CoSimServer::Stop(const SimulationTime simulationTime) {
    if (!_channel) {
        return;
    }

    if (!StopInternal(simulationTime)) {
        CloseConnection();
    }
}

void CoSimServer::Terminate(const SimulationTime simulationTime, const TerminateReason reason) {
    if (!_channel) {
        return;
    }

    if (!TerminateInternal(simulationTime, reason)) {
        CloseConnection();
    }
}

void CoSimServer::Pause(const SimulationTime simulationTime) {
    if (!_channel) {
        return;
    }

    if (!PauseInternal(simulationTime)) {
        CloseConnection();
    }
}

void CoSimServer::Continue(const SimulationTime simulationTime) {
    if (!_channel) {
        return;
    }

    if (!ContinueInternal(simulationTime)) {
        CloseConnection();
    }
}

void CoSimServer::Step(const SimulationTime simulationTime, SimulationTime& nextSimulationTime) {
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

void CoSimServer::Write(const IoSignalId signalId, const uint32_t length, const void* value) const {
    if (!_channel) {
        return;
    }

    _ioBuffer->Write(signalId, length, value);
}

void CoSimServer::Read(const IoSignalId signalId, uint32_t& length, const void** value) const {
    if (!_channel) {
        return;
    }

    _ioBuffer->Read(signalId, length, value);
}

[[nodiscard]] bool CoSimServer::Transmit(const CanMessage& message) const {
    if (!_channel) {
        return true;
    }

    return _busBuffer->Transmit(message);
}

[[nodiscard]] bool CoSimServer::Transmit(const EthMessage& message) const {
    if (!_channel) {
        return true;
    }

    return _busBuffer->Transmit(message);
}

[[nodiscard]] bool CoSimServer::Transmit(const LinMessage& message) const {
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

[[nodiscard]] uint16_t CoSimServer::GetLocalPort() const {
    if (_tcpChannelServer) {
        return _tcpChannelServer->GetLocalPort();
    }

    return 0;
}

[[nodiscard]] bool CoSimServer::StartInternal(const SimulationTime simulationTime) const {
    CheckResultWithMessage(Protocol::SendStart(_channel->GetWriter(), simulationTime), "Could not send start frame.");
    CheckResultWithMessage(WaitForOkFrame(), "Could not receive ok frame.");
    return true;
}

[[nodiscard]] bool CoSimServer::StopInternal(const SimulationTime simulationTime) const {
    CheckResultWithMessage(Protocol::SendStop(_channel->GetWriter(), simulationTime), "Could not send stop frame.");
    CheckResultWithMessage(WaitForOkFrame(), "Could not receive ok frame.");
    return true;
}

[[nodiscard]] bool CoSimServer::TerminateInternal(const SimulationTime simulationTime,
                                                  const TerminateReason reason) const {
    CheckResultWithMessage(Protocol::SendTerminate(_channel->GetWriter(), simulationTime, reason),
                           "Could not send terminate frame.");
    CheckResultWithMessage(WaitForOkFrame(), "Could not receive ok frame.");
    return true;
}

[[nodiscard]] bool CoSimServer::PauseInternal(const SimulationTime simulationTime) const {
    CheckResultWithMessage(Protocol::SendPause(_channel->GetWriter(), simulationTime), "Could not send pause frame.");
    CheckResultWithMessage(WaitForOkFrame(), "Could not receive ok frame.");
    return true;
}

[[nodiscard]] bool CoSimServer::ContinueInternal(const SimulationTime simulationTime) const {
    CheckResultWithMessage(Protocol::SendContinue(_channel->GetWriter(), simulationTime),
                           "Could not send continue frame.");
    CheckResultWithMessage(WaitForOkFrame(), "Could not receive ok frame.");
    return true;
}

[[nodiscard]] bool CoSimServer::StepInternal(const SimulationTime simulationTime,
                                             SimulationTime& nextSimulationTime,
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
        _callbacks.simulationStoppedCallback(0ns);
    }

    StartAccepting();
}

[[nodiscard]] bool CoSimServer::Ping(Command& command) const {
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

        const std::string localIpAddress = _enableRemoteAccess ? "0.0.0.0" : "127.0.0.1";
        LogInfo("dSPACE VEOS CoSim server '" + _serverName + "' is listening on " + localIpAddress + ":" +
                std::to_string(port) + ".");
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

[[nodiscard]] bool CoSimServer::AcceptChannel() {
    if (_channel) {
        return true;
    }

#ifdef _WIN32
    if (_localChannelServer) {
        if (std::optional<LocalChannel> channel = _localChannelServer->TryAccept()) {
            _channel = std::make_unique<LocalChannel>(std::move(*channel));
            _connectionKind = ConnectionKind::Local;
            return true;
        }
    }
#else
    if (_udsChannelServer) {
        if (std::optional<SocketChannel> channel = _udsChannelServer->TryAccept()) {
            _channel = std::make_unique<SocketChannel>(std::move(*channel));
            _connectionKind = ConnectionKind::Local;
            return true;
        }
    }
#endif

    if (_tcpChannelServer) {
        if (std::optional<SocketChannel> channel = _tcpChannelServer->TryAccept()) {
            _channel = std::make_unique<SocketChannel>(std::move(*channel));
            _connectionKind = ConnectionKind::Remote;
            return true;
        }
    }

    return false;
}

[[nodiscard]] bool CoSimServer::OnHandleConnect() {
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

    std::vector<IoSignal> incomingSignalsExtern = Convert(_incomingSignals);
    std::vector<IoSignal> outgoingSignalsExtern = Convert(_outgoingSignals);
    _ioBuffer = std::make_unique<IoBuffer>(CoSimType::Server,
                                           _connectionKind,
                                           _serverName,
                                           incomingSignalsExtern,
                                           outgoingSignalsExtern);

    std::vector<CanController> canControllersExtern = Convert(_canControllers);
    std::vector<EthController> ethControllersExtern = Convert(_ethControllers);
    std::vector<LinController> linControllersExtern = Convert(_linControllers);
    _busBuffer = std::make_unique<BusBuffer>(CoSimType::Server,
                                             _connectionKind,
                                             _serverName,
                                             canControllersExtern,
                                             ethControllersExtern,
                                             linControllersExtern);

    StopAccepting();

    if (_connectionKind == ConnectionKind::Remote) {
        const auto [ipAddress, port] = reinterpret_cast<SocketChannel*>(_channel.get())->GetRemoteAddress();
        if (clientName.empty()) {
            LogInfo("dSPACE VEOS CoSim client at " + ipAddress + ":" + std::to_string(port) + " connected.");
        } else {
            LogInfo("dSPACE VEOS CoSim client '" + clientName + "' at " + ipAddress + ":" + std::to_string(port) +
                    " connected.");
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

[[nodiscard]] bool CoSimServer::WaitForOkFrame() const {
    FrameKind frameKind{};
    CheckResult(Protocol::ReceiveHeader(_channel->GetReader(), frameKind));

    switch (frameKind) {  // NOLINT
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

[[nodiscard]] bool CoSimServer::WaitForPingOkFrame(Command& command) const {
    FrameKind frameKind{};
    CheckResult(Protocol::ReceiveHeader(_channel->GetReader(), frameKind));

    switch (frameKind) {  // NOLINT
        case FrameKind::PingOk:
            CheckResultWithMessage(Protocol::ReadPingOk(_channel->GetReader(), command),
                                   "Could not read ping ok frame.");
            return true;
        default:
            throw CoSimException("Received unexpected frame " + ToString(frameKind) + ".");
    }
}

[[nodiscard]] bool CoSimServer::WaitForConnectFrame(uint32_t& version, std::string& clientName) const {
    FrameKind frameKind{};
    CheckResult(Protocol::ReceiveHeader(_channel->GetReader(), frameKind));

    switch (frameKind) {  // NOLINT
        case FrameKind::Connect: {
            Mode mode{};
            std::string serverName;
            CheckResultWithMessage(Protocol::ReadConnect(_channel->GetReader(), version, mode, serverName, clientName),
                                   "Could not read connect frame.");
            return true;
        }
        default:
            throw CoSimException("Received unexpected frame " + ToString(frameKind) + ".");
    }
}

[[nodiscard]] bool CoSimServer::WaitForStepOkFrame(SimulationTime& simulationTime, Command& command) const {
    FrameKind frameKind{};
    CheckResult(Protocol::ReceiveHeader(_channel->GetReader(), frameKind));

    switch (frameKind) {  // NOLINT
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

void CoSimServer::HandlePendingCommand(const Command command) const {
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

}  // namespace DsVeosCoSim
