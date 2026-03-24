// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

#include "Result.hpp"

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

[[nodiscard]] Result CreatePortMapperServer(bool enableRemoteAccess, std::unique_ptr<PortMapperServer>& portMapperServer);

[[nodiscard]] Result PortMapperGetPort(const std::string& ipAddress, std::string_view serverName, uint16_t& port);
[[nodiscard]] Result PortMapperSetPort(std::string_view name, uint16_t port);
[[nodiscard]] Result PortMapperUnsetPort(std::string_view name);

}  // namespace DsVeosCoSim
