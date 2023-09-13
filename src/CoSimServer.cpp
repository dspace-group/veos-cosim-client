// Copyright dSPACE GmbH. All rights reserved.

#include "CoSimServer.h"

#include <chrono>

#include "Logger.h"
#include "Protocol.h"

using namespace std::chrono;

namespace DsVeosCoSim {

CoSimServer::~CoSimServer() {
    (void)Unload();
}

Result CoSimServer::Load(const CoSimServerConfig& config) {
    _localPort = config.localPort;
    _serverName = config.serverName;
    _isClientOptional = config.isClientOptional;
    _enableRemoteAccess = config.enableRemoteAccess;
    _incomingSignals = config.incomingSignals;
    _outgoingSignals = config.outgoingSignals;
    _canControllers = config.canControllers;
    _ethControllers = config.ethControllers;
    _linControllers = config.linControllers;

    _callbacks.simulationStoppedCallback = config.simulationStoppedCallback;
    _callbacks.canMessageReceivedCallback = config.canMessageReceivedCallback;
    _callbacks.linMessageReceivedCallback = config.linMessageReceivedCallback;
    _callbacks.ethMessageReceivedCallback = config.ethMessageReceivedCallback;

    SetLogCallback(config.logCallback);

    if (config.startPortMapper) {
        CheckResult(_portMapperServer.Start(_enableRemoteAccess));
    }

    // Switched read and write signals on purpose
    CheckResult(_ioBuffer.Initialize(_outgoingSignals, _incomingSignals));
    CheckResult(_busBuffer.Initialize(_canControllers, _ethControllers, _linControllers));

    CheckResult(StartAccepting());

    const std::string localIpAddress = _enableRemoteAccess ? "0.0.0.0" : "127.0.0.1";
    LogInfo("dSPACE VEOS CoSim server '" + _serverName + "' is listening on " + localIpAddress + ":" + std::to_string(_localPort) + ".");

    return Result::Ok;
}

Result CoSimServer::Unload() {
    _stopAcceptingThread = true;

    if (_acceptingThread.joinable()) {
        _acceptingThread.join();
    }

    (void)PortMapper_UnsetPort(_serverName);
    _server.Stop();

    _channel.Disconnect();
    return Result::Ok;
}

Result CoSimServer::Start(SimulationTime simulationTime) {
    if (!_isConnected) {
        if (_isClientOptional) {
            return Result::Ok;
        }

        LogInfo("Waiting for dSPACE VEOS CoSim client to connect to dSPACE VEOS CoSim server '" + _serverName + "' ...");
        if (_acceptingThread.joinable()) {
            _acceptingThread.join();
        }
    }

    const Result result = StartInternal(simulationTime);
    if (result != Result::Ok) {
        return CloseConnection();
    }

    return UpdateTime();
}

Result CoSimServer::Stop(SimulationTime simulationTime) {
    if (!_isConnected) {
        return Result::Ok;
    }

    const Result result = StopInternal(simulationTime);
    if (result != Result::Ok) {
        return CloseConnection();
    }

    return UpdateTime();
}

Result CoSimServer::Terminate(SimulationTime simulationTime, TerminateReason reason) {
    if (!_isConnected) {
        return Result::Ok;
    }

    const Result result = TerminateInternal(simulationTime, reason);
    if (result != Result::Ok) {
        return CloseConnection();
    }

    return UpdateTime();
}

Result CoSimServer::Pause(SimulationTime simulationTime) {
    if (!_isConnected) {
        return Result::Ok;
    }

    const Result result = PauseInternal(simulationTime);
    if (result != Result::Ok) {
        return CloseConnection();
    }

    return UpdateTime();
}

Result CoSimServer::Continue(SimulationTime simulationTime) {
    if (!_isConnected) {
        return Result::Ok;
    }

    const Result result = ContinueInternal(simulationTime);
    if (result != Result::Ok) {
        return CloseConnection();
    }

    return UpdateTime();
}

Result CoSimServer::Step(SimulationTime simulationTime, SimulationTime& nextSimulationTime) {
    if (!_isConnected) {
        return Result::Ok;
    }

    const Result result = StepInternal(simulationTime, nextSimulationTime);
    if (result != Result::Ok) {
        return CloseConnection();
    }

    return UpdateTime();
}

Result CoSimServer::Read(IoSignalId signalId, uint32_t& length, const void** value) {
    if (!_isConnected) {
        return Result::Ok;
    }

    return _ioBuffer.Read(signalId, length, value);
}

Result CoSimServer::Write(IoSignalId signalId, uint32_t length, const void* value) {
    if (!_isConnected) {
        return Result::Ok;
    }

    return _ioBuffer.Write(signalId, length, value);
}

Result CoSimServer::Transmit(const CanMessage& message) {
    if (!_isConnected) {
        return Result::Ok;
    }

    return _busBuffer.Transmit(message);
}

Result CoSimServer::Transmit(const LinMessage& message) {
    if (!_isConnected) {
        return Result::Ok;
    }

    return _busBuffer.Transmit(message);
}

Result CoSimServer::Transmit(const EthMessage& message) {
    if (!_isConnected) {
        return Result::Ok;
    }

    return _busBuffer.Transmit(message);
}

Result CoSimServer::BackgroundService() {
    if (!_isConnected) {
        return Result::Ok;
    }

    const time_t currentTime = std::time(nullptr);
    if (currentTime > _lastCommandSentOrReceived) {
        if ((Ping()) != Result::Ok) {
            return CloseConnection();
        }

        return UpdateTime();
    }

    return Result::Ok;
}

Result CoSimServer::StartInternal(SimulationTime simulationTime) {
    CheckResult(Protocol::SendStart(_channel, simulationTime));
    return WaitForOkFrame();
}

Result CoSimServer::StopInternal(SimulationTime simulationTime) {
    CheckResult(Protocol::SendStop(_channel, simulationTime));
    return WaitForOkFrame();
}

Result CoSimServer::TerminateInternal(SimulationTime simulationTime, TerminateReason reason) {
    CheckResult(Protocol::SendTerminate(_channel, simulationTime, reason));
    return WaitForOkFrame();
}

Result CoSimServer::PauseInternal(SimulationTime simulationTime) {
    CheckResult(Protocol::SendPause(_channel, simulationTime));
    return WaitForOkFrame();
}

Result CoSimServer::ContinueInternal(SimulationTime simulationTime) {
    CheckResult(Protocol::SendContinue(_channel, simulationTime));
    return WaitForOkFrame();
}

Result CoSimServer::StepInternal(SimulationTime simulationTime, SimulationTime& nextSimulationTime) {
    CheckResult(Protocol::SendStep(_channel, simulationTime, _ioBuffer, _busBuffer));
    return WaitForStepResponseFrame(nextSimulationTime);
}

Result CoSimServer::UpdateTime() {
    _lastCommandSentOrReceived = std::time(nullptr);
    return Result::Ok;
}

Result CoSimServer::CloseConnection() {
    LogWarning("dSPACE VEOS CoSim client disconnected.");

    _isConnected = false;

    _channel.Disconnect();

    if (!_isClientOptional && _callbacks.simulationStoppedCallback) {
        _callbacks.simulationStoppedCallback(0);
    }

    return StartAccepting();
}

Result CoSimServer::Ping() {
    CheckResult(Protocol::SendPing(_channel));
    return WaitForOkFrame();
}

Result CoSimServer::OnHandleConnect(std::string_view remoteIpAddress, uint16_t remotePort) {
    uint32_t clientProtocolVersion{};
    std::string clientName;
    CheckResult(WaitForConnectFrame(clientProtocolVersion, clientName));

    CheckResult(Protocol::SendAccepted(_channel,
                                       CoSimProtocolVersion,
                                       Mode::None,
                                       _incomingSignals,
                                       _outgoingSignals,
                                       _canControllers,
                                       _ethControllers,
                                       _linControllers));

    if (clientName.empty()) {
        LogInfo("dSPACE VEOS CoSim client at " + std::string(remoteIpAddress) + ":" + std::to_string(remotePort) + " connected.");
    } else {
        LogInfo("dSPACE VEOS CoSim client '" + clientName + "' at " + std::string(remoteIpAddress) + ":" + std::to_string(remotePort) + " connected.");
    }

    return UpdateTime();
}

Result CoSimServer::StartAccepting() {
    CheckResult(_server.Start(_localPort, _enableRemoteAccess));

    CheckResult(PortMapper_SetPort(_serverName, _localPort));

    if (_acceptingThread.joinable()) {
        _acceptingThread.join();
    }

    _stopAcceptingThread = false;
    _acceptingThread = std::thread([this] {
        (void)Accepting();
    });

    return Result::Ok;
}

Result CoSimServer::Accepting() {
    while (!_stopAcceptingThread) {
        const Result result = _server.Accept(_channel);
        if (result == Result::TryAgain) {
            continue;
        }

        if (result != Result::Ok) {
            return result;
        }

        std::string acceptedIpAddress;
        uint16_t acceptedPort = 0;
        if (_channel.GetRemoteAddress(acceptedIpAddress, acceptedPort) != Result::Ok) {
            _channel.Disconnect();
            continue;
        }

        if (OnHandleConnect(acceptedIpAddress, acceptedPort) != Result::Ok) {
            _channel.Disconnect();
            continue;
        }

        (void)PortMapper_UnsetPort(_serverName);
        _server.Stop();

        _isConnected = true;
        return Result::Ok;
    }

    return Result::Ok;
}

Result CoSimServer::WaitForOkFrame() {
    FrameKind frameKind = FrameKind::Unknown;
    CheckResult(Protocol::ReceiveHeader(_channel, frameKind));

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

Result CoSimServer::WaitForConnectFrame(uint32_t& version, std::string& clientName) {
    FrameKind frameKind = FrameKind::Unknown;
    CheckResult(Protocol::ReceiveHeader(_channel, frameKind));

    switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
        case FrameKind::Connect: {
            Mode mode{};
            std::string serverName;
            return Protocol::ReadConnect(_channel, version, mode, serverName, clientName);
        }
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

Result CoSimServer::WaitForStepResponseFrame(SimulationTime& simulationTime) {
    FrameKind frameKind = FrameKind::Unknown;
    CheckResult(Protocol::ReceiveHeader(_channel, frameKind));

    switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
        case FrameKind::StepResponse:
            return Protocol::ReadStepResponse(_channel, simulationTime, _ioBuffer, _busBuffer, _callbacks);
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

}  // namespace DsVeosCoSim
