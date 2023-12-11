// Copyright dSPACE GmbH. All rights reserved.

#include "Logger.h"

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
        logCallback(Severity::Error, message.data());
    }
}

void LogSystemError(std::string_view prefix, int errorCode) {
    const std::string message = std::string(prefix) + " Error code: " + std::to_string(errorCode) + ". " + std::system_category().message(errorCode);

    const auto logCallback = g_logCallback;
    if (logCallback) {
        logCallback(Severity::Error, message);
    }
}

void LogWarning(std::string_view message) {
    const auto logCallback = g_logCallback;
    if (logCallback) {
        logCallback(Severity::Warning, message);
    }
}

void LogInfo(std::string_view message) {
    const auto logCallback = g_logCallback;
    if (logCallback) {
        logCallback(Severity::Info, message);
    }
}

void LogTrace(std::string_view message) {
    const auto logCallback = g_logCallback;
    if (logCallback) {
        logCallback(Severity::Trace, message);
    }
}

}  // namespace DsVeosCoSim
