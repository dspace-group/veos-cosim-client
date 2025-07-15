// Copyright dSPACE GmbH. All rights reserved.

#include "CoSimHelper.h"

#include <cstdint>
#include <string>
#include <utility>

#if _WIN32
#include "OsUtilities.h"
#else
#include <system_error>
#endif

#include "DsVeosCoSim/CoSimTypes.h"

namespace DsVeosCoSim {

namespace {

LogCallback LogCallbackHandler;

}  // namespace

void SetLogCallback(LogCallback logCallback) {
    LogCallbackHandler = std::move(logCallback);
}

void LogError(const std::string& message) {
    auto logCallback = LogCallbackHandler;
    if (logCallback) {
        logCallback(Severity::Error, message);
    }
}

void LogWarning(const std::string& message) {
    auto logCallback = LogCallbackHandler;
    if (logCallback) {
        logCallback(Severity::Warning, message);
    }
}

void LogInfo(const std::string& message) {
    auto logCallback = LogCallbackHandler;
    if (logCallback) {
        logCallback(Severity::Info, message);
    }
}

void LogTrace(const std::string& message) {
    auto logCallback = LogCallbackHandler;
    if (logCallback) {
        logCallback(Severity::Trace, message);
    }
}

void LogSystemError(const std::string& message, int32_t errorCode) {
    auto logCallback = LogCallbackHandler;
    if (logCallback) {
        std::string fullMessage(message);
        fullMessage.append(" ");
        fullMessage.append(GetSystemErrorMessage(errorCode));
        logCallback(Severity::Error, fullMessage);
    }
}

void LogProtocolBeginTrace(const std::string& message) {
    std::string traceMessage = "PROT BEGIN ";
    traceMessage.append(message);
    LogTrace(traceMessage);
}

void LogProtocolEndTrace(const std::string& message) {
    std::string traceMessage = "PROT END   ";
    traceMessage.append(message);
    LogTrace(traceMessage);
}

void LogProtocolDataTrace(const std::string& message) {
    std::string traceMessage = "PROT DATA  ";
    traceMessage.append(message);
    LogTrace(traceMessage);
}

[[nodiscard]] std::string GetSystemErrorMessage(int32_t errorCode) {
    std::string message = "Error code: ";
    message.append(std::to_string(errorCode));
    message.append(". ");

#if _WIN32
    message.append(GetEnglishErrorMessage(errorCode));
#else
    message.append(std::system_category().message(errorCode));
#endif

    return message;
}

}  // namespace DsVeosCoSim
