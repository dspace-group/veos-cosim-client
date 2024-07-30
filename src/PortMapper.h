// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include "CoSimTypes.h"
#include "Communication.h"

#include <thread>

namespace DsVeosCoSim {

class PortMapperServer {
public:
    PortMapperServer() = default;
    ~PortMapperServer() noexcept;

    PortMapperServer(const PortMapperServer&) = delete;
    PortMapperServer& operator=(const PortMapperServer&) = delete;

    PortMapperServer(PortMapperServer&&) = delete;
    PortMapperServer& operator=(PortMapperServer&&) = delete;

    [[nodiscard]] Result Start(bool enableRemoteAccess);
    void Stop();

private:
    void RunPortMapperServer();
    [[nodiscard]] Result HandleClient();
    [[nodiscard]] Result HandleGetPort();
    [[nodiscard]] Result HandleSetPort();
    [[nodiscard]] Result HandleUnsetPort();
    void DumpEntries();

    std::unordered_map<std::string, uint16_t> _ports;

    Server _server;
    Channel _channel;

    std::thread _thread;

    bool _isRunning = false;
};

[[nodiscard]] Result PortMapper_GetPort(std::string_view ipAddress, std::string_view serverName, uint16_t& port);
[[nodiscard]] Result PortMapper_SetPort(std::string_view name, uint16_t port);
[[nodiscard]] Result PortMapper_UnsetPort(std::string_view name);

}  // namespace DsVeosCoSim
