// Copyright dSPACE GmbH. All rights reserved.

#include "PortMapper.h"

#include "CoSimHelper.h"
#include "Protocol.h"
#include "SocketChannel.h"

namespace DsVeosCoSim {

namespace {

constexpr uint32_t ClientTimeoutInMilliseconds = 1000;

bool IsPortMapperServerVerbose() {
    constexpr bool defaultVerbose = false;

    const char* verboseString = std::getenv("VEOS_COSIM_PORTMAPPER_SERVER_VERBOSE");  // NOLINT(concurrency-mt-unsafe)
    if (verboseString) {
        const int32_t verbose = std::atoi(verboseString);  // NOLINT(cert-err34-c)
        return verbose != 0;
    }

    return defaultVerbose;
}

bool IsPortMapperClientVerbose() {
    constexpr bool defaultVerbose = false;

    const char* verboseString = std::getenv("VEOS_COSIM_PORTMAPPER_CLIENT_VERBOSE");  // NOLINT(concurrency-mt-unsafe)
    if (verboseString) {
        const int32_t verbose = std::atoi(verboseString);  // NOLINT(cert-err34-c)
        return verbose != 0;
    }

    return defaultVerbose;
}

[[nodiscard]] uint16_t GetPortMapperPortInitial() {
    constexpr uint16_t defaultPort = 27027;

    const char* portString = std::getenv("VEOS_COSIM_PORTMAPPER_PORT");  // NOLINT(concurrency-mt-unsafe)
    if (portString) {
        const int32_t port = std::atoi(portString);  // NOLINT(cert-err34-c)
        if (port > 0 && port <= UINT16_MAX) {
            return static_cast<uint16_t>(port);
        }
    }

    return defaultPort;
}

uint16_t GetPortMapperPort() {
    static uint16_t port = GetPortMapperPortInitial();
    return port;
}

}  // namespace

PortMapperServer::PortMapperServer(bool enableRemoteAccess) : _server(GetPortMapperPort(), enableRemoteAccess) {
    _thread = std::thread([this] {
        RunPortMapperServer();
    });
}

PortMapperServer::~PortMapperServer() noexcept {
    _stopEvent.Set();

    if (_thread.joinable()) {
        _thread.join();
    }
}

void PortMapperServer::RunPortMapperServer() {
    while (!_stopEvent.Wait(10)) {
        try {
            std::optional<SocketChannel> channel = _server.TryAccept();
            if (channel) {
                if (!HandleClient(*channel)) {
                    LogTrace("Port mapper client disconnected unexpectedly.");
                }
            }
        } catch (const std::exception& e) {
            LogError("The following exception occurred in port mapper thread: {}", e.what());
        }
    }
}

bool PortMapperServer::HandleClient(Channel& channel) {
    FrameKind frameKind{};
    CheckResult(Protocol::ReceiveHeader(channel.GetReader(), frameKind));

    switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
        case FrameKind::GetPort:
            CheckResultWithMessage(HandleGetPort(channel), "Could not handle get port request.");
            return true;
        case FrameKind::SetPort:
            CheckResultWithMessage(HandleSetPort(channel), "Could not handle set port request.");
            return true;
        case FrameKind::UnsetPort:
            CheckResultWithMessage(HandleUnsetPort(channel), "Could not handle unset port request.");
            return true;
        default:
            throw CoSimException(fmt::format("Received unexpected frame {}.", ToString(frameKind)));
    }
}

bool PortMapperServer::HandleGetPort(Channel& channel) {
    std::string name;
    CheckResultWithMessage(Protocol::ReadGetPort(channel.GetReader(), name), "Could not read get port frame.");

    if (IsPortMapperServerVerbose()) {
        LogTrace("Get '{}'", name);
    }

    const auto search = _ports.find(name);
    if (search == _ports.end()) {
        CheckResultWithMessage(
            Protocol::SendError(channel.GetWriter(),
                                fmt::format("Could not find port for dSPACE VEOS CoSim server '{}'.", name)),
            "Could not send error frame.");
        return true;
    }

    CheckResultWithMessage(Protocol::SendGetPortOk(channel.GetWriter(), search->second),
                           "Could not send get port ok frame.");
    return true;
}

bool PortMapperServer::HandleSetPort(Channel& channel) {
    std::string name;
    uint16_t port = 0;
    CheckResultWithMessage(Protocol::ReadSetPort(channel.GetReader(), name, port), "Could not read set port frame.");

    if (IsPortMapperServerVerbose()) {
        LogTrace("Set '{}':{}", name, port);
    }

    _ports[name] = port;

    if (IsPortMapperServerVerbose()) {
        DumpEntries();
    }

    CheckResultWithMessage(Protocol::SendOk(channel.GetWriter()), "Could not send ok frame.");
    return true;
}

bool PortMapperServer::HandleUnsetPort(Channel& channel) {
    std::string name;
    CheckResultWithMessage(Protocol::ReadUnsetPort(channel.GetReader(), name), "Could not read unset port frame.");

    if (IsPortMapperServerVerbose()) {
        LogTrace("Unset '{}'", name);
    }

    _ports.erase(name);

    if (IsPortMapperServerVerbose()) {
        DumpEntries();
    }

    CheckResultWithMessage(Protocol::SendOk(channel.GetWriter()), "Could not send ok frame.");
    return true;
}

void PortMapperServer::DumpEntries() {
    if (_ports.empty()) {
        LogTrace("No PortMapper Ports.");
    } else {
        LogTrace("PortMapper Ports:");

        for (auto& [name, port] : _ports) {
            LogTrace("  '{}': {}", name, port);
        }
    }
}

bool PortMapper_GetPort(std::string_view ipAddress, std::string_view serverName, uint16_t& port) {
    if (IsPortMapperClientVerbose()) {
        LogTrace("PortMapper_GetPort(ipAddress: '{}', serverName: '{}')", ipAddress, serverName);
    }

    std::optional<SocketChannel> channel =
        TryConnectToTcpChannel(ipAddress, GetPortMapperPort(), 0, ClientTimeoutInMilliseconds);
    CheckResultWithMessage(channel, "Could not connect to port mapper.");

    CheckResultWithMessage(Protocol::SendGetPort(channel->GetWriter(), serverName), "Could not send get port frame.");

    FrameKind frameKind{};
    CheckResult(Protocol::ReceiveHeader(channel->GetReader(), frameKind));

    switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
        case FrameKind::GetPortOk: {
            CheckResultWithMessage(Protocol::ReadGetPortOk(channel->GetReader(), port),
                                   "Could not receive port ok frame.");
            return true;
        }
        case FrameKind::Error: {
            std::string errorMessage;
            CheckResultWithMessage(Protocol::ReadError(channel->GetReader(), errorMessage),
                                   "Could not read error frame.");
            throw CoSimException(errorMessage);
        }
        default:
            throw CoSimException(fmt::format("PortMapper_GetPort: Received unexpected frame {}.", ToString(frameKind)));
    }
}

bool PortMapper_SetPort(std::string_view name, uint16_t port) {
    std::optional<SocketChannel> channel =
        TryConnectToTcpChannel("127.0.0.1", GetPortMapperPort(), 0, ClientTimeoutInMilliseconds);
    CheckResultWithMessage(channel, "Could not connect to port mapper.");

    CheckResultWithMessage(Protocol::SendSetPort(channel->GetWriter(), name, port), "Could not send set port frame.");

    FrameKind frameKind{};
    CheckResult(Protocol::ReceiveHeader(channel->GetReader(), frameKind));

    switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
        case FrameKind::Ok:
            return true;
        case FrameKind::Error: {
            std::string errorString;
            CheckResultWithMessage(Protocol::ReadError(channel->GetReader(), errorString),
                                   "Could not read error frame.");
            throw CoSimException(errorString);
        }
        default:
            throw CoSimException(fmt::format("Received unexpected frame {}.", ToString(frameKind)));
    }
}

bool PortMapper_UnsetPort(std::string_view name) {
    std::optional<SocketChannel> channel =
        TryConnectToTcpChannel("127.0.0.1", GetPortMapperPort(), 0, ClientTimeoutInMilliseconds);
    CheckResultWithMessage(channel, "Could not connect to port mapper.");

    CheckResultWithMessage(Protocol::SendUnsetPort(channel->GetWriter(), name), "Could not send unset port frame.");

    FrameKind frameKind{};
    CheckResult(Protocol::ReceiveHeader(channel->GetReader(), frameKind));

    switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
        case FrameKind::Ok:
            return true;
        case FrameKind::Error: {
            std::string errorString;
            CheckResultWithMessage(Protocol::ReadError(channel->GetReader(), errorString),
                                   "Could not read error frame.");
            throw CoSimException(errorString);
        }
        default:
            throw CoSimException(fmt::format("Received unexpected frame {}.", ToString(frameKind)));
    }
}

}  // namespace DsVeosCoSim
