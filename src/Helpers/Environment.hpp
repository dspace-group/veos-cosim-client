// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <cstdint>
#include <string_view>

namespace DsVeosCoSim {

[[nodiscard]] bool IsProtocolTracingEnabled();
[[nodiscard]] bool IsProtocolHeaderTracingEnabled();
[[nodiscard]] bool IsProtocolPingTracingEnabled();

[[nodiscard]] bool IsPortMapperServerVerbose();
[[nodiscard]] bool IsPortMapperClientVerbose();

[[nodiscard]] uint16_t GetPortMapperPort();

[[nodiscard]] uint32_t GetSpinCount();

[[nodiscard]] bool TryGetAffinityMask(std::string_view name, size_t& mask);

}  // namespace DsVeosCoSim
