// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "Logger.h"

#include <string>
#include <utility>

#ifndef _WIN32
#include <system_error>
#endif

#include <fmt/format.h>

#include "CoSimTypes.h"
#include "OsUtilities.h"

namespace DsVeosCoSim {

namespace {

[[nodiscard]] std::string GetSystemErrorMessage(int32_t errorCode) {
#if _WIN32
    return fmt::format("Error code: {}. {}", errorCode, GetEnglishErrorMessage(errorCode));
#else
    return fmt::format("Error code: {}. {}", errorCode, std::system_category().message(errorCode));
#endif
}

}  // namespace

void Logger::SetLogCallback(LogCallback logCallback) {
    _logCallback = std::move(logCallback);
}

void Logger::LogMessage(Severity severity, const std::string& message) const {
    if (auto logCallback = _logCallback; logCallback) {
        logCallback(severity, message);
    }
}

void Logger::LogError(const std::string& message) {
    LogMessage(Severity::Error, message);
}

void Logger::LogWarning(const std::string& message) {
    LogMessage(Severity::Warning, message);
}

void Logger::LogInfo(const std::string& message) {
    LogMessage(Severity::Info, message);
}

void Logger::LogTrace(const std::string& message) {
    LogMessage(Severity::Trace, message);
}

void Logger::LogSystemError(const std::string& message, int32_t errorCode) {
    LogMessage(Severity::Error, fmt::format("{} {}", message, GetSystemErrorMessage(errorCode)));
}

void Logger::LogProtBegin(const std::string& message) {
    LogTrace("PROT BEGIN {}", message);
}

void Logger::LogProtEnd(const std::string& message) {
    LogTrace("PROT END   {}", message);
}

void Logger::LogProtData(const std::string& message) {
    LogTrace("PROT DATA  {}", message);
}

}  // namespace DsVeosCoSim
