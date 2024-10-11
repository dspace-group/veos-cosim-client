// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <stdexcept>
#include <string>
#include <string_view>

#include "CoSimTypes.h"

namespace DsVeosCoSim {

void SetLogCallback(LogCallback logCallback);

void LogError(std::string_view message);
void LogWarning(std::string_view message);
void LogInfo(std::string_view message);
void LogTrace(std::string_view message);
void LogProtocolBeginTrace(const std::string& message);
void LogProtocolEndTrace(const std::string& message);

#define CheckResultWithMessage(result, message) \
    do {                                        \
        if (!(result)) [[unlikely]] {           \
            LogTrace(message);                  \
            return false;                       \
        }                                       \
    } while (0)

#define CheckResult(result)           \
    do {                              \
        if (!(result)) [[unlikely]] { \
            return false;             \
        }                             \
    } while (0)

[[nodiscard]] std::string GetSystemErrorMessage(int32_t errorCode);

class CoSimException final : public std::runtime_error {
public:
    explicit CoSimException(std::string_view message) : std::runtime_error(message.data()) {
    }

    CoSimException(const std::string& message, int32_t errorCode)
        : std::runtime_error(message + " " + GetSystemErrorMessage(errorCode)) {
    }
};

}  // namespace DsVeosCoSim
