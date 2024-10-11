// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>

#include "Event.h"
#include "SocketChannel.h"

namespace DsVeosCoSim {

class PortMapperServer {
public:
    explicit PortMapperServer(bool enableRemoteAccess);
    ~PortMapperServer() noexcept;

    PortMapperServer(const PortMapperServer&) = delete;
    PortMapperServer& operator=(const PortMapperServer&) = delete;

    PortMapperServer(PortMapperServer&&) = delete;
    PortMapperServer& operator=(PortMapperServer&&) = delete;

private:
    void RunPortMapperServer();
    [[nodiscard]] bool HandleClient(Channel& channel);
    [[nodiscard]] bool HandleGetPort(Channel& channel);
    [[nodiscard]] bool HandleSetPort(Channel& channel);
    [[nodiscard]] bool HandleUnsetPort(Channel& channel);
    void DumpEntries();

    std::unordered_map<std::string, uint16_t> _ports;

    TcpChannelServer _server;
    std::thread _thread;
    Event _stopEvent;
};

[[nodiscard]] bool PortMapper_GetPort(const std::string& ipAddress, const std::string& serverName, uint16_t& port);
[[nodiscard]] bool PortMapper_SetPort(std::string_view name, uint16_t port);
[[nodiscard]] bool PortMapper_UnsetPort(std::string_view name);

}  // namespace DsVeosCoSim
