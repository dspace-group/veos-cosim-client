// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <string>
#include <string_view>

namespace DsVeosCoSim {

void LogError(std::string_view message);
void LogSystemError(std::string_view message, int32_t errorCode);
void LogWarning(std::string_view message);
void LogInfo(std::string_view message);
void LogTrace(std::string_view message);
void LogProtocolBeginTrace(std::string_view message);
void LogProtocolEndTrace(std::string_view message);
void LogProtocolDataTrace(std::string_view message);

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
