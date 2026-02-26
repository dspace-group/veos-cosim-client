// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "Logger.hpp"

#ifdef _WIN32

#include <Windows.h>
#undef min

#endif

namespace DsVeosCoSim {

[[nodiscard]] const char* format_as(Severity severity) noexcept {
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

#ifdef _WIN32

[[nodiscard]] std::string GetEnglishErrorMessage(int32_t errorCode) {
    constexpr DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
    constexpr DWORD languageId = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
    char* buffer = nullptr;
    DWORD size = FormatMessageA(flags, nullptr, static_cast<DWORD>(errorCode), languageId, reinterpret_cast<char*>(&buffer), 0, nullptr);

    std::string message;
    if ((size > 0) && buffer) {
        message.assign(buffer, size);
        LocalFree(buffer);

        while (!message.empty() && (std::isspace(message.back()) != 0)) {
            message.pop_back();
        }
    } else {
        message = "Unknown error.";
    }

    return message;
}

#endif

void LogError(std::string_view message) {
    Logger::Instance().Log(Severity::Error, message);
}

void LogWarning(std::string_view message) {
    Logger::Instance().Log(Severity::Warning, message);
}

void LogInfo(std::string_view message) {
    Logger::Instance().Log(Severity::Info, message);
}

void LogTrace(std::string_view message) {
    Logger::Instance().Log(Severity::Trace, message);
}

void LogError(int32_t errorCode, std::string_view message) {
    std::string fullMessage(message);
    fullMessage.append(" Error code: ").append(std::to_string(errorCode)).append(". ");

#ifdef _WIN32
    fullMessage.append(GetEnglishErrorMessage(errorCode));
#else
    fullMessage.append(std::system_category().message(errorCode));
#endif

    LogError(fullMessage);
}

}  // namespace DsVeosCoSim
