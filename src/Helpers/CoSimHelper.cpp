// Copyright dSPACE GmbH. All rights reserved.

#include "CoSimHelper.h"

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

std::string GetSystemErrorMessage(int32_t errorCode) {
    return fmt::format("Error code: {}. {}", errorCode, std::system_category().message(errorCode));
}

}  // namespace DsVeosCoSim
