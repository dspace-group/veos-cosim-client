// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <string>
#include <string_view>  // IWYU pragma: keep

namespace DsVeosCoSim {

void LogError(std::string_view message);
void LogWarning(std::string_view message);
void LogInfo(std::string_view message);
void LogTrace(std::string_view message);
void LogProtocolBeginTrace(const std::string& message);
void LogProtocolEndTrace(const std::string& message);
void LogProtocolDataTrace(const std::string& message);

#define CheckResultWithMessage(result, message) \
    do {                                        \
        if (!(result)) {                        \
            LogTrace(message);                  \
            return {};                          \
        }                                       \
    } while (0)

#define CheckResult(result) \
    do {                    \
        if (!(result)) {    \
            return {};      \
        }                   \
    } while (0)

[[nodiscard]] std::string GetSystemErrorMessage(int32_t errorCode);

}  // namespace DsVeosCoSim
