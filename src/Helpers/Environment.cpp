// Copyright dSPACE GmbH. All rights reserved.

#include "Environment.h"

#include <cstdlib>
#include <string>

namespace DsVeosCoSim {

namespace {

[[nodiscard]] bool GetBoolValue(const std::string& name) {
    const char* stringValue = std::getenv(name.c_str());  // NOLINT(concurrency-mt-unsafe)
    if (stringValue) {
        const int32_t intValue = std::atoi(stringValue);  // NOLINT(cert-err34-c)
        return intValue != 0;
    }

    return false;
}

[[nodiscard]] uint16_t GetPortMapperPortInitial() {
    constexpr uint16_t defaultPort = 27027;

    const char* portString = std::getenv("VEOS_COSIM_PORTMAPPER_PORT");  // NOLINT(concurrency-mt-unsafe)
    if (portString) {
        const int32_t port = std::atoi(portString);  // NOLINT(cert-err34-c)
        if (port > 0 && port <= UINT16_MAX) {
            return static_cast<uint16_t>(port);
        }
    }

    return defaultPort;
}

}  // namespace

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

}  // namespace DsVeosCoSim
