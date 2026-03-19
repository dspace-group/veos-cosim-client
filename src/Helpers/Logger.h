// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <type_traits>
#include <utility>

#include <fmt/format.h>

namespace DsVeosCoSim {

enum class Severity : uint32_t;

using LogCallback = std::function<void(Severity, const std::string&)>;

class Logger {  // NOLINT(cppcoreguidelines-special-member-functions)
private:
    Logger() {
    }

    ~Logger() = default;

public:
    static Logger& Instance() {
        static Logger instance;
        return instance;
    }

    void SetLogCallback(LogCallback logCallback);
    void LogError(const std::string& message);
    void LogWarning(const std::string& message);
    void LogInfo(const std::string& message);
    void LogTrace(const std::string& message);
    void LogSystemError(const std::string& message, int32_t errorCode);
    void LogProtBegin(const std::string& message);
    void LogProtEnd(const std::string& message);
    void LogProtData(const std::string& message);

    template <typename... TArgs, typename = std::enable_if_t<(sizeof...(TArgs) > 0)>>
    void LogError(fmt::format_string<TArgs...> formatString, TArgs&&... args);

    template <typename... TArgs, typename = std::enable_if_t<(sizeof...(TArgs) > 0)>>
    void LogWarning(fmt::format_string<TArgs...> formatString, TArgs&&... args);

    template <typename... TArgs, typename = std::enable_if_t<(sizeof...(TArgs) > 0)>>
    void LogInfo(fmt::format_string<TArgs...> formatString, TArgs&&... args);

    template <typename... TArgs, typename = std::enable_if_t<(sizeof...(TArgs) > 0)>>
    void LogTrace(fmt::format_string<TArgs...> formatString, TArgs&&... args);

    template <typename... TArgs, typename = std::enable_if_t<(sizeof...(TArgs) > 0)>>
    void LogProtBegin(fmt::format_string<TArgs...> formatString, TArgs&&... args);

    template <typename... TArgs, typename = std::enable_if_t<(sizeof...(TArgs) > 0)>>
    void LogProtEnd(fmt::format_string<TArgs...> formatString, TArgs&&... args);

    template <typename... TArgs, typename = std::enable_if_t<(sizeof...(TArgs) > 0)>>
    void LogProtData(fmt::format_string<TArgs...> formatString, TArgs&&... args);

private:
    void LogMessage(Severity severity, const std::string& message) const;

    LogCallback _logCallback;
};

template <typename... TArgs, typename>
void Logger::LogError(fmt::format_string<TArgs...> formatString, TArgs&&... args) {
    LogError(fmt::format(formatString, std::forward<TArgs>(args)...));
}

template <typename... TArgs, typename>
void Logger::LogWarning(fmt::format_string<TArgs...> formatString, TArgs&&... args) {
    LogWarning(fmt::format(formatString, std::forward<TArgs>(args)...));
}

template <typename... TArgs, typename>
void Logger::LogInfo(fmt::format_string<TArgs...> formatString, TArgs&&... args) {
    LogInfo(fmt::format(formatString, std::forward<TArgs>(args)...));
}

template <typename... TArgs, typename>
void Logger::LogTrace(fmt::format_string<TArgs...> formatString, TArgs&&... args) {
    LogTrace(fmt::format(formatString, std::forward<TArgs>(args)...));
}

template <typename... TArgs, typename>
void Logger::LogProtBegin(fmt::format_string<TArgs...> formatString, TArgs&&... args) {
    LogProtBegin(fmt::format(formatString, std::forward<TArgs>(args)...));
}

template <typename... TArgs, typename>
void Logger::LogProtEnd(fmt::format_string<TArgs...> formatString, TArgs&&... args) {
    LogProtEnd(fmt::format(formatString, std::forward<TArgs>(args)...));
}

template <typename... TArgs, typename>
void Logger::LogProtData(fmt::format_string<TArgs...> formatString, TArgs&&... args) {
    LogProtData(fmt::format(formatString, std::forward<TArgs>(args)...));
}

}  // namespace DsVeosCoSim
