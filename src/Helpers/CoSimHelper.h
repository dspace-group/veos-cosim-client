// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <format>

#include "CoSimTypes.h"

namespace DsVeosCoSim {

void SetLogCallback(LogCallback logCallback);

void LogError(std::string_view message);
void LogWarning(std::string_view message);
void LogInfo(std::string_view message);
void LogTrace(std::string_view message);

template <typename... T>
void LogError(std::format_string<T...> format, T&&... args) {
    LogError(std::vformat(format.get(), std::make_format_args(args...)));
}

template <typename... T>
void LogWarning(std::format_string<T...> format, T&&... args) {
    LogWarning(std::vformat(format.get(), std::make_format_args(args...)));
}

template <typename... T>
void LogInfo(std::format_string<T...> format, T&&... args) {
    LogInfo(std::vformat(format.get(), std::make_format_args(args...)));
}

template <typename... T>
void LogTrace(std::format_string<T...> format, T&&... args) {
    LogTrace(std::vformat(format.get(), std::make_format_args(args...)));
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
        : std::runtime_error(std::format("{} {}", message, GetSystemErrorMessage(errorCode))) {
    }
};

}  // namespace DsVeosCoSim
