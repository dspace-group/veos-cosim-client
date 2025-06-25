// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <string>
#include <string_view>

namespace DsVeosCoSim {

void LogError(std::string_view message);
void LogWarning(std::string_view message);
void LogInfo(std::string_view message);
void LogTrace(std::string_view message);
void LogProtocolBeginTrace(std::string_view message);
void LogProtocolEndTrace(std::string_view message);
void LogProtocolDataTrace(std::string_view message);

#define CheckResultWithMessage(result, message) /* NOLINT(cppcoreguidelines-macro-usage) */ \
    do {                                                                                    \
        if (!(result)) {                                                                    \
            LogTrace(message);                                                              \
            return {};                                                                      \
        }                                                                                   \
    } while (0)

#define CheckResult(result) /* NOLINT(cppcoreguidelines-macro-usage) */ \
    do {                                                                \
        if (!(result)) {                                                \
            return {};                                                  \
        }                                                               \
    } while (0)

[[nodiscard]] std::string GetSystemErrorMessage(int32_t errorCode);

}  // namespace DsVeosCoSim
