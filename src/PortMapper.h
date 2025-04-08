// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace DsVeosCoSim {

class PortMapperServer {
protected:
    PortMapperServer() = default;

public:
    virtual ~PortMapperServer() noexcept = default;

    PortMapperServer(const PortMapperServer&) = delete;
    PortMapperServer& operator=(const PortMapperServer&) = delete;

    PortMapperServer(PortMapperServer&&) = delete;
    PortMapperServer& operator=(PortMapperServer&&) = delete;
};

[[nodiscard]] std::unique_ptr<PortMapperServer> CreatePortMapperServer(bool enableRemoteAccess);

[[nodiscard]] bool PortMapper_GetPort(const std::string& ipAddress, const std::string& serverName, uint16_t& port);
[[nodiscard]] bool PortMapper_SetPort(const std::string& name, uint16_t port);
[[nodiscard]] bool PortMapper_UnsetPort(const std::string& name);

}  // namespace DsVeosCoSim
