// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <cstdint>

namespace DsVeosCoSim {

[[nodiscard]] bool IsPortMapperServerVerbose();
[[nodiscard]] bool IsPortMapperClientVerbose();

[[nodiscard]] uint16_t GetPortMapperPort();

}  // namespace DsVeosCoSim
