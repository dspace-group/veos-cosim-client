// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "DsVeosCoSim/CoSimTypes.h"

namespace DsVeosCoSim {

class PortMapperServer {
protected:
    PortMapperServer() = default;

public:
    virtual ~PortMapperServer() = default;

    PortMapperServer(const PortMapperServer&) = delete;
    PortMapperServer& operator=(const PortMapperServer&) = delete;

    PortMapperServer(PortMapperServer&&) = delete;
    PortMapperServer& operator=(PortMapperServer&&) = delete;
};

[[nodiscard]] Result CreatePortMapperServer(bool enableRemoteAccess, std::unique_ptr<PortMapperServer>& portMapperServer);

[[nodiscard]] Result PortMapperGetPort(const std::string& ipAddress, const std::string& serverName, uint16_t& port);
[[nodiscard]] Result PortMapperSetPort(const std::string& name, uint16_t port);
[[nodiscard]] Result PortMapperUnsetPort(const std::string& name);

}  // namespace DsVeosCoSim
