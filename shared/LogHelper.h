// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <fmt/format.h>

#include <string>
#include <string_view>

#include "CoSimTypes.h"

void InitializeOutput();

void OnLogCallback(DsVeosCoSim::Severity severity, std::string_view message);

template <typename... T>
void LogError(fmt::format_string<T...> format, T&&... args) {
    OnLogCallback(DsVeosCoSim::Severity::Error, fmt::vformat(format, fmt::make_format_args(args...)));
}

template <typename... T>
void LogWarning(fmt::format_string<T...> format, T&&... args) {
    OnLogCallback(DsVeosCoSim::Severity::Warning, fmt::vformat(format, fmt::make_format_args(args...)));
}

template <typename... T>
void LogInfo(fmt::format_string<T...> format, T&&... args) {
    OnLogCallback(DsVeosCoSim::Severity::Info, fmt::vformat(format, fmt::make_format_args(args...)));
}

template <typename... T>
void LogTrace(fmt::format_string<T...> format, T&&... args) {
    OnLogCallback(DsVeosCoSim::Severity::Trace, fmt::vformat(format, fmt::make_format_args(args...)));
}

void ClearLastMessage();
[[nodiscard]] std::string GetLastMessage();
