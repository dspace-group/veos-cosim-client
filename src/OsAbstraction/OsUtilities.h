// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#ifdef _WIN32

#include <cstdint>
#include <string>
#include <string_view>  // IWYU pragma: keep

namespace DsVeosCoSim {

constexpr uint32_t Infinite = UINT32_MAX;  // NOLINT

[[nodiscard]] std::wstring Utf8ToWide(std::string_view utf8String);

[[nodiscard]] int32_t GetLastWindowsError();

[[nodiscard]] uint32_t GetCurrentProcessId();

[[nodiscard]] bool IsProcessRunning(uint32_t processId);

}  // namespace DsVeosCoSim

#endif
