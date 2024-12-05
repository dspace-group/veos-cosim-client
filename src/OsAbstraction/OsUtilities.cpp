// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32

#include "OsUtilities.h"

#include <Windows.h>

#include <cstdint>
#include <string>
#include <string_view>

namespace DsVeosCoSim {

[[nodiscard]] std::wstring Utf8ToWide(std::string_view utf8String) {
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

[[nodiscard]] int32_t GetLastWindowsError() {
    return static_cast<int32_t>(::GetLastError());
}

[[nodiscard]] uint32_t GetCurrentProcessId() {
    static uint32_t processId{};
    if (processId == 0) {
        processId = static_cast<uint32_t>(::GetCurrentProcessId());
    }

    return processId;
}

[[nodiscard]] bool IsProcessRunning(uint32_t processId) {
    void* processHandle = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | SYNCHRONIZE, FALSE, processId);
    if (processHandle == nullptr) {
        return false;
    }

    DWORD exitCode{};
    BOOL result = GetExitCodeProcess(processHandle, &exitCode);
    return (result != 0) && (exitCode == STILL_ACTIVE);
}

}  // namespace DsVeosCoSim

#endif
