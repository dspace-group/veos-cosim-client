// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "Logger.hpp"

#ifdef _WIN32

#include <cctype>  // IWYU pragma: keep
#include <cstdint>
#include <string>

#include <Windows.h>  // IWYU pragma: keep
#undef min

#endif

namespace DsVeosCoSim {

namespace {

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

}  // namespace

void LogError(int32_t errorCode, const std::string& message) {
    std::string reason;

#ifdef _WIN32
    reason = GetEnglishErrorMessage(errorCode);
#else
    reason = std::system_category().message(errorCode);
#endif

    LogError("{} Error code: {}. {}", message, errorCode, reason);
}

}  // namespace DsVeosCoSim
