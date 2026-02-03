// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "Environment.h"

#include <cstddef>  // IWYU pragma: keep
#include <cstdint>
#include <cstdlib>
#include <string>

namespace DsVeosCoSim {

namespace {

[[nodiscard]] bool TryGetDecimalValue(const std::string& name, size_t& intValue) {
    if (char* stringValue = std::getenv(name.c_str()); stringValue) {  // NOLINT(concurrency-mt-unsafe)
        char* end{};
        if constexpr (sizeof(void*) == 8) {
            intValue = std::strtoull(stringValue, &end, 10);
        } else {
            intValue = std::strtoul(stringValue, &end, 10);
        }

        return true;
    }

    return false;
}

[[nodiscard]] bool TryGetHexValue(const std::string& name, size_t& hexValue) {
    if (char* stringValue = std::getenv(name.c_str()); stringValue) {  // NOLINT(concurrency-mt-unsafe)
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

[[nodiscard]] bool GetBoolValue(const std::string& name) {
    size_t intValue{};
    if (TryGetDecimalValue(name, intValue)) {
        return intValue != 0;
    }

    return false;
}

[[nodiscard]] uint16_t GetPortMapperPortInitial() {
    constexpr uint16_t defaultPort = 27027;
    constexpr int32_t maxPort = 65535;

    size_t intValue{};
    if (TryGetDecimalValue("VEOS_COSIM_PORTMAPPER_PORT", intValue)) {
        if ((intValue > 0) && (intValue <= maxPort)) {
            return static_cast<uint16_t>(intValue);
        }
    }

    return defaultPort;
}

[[nodiscard]] bool TryGetSpinCount(const std::string& name, uint32_t& spinCount) {
    size_t intValue{};
    if (TryGetDecimalValue(name, intValue)) {
        spinCount = static_cast<uint32_t>(intValue);
        return true;
    }

    return false;
}

}  // namespace

[[nodiscard]] bool IsProtocolTracingEnabled() {
    static bool verbose = GetBoolValue("VEOS_COSIM_PROTOCOL_TRACING");
    return verbose;
}

[[nodiscard]] bool IsProtocolHeaderTracingEnabled() {
    static bool verbose = GetBoolValue("VEOS_COSIM_PROTOCOL_HEADER_TRACING");
    return verbose;
}

[[nodiscard]] bool IsProtocolPingTracingEnabled() {
    static bool verbose = GetBoolValue("VEOS_COSIM_PROTOCOL_PING_TRACING");
    return verbose;
}

[[nodiscard]] bool IsPortMapperServerVerbose() {
    static bool verbose = GetBoolValue("VEOS_COSIM_PORTMAPPER_SERVER_VERBOSE");
    return verbose;
}

[[nodiscard]] bool IsPortMapperClientVerbose() {
    static bool verbose = GetBoolValue("VEOS_COSIM_PORTMAPPER_CLIENT_VERBOSE");
    return verbose;
}

[[nodiscard]] uint16_t GetPortMapperPort() {
    static uint16_t port = GetPortMapperPortInitial();
    return port;
}

[[nodiscard]] uint32_t GetSpinCount(const std::string& name, const std::string& part, const std::string& direction) {
    constexpr uint32_t defaultSpinCount = 0;
    constexpr char environmentVariableName[] = "VEOS_COSIM_SPIN_COUNT";

    uint32_t spinCount{};

    std::string directionFullName(environmentVariableName);
    directionFullName.append("_").append(name).append(".").append(part).append(".").append(direction);
    if (TryGetSpinCount(directionFullName, spinCount)) {
        return spinCount;
    }

    std::string partFullName(environmentVariableName);
    partFullName.append("_").append(name).append(".").append(part);
    if (TryGetSpinCount(partFullName, spinCount)) {
        return spinCount;
    }

    std::string fullName(environmentVariableName);
    fullName.append("_").append(name);
    if (TryGetSpinCount(fullName, spinCount)) {
        return spinCount;
    }

    if (TryGetSpinCount(environmentVariableName, spinCount)) {
        return spinCount;
    }

    return defaultSpinCount;
}

[[nodiscard]] bool TryGetAffinityMask(const std::string& name, size_t& mask) {
    constexpr char environmentVariableName[] = "VEOS_COSIM_AFFINITY_MASK";

    std::string fullName(environmentVariableName);
    fullName.append("_");
    fullName.append(name);
    if (TryGetHexValue(fullName, mask)) {
        return true;
    }

    return TryGetHexValue(environmentVariableName, mask);
}

}  // namespace DsVeosCoSim
