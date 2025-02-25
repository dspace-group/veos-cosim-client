// Copyright dSPACE GmbH. All rights reserved.

#include "CoSimHelper.h"

#include <string>
#include <string_view>  // IWYU pragma: keep
#include <system_error>

namespace DsVeosCoSim {

namespace {

LogCallback LogCallbackHandler;

}  // namespace

void SetLogCallback(LogCallback logCallback) {
    LogCallbackHandler = std::move(logCallback);
}

void LogError(const std::string_view message) {
    if (const auto logCallback = LogCallbackHandler) {
        logCallback(Severity::Error, message);
    }
}

void LogWarning(const std::string_view message) {
    if (const auto logCallback = LogCallbackHandler) {
        logCallback(Severity::Warning, message);
    }
}

void LogInfo(const std::string_view message) {
    if (const auto logCallback = LogCallbackHandler) {
        logCallback(Severity::Info, message);
    }
}

void LogTrace(const std::string_view message) {
    if (const auto logCallback = LogCallbackHandler) {
        logCallback(Severity::Trace, message);
    }
}

void LogProtocolBeginTrace(const std::string& message) {
    LogTrace("PROT BEGIN " + message);
}

void LogProtocolEndTrace(const std::string& message) {
    LogTrace("PROT END   " + message);
}

void LogProtocolDataTrace(const std::string& message) {
    LogTrace("PROT DATA  " + message);
}

[[nodiscard]] std::string GetSystemErrorMessage(const int32_t errorCode) {
    return "Error code: " + std::to_string(errorCode) + ". " + std::system_category().message(errorCode);
}

}  // namespace DsVeosCoSim
