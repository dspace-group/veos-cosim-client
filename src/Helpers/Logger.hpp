// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>

#include <fmt/format.h>

#ifndef _WIN32

#include <system_error>

#endif

namespace DsVeosCoSim {

enum class Severity : uint32_t {
    Error,
    Warning,
    Info,
    Trace
};

[[nodiscard]] const char* format_as(Severity severity) noexcept;

using LogCallback = std::function<void(Severity, std::string_view)>;

class Logger {  // NOLINT(cppcoreguidelines-special-member-functions)
    Logger() = default;
    ~Logger() noexcept = default;

public:
    static Logger& Instance() {
        static Logger instance;
        return instance;
    }

    void SetLogCallback(LogCallback logCallback) {
        _logCallback = std::move(logCallback);
    }

    void Log(Severity severity, std::string_view message) {
        if (auto logCallback = _logCallback; logCallback) {
            logCallback(severity, message);
        }
    }

private:
    LogCallback _logCallback;
};

#ifdef _WIN32

[[nodiscard]] std::string GetEnglishErrorMessage(int32_t errorCode);

#endif

void LogError(int32_t errorCode, std::string_view message);
void LogError(std::string_view message);
void LogWarning(std::string_view message);
void LogInfo(std::string_view message);
void LogTrace(std::string_view message);

template <typename... T>
void LogError(int32_t errorCode, fmt::format_string<T...> format, T&&... args) {  // NOLINT(cppcoreguidelines-missing-std-forward)
    LogError(errorCode, fmt::vformat(format, fmt::make_format_args(args...)));
}

template <typename... T>
void LogError(fmt::format_string<T...> format, T&&... args) {  // NOLINT(cppcoreguidelines-missing-std-forward)
    LogError(fmt::vformat(format, fmt::make_format_args(args...)));
}

template <typename... T>
void LogWarning(fmt::format_string<T...> format, T&&... args) {  // NOLINT(cppcoreguidelines-missing-std-forward)
    LogWarning(fmt::vformat(format, fmt::make_format_args(args...)));
}

template <typename... T>
void LogInfo(fmt::format_string<T...> format, T&&... args) {  // NOLINT(cppcoreguidelines-missing-std-forward)
    LogInfo(fmt::vformat(format, fmt::make_format_args(args...)));
}

template <typename... T>
void LogTrace(fmt::format_string<T...> format, T&&... args) {  // NOLINT(cppcoreguidelines-missing-std-forward)
    LogTrace(fmt::vformat(format, fmt::make_format_args(args...)));
}

}  // namespace DsVeosCoSim
