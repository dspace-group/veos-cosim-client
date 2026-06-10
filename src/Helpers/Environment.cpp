// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "Environment.hpp"

#include <cstddef>  // IWYU pragma: keep
#include <cstdint>
#include <cstdlib>  // IWYU pragma: keep
#include <string>
#include <string_view>

#include <fmt/format.h>

namespace DsVeosCoSim {

namespace {

[[nodiscard]] bool TryGetEnvValue(const std::string& name, size_t& value, int base) {
    if (const char* stringValue = std::getenv(name.c_str()); stringValue) {  // NOLINT(concurrency-mt-unsafe)
        char* end{};
        if constexpr (sizeof(void*) == 8) {
            value = std::strtoull(stringValue, &end, base);
        } else {
            value = std::strtoul(stringValue, &end, base);
        }

        return true;
    }

    return false;
}

[[nodiscard]] bool TryGetDecimalValue(const std::string& name, size_t& intValue) {
    return TryGetEnvValue(name, intValue, 10);
}

[[nodiscard]] bool TryGetHexValue(const std::string& name, size_t& hexValue) {
    return TryGetEnvValue(name, hexValue, 16);
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

[[nodiscard]] uint32_t GetSpinCountInitial() {
    constexpr uint32_t defaultSpinCount = 512;

    size_t intValue{};
    if (TryGetDecimalValue("VEOS_COSIM_SPIN_COUNT", intValue)) {
        return static_cast<uint32_t>(intValue);
    }

    return defaultSpinCount;
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

[[nodiscard]] uint32_t GetSpinCount() {
    static uint32_t spinCount = GetSpinCountInitial();
    return spinCount;
}

[[nodiscard]] bool TryGetAffinityMask(std::string_view name, size_t& mask) {
    constexpr char environmentVariableName[] = "VEOS_COSIM_AFFINITY_MASK";

    std::string fullName = fmt::format("{}_{}", environmentVariableName, name);
    if (TryGetHexValue(fullName, mask)) {
        return true;
    }

    return TryGetHexValue(environmentVariableName, mask);
}

}  // namespace DsVeosCoSim
