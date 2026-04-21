// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "PortMapper.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <utility>

#include <fmt/format.h>

#include "Channel.hpp"
#include "CoSimTypes.hpp"
#include "Environment.hpp"
#include "Event.hpp"
#include "Logger.hpp"
#include "Protocol.hpp"
#include "Result.hpp"

namespace DsVeosCoSim {

constexpr uint32_t ClientTimeoutInMilliseconds = 1000;

PortMapperServer::PortMapperServer(std::unique_ptr<ChannelServer> channelServer, std::unique_ptr<IProtocol> protocol)
    : _server(std::move(channelServer)), _protocol(std::move(protocol)) {
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
    constexpr uint32_t timeoutInMilliseconds = 10U;
    while (!_stopEvent.Wait(timeoutInMilliseconds)) {
        Result result = CheckForClient();
        if (IsError(result)) {
            return;
        }
    }
}

Result PortMapperServer::CheckForClient() {
    std::unique_ptr<Channel> channel;
    CheckResult(_server->TryAccept(channel));
    CheckResultWithMessage(HandleClient(*channel), "Port mapper client disconnected unexpectedly.");
    return CreateOk();
}

Result PortMapperServer::HandleClient(Channel& channel) {
    FrameKind frameKind{};
    CheckResult(_protocol->ReceiveHeader(channel.GetReader(), frameKind));

    switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
        case FrameKind::GetPort:
            CheckResultWithMessage(HandleGetPort(channel), "Could not handle get port request.");
            return CreateOk();
        case FrameKind::SetPort:
            CheckResultWithMessage(HandleSetPort(channel), "Could not handle set port request.");
            return CreateOk();
        case FrameKind::UnsetPort:
            CheckResultWithMessage(HandleUnsetPort(channel), "Could not handle unset port request.");
            return CreateOk();
        default:
            LogError("Received unexpected frame '{}'.", frameKind);
            return CreateError();
    }
}

Result PortMapperServer::HandleGetPort(Channel& channel) {
    std::string name;
    CheckResultWithMessage(_protocol->ReadGetPort(channel.GetReader(), name), "Could not read get port frame.");

    if (IsPortMapperServerVerbose()) {
        LogTrace("Get '{}'", name);
    }

    auto search = _ports.find(name);
    if (search == _ports.end()) {
        std::string message = fmt::format("Could not find port for dSPACE VEOS CoSim server '{}'.", name);
        CheckResultWithMessage(_protocol->SendError(channel.GetWriter(), message), "Could not send error frame.");
        return CreateOk();
    }

    CheckResultWithMessage(_protocol->SendGetPortOk(channel.GetWriter(), search->second), "Could not send get port ok frame.");
    return CreateOk();
}

Result PortMapperServer::HandleSetPort(Channel& channel) {
    std::string name;
    uint16_t port = 0;
    CheckResultWithMessage(_protocol->ReadSetPort(channel.GetReader(), name, port), "Could not read set port frame.");

    if (IsPortMapperServerVerbose()) {
        LogTrace("Set '{}': {}", name, port);
    }

    _ports[name] = port;

    if (IsPortMapperServerVerbose()) {
        DumpEntries();
    }

    CheckResultWithMessage(_protocol->SendOk(channel.GetWriter()), "Could not send ok frame.");
    return CreateOk();
}

Result PortMapperServer::HandleUnsetPort(Channel& channel) {
    std::string name;
    CheckResultWithMessage(_protocol->ReadUnsetPort(channel.GetReader(), name), "Could not read unset port frame.");

    if (IsPortMapperServerVerbose()) {
        LogTrace("Unset '{}'", name);
    }

    _ports.erase(name);

    if (IsPortMapperServerVerbose()) {
        DumpEntries();
    }

    CheckResultWithMessage(_protocol->SendOk(channel.GetWriter()), "Could not send ok frame.");
    return CreateOk();
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

[[nodiscard]] Result CreatePortMapperServer(bool enableRemoteAccess, std::unique_ptr<PortMapperServer>& portMapperServer) {
    std::unique_ptr<ChannelServer> channelServer;
    CheckResult(CreateTcpChannelServer(GetPortMapperPort(), enableRemoteAccess, channelServer));

    std::unique_ptr<IProtocol> protocol;
    CheckResult(CreateProtocol(ProtocolVersion1, protocol));
    portMapperServer = std::make_unique<PortMapperServer>(std::move(channelServer), std::move(protocol));
    return CreateOk();
}

namespace {

[[nodiscard]] Result ConnectToPortMapper(const std::string& ipAddress, std::unique_ptr<Channel>& channel, std::unique_ptr<IProtocol>& protocol) {
    CheckResult(TryConnectToTcpChannel(ipAddress, GetPortMapperPort(), 0, ClientTimeoutInMilliseconds, channel));
    CheckResult(CreateProtocol(ProtocolVersion1, protocol));
    return CreateOk();
}

[[nodiscard]] Result ReadOkOrError(IProtocol& protocol, Channel& channel, std::string_view callerName) {
    FrameKind frameKind{};
    CheckResult(protocol.ReceiveHeader(channel.GetReader(), frameKind));

    switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
        case FrameKind::Ok:
            CheckResultWithMessage(protocol.ReadOk(channel.GetReader()), "Could not read ok frame.");
            return CreateOk();
        case FrameKind::Error: {
            std::string errorString;
            CheckResultWithMessage(protocol.ReadError(channel.GetReader(), errorString), "Could not read error frame.");
            LogError(errorString);
            return CreateError();
        }
        default:
            LogError("{}: Received unexpected frame '{}'.", callerName, frameKind);
            return CreateError();
    }
}

}  // namespace

[[nodiscard]] Result PortMapperGetPort(const std::string& ipAddress, std::string_view serverName, uint16_t& port) {
    if (IsPortMapperClientVerbose()) {
        LogTrace("PortMapperGetPort(ipAddress: '{}', serverName: '{}')", ipAddress, serverName);
    }

    std::unique_ptr<Channel> channel;
    std::unique_ptr<IProtocol> protocol;
    CheckResult(ConnectToPortMapper(ipAddress, channel, protocol));

    CheckResultWithMessage(protocol->SendGetPort(channel->GetWriter(), serverName), "Could not send get port frame.");

    FrameKind frameKind{};
    CheckResult(protocol->ReceiveHeader(channel->GetReader(), frameKind));

    switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
        case FrameKind::GetPortOk:
            CheckResultWithMessage(protocol->ReadGetPortOk(channel->GetReader(), port), "Could not receive port ok frame.");
            return CreateOk();
        case FrameKind::Error: {
            std::string errorMessage;
            CheckResultWithMessage(protocol->ReadError(channel->GetReader(), errorMessage), "Could not read error frame.");
            LogError(errorMessage);
            return CreateError();
        }
        default:
            LogError("PortMapperGetPort: Received unexpected frame '{}'.", frameKind);
            return CreateError();
    }
}

[[nodiscard]] Result PortMapperSetPort(std::string_view name, uint16_t port) {
    std::unique_ptr<Channel> channel;
    std::unique_ptr<IProtocol> protocol;
    CheckResult(ConnectToPortMapper("127.0.0.1", channel, protocol));

    CheckResultWithMessage(protocol->SendSetPort(channel->GetWriter(), name, port), "Could not send set port frame.");
    return ReadOkOrError(*protocol, *channel, "PortMapperSetPort");
}

[[nodiscard]] Result PortMapperUnsetPort(std::string_view name) {
    std::unique_ptr<Channel> channel;
    std::unique_ptr<IProtocol> protocol;
    CheckResult(ConnectToPortMapper("127.0.0.1", channel, protocol));

    CheckResultWithMessage(protocol->SendUnsetPort(channel->GetWriter(), name), "Could not send unset port frame.");
    return ReadOkOrError(*protocol, *channel, "PortMapperUnsetPort");
}

}  // namespace DsVeosCoSim
