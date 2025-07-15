// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <string>

namespace DsVeosCoSim {

void LogError(const std::string& message);
void LogWarning(const std::string& message);
void LogInfo(const std::string& message);
void LogTrace(const std::string& message);
void LogSystemError(const std::string& message, int32_t errorCode);
void LogProtocolBeginTrace(const std::string& message);
void LogProtocolEndTrace(const std::string& message);
void LogProtocolDataTrace(const std::string& message);

#define CheckResultWithMessage(result, message) \
    do {                                        \
        Result _result_ = (result);             \
        if (!IsOk(_result_)) {                  \
            LogTrace(message);                  \
            return _result_;                    \
        }                                       \
    } while (0)

#define CheckBoolWithMessage(result, message) \
    do {                                      \
        if (!(result)) {                      \
            LogTrace(message);                \
            return Result::Error;             \
        }                                     \
    } while (0)

[[nodiscard]] std::string GetSystemErrorMessage(int32_t errorCode);

}  // namespace DsVeosCoSim
