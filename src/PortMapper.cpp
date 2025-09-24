// Copyright dSPACE GmbH. All rights reserved.

#include "PortMapper.h"

#include <cstdint>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>

#include "Channel.h"
#include "CoSimHelper.h"
#include "DsVeosCoSim/CoSimTypes.h"
#include "Environment.h"
#include "Event.h"
#include "Protocol.h"

namespace DsVeosCoSim {

namespace {

constexpr uint32_t ClientTimeoutInMilliseconds = 1000;

class PortMapperServerImpl final : public PortMapperServer {
public:
    explicit PortMapperServerImpl(std::unique_ptr<ChannelServer> channelServer) : _server(std::move(channelServer)) {
        _thread = std::thread([this] {
            RunPortMapperServer();
        });
    }

    ~PortMapperServerImpl() noexcept override {
        _stopEvent.Set();

        if (_thread.joinable()) {
            _thread.join();
        }
    }

    PortMapperServerImpl(const PortMapperServerImpl&) = delete;
    PortMapperServerImpl& operator=(const PortMapperServerImpl&) = delete;

    PortMapperServerImpl(PortMapperServerImpl&&) = delete;
    PortMapperServerImpl& operator=(PortMapperServerImpl&&) = delete;

private:
    void RunPortMapperServer() {
        constexpr uint32_t timeoutInMilliseconds = 100U;
        while (!_stopEvent.Wait(timeoutInMilliseconds)) {
            std::unique_ptr<Channel> channel;
            if (!IsOk(_server->TryAccept(channel))) {
                LogError("Could not accept port mapper client.");
                return;
            }

            if (!channel) {
                continue;
            }

            if (!IsOk(HandleClient(*channel))) {
                LogTrace("Port mapper client disconnected unexpectedly.");
            }
        }
    }

    [[nodiscard]] Result HandleClient(Channel& channel) {
        FrameKind frameKind{};
        CheckResult(Protocol::ReceiveHeader(channel.GetReader(), frameKind));

        switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
            case FrameKind::GetPort:
                CheckResultWithMessage(HandleGetPort(channel), "Could not handle get port request.");
                return Result::Ok;
            case FrameKind::SetPort:
                CheckResultWithMessage(HandleSetPort(channel), "Could not handle set port request.");
                return Result::Ok;
            case FrameKind::UnsetPort:
                CheckResultWithMessage(HandleUnsetPort(channel), "Could not handle unset port request.");
                return Result::Ok;
            default:
                std::string message = "Received unexpected frame '";
                message.append(ToString(frameKind));
                message.append("'.");
                LogError(message);
                return Result::Error;
        }
    }

    [[nodiscard]] Result HandleGetPort(Channel& channel) {
        std::string name;
        CheckResultWithMessage(Protocol::ReadGetPort(channel.GetReader(), name), "Could not read get port frame.");

        if (IsPortMapperServerVerbose()) {
            std::string message = "Get '";
            message.append(name);
            message.append("'");
            LogTrace(message);
        }

        auto search = _ports.find(name);
        if (search == _ports.end()) {
            std::string message = "Could not find port for dSPACE VEOS CoSim server '";
            message.append(name);
            message.append("'.");
            CheckResultWithMessage(Protocol::SendError(channel.GetWriter(), message), "Could not send error frame.");
            return Result::Ok;
        }

        CheckResultWithMessage(Protocol::SendGetPortOk(channel.GetWriter(), search->second),
                               "Could not send get port ok frame.");
        return Result::Ok;
    }

    [[nodiscard]] Result HandleSetPort(Channel& channel) {
        std::string name;
        uint16_t port = 0;
        CheckResultWithMessage(Protocol::ReadSetPort(channel.GetReader(), name, port),
                               "Could not read set port frame.");

        if (IsPortMapperServerVerbose()) {
            std::string message = "Set '";
            message.append(name);
            message.append("': ");
            message.append(std::to_string(port));
            LogTrace(message);
        }

        _ports[name] = port;

        if (IsPortMapperServerVerbose()) {
            DumpEntries();
        }

        CheckResultWithMessage(Protocol::SendOk(channel.GetWriter()), "Could not send ok frame.");
        return Result::Ok;
    }

    [[nodiscard]] Result HandleUnsetPort(Channel& channel) {
        std::string name;
        CheckResultWithMessage(Protocol::ReadUnsetPort(channel.GetReader(), name), "Could not read unset port frame.");

        if (IsPortMapperServerVerbose()) {
            std::string message = "Unset '";
            message.append(name);
            message.append("'");
            LogTrace(message);
        }

        _ports.erase(name);

        if (IsPortMapperServerVerbose()) {
            DumpEntries();
        }

        CheckResultWithMessage(Protocol::SendOk(channel.GetWriter()), "Could not send ok frame.");
        return Result::Ok;
    }

    void DumpEntries() {
        if (_ports.empty()) {
            LogTrace("No PortMapper Ports.");
        } else {
            LogTrace("PortMapper Ports:");

            for (auto& [name, port] : _ports) {
                std::string message = "  '";
                message.append(name);
                message.append("': ");
                message.append(std::to_string(port));
                LogTrace(message);
            }
        }
    }

    std::unordered_map<std::string, uint16_t> _ports;

    std::unique_ptr<ChannelServer> _server;
    std::thread _thread;
    Event _stopEvent;
};

}  // namespace

[[nodiscard]] Result CreatePortMapperServer(bool enableRemoteAccess,
                                            std::unique_ptr<PortMapperServer>& portMapperServer) {
    std::unique_ptr<ChannelServer> channelServer;
    CheckResult(CreateTcpChannelServer(GetPortMapperPort(), enableRemoteAccess, channelServer));
    portMapperServer = std::make_unique<PortMapperServerImpl>(std::move(channelServer));
    return Result::Ok;
}

[[nodiscard]] Result PortMapperGetPort(const std::string& ipAddress, const std::string& serverName, uint16_t& port) {
    if (IsPortMapperClientVerbose()) {
        std::string message = "PortMapperGetPort(ipAddress: '";
        message.append(ipAddress);
        message.append("', serverName: '");
        message.append(serverName);
        message.append("')");
        LogTrace(message);
    }

    std::unique_ptr<Channel> channel;
    CheckResult(TryConnectToTcpChannel(ipAddress, GetPortMapperPort(), 0, ClientTimeoutInMilliseconds, channel));
    CheckBoolWithMessage(channel, "Could not connect to port mapper.");

    CheckResultWithMessage(Protocol::SendGetPort(channel->GetWriter(), serverName), "Could not send get port frame.");

    FrameKind frameKind{};
    CheckResult(Protocol::ReceiveHeader(channel->GetReader(), frameKind));

    switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
        case FrameKind::GetPortOk: {
            CheckResultWithMessage(Protocol::ReadGetPortOk(channel->GetReader(), port),
                                   "Could not receive port ok frame.");
            return Result::Ok;
        }
        case FrameKind::Error: {
            std::string errorMessage;
            CheckResultWithMessage(Protocol::ReadError(channel->GetReader(), errorMessage),
                                   "Could not read error frame.");
            LogError(errorMessage);
            return Result::Error;
        }
        default:
            std::string message = "PortMapperGetPort: Received unexpected frame '";
            message.append(ToString(frameKind));
            message.append("'.");
            LogError(message);
            return Result::Error;
    }
}

[[nodiscard]] Result PortMapperSetPort(const std::string& name, uint16_t port) {
    std::unique_ptr<Channel> channel;
    CheckResult(TryConnectToTcpChannel("127.0.0.1", GetPortMapperPort(), 0, ClientTimeoutInMilliseconds, channel));
    CheckBoolWithMessage(channel, "Could not connect to port mapper.");

    CheckResultWithMessage(Protocol::SendSetPort(channel->GetWriter(), name, port), "Could not send set port frame.");

    FrameKind frameKind{};
    CheckResult(Protocol::ReceiveHeader(channel->GetReader(), frameKind));

    switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
        case FrameKind::Ok:
            return Result::Ok;
        case FrameKind::Error: {
            std::string errorString;
            CheckResultWithMessage(Protocol::ReadError(channel->GetReader(), errorString),
                                   "Could not read error frame.");
            LogError(errorString);
            return Result::Error;
        }
        default:
            std::string message = "PortMapperSetPort: Received unexpected frame '";
            message.append(ToString(frameKind));
            message.append("'.");
            LogError(message);
            return Result::Error;
    }
}

[[nodiscard]] Result PortMapperUnsetPort(const std::string& name) {
    std::unique_ptr<Channel> channel;
    CheckResult(TryConnectToTcpChannel("127.0.0.1", GetPortMapperPort(), 0, ClientTimeoutInMilliseconds, channel));
    CheckBoolWithMessage(channel, "Could not connect to port mapper.");

    CheckResultWithMessage(Protocol::SendUnsetPort(channel->GetWriter(), name), "Could not send unset port frame.");

    FrameKind frameKind{};
    CheckResult(Protocol::ReceiveHeader(channel->GetReader(), frameKind));

    switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
        case FrameKind::Ok:
            return Result::Ok;
        case FrameKind::Error: {
            std::string errorString;
            CheckResultWithMessage(Protocol::ReadError(channel->GetReader(), errorString),
                                   "Could not read error frame.");
            LogError(errorString);
            return Result::Error;
        }
        default:
            std::string message = "PortMapperUnsetPort: Received unexpected frame '";
            message.append(ToString(frameKind));
            message.append("'.");
            LogError(message);
            return Result::Error;
    }
}

}  // namespace DsVeosCoSim
