// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include "CoSimTypes.h"

namespace DsVeosCoSim {

[[nodiscard]] uint16_t GetPortMapperPort();
[[nodiscard]] Result PortMapper_GetPort(std::string_view ipAddress, std::string_view serverName, uint16_t& port);

}  // namespace DsVeosCoSim
