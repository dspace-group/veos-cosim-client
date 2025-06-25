// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <memory>
#include <string_view>

namespace DsVeosCoSim {

class PortMapperServer {
protected:
    PortMapperServer() noexcept = default;

public:
    virtual ~PortMapperServer() noexcept = default;

    PortMapperServer(const PortMapperServer&) = delete;
    PortMapperServer& operator=(const PortMapperServer&) = delete;

    PortMapperServer(PortMapperServer&&) = delete;
    PortMapperServer& operator=(PortMapperServer&&) = delete;
};

[[nodiscard]] std::unique_ptr<PortMapperServer> CreatePortMapperServer(bool enableRemoteAccess);

[[nodiscard]] bool PortMapper_GetPort(std::string_view ipAddress, std::string_view serverName, uint16_t& port);
[[nodiscard]] bool PortMapper_SetPort(std::string_view name, uint16_t port);
[[nodiscard]] bool PortMapper_UnsetPort(std::string_view name);

}  // namespace DsVeosCoSim
