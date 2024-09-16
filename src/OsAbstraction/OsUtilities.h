// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <fmt/format.h>
#include <stdexcept>
#include <string>

namespace DsVeosCoSim {

[[nodiscard]] int64_t GetCurrentTimeInMilliseconds();

#ifdef _WIN32

constexpr uint32_t Infinite = UINT32_MAX;  // NOLINT

[[nodiscard]] std::wstring Utf8ToWide(std::string_view utf8String);

[[nodiscard]] int32_t GetLastWindowsError();

[[nodiscard]] uint32_t GetCurrentProcessId();

[[nodiscard]] bool IsProcessRunning(uint32_t processId);

#endif

[[nodiscard]] std::string GetSystemErrorMessage(int32_t errorCode);

[[nodiscard]] std::string GetUdsPath(const std::string& name);

class OsAbstractionException final : public std::runtime_error {
public:
    explicit OsAbstractionException(std::string_view message) : std::runtime_error(message.data()) {
    }

    OsAbstractionException(std::string_view message, int32_t errorCode)
        : std::runtime_error(fmt::format("{} {}", message, GetSystemErrorMessage(errorCode))) {
    }
};

}  // namespace DsVeosCoSim
