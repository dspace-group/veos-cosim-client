// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>

#include <fmt/format.h>

namespace DsVeosCoSim {

enum class Severity : uint32_t {
    Error,
    Warning,
    Info,
    Trace
};

[[nodiscard]] constexpr std::string_view format_as(Severity severity) noexcept {
    switch (severity) {
        case Severity::Error:
            return "Error";
        case Severity::Warning:
            return "Warning";
        case Severity::Info:
            return "Info";
        case Severity::Trace:
            return "Trace";
    }

    return "<Invalid Severity>";
}

using LogCallback = std::function<void(Severity, const std::string&)>;

class Logger {
    Logger() = default;
    ~Logger() noexcept = default;

public:
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    static Logger& Instance();

    void SetLogCallback(LogCallback logCallback) {
        std::scoped_lock lock(_mutex);
        _logCallback = std::move(logCallback);
    }

    void Log(Severity severity, const std::string& message) const {
        LogCallback logCallback;
        {
            std::scoped_lock lock(_mutex);
            logCallback = _logCallback;
        }

        if (logCallback) {
            logCallback(severity, message);
        }
    }

private:
    mutable std::mutex _mutex;
    LogCallback _logCallback;
};

void LogError(int32_t errorCode, const std::string& message);

inline void LogError(const std::string& message) {
    Logger::Instance().Log(Severity::Error, message);
}

inline void LogWarning(const std::string& message) {
    Logger::Instance().Log(Severity::Warning, message);
}

inline void LogInfo(const std::string& message) {
    Logger::Instance().Log(Severity::Info, message);
}

inline void LogTrace(const std::string& message) {
    Logger::Instance().Log(Severity::Trace, message);
}

template <typename... TArgs>
std::string FormatLogMessage(fmt::format_string<TArgs...> formatString, const TArgs&... args) {
    return fmt::vformat(static_cast<fmt::string_view>(formatString), fmt::make_format_args(args...));
}

template <typename... TArgs>
void LogError(int32_t errorCode, fmt::format_string<TArgs...> formatString, const TArgs&... args) {
    LogError(errorCode, FormatLogMessage(formatString, args...));
}

template <typename... TArgs>
void LogError(fmt::format_string<TArgs...> formatString, const TArgs&... args) {
    LogError(FormatLogMessage(formatString, args...));
}

template <typename... TArgs>
void LogWarning(fmt::format_string<TArgs...> formatString, const TArgs&... args) {
    LogWarning(FormatLogMessage(formatString, args...));
}

template <typename... TArgs>
void LogInfo(fmt::format_string<TArgs...> formatString, const TArgs&... args) {
    LogInfo(FormatLogMessage(formatString, args...));
}

template <typename... TArgs>
void LogTrace(fmt::format_string<TArgs...> formatString, const TArgs&... args) {
    LogTrace(FormatLogMessage(formatString, args...));
}

inline void LogProtBegin(const std::string& message) {
    LogTrace("PROT BEGIN {}", message);
}

inline void LogProtEnd(const std::string& message) {
    LogTrace("PROT END   {}", message);
}

inline void LogProtData(const std::string& message) {
    LogTrace("PROT DATA  {}", message);
}

template <typename... TArgs>
void LogProtBegin(fmt::format_string<TArgs...> formatString, const TArgs&... args) {
    LogProtBegin(FormatLogMessage(formatString, args...));
}

template <typename... TArgs>
void LogProtEnd(fmt::format_string<TArgs...> formatString, const TArgs&... args) {
    LogProtEnd(FormatLogMessage(formatString, args...));
}

template <typename... TArgs>
void LogProtData(fmt::format_string<TArgs...> formatString, const TArgs&... args) {
    LogProtData(FormatLogMessage(formatString, args...));
}

}  // namespace DsVeosCoSim
