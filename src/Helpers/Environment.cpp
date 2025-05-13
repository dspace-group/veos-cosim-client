// Copyright dSPACE GmbH. All rights reserved.

#include "Environment.h"

#include <cstdint>
#include <cstdlib>
#include <string>

namespace DsVeosCoSim {

namespace {

[[nodiscard]] bool GetBoolValue(const std::string& name) {
    const char* stringValue = std::getenv(name.c_str());
    if (stringValue) {
        const int32_t intValue = std::atoi(stringValue);
        return intValue != 0;
    }

    return false;
}

[[nodiscard]] uint16_t GetPortMapperPortInitial() {
    constexpr uint16_t defaultPort = 27027;

    const char* portString = std::getenv("VEOS_COSIM_PORTMAPPER_PORT");
    if (portString) {
        const int32_t port = std::atoi(portString);
        if ((port > 0) && (port <= UINT16_MAX)) {
            return static_cast<uint16_t>(port);
        }
    }

    return defaultPort;
}

[[nodiscard]] bool TryGetAffinityMaskInitial(const std::string& environmentVariableName, size_t& mask) {
    constexpr size_t defaultMask = SIZE_MAX;

    const char* maskString = std::getenv(environmentVariableName.c_str());
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

[[nodiscard]] bool TryGetAffinityMask(const std::string_view name, size_t& mask) {
    const std::string environmentVariableName = "VEOS_COSIM_AFFINITY_MASK";

    std::string fullName = environmentVariableName;
    fullName.append("_");
    fullName.append(name);
    if (TryGetAffinityMaskInitial(fullName, mask)) {
        return true;
    }

    return TryGetAffinityMaskInitial(environmentVariableName, mask);
}

}  // namespace DsVeosCoSim
