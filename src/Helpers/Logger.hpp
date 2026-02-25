// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>

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

    void LogError(std::string_view message) {
        if (auto logCallback = _logCallback; logCallback) {
            logCallback(Severity::Error, message);
        }
    }

    void LogWarning(std::string_view message) {
        if (auto logCallback = _logCallback; logCallback) {
            logCallback(Severity::Warning, message);
        }
    }

    void LogInfo(std::string_view message) {
        if (auto logCallback = _logCallback; logCallback) {
            logCallback(Severity::Info, message);
        }
    }

    void LogTrace(std::string_view message) {
        if (auto logCallback = _logCallback; logCallback) {
            logCallback(Severity::Trace, message);
        }
    }

    void LogError(std::string_view message, int32_t errorCode) {
        if (auto logCallback = _logCallback; logCallback) {
            std::string fullMessage(message);
            fullMessage.append(" Error code: ").append(std::to_string(errorCode)).append(". ");

#ifdef _WIN32
            fullMessage.append(GetEnglishErrorMessage(errorCode));
#else
            fullMessage.append(std::system_category().message(errorCode));
#endif

            logCallback(Severity::Error, fullMessage);
        }
    }

private:
    [[nodiscard]] static std::string GetEnglishErrorMessage(int32_t errorCode);

    LogCallback _logCallback;
};

}  // namespace DsVeosCoSim
