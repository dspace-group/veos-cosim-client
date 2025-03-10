// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32

#include "OsUtilities.h"

#include <Windows.h>  // NOLINT

#include <cstdint>
#include <string>
#include <string_view>

namespace DsVeosCoSim {

[[nodiscard]] std::wstring Utf8ToWide(const std::string_view utf8String) {
    if (utf8String.empty()) {
        return {};
    }

    const int32_t sizeNeeded = MultiByteToWideChar(CP_UTF8,  // NOLINT
                                                   0,
                                                   utf8String.data(),
                                                   static_cast<int32_t>(utf8String.size()),
                                                   nullptr,
                                                   0);

    std::wstring wideString(sizeNeeded, L'\0');
    (void)MultiByteToWideChar(CP_UTF8,
                              0,
                              utf8String.data(),
                              static_cast<int32_t>(utf8String.size()),
                              wideString.data(),
                              sizeNeeded);

    return wideString;
}

[[nodiscard]] int32_t GetLastWindowsError() {
    return static_cast<int32_t>(GetLastError());  // NOLINT
}

[[nodiscard]] uint32_t GetCurrentProcessId() {
    static uint32_t processId{};
    if (processId == 0) {
        processId = static_cast<uint32_t>(::GetCurrentProcessId());  // NOLINT
    }

    return processId;
}

[[nodiscard]] bool IsProcessRunning(const uint32_t processId) {
    void* processHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | SYNCHRONIZE, FALSE, processId);  // NOLINT
    if (processHandle == nullptr) {
        return false;
    }

    DWORD exitCode{};                                                  // NOLINT
    const BOOL result = GetExitCodeProcess(processHandle, &exitCode);  // NOLINT
    return (result != 0) && (exitCode == STILL_ACTIVE);
}

}  // namespace DsVeosCoSim

#endif
