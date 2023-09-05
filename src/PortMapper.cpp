// Copyright dSPACE GmbH. All rights reserved.

#include "PortMapper.h"

#include "CoSimTypes.h"
#include "Communication.h"
#include "Logger.h"
#include "Protocol.h"

namespace DsVeosCoSim {

namespace {

bool IsPortMapperClientVerbose() {
    constexpr bool defaultVerbose = false;

    const char* verboseString = std::getenv("VEOS_COSIM_PORTMAPPER_CLIENT_VERBOSE");  // NOLINT(concurrency-mt-unsafe)
    if (verboseString) {
        const int verbose = std::atoi(verboseString);  // NOLINT(cert-err34-c)
        return verbose != 0;
    }

    return defaultVerbose;
}

[[nodiscard]] uint16_t GetPortMapperPortInitial() {
    constexpr uint16_t defaultPort = 27027;

    const char* portString = getenv("VEOS_COSIM_PORTMAPPER_PORT");  // NOLINT(concurrency-mt-unsafe)
    if (portString) {
        const int port = std::atoi(portString);  // NOLINT(cert-err34-c)
        if (port > 0 && port <= UINT16_MAX) {
            return static_cast<uint16_t>(port);
        }
    }

    return defaultPort;
}

}  // namespace

uint16_t GetPortMapperPort() {
    static uint16_t port = GetPortMapperPortInitial();
    return port;
}

Result PortMapper_GetPort(std::string_view ipAddress, std::string_view serverName, uint16_t& port) {
    if (IsPortMapperClientVerbose()) {
        LogTrace("PortMapper_GetPort(ipAddress: " + std::string(ipAddress) + ", serverName: " + std::string(serverName) + ", port: " + std::to_string(port) +
                 ")");
    }

    Channel channel;
    CheckResult(ConnectToServer(ipAddress, GetPortMapperPort(), 0, channel));
    CheckResult(Protocol::SendGetPort(channel, serverName));

    FrameKind frameKind = FrameKind::Unknown;
    CheckResult(Protocol::ReceiveHeader(channel, frameKind));

    switch (frameKind) {  // NOLINT(clang-diagnostic-switch-enum)
        case FrameKind::GetPortResponse:
            return Protocol::ReadGetPortResponse(channel, port);
        case FrameKind::Error: {
            std::string errorMessage;
            CheckResult(Protocol::ReadError(channel, errorMessage));
            LogError(errorMessage);
            return Result::Error;
        }
        default:
            LogError("PortMapper_GetPort: Received unexpected frame " + ToString(frameKind) + ".");
            return Result::Error;
    }
}

}  // namespace DsVeosCoSim
