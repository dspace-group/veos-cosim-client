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

void LogIoSignal(const DsVeosCoSim_IoSignal& ioSignal);

void LogIoData(DsVeosCoSim_SimulationTime simulationTime,
               const DsVeosCoSim_IoSignal& ioSignal,
               uint32_t length,
               const void* value);

void LogCanController(const DsVeosCoSim_CanController& controller);
void LogEthController(const DsVeosCoSim_EthController& controller);
void LogLinController(const DsVeosCoSim_LinController& controller);

void LogCanMessage(DsVeosCoSim_SimulationTime simulationTime,
                   const DsVeosCoSim_CanController& controller,
                   const DsVeosCoSim_CanMessage& message);
void LogEthMessage(DsVeosCoSim_SimulationTime simulationTime,
                   const DsVeosCoSim_EthController& controller,
                   const DsVeosCoSim_EthMessage& message);
void LogLinMessage(DsVeosCoSim_SimulationTime simulationTime,
                   const DsVeosCoSim_LinController& controller,
                   const DsVeosCoSim_LinMessage& message);

void ClearLastMessage();
[[nodiscard]] std::string GetLastMessage();
