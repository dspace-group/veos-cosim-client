// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include "CoSimTypes.h"

namespace DsVeosCoSim {

void SetLogCallback(LogCallback logCallback);

void LogError(std::string_view message);
void LogSystemError(std::string_view prefix, int errorCode);
void LogWarning(std::string_view message);
void LogInfo(std::string_view message);
void LogTrace(std::string_view message);

}  // namespace DsVeosCoSim
