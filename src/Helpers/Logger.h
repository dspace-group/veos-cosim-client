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

}  // namespace DsVeosCoSim
