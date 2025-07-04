// Copyright dSPACE GmbH. All rights reserved.

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

[[nodiscard]] uint32_t GetSpinCount(std::string_view name, std::string_view part, std::string_view direction);

[[nodiscard]] bool TryGetAffinityMask(std::string_view name, size_t& mask);

}  // namespace DsVeosCoSim
