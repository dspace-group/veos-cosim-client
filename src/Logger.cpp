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
    const auto callback = g_logCallback;
    if (callback) {
        callback(Severity::Error, message.data());
    }
}

void LogSystemError(std::string_view prefix, int errorCode) {
    const std::string message = std::string(prefix) + " Error code: " + std::to_string(errorCode) + ". " + std::system_category().message(errorCode);

    const auto callback = g_logCallback;
    if (callback) {
        callback(Severity::Error, message);
    }
}

void LogWarning(std::string_view message) {
    const auto callback = g_logCallback;
    if (callback) {
        callback(Severity::Warning, message);
    }
}

void LogInfo(std::string_view message) {
    const auto callback = g_logCallback;
    if (callback) {
        callback(Severity::Info, message);
    }
}

void LogTrace(std::string_view message) {
    const auto callback = g_logCallback;
    if (callback) {
        callback(Severity::Trace, message);
    }
}

}  // namespace DsVeosCoSim
