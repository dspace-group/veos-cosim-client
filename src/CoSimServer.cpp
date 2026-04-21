// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "CoSimServer.hpp"

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "BusExchange.hpp"
#include "Channel.hpp"
#include "CoSimTypes.hpp"
#include "Logger.hpp"
#include "OsUtilities.hpp"
#include "PortMapper.hpp"
#include "Protocol.hpp"
#include "Result.hpp"
#include "SignalExchange.hpp"

using namespace std::chrono;

namespace DsVeosCoSim {

CoSimServer::CoSimServer() {
    _serializeIoData = [this](ChannelWriter& writer) {
        return _signalExchange->Serialize(writer);
    };

    _serializeBusMessages = [this](ChannelWriter& writer) {
        return _busExchange->Serialize(writer);
    };

    _deserializeIoData = [this](ChannelReader& reader, SimulationTime simulationTime, const Callbacks& callbacks) {
        return _signalExchange->Deserialize(reader, simulationTime, callbacks);
    };

    _deserializeBusMessages = [this](ChannelReader& reader, SimulationTime simulationTime, const Callbacks& callbacks) {
        return _busExchange->Deserialize(reader, simulationTime, callbacks);
    };
}

CoSimServer::~CoSimServer() noexcept {
    Unload();
}

Result CoSimServer::Load(const CoSimServerConfig& config) {
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
    _frControllers = config.frControllers;

    _callbacks.simulationStartedCallback = config.simulationStartedCallback;
    _callbacks.simulationStoppedCallback = config.simulationStoppedCallback;
    _callbacks.simulationPausedCallback = config.simulationPausedCallback;
    _callbacks.simulationContinuedCallback = config.simulationContinuedCallback;
    _callbacks.simulationTerminatedCallback = config.simulationTerminatedCallback;
    _callbacks.incomingSignalChangedCallback = config.incomingSignalChangedCallback;
    _callbacks.canMessageContainerReceivedCallback = config.canMessageContainerReceivedCallback;
    _callbacks.linMessageContainerReceivedCallback = config.linMessageContainerReceivedCallback;
    _callbacks.frMessageContainerReceivedCallback = config.frMessageContainerReceivedCallback;
    _callbacks.ethMessageContainerReceivedCallback = config.ethMessageContainerReceivedCallback;

    CheckResult(CreateProtocol(ProtocolVersion1, _protocol));

    if (config.startPortMapper) {
        CheckResult(CreatePortMapperServer(_enableRemoteAccess, _portMapperServer));
    }

    CheckResult(StartAccepting());
    _simulationState = SimulationState::Stopped;
    return CreateOk();
}

void CoSimServer::Unload() {
    _simulationState = SimulationState::Unloaded;

    if (_channel) {
        _channel.reset();
    }

    StopAccepting();

    if (_portMapperServer) {
        _portMapperServer.reset();
    }
}

Result CoSimServer::Start(SimulationTime simulationTime) {
    _simulationState = SimulationState::Running;

    if (!_channel) {
        if (_isClientOptional) {
            return CreateOk();
        }

        LogInfo("Waiting for dSPACE VEOS CoSim client to connect to dSPACE VEOS CoSim server '{}' ...", _serverName);

        while (true) {
            Result result = AcceptChannel();
            if (IsOk(result)) {
                break;
            }

            if (IsNotConnected(result)) {
                std::this_thread::sleep_for(milliseconds(1));
                continue;
            }

            return result;
        }

        if (!IsOk(OnHandleConnect())) {
            return CloseConnection();
        }
    }

    if (!IsOk(StartInternal(simulationTime))) {
        return CloseConnection();
    }

    return CreateOk();
}

Result CoSimServer::Stop(SimulationTime simulationTime) {
    _simulationState = SimulationState::Stopped;

    if (!_channel) {
        return CreateOk();
    }

    if (!IsOk(StopInternal(simulationTime))) {
        return CloseConnection();
    }

    return CreateOk();
}

Result CoSimServer::Terminate(SimulationTime simulationTime, TerminateReason reason) {
    _simulationState = SimulationState::Terminated;

    if (!_channel) {
        return CreateOk();
    }

    if (!IsOk(TerminateInternal(simulationTime, reason))) {
        return CloseConnection();
    }

    return CreateOk();
}

Result CoSimServer::Pause(SimulationTime simulationTime) {
    _simulationState = SimulationState::Paused;

    if (!_channel) {
        return CreateOk();
    }

    if (!IsOk(PauseInternal(simulationTime))) {
        return CloseConnection();
    }

    return CreateOk();
}

Result CoSimServer::Continue(SimulationTime simulationTime) {
    _simulationState = SimulationState::Running;

    if (!_channel) {
        return CreateOk();
    }

    if (!IsOk(ContinueInternal(simulationTime))) {
        return CloseConnection();
    }

    return CreateOk();
}

Result CoSimServer::Step(SimulationTime simulationTime, SimulationTime& nextSimulationTime) {
    nextSimulationTime = {};

    if (!_channel) {
        return CreateOk();
    }

    Command command{};
    if (!IsOk(StepInternal(simulationTime, nextSimulationTime, command))) {
        return CloseConnection();
    }

    HandlePendingCommand(command);
    return CreateOk();
}

Result CoSimServer::Write(IoSignalId signalId, uint32_t length, const void* value) const {
    if (!_channel) {
        return CreateOk();
    }

    return _signalExchange->Write(signalId, length, value);
}

Result CoSimServer::Read(IoSignalId signalId, uint32_t& length, const void** value, bool& valueRead) const {
    if (!_channel) {
        valueRead = false;
        return CreateOk();
    }

    valueRead = true;
    return _signalExchange->Read(signalId, length, value);
}

Result CoSimServer::Transmit(const CanMessage& message) const {
    if (!_channel) {
        return CreateOk();
    }

    return _busExchange->Transmit(message);
}

Result CoSimServer::Transmit(const EthMessage& message) const {
    if (!_channel) {
        return CreateOk();
    }

    return _busExchange->Transmit(message);
}

Result CoSimServer::Transmit(const LinMessage& message) const {
    if (!_channel) {
        return CreateOk();
    }

    return _busExchange->Transmit(message);
}

Result CoSimServer::Transmit(const FrMessage& message) const {
    if (!_channel) {
        return CreateOk();
    }

    return _busExchange->Transmit(message);
}

Result CoSimServer::Transmit(const CanMessageContainer& messageContainer) const {
    if (!_channel) {
        return CreateOk();
    }

    return _busExchange->Transmit(messageContainer);
}

Result CoSimServer::Transmit(const EthMessageContainer& messageContainer) const {
    if (!_channel) {
        return CreateOk();
    }

    return _busExchange->Transmit(messageContainer);
}

Result CoSimServer::Transmit(const LinMessageContainer& messageContainer) const {
    if (!_channel) {
        return CreateOk();
    }

    return _busExchange->Transmit(messageContainer);
}

Result CoSimServer::Transmit(const FrMessageContainer& messageContainer) const {
    if (!_channel) {
        return CreateOk();
    }

    return _busExchange->Transmit(messageContainer);
}

Result CoSimServer::BackgroundService(SimulationTime& roundTripTime) {
    roundTripTime = {};
    if (!_channel) {
        Result result = AcceptChannel();
        if (IsNotConnected(result)) {
            return CreateOk();
        }

        if (!IsOk(result)) {
            return result;
        }

        if (!IsOk(OnHandleConnect())) {
            return CloseConnection();
        }

        return CreateOk();
    }

    Command command{};
    if (!IsOk(Ping(command))) {
        return CloseConnection();
    }

    roundTripTime = _roundTripTime;
    HandlePendingCommand(command);
    return CreateOk();
}

Result CoSimServer::GetLocalPort(uint16_t& localPort) const {
    if (_tcpChannelServer) {
        localPort = _tcpChannelServer->GetLocalPort();
        return CreateOk();
    }

    localPort = 0;
    return CreateError();
}

Result CoSimServer::StartInternal(SimulationTime simulationTime) {
    CheckResultWithMessage(_protocol->SendStart(_channel->GetWriter(), simulationTime), "Could not send start frame.");
    CheckResultWithMessage(WaitForOkFrame(), "Could not receive ok frame.");
    return CreateOk();
}

Result CoSimServer::StopInternal(SimulationTime simulationTime) {
    CheckResultWithMessage(_protocol->SendStop(_channel->GetWriter(), simulationTime), "Could not send stop frame.");
    CheckResultWithMessage(WaitForOkFrame(), "Could not receive ok frame.");
    return CreateOk();
}

Result CoSimServer::TerminateInternal(SimulationTime simulationTime, TerminateReason reason) {
    CheckResultWithMessage(_protocol->SendTerminate(_channel->GetWriter(), simulationTime, reason), "Could not send terminate frame.");
    CheckResultWithMessage(WaitForOkFrame(), "Could not receive ok frame.");
    return CreateOk();
}

Result CoSimServer::PauseInternal(SimulationTime simulationTime) {
    CheckResultWithMessage(_protocol->SendPause(_channel->GetWriter(), simulationTime), "Could not send pause frame.");
    CheckResultWithMessage(WaitForOkFrame(), "Could not receive ok frame.");
    return CreateOk();
}

Result CoSimServer::ContinueInternal(SimulationTime simulationTime) {
    CheckResultWithMessage(_protocol->SendContinue(_channel->GetWriter(), simulationTime), "Could not send continue frame.");
    CheckResultWithMessage(WaitForOkFrame(), "Could not receive ok frame.");
    return CreateOk();
}

Result CoSimServer::StepInternal(SimulationTime simulationTime, SimulationTime& nextSimulationTime, Command& command) {
    if (_firstStep) {
        SetThreadAffinity(_serverName);
        _firstStep = false;
    }

    CheckResultWithMessage(_protocol->SendStep(_channel->GetWriter(), simulationTime, _serializeIoData, _serializeBusMessages), "Could not send step frame.");
    CheckResultWithMessage(WaitForStepOkFrame(nextSimulationTime, command), "Could not receive step ok frame");
    return CreateOk();
}

Result CoSimServer::CloseConnection() {
    LogWarning("dSPACE VEOS CoSim client disconnected.");

    _channel.reset();

    if (!_isClientOptional && _callbacks.simulationStoppedCallback) {
        _callbacks.simulationStoppedCallback(SimulationTime{});
    }

    return StartAccepting();
}

Result CoSimServer::Ping(Command& command) {
    auto start = high_resolution_clock::now();
    CheckResultWithMessage(_protocol->SendPing(_channel->GetWriter(), _roundTripTime), "Could not send ping frame.");
    CheckResultWithMessage(WaitForPingOkFrame(command), "Could not receive ping ok frame.");
    auto stop = high_resolution_clock::now();
    _roundTripTime = duration_cast<nanoseconds>(stop - start);
    return CreateOk();
}

Result CoSimServer::StartAccepting() {
    uint16_t port{};
    if (!_tcpChannelServer) {
        CheckResult(CreateTcpChannelServer(_localPort, _enableRemoteAccess, _tcpChannelServer));
        port = _tcpChannelServer->GetLocalPort();
    }

    if (!_localChannelServer) {
        CheckResult(CreateLocalChannelServer(_serverName, _localChannelServer));
    }

    if (port != 0) {
        if (_registerAtPortMapper) {
            if (!IsOk(PortMapperSetPort(_serverName, port))) {
                LogTrace("Could not set port in port mapper.");
            }
        }

        std::string_view address = _enableRemoteAccess ? "0.0.0.0" : "127.0.0.1";

        LogInfo("dSPACE VEOS CoSim server '{}' is listening on {}:{}.", _serverName, address, port);
    }

    return CreateOk();
}

void CoSimServer::StopAccepting() {
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

Result CoSimServer::AcceptChannel() {
    if (_channel) {
        return CreateOk();
    }

    if (_localChannelServer) {
        Result result = _localChannelServer->TryAccept(_channel);
        if (IsOk(result)) {
            _connectionKind = ConnectionKind::Local;
            _firstStep = true;
            return CreateOk();
        }

        if (IsError(result)) {
            return result;
        }
    }

    if (_tcpChannelServer) {
        CheckResult(_tcpChannelServer->TryAccept(_channel));
        _connectionKind = ConnectionKind::Remote;
        _firstStep = true;
        return CreateOk();
    }

    LogError("No server found.");
    return CreateError();
}

Result CoSimServer::OnHandleConnect() {
    uint32_t clientProtocolVersion{};
    std::string clientName;
    uint32_t coSimProtocolVersion = ProtocolVersion1;
    CheckResultWithMessage(WaitForConnectFrame(clientProtocolVersion, clientName), "Could not receive connect frame.");

    if (clientProtocolVersion >= ProtocolVersionLatest) {
        coSimProtocolVersion = ProtocolVersionLatest;
    } else {
        coSimProtocolVersion = clientProtocolVersion;
    }

    if (_protocol->GetVersion() != coSimProtocolVersion) {
        CheckResult(CreateProtocol(coSimProtocolVersion, _protocol));
    }

    CheckResultWithMessage(_protocol->SendConnectOk(_channel->GetWriter(),
                                                    coSimProtocolVersion,
                                                    {},
                                                    _stepSize,
                                                    _simulationState,
                                                    _incomingSignals,
                                                    _outgoingSignals,
                                                    _canControllers,
                                                    _ethControllers,
                                                    _linControllers,
                                                    _frControllers),
                           "Could not send connect ok frame.");

    std::vector<IoSignal> incomingSignalsExtern = Convert(_incomingSignals);
    std::vector<IoSignal> outgoingSignalsExtern = Convert(_outgoingSignals);
    CheckResult(
        CreateSignalExchange(CoSimType::Server, _connectionKind, _serverName, incomingSignalsExtern, outgoingSignalsExtern, *_protocol, _signalExchange));

    std::vector<CanController> canControllersExtern = Convert(_canControllers);
    std::vector<EthController> ethControllersExtern = Convert(_ethControllers);
    std::vector<LinController> linControllersExtern = Convert(_linControllers);
    std::vector<FrController> frControllersExtern = Convert(_frControllers);
    CheckResult(CreateBusExchange(CoSimType::Server,
                                  _connectionKind,
                                  _serverName,
                                  canControllersExtern,
                                  ethControllersExtern,
                                  linControllersExtern,
                                  frControllersExtern,
                                  *_protocol,
                                  _busExchange));

    StopAccepting();

    if (_connectionKind == ConnectionKind::Remote) {
        std::string remoteAddress;
        CheckResult(_channel->GetRemoteAddress(remoteAddress));
        if (clientName.empty()) {
            LogInfo("dSPACE VEOS CoSim client at {} connected.", remoteAddress);
        } else {
            LogInfo("dSPACE VEOS CoSim client '{}' at {} connected.", clientName, remoteAddress);
        }
    } else {
        if (clientName.empty()) {
            LogInfo("Local dSPACE VEOS CoSim client connected.");
        } else {
            LogInfo("Local dSPACE VEOS CoSim client '{}' connected.", clientName);
        }
    }

    return CreateOk();
}

Result CoSimServer::WaitForOkFrame() const {
    FrameKind frameKind{};
    CheckResult(_protocol->ReceiveHeader(_channel->GetReader(), frameKind));

    switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
        case FrameKind::Ok:
            CheckResultWithMessage(_protocol->ReadOk(_channel->GetReader()), "Could not read ok frame.");
            return CreateOk();
        case FrameKind::Error:
            return OnError();
        default:
            return OnUnexpectedFrame(frameKind);
    }
}

Result CoSimServer::WaitForPingOkFrame(Command& command) const {
    FrameKind frameKind{};
    CheckResult(_protocol->ReceiveHeader(_channel->GetReader(), frameKind));

    switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
        case FrameKind::PingOk:
            CheckResultWithMessage(_protocol->ReadPingOk(_channel->GetReader(), command), "Could not read ping ok frame.");
            return CreateOk();
        default:
            return OnUnexpectedFrame(frameKind);
    }
}

Result CoSimServer::WaitForConnectFrame(uint32_t& version, std::string& clientName) const {
    FrameKind frameKind{};
    CheckResult(_protocol->ReceiveHeader(_channel->GetReader(), frameKind));

    switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
        case FrameKind::Connect: {
            Mode mode{};
            std::string serverName;
            CheckResultWithMessage(_protocol->ReadConnect(_channel->GetReader(), version, mode, serverName, clientName), "Could not read connect frame.");
            return CreateOk();
        }
        default:
            return OnUnexpectedFrame(frameKind);
    }
}

Result CoSimServer::WaitForStepOkFrame(SimulationTime& simulationTime, Command& command) const {
    FrameKind frameKind{};
    CheckResult(_protocol->ReceiveHeader(_channel->GetReader(), frameKind));

    switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
        case FrameKind::StepOk:
            CheckResultWithMessage(
                _protocol->ReadStepOk(_channel->GetReader(), simulationTime, command, _deserializeIoData, _deserializeBusMessages, _callbacks),
                "Could not receive step ok frame.");
            return CreateOk();
        case FrameKind::Error:
            return OnError();
        default:
            return OnUnexpectedFrame(frameKind);
    }
}

Result CoSimServer::OnError() const {
    std::string errorMessage;
    CheckResultWithMessage(_protocol->ReadError(_channel->GetReader(), errorMessage), "Could not read error frame.");
    LogError(errorMessage);
    return CreateError();
}

void CoSimServer::HandlePendingCommand(Command command) const {
    switch (command) {
        case Command::Start:
            if (_callbacks.simulationStartedCallback) {
                _callbacks.simulationStartedCallback({});
            } else {
                LogWarning("Ignoring pending '{}' command because no started callback is registered.", command);
            }
            break;
        case Command::Stop:
            if (_callbacks.simulationStoppedCallback) {
                _callbacks.simulationStoppedCallback({});
            } else {
                LogWarning("Ignoring pending '{}' command because no stopped callback is registered.", command);
            }
            break;
        case Command::Terminate:
            if (_callbacks.simulationTerminatedCallback) {
                _callbacks.simulationTerminatedCallback({}, TerminateReason::Error);
            } else {
                LogWarning("Ignoring pending '{}' command because no terminated callback is registered.", command);
            }
            break;
        case Command::Pause:
            if (_callbacks.simulationPausedCallback) {
                _callbacks.simulationPausedCallback({});
            } else {
                LogWarning("Ignoring pending '{}' command because no paused callback is registered.", command);
            }
            break;
        case Command::Continue:
            if (_callbacks.simulationContinuedCallback) {
                _callbacks.simulationContinuedCallback({});
            } else {
                LogWarning("Ignoring pending '{}' command because no continued callback is registered.", command);
            }
            break;
        case Command::TerminateFinished:
            if (_callbacks.simulationTerminatedCallback) {
                _callbacks.simulationTerminatedCallback({}, TerminateReason::Finished);
            } else {
                LogWarning("Ignoring pending '{}' command because no terminated callback is registered.", command);
            }
            break;
        case Command::None:
        case Command::Step:
        case Command::Ping:
            break;
    }
}

Result CoSimServer::OnUnexpectedFrame(FrameKind frameKind) {
    LogError("Received unexpected frame '{}'.", frameKind);
    return CreateError();
}

}  // namespace DsVeosCoSim
