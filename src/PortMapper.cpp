// Copyright dSPACE GmbH. All rights reserved.

#include "PortMapper.h"

#include "CoSimTypes.h"
#include "Communication.h"
#include "Logger.h"
#include "Protocol.h"

namespace DsVeosCoSim {

namespace {

bool IsPortMapperServerVerbose() {
    constexpr bool defaultVerbose = false;

    const char* verboseString = std::getenv("VEOS_COSIM_PORTMAPPER_SERVER_VERBOSE");  // NOLINT(concurrency-mt-unsafe)
    if (verboseString) {
        const int verbose = std::atoi(verboseString);  // NOLINT(cert-err34-c)
        return verbose != 0;
    }

    return defaultVerbose;
}

bool IsPortMapperClientVerbose() {
    constexpr bool defaultVerbose = false;

    const char* verboseString = std::getenv("VEOS_COSIM_PORTMAPPER_CLIENT_VERBOSE");  // NOLINT(concurrency-mt-unsafe)
    if (verboseString) {
        const int verbose = std::atoi(verboseString);  // NOLINT(cert-err34-c)
        return verbose != 0;
    }

    return defaultVerbose;
}

[[nodiscard]] uint16_t GetPortMapperPortInitial() {
    constexpr uint16_t defaultPort = 27027;

    const char* portString = getenv("VEOS_COSIM_PORTMAPPER_PORT");  // NOLINT(concurrency-mt-unsafe)
    if (portString) {
        const int port = std::atoi(portString);  // NOLINT(cert-err34-c)
        if (port > 0 && port <= UINT16_MAX) {
            return static_cast<uint16_t>(port);
        }
    }

    return defaultPort;
}

}  // namespace

PortMapperServer::~PortMapperServer() {
    Stop();
}

Result PortMapperServer::Start(bool enableRemoteAccess) {
    uint16_t port = GetPortMapperPort();
    CheckResult(_server.Start(port, enableRemoteAccess));

    _isRunning = true;

    _thread = std::thread([this] {
        RunPortMapperServer();
    });
    return Result::Ok;
}

void PortMapperServer::Stop() {
    _isRunning = false;

    _server.Stop();

    if (_thread.joinable()) {
        _thread.join();
    }
}

void PortMapperServer::RunPortMapperServer() {
    while (_isRunning) {
        const Result result = _server.Accept(_channel);
        if (IsResultError(result)) {
            break;
        }

        (void)HandleClient();
        _channel.Disconnect();
    }
}

Result PortMapperServer::HandleClient() {
    FrameKind frameKind = FrameKind::Unknown;
    CheckResult(Protocol::ReceiveHeader(_channel, frameKind));

    switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
        case FrameKind::SetPort:
            return HandleSetPort();
        case FrameKind::UnsetPort:
            return HandleUnsetPort();
        case FrameKind::GetPort:
            return HandleGetPort();
        default:
            LogError("Received unexpected frame " + ToString(frameKind) + ".");
            return Result::Error;
    }
}

Result PortMapperServer::HandleSetPort() {
    std::string name;
    uint16_t port = 0;
    CheckResult(Protocol::ReadSetPort(_channel, name, port));

    if (IsPortMapperServerVerbose()) {
        LogTrace("Set " + name + ": " + std::to_string(port));
    }

    _ports[name] = port;

    if (IsPortMapperServerVerbose()) {
        DumpEntries();
    }

    return Protocol::SendOk(_channel);
}

Result PortMapperServer::HandleUnsetPort() {
    std::string name;
    CheckResult(Protocol::ReadUnsetPort(_channel, name));

    if (IsPortMapperServerVerbose()) {
        LogTrace("Unset " + name);
    }

    _ports.erase(name);

    if (IsPortMapperServerVerbose()) {
        DumpEntries();
    }

    return Protocol::SendOk(_channel);
}

Result PortMapperServer::HandleGetPort() {
    std::string name;
    CheckResult(Protocol::ReadGetPort(_channel, name));

    if (IsPortMapperServerVerbose()) {
        LogTrace("Get " + name);
    }

    const auto search = _ports.find(name);
    if (search == _ports.end()) {
        return Protocol::SendError(_channel, "Could not find port for dSPACE VEOS CoSim server '" + name + "'.");
    }

    return Protocol::SendGetPortResponse(_channel, search->second);
}

void PortMapperServer::DumpEntries() {
    if (_ports.empty()) {
        LogTrace("No PortMapper Ports.");
    } else {
        LogTrace("PortMapper Ports:");

        for (auto& [name, port] : _ports) {
            LogTrace("  " + name + ": " + std::to_string(port));
        }
    }
}

Result PortMapper_SetPort(std::string_view name, uint16_t port) {
    Channel channel;
    CheckResult(ConnectToServer("127.0.0.1", GetPortMapperPort(), 0, channel));

    CheckResult(Protocol::SendSetPort(channel, name, port));

    FrameKind frameKind = FrameKind::Unknown;
    CheckResult(Protocol::ReceiveHeader(channel, frameKind));

    switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
        case FrameKind::Ok:
            return Result::Ok;
        case FrameKind::Error: {
            std::string errorString;
            CheckResult(Protocol::ReadError(channel, errorString));
            LogError(errorString);
            return Result::Error;
        }
        default:
            LogError("Received unexpected frame " + ToString(frameKind) + ".");
            return Result::Error;
    }
}

Result PortMapper_UnsetPort(std::string_view name) {
    Channel channel;
    CheckResult(ConnectToServer("127.0.0.1", GetPortMapperPort(), 0, channel));

    CheckResult(Protocol::SendUnsetPort(channel, name));

    FrameKind frameKind = FrameKind::Unknown;
    CheckResult(Protocol::ReceiveHeader(channel, frameKind));

    switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
        case FrameKind::Ok:
            return Result::Ok;
        case FrameKind::Error: {
            std::string errorString;
            CheckResult(Protocol::ReadError(channel, errorString));
            LogError(errorString);
            return Result::Error;
        }
        default:
            LogError("Received unexpected frame " + ToString(frameKind) + ".");
            return Result::Error;
    }
}

uint16_t GetPortMapperPort() {
    static uint16_t port = GetPortMapperPortInitial();
    return port;
}

Result PortMapper_GetPort(std::string_view ipAddress, std::string_view serverName, uint16_t& port) {
    if (IsPortMapperClientVerbose()) {
        LogTrace("PortMapper_GetPort(ipAddress: " + std::string(ipAddress) + ", serverName: " + std::string(serverName) + ", port: " + std::to_string(port) +
                 ")");
    }

    Channel channel;
    CheckResult(ConnectToServer(ipAddress, GetPortMapperPort(), 0, channel));
    CheckResult(Protocol::SendGetPort(channel, serverName));

    FrameKind frameKind = FrameKind::Unknown;
    CheckResult(Protocol::ReceiveHeader(channel, frameKind));

    switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
        case FrameKind::GetPortResponse:
            return Protocol::ReadGetPortResponse(channel, port);
        case FrameKind::Error: {
            std::string errorMessage;
            CheckResult(Protocol::ReadError(channel, errorMessage));
            LogError(errorMessage);
            return Result::Error;
        }
        default:
            LogError("PortMapper_GetPort: Received unexpected frame " + ToString(frameKind) + ".");
            return Result::Error;
    }
}

}  // namespace DsVeosCoSim
