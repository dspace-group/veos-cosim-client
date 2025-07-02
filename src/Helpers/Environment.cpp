// Copyright dSPACE GmbH. All rights reserved.

#include "Environment.h"

#include <cstdint>
#include <cstdlib>
#include <string>
#include <string_view>

namespace DsVeosCoSim {

namespace {

[[nodiscard]] bool TryGetDecimalValue(std::string_view name, ptrdiff_t& intValue) {
    char* stringValue = std::getenv(name.data());
    if (stringValue) {
        char* end{};
        if constexpr (sizeof(void*) == 8) {
            intValue = std::strtoll(stringValue, &end, 10);
        } else {
            intValue = std::strtol(stringValue, &end, 10);
        }

        return true;
    }

    return false;
}

[[nodiscard]] bool TryGetHexValue(std::string_view name, size_t& hexValue) {
    char* stringValue = std::getenv(name.data());
    if (stringValue) {
        char* end{};
        if constexpr (sizeof(void*) == 8) {
            hexValue = std::strtoull(stringValue, &end, 16);
        } else {
            hexValue = std::strtoul(stringValue, &end, 16);
        }

        return true;
    }

    return false;
}

[[nodiscard]] bool GetBoolValue(std::string_view name) {
    ptrdiff_t intValue{};
    if (TryGetDecimalValue(name, intValue)) {
        return intValue != 0;
    }

    return false;
}

[[nodiscard]] uint16_t GetPortMapperPortInitial() {
    constexpr uint16_t defaultPort = 27027;
    constexpr int32_t maxPort = 65535;

    ptrdiff_t intValue{};
    if (TryGetDecimalValue("VEOS_COSIM_PORTMAPPER_PORT", intValue)) {
        if ((intValue > 0) && (intValue <= maxPort)) {
            return static_cast<uint16_t>(intValue);
        }
    }

    return defaultPort;
}

[[nodiscard]] bool TryGetSpinCount(std::string_view name, uint32_t& spinCount) {
    ptrdiff_t intValue{};
    if (TryGetDecimalValue(name, intValue)) {
        if ((intValue >= 0) && (intValue <= UINT32_MAX)) {
            spinCount = static_cast<uint32_t>(intValue);
            return true;
        }
    }

    return false;
}

}  // namespace

[[nodiscard]] bool IsProtocolTracingEnabled() {
    static bool Verbose = GetBoolValue("VEOS_COSIM_PROTOCOL_TRACING");
    return Verbose;
}

[[nodiscard]] bool IsProtocolHeaderTracingEnabled() {
    static bool Verbose = GetBoolValue("VEOS_COSIM_PROTOCOL_HEADER_TRACING");
    return Verbose;
}

[[nodiscard]] bool IsProtocolPingTracingEnabled() {
    static bool Verbose = GetBoolValue("VEOS_COSIM_PROTOCOL_PING_TRACING");
    return Verbose;
}

[[nodiscard]] bool IsPortMapperServerVerbose() {
    static bool Verbose = GetBoolValue("VEOS_COSIM_PORTMAPPER_SERVER_VERBOSE");
    return Verbose;
}

[[nodiscard]] bool IsPortMapperClientVerbose() {
    static bool Verbose = GetBoolValue("VEOS_COSIM_PORTMAPPER_CLIENT_VERBOSE");
    return Verbose;
}

[[nodiscard]] uint16_t GetPortMapperPort() {
    static uint16_t Port = GetPortMapperPortInitial();
    return Port;
}

[[nodiscard]] uint32_t GetSpinCount(std::string_view name) {
    constexpr uint32_t defaultSpinCount = 0;
    std::string_view environmentVariableName = "VEOS_COSIM_SPIN_COUNT";

    uint32_t spinCount{};

    std::string fullName(environmentVariableName);
    fullName.append("_");
    fullName.append(name);
    if (TryGetSpinCount(fullName, spinCount)) {
        return spinCount;
    }

    if (TryGetSpinCount(environmentVariableName, spinCount)) {
        return spinCount;
    }

    return defaultSpinCount;
}

[[nodiscard]] bool TryGetAffinityMask(std::string_view name, size_t& mask) {
    std::string_view environmentVariableName = "VEOS_COSIM_AFFINITY_MASK";

    std::string fullName(environmentVariableName);
    fullName.append("_");
    fullName.append(name);
    if (TryGetHexValue(fullName, mask)) {
        return true;
    }

    return TryGetHexValue(environmentVariableName, mask);
}

}  // namespace DsVeosCoSim
