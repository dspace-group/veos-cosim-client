// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <fmt/format.h>
#include <string>
#include <string_view>

#include "CoSimHelper.h"
#include "DsVeosCoSim/DsVeosCoSim.h"

void InitializeOutput();

template <typename... T>
void LogError(fmt::format_string<T...> format, T&&... args) {
    DsVeosCoSim::LogError(fmt::vformat(format, fmt::make_format_args(args...)));
}

template <typename... T>
void LogWarning(fmt::format_string<T...> format, T&&... args) {
    DsVeosCoSim::LogWarning(fmt::vformat(format, fmt::make_format_args(args...)));
}

template <typename... T>
void LogInfo(fmt::format_string<T...> format, T&&... args) {
    DsVeosCoSim::LogInfo(fmt::vformat(format, fmt::make_format_args(args...)));
}

template <typename... T>
void LogTrace(fmt::format_string<T...> format, T&&... args) {
    DsVeosCoSim::LogTrace(fmt::vformat(format, fmt::make_format_args(args...)));
}

void OnLogCallback(DsVeosCoSim_Severity severity, std::string_view message);

void ClearLastMessage();
[[nodiscard]] std::string GetLastMessage();
