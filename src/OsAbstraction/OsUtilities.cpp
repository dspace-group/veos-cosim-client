// Copyright dSPACE GmbH. All rights reserved.

#include "OsUtilities.h"

#include <fmt/format.h>
#include <system_error>

#ifdef _WIN32
#include <Windows.h>
#include <filesystem>
#else
#include <sys/time.h>
#endif

#ifdef _WIN32
namespace fs = std::filesystem;
#endif

namespace DsVeosCoSim {

int64_t GetCurrentTimeInMilliseconds() {
#ifdef _WIN32
    FILETIME fileTime{};
    GetSystemTimeAsFileTime(&fileTime);

    ULARGE_INTEGER largeInteger{};
    largeInteger.LowPart = fileTime.dwLowDateTime;
    largeInteger.HighPart = fileTime.dwHighDateTime;

    return static_cast<int64_t>(largeInteger.QuadPart / 10000);
#else
    timeval tv{};
    (void)::gettimeofday(&tv, nullptr);

    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
#endif
}

#ifdef _WIN32
std::wstring Utf8ToWide(std::string_view utf8String) {
    if (utf8String.empty()) {
        return {};
    }

    int32_t sizeNeeded =
        ::MultiByteToWideChar(CP_UTF8, 0, utf8String.data(), static_cast<int32_t>(utf8String.size()), nullptr, 0);

    std::wstring wideString(sizeNeeded, L'\0');
    (void)::MultiByteToWideChar(CP_UTF8,
                                0,
                                utf8String.data(),
                                static_cast<int32_t>(utf8String.size()),
                                wideString.data(),
                                sizeNeeded);

    return wideString;
}

int32_t GetLastWindowsError() {
    return static_cast<int32_t>(::GetLastError());
}

uint32_t GetCurrentProcessId() {
    static uint32_t processId{};
    if (processId == 0) {
        processId = static_cast<uint32_t>(::GetCurrentProcessId());
    }

    return processId;
}

bool IsProcessRunning(uint32_t processId) {
    void* processHandle = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | SYNCHRONIZE, FALSE, processId);
    if (processHandle == nullptr) {
        return false;
    }

    DWORD exitCode{};
    BOOL result = GetExitCodeProcess(processHandle, &exitCode);
    return (result != 0) && (exitCode == STILL_ACTIVE);
}
#endif

std::string GetSystemErrorMessage(int32_t errorCode) {
    return fmt::format("Error code: {}. {}", errorCode, std::system_category().message(errorCode));
}

std::string GetUdsPath(const std::string& name) {
#ifdef _WIN32
    fs::path tempDir = fs::temp_directory_path();
    fs::path fileDir = tempDir / fmt::format("dSPACE.VEOS.CoSim.{}", name);
    return fileDir.string();
#else
    return fmt::format("dSPACE.VEOS.CoSim.{}", name);
#endif
}

}  // namespace DsVeosCoSim
