// Copyright dSPACE GmbH. All rights reserved.

#include "Environment.h"

#include <cstdint>
#include <cstdlib>
#include <string>
#include <string_view>

namespace DsVeosCoSim {

namespace {

[[nodiscard]] bool GetBoolValue(const std::string& name) {
    char* stringValue = std::getenv(name.c_str());  // NOLINT(concurrency-mt-unsafe)
    if (stringValue) {
        char* end{};
        int32_t intValue = std::strtol(stringValue, &end, 10);
        return intValue != 0;
    }

    return false;
}

[[nodiscard]] uint16_t GetPortMapperPortInitial() {
    constexpr uint16_t defaultPort = 27027;
    constexpr int32_t maxPort = 65535;

    char* portString = std::getenv("VEOS_COSIM_PORTMAPPER_PORT");  // NOLINT(concurrency-mt-unsafe)
    if (portString) {
        char* end{};
        int32_t port = std::strtol(portString, &end, 10);
        if ((port > 0) && (port <= maxPort)) {
            return static_cast<uint16_t>(port);
        }
    }

    return defaultPort;
}

[[nodiscard]] bool TryGetAffinityMaskInitial(const std::string& environmentVariableName, size_t& mask) {
    constexpr size_t defaultMask = SIZE_MAX;

    char* maskString = std::getenv(environmentVariableName.c_str());  // NOLINT(concurrency-mt-unsafe)
    if (maskString) {
        char* end{};
        if constexpr (sizeof(void*) == 8) {
            mask = std::strtoull(maskString, &end, 16);
        } else {
            mask = std::strtoul(maskString, &end, 16);
        }

        return true;
    }

    mask = defaultMask;
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

[[nodiscard]] bool TryGetAffinityMask(std::string_view name, size_t& mask) {
    std::string environmentVariableName = "VEOS_COSIM_AFFINITY_MASK";

    std::string fullName = environmentVariableName;
    fullName.append("_");
    fullName.append(name);
    if (TryGetAffinityMaskInitial(fullName, mask)) {
        return true;
    }

    return TryGetAffinityMaskInitial(environmentVariableName, mask);
}

}  // namespace DsVeosCoSim
