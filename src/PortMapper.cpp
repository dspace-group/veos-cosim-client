// Copyright dSPACE GmbH. All rights reserved.

#include "PortMapper.h"

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>

#include "Channel.h"
#include "CoSimHelper.h"
#include "Environment.h"
#include "Event.h"
#include "Protocol.h"

namespace DsVeosCoSim {

namespace {

constexpr uint32_t ClientTimeoutInMilliseconds = 1000;

class PortMapperServerImpl final : public PortMapperServer {
public:
    explicit PortMapperServerImpl(const bool enableRemoteAccess)
        : _server(CreateTcpChannelServer(GetPortMapperPort(), enableRemoteAccess)) {
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
            try {
                const std::unique_ptr<Channel> channel = _server->TryAccept();
                if (!channel) {
                    continue;
                }

                if (!HandleClient(*channel)) {
                    LogTrace("Port mapper client disconnected unexpectedly.");
                }
            } catch (const std::exception& e) {
                std::string message = "The following exception occurred in port mapper thread: ";
                message.append(e.what());
                LogError(message);
            }
        }
    }

    [[nodiscard]] bool HandleClient(Channel& channel) {
        FrameKind frameKind{};
        CheckResult(Protocol::ReceiveHeader(channel.GetReader(), frameKind));

        switch (frameKind) {
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
                std::string message = "Received unexpected frame '";
                message.append(ToString(frameKind));
                message.append("'.");
                throw std::runtime_error(message);
        }
    }

    [[nodiscard]] bool HandleGetPort(Channel& channel) {
        std::string name;
        CheckResultWithMessage(Protocol::ReadGetPort(channel.GetReader(), name), "Could not read get port frame.");

        if (IsPortMapperServerVerbose()) {
            std::string message = "Get '";
            message.append(name);
            message.append("'");
            LogTrace(message);
        }

        const auto search = _ports.find(name);
        if (search == _ports.end()) {
            std::string message = "Could not find port for dSPACE VEOS CoSim server '";
            message.append(name);
            message.append("'.");
            CheckResultWithMessage(Protocol::SendError(channel.GetWriter(), message), "Could not send error frame.");
            return true;
        }

        CheckResultWithMessage(Protocol::SendGetPortOk(channel.GetWriter(), search->second),
                               "Could not send get port ok frame.");
        return true;
    }

    [[nodiscard]] bool HandleSetPort(Channel& channel) {
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
        return true;
    }

    [[nodiscard]] bool HandleUnsetPort(Channel& channel) {
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
        return true;
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

[[nodiscard]] std::unique_ptr<PortMapperServer> CreatePortMapperServer(const bool enableRemoteAccess) {
    return std::make_unique<PortMapperServerImpl>(enableRemoteAccess);
}

[[nodiscard]] bool PortMapper_GetPort(const std::string& ipAddress, const std::string& serverName, uint16_t& port) {
    if (IsPortMapperClientVerbose()) {
        std::string message = "PortMapper_GetPort(ipAddress: '";
        message.append(ipAddress);
        message.append("', serverName: '");
        message.append(serverName);
        message.append("')");
        LogTrace(message);
    }

    const std::unique_ptr<Channel> channel =
        TryConnectToTcpChannel(ipAddress, GetPortMapperPort(), 0, ClientTimeoutInMilliseconds);
    CheckResultWithMessage(channel, "Could not connect to port mapper.");

    CheckResultWithMessage(Protocol::SendGetPort(channel->GetWriter(), serverName), "Could not send get port frame.");

    FrameKind frameKind{};
    CheckResult(Protocol::ReceiveHeader(channel->GetReader(), frameKind));

    switch (frameKind) {
        case FrameKind::GetPortOk: {
            CheckResultWithMessage(Protocol::ReadGetPortOk(channel->GetReader(), port),
                                   "Could not receive port ok frame.");
            return true;
        }
        case FrameKind::Error: {
            std::string errorMessage;
            CheckResultWithMessage(Protocol::ReadError(channel->GetReader(), errorMessage),
                                   "Could not read error frame.");
            throw std::runtime_error(errorMessage);
        }
        default:
            std::string message = "PortMapper_GetPort: Received unexpected frame '";
            message.append(ToString(frameKind));
            message.append("'.");
            throw std::runtime_error(message);
    }
}

[[nodiscard]] bool PortMapper_SetPort(const std::string& name, const uint16_t port) {
    const std::unique_ptr<Channel> channel =
        TryConnectToTcpChannel("127.0.0.1", GetPortMapperPort(), 0, ClientTimeoutInMilliseconds);
    CheckResultWithMessage(channel, "Could not connect to port mapper.");

    CheckResultWithMessage(Protocol::SendSetPort(channel->GetWriter(), name, port), "Could not send set port frame.");

    FrameKind frameKind{};
    CheckResult(Protocol::ReceiveHeader(channel->GetReader(), frameKind));

    switch (frameKind) {
        case FrameKind::Ok:
            return true;
        case FrameKind::Error: {
            std::string errorString;
            CheckResultWithMessage(Protocol::ReadError(channel->GetReader(), errorString),
                                   "Could not read error frame.");
            throw std::runtime_error(errorString);
        }
        default:
            std::string message = "PortMapper_SetPort: Received unexpected frame '";
            message.append(ToString(frameKind));
            message.append("'.");
            throw std::runtime_error(message);
    }
}

[[nodiscard]] bool PortMapper_UnsetPort(const std::string& name) {
    const std::unique_ptr<Channel> channel =
        TryConnectToTcpChannel("127.0.0.1", GetPortMapperPort(), 0, ClientTimeoutInMilliseconds);
    CheckResultWithMessage(channel, "Could not connect to port mapper.");

    CheckResultWithMessage(Protocol::SendUnsetPort(channel->GetWriter(), name), "Could not send unset port frame.");

    FrameKind frameKind{};
    CheckResult(Protocol::ReceiveHeader(channel->GetReader(), frameKind));

    switch (frameKind) {
        case FrameKind::Ok:
            return true;
        case FrameKind::Error: {
            std::string errorString;
            CheckResultWithMessage(Protocol::ReadError(channel->GetReader(), errorString),
                                   "Could not read error frame.");
            throw std::runtime_error(errorString);
        }
        default:
            std::string message = "PortMapper_UnsetPort: Received unexpected frame '";
            message.append(ToString(frameKind));
            message.append("'.");
            throw std::runtime_error(message);
    }
}

}  // namespace DsVeosCoSim
