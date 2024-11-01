// Copyright dSPACE GmbH. All rights reserved.

#include "CoSimHelper.h"

#include <string>
#include <string_view>
#include <system_error>

namespace DsVeosCoSim {

namespace {

LogCallback g_logCallback;

}  // namespace

void SetLogCallback(LogCallback logCallback) {
    g_logCallback = std::move(logCallback);
}

void LogError(std::string_view message) {
    const auto logCallback = g_logCallback;
    if (logCallback) {
        logCallback(DsVeosCoSim_Severity_Error, message.data());
    }
}

void LogWarning(std::string_view message) {
    const auto logCallback = g_logCallback;
    if (logCallback) {
        logCallback(DsVeosCoSim_Severity_Warning, message);
    }
}

void LogInfo(std::string_view message) {
    const auto logCallback = g_logCallback;
    if (logCallback) {
        logCallback(DsVeosCoSim_Severity_Info, message);
    }
}

void LogTrace(std::string_view message) {
    const auto logCallback = g_logCallback;
    if (logCallback) {
        logCallback(DsVeosCoSim_Severity_Trace, message);
    }
}

void LogProtocolBeginTrace(const std::string& message) {
    LogTrace("PROT BEGIN " + message);
}

void LogProtocolEndTrace(const std::string& message) {
    LogTrace("PROT END   " + message);
}

[[nodiscard]] std::string GetSystemErrorMessage(int32_t errorCode) {
    return "Error code: " + std::to_string(errorCode) + ". " + std::system_category().message(errorCode);
}

}  // namespace DsVeosCoSim
