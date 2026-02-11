// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "PortMapper.h"

#include <cstdint>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>

#include "Channel.h"
#include "DsVeosCoSim/CoSimTypes.h"
#include "Environment.h"
#include "Event.h"
#include "Protocol.h"

namespace DsVeosCoSim {

namespace {

constexpr uint32_t ClientTimeoutInMilliseconds = 1000;

class PortMapperServerImpl final : public PortMapperServer {
public:
    explicit PortMapperServerImpl(std::unique_ptr<ChannelServer> channelServer, std::unique_ptr<IProtocol> protocol)
        : _server(std::move(channelServer)), _protocol(std::move(protocol)) {
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
                Logger::Instance().LogError("Could not accept port mapper client.");
                return;
            }

            if (!channel) {
                continue;
            }

            if (!IsOk(HandleClient(*channel))) {
                Logger::Instance().LogTrace("Port mapper client disconnected unexpectedly.");
            }
        }
    }

    [[nodiscard]] Result HandleClient(Channel& channel) {
        FrameKind frameKind{};
        CheckResult(_protocol->ReceiveHeader(channel.GetReader(), frameKind));

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
                std::ostringstream oss;
                oss << "Received unexpected frame '" << frameKind << "'.";
                Logger::Instance().LogError(oss.str());
                return Result::Error;
        }
    }

    [[nodiscard]] Result HandleGetPort(Channel& channel) {
        std::string name;
        CheckResultWithMessage(_protocol->ReadGetPort(channel.GetReader(), name), "Could not read get port frame.");

        if (IsPortMapperServerVerbose()) {
            std::ostringstream oss;
            oss << "Get '" << name << "'";
            Logger::Instance().LogTrace(oss.str());
        }

        auto search = _ports.find(name);
        if (search == _ports.end()) {
            std::ostringstream oss;
            oss << "Could not find port for dSPACE VEOS CoSim server '" << name << "'.";
            CheckResultWithMessage(_protocol->SendError(channel.GetWriter(), oss.str()), "Could not send error frame.");
            return Result::Ok;
        }

        CheckResultWithMessage(_protocol->SendGetPortOk(channel.GetWriter(), search->second), "Could not send get port ok frame.");
        return Result::Ok;
    }

    [[nodiscard]] Result HandleSetPort(Channel& channel) {
        std::string name;
        uint16_t port = 0;
        CheckResultWithMessage(_protocol->ReadSetPort(channel.GetReader(), name, port), "Could not read set port frame.");

        if (IsPortMapperServerVerbose()) {
            std::ostringstream oss;
            oss << "Set '" << name << "': " << port;
            Logger::Instance().LogTrace(oss.str());
        }

        _ports[name] = port;

        if (IsPortMapperServerVerbose()) {
            DumpEntries();
        }

        CheckResultWithMessage(_protocol->SendOk(channel.GetWriter()), "Could not send ok frame.");
        return Result::Ok;
    }

    [[nodiscard]] Result HandleUnsetPort(Channel& channel) {
        std::string name;
        CheckResultWithMessage(_protocol->ReadUnsetPort(channel.GetReader(), name), "Could not read unset port frame.");

        if (IsPortMapperServerVerbose()) {
            std::ostringstream oss;
            oss << "Unset '" << name << '\'';
            Logger::Instance().LogTrace(oss.str());
        }

        _ports.erase(name);

        if (IsPortMapperServerVerbose()) {
            DumpEntries();
        }

        CheckResultWithMessage(_protocol->SendOk(channel.GetWriter()), "Could not send ok frame.");
        return Result::Ok;
    }

    void DumpEntries() {
        if (_ports.empty()) {
            Logger::Instance().LogTrace("No PortMapper Ports.");
        } else {
            Logger::Instance().LogTrace("PortMapper Ports:");

            for (auto& [name, port] : _ports) {
                std::ostringstream oss;
                oss << "  '" << name << "': " << port;
                Logger::Instance().LogTrace(oss.str());
            }
        }
    }

    std::unordered_map<std::string, uint16_t> _ports;

    std::unique_ptr<ChannelServer> _server;
    std::thread _thread;
    Event _stopEvent;
    std::unique_ptr<IProtocol> _protocol;
};

}  // namespace

[[nodiscard]] Result CreatePortMapperServer(bool enableRemoteAccess, std::unique_ptr<PortMapperServer>& portMapperServer) {
    std::unique_ptr<ChannelServer> channelServer;
    CheckResult(CreateTcpChannelServer(GetPortMapperPort(), enableRemoteAccess, channelServer));

    std::unique_ptr<IProtocol> protocol;
    CheckResult(CreateProtocol(ProtocolVersion1, protocol));
    portMapperServer = std::make_unique<PortMapperServerImpl>(std::move(channelServer), std::move(protocol));
    return Result::Ok;
}

[[nodiscard]] Result PortMapperGetPort(const std::string& ipAddress, const std::string& serverName, uint16_t& port) {
    if (IsPortMapperClientVerbose()) {
        std::ostringstream oss;
        oss << "PortMapperGetPort(ipAddress: '" << ipAddress << "', serverName: '" << serverName << "')";
        Logger::Instance().LogTrace(oss.str());
    }

    std::unique_ptr<Channel> channel;
    CheckResult(TryConnectToTcpChannel(ipAddress, GetPortMapperPort(), 0, ClientTimeoutInMilliseconds, channel));
    CheckBoolWithMessage(channel, "Could not connect to port mapper.");

    std::unique_ptr<IProtocol> protocol;
    CheckResult(CreateProtocol(ProtocolVersion1, protocol));

    CheckResultWithMessage(protocol->SendGetPort(channel->GetWriter(), serverName), "Could not send get port frame.");

    FrameKind frameKind{};
    CheckResult(protocol->ReceiveHeader(channel->GetReader(), frameKind));

    switch (frameKind) {
        // NOLINT(clang-diagnostic-switch-enum)
        case FrameKind::GetPortOk: {
            CheckResultWithMessage(protocol->ReadGetPortOk(channel->GetReader(), port), "Could not receive port ok frame.");
            return Result::Ok;
        }
        case FrameKind::Error: {
            std::string errorMessage;
            CheckResultWithMessage(protocol->ReadError(channel->GetReader(), errorMessage), "Could not read error frame.");
            Logger::Instance().LogError(errorMessage);
            return Result::Error;
        }
        default:
            std::ostringstream oss;
            oss << "PortMapperGetPort: Received unexpected frame '" << frameKind << "'.";
            Logger::Instance().LogError(oss.str());
            return Result::Error;
    }
}

[[nodiscard]] Result PortMapperSetPort(const std::string& name, uint16_t port) {
    std::unique_ptr<Channel> channel;
    CheckResult(TryConnectToTcpChannel("127.0.0.1", GetPortMapperPort(), 0, ClientTimeoutInMilliseconds, channel));
    CheckBoolWithMessage(channel, "Could not connect to port mapper.");

    std::unique_ptr<IProtocol> protocol;
    CheckResult(CreateProtocol(ProtocolVersion1, protocol));

    CheckResultWithMessage(protocol->SendSetPort(channel->GetWriter(), name, port), "Could not send set port frame.");

    FrameKind frameKind{};
    CheckResult(protocol->ReceiveHeader(channel->GetReader(), frameKind));

    switch (frameKind) {
        // NOLINT(clang-diagnostic-switch-enum)
        case FrameKind::Ok:
            CheckResultWithMessage(protocol->ReadOk(channel->GetReader()), "Could not read ok frame.");
            return Result::Ok;
        case FrameKind::Error: {
            std::string errorString;
            CheckResultWithMessage(protocol->ReadError(channel->GetReader(), errorString), "Could not read error frame.");
            Logger::Instance().LogError(errorString);
            return Result::Error;
        }
        default:
            std::ostringstream oss;
            oss << "PortMapperSetPort: Received unexpected frame '" << frameKind << "'.";
            Logger::Instance().LogError(oss.str());
            return Result::Error;
    }
}

[[nodiscard]] Result PortMapperUnsetPort(const std::string& name) {
    std::unique_ptr<Channel> channel;
    CheckResult(TryConnectToTcpChannel("127.0.0.1", GetPortMapperPort(), 0, ClientTimeoutInMilliseconds, channel));
    CheckBoolWithMessage(channel, "Could not connect to port mapper.");

    std::unique_ptr<IProtocol> protocol;
    CheckResult(CreateProtocol(ProtocolVersion1, protocol));

    CheckResultWithMessage(protocol->SendUnsetPort(channel->GetWriter(), name), "Could not send unset port frame.");

    FrameKind frameKind{};
    CheckResult(protocol->ReceiveHeader(channel->GetReader(), frameKind));

    switch (frameKind) {
        // NOLINT(clang-diagnostic-switch-enum)
        case FrameKind::Ok:
            CheckResultWithMessage(protocol->ReadOk(channel->GetReader()), "Could not read ok frame.");
            return Result::Ok;
        case FrameKind::Error: {
            std::string errorString;
            CheckResultWithMessage(protocol->ReadError(channel->GetReader(), errorString), "Could not read error frame.");
            Logger::Instance().LogError(errorString);
            return Result::Error;
        }
        default:
            std::ostringstream oss;
            oss << "PortMapperUnsetPort: Received unexpected frame '" << frameKind << "'.";
            Logger::Instance().LogError(oss.str());
            return Result::Error;
    }
}

}  // namespace DsVeosCoSim
