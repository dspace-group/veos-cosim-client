// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>

#include "Channel.hpp"
#include "Event.hpp"
#include "Protocol.hpp"
#include "Result.hpp"

namespace DsVeosCoSim {

class PortMapperServer final {
public:
    PortMapperServer(std::unique_ptr<ChannelServer> channelServer, std::unique_ptr<IProtocol> protocol);
    ~PortMapperServer() noexcept;

    PortMapperServer(const PortMapperServer&) = delete;
    PortMapperServer& operator=(const PortMapperServer&) = delete;

    PortMapperServer(PortMapperServer&&) = delete;
    PortMapperServer& operator=(PortMapperServer&&) = delete;

private:
    void RunPortMapperServer();
    [[nodiscard]] Result CheckForClient();
    [[nodiscard]] Result HandleClient(Channel& channel);
    [[nodiscard]] Result HandleGetPort(Channel& channel);
    [[nodiscard]] Result HandleSetPort(Channel& channel);
    [[nodiscard]] Result HandleUnsetPort(Channel& channel);
    void DumpEntries();

    std::unordered_map<std::string, uint16_t> _ports;
    std::unique_ptr<ChannelServer> _server;
    std::thread _thread;
    Event _stopEvent;
    std::unique_ptr<IProtocol> _protocol;
};

[[nodiscard]] Result CreatePortMapperServer(bool enableRemoteAccess, std::unique_ptr<PortMapperServer>& portMapperServer);

[[nodiscard]] Result PortMapperGetPort(const std::string& ipAddress, std::string_view serverName, uint16_t& port);
[[nodiscard]] Result PortMapperSetPort(std::string_view name, uint16_t port);
[[nodiscard]] Result PortMapperUnsetPort(std::string_view name);

}  // namespace DsVeosCoSim
