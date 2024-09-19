// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <fmt/format.h>

#include "CoSimTypes.h"

namespace DsVeosCoSim {

void SetLogCallback(LogCallback logCallback);

void LogError(std::string_view message);
void LogWarning(std::string_view message);
void LogInfo(std::string_view message);
void LogTrace(std::string_view message);

template <typename... T>
void LogError(fmt::format_string<T...> format, T&&... args) {
    LogError(vformat(format, fmt::make_format_args(args...)));
}

template <typename... T>
void LogWarning(fmt::format_string<T...> format, T&&... args) {
    LogWarning(vformat(format, fmt::make_format_args(args...)));
}

template <typename... T>
void LogInfo(fmt::format_string<T...> format, T&&... args) {
    LogInfo(vformat(format, fmt::make_format_args(args...)));
}

template <typename... T>
void LogTrace(fmt::format_string<T...> format, T&&... args) {
    LogTrace(vformat(format, fmt::make_format_args(args...)));
}

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

    CoSimException(std::string_view message, int32_t errorCode)
        : std::runtime_error(fmt::format("{} {}", message, GetSystemErrorMessage(errorCode))) {
    }
};

}  // namespace DsVeosCoSim
