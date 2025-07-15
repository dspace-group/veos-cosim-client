// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <string>

namespace DsVeosCoSim {

[[nodiscard]] bool IsProtocolTracingEnabled();
[[nodiscard]] bool IsProtocolHeaderTracingEnabled();
[[nodiscard]] bool IsProtocolPingTracingEnabled();

[[nodiscard]] bool IsPortMapperServerVerbose();
[[nodiscard]] bool IsPortMapperClientVerbose();

[[nodiscard]] uint16_t GetPortMapperPort();

[[nodiscard]] uint32_t GetSpinCount(const std::string& name, const std::string& part, const std::string& direction);

[[nodiscard]] bool TryGetAffinityMask(const std::string& name, size_t& mask);

}  // namespace DsVeosCoSim
