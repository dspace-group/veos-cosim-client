// Copyright dSPACE GmbH. All rights reserved.

#include "LogHelper.h"

#include <fmt/color.h>
#include <string_view>

#include "CoSimHelper.h"

#ifdef _WIN32
#include <Windows.h>
#endif

using namespace DsVeosCoSim;

namespace {

std::string g_lastMessage;

fmt::text_style red = fg(fmt::color::red);
fmt::text_style yellow = fg(fmt::color::yellow);
fmt::text_style white = fg(fmt::color::white);
fmt::text_style gray = fg(fmt::color::light_gray);

}  // namespace

void InitializeOutput() {
#if _WIN32
    (void)::SetConsoleOutputCP(CP_UTF8);
    (void)::setvbuf(stdout, nullptr, _IONBF, 0);

    HANDLE console = ::GetStdHandle(STD_OUTPUT_HANDLE);

    DWORD dwMode = 0;
    if (::GetConsoleMode(console, &dwMode) != 0) {
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        (void)::SetConsoleMode(console, dwMode);
    }
#endif

    SetLogCallback(OnLogCallback);
}

void OnLogCallback(DsVeosCoSim_Severity severity, std::string_view message) {
    g_lastMessage = message;
    switch (severity) {
        case DsVeosCoSim_Severity_Error:
            print(red, "{}\n", message);
            break;
        case DsVeosCoSim_Severity_Warning:
            print(yellow, "{}\n", message);
            break;
        case DsVeosCoSim_Severity_Info:
            print(white, "{}\n", message);
            break;
        case DsVeosCoSim_Severity_Trace:
            print(gray, "{}\n", message);
            break;
        case DsVeosCoSim_Severity_INT_MAX_SENTINEL_DO_NOT_USE_:
            break;
    }
}

void ClearLastMessage() {
    g_lastMessage = "";
}

[[nodiscard]] std::string GetLastMessage() {
    return g_lastMessage;
}
