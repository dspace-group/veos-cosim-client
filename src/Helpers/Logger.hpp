// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <cstdint>
#include <functional>
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

    void Log(Severity severity, const std::string& message) const {
        if (auto logCallback = _logCallback; logCallback) {
            logCallback(severity, message);
        }
    }

private:
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

void LogProtBegin(const std::string& message);
void LogProtEnd(const std::string& message);
void LogProtData(const std::string& message);

template <typename... TArgs>
void LogError(int32_t errorCode, fmt::format_string<TArgs...> formatString, TArgs&&... args) {
    LogError(errorCode, fmt::format(formatString, std::forward<TArgs>(args)...));
}

template <typename... TArgs>
void LogError(fmt::format_string<TArgs...> formatString, TArgs&&... args) {
    LogError(fmt::format(formatString, std::forward<TArgs>(args)...));
}

template <typename... TArgs>
void LogWarning(fmt::format_string<TArgs...> formatString, TArgs&&... args) {
    LogWarning(fmt::format(formatString, std::forward<TArgs>(args)...));
}

template <typename... TArgs>
void LogInfo(fmt::format_string<TArgs...> formatString, TArgs&&... args) {
    LogInfo(fmt::format(formatString, std::forward<TArgs>(args)...));
}

template <typename... TArgs>
void LogTrace(fmt::format_string<TArgs...> formatString, TArgs&&... args) {
    LogTrace(fmt::format(formatString, std::forward<TArgs>(args)...));
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
void LogProtBegin(fmt::format_string<TArgs...> formatString, TArgs&&... args) {
    LogProtBegin(fmt::format(formatString, std::forward<TArgs>(args)...));
}

template <typename... TArgs>
void LogProtEnd(fmt::format_string<TArgs...> formatString, TArgs&&... args) {
    LogProtEnd(fmt::format(formatString, std::forward<TArgs>(args)...));
}

template <typename... TArgs>
void LogProtData(fmt::format_string<TArgs...> formatString, TArgs&&... args) {
    LogProtData(fmt::format(formatString, std::forward<TArgs>(args)...));
}

}  // namespace DsVeosCoSim
