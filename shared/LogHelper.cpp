// Copyright dSPACE GmbH. All rights reserved.

#include "LogHelper.h"

#include <fmt/color.h>
#include <string_view>  // IWYU pragma: keep

#include "CoSimHelper.h"

#ifdef _WIN32
#include <Windows.h>
#endif

using namespace DsVeosCoSim;

namespace {

std::string LastMessage;

fmt::text_style Red = fg(fmt::color::red);
fmt::text_style Yellow = fg(fmt::color::yellow);
fmt::text_style White = fg(fmt::color::white);
fmt::text_style Gray = fg(fmt::color::light_gray);

}  // namespace

void InitializeOutput() {
#if _WIN32
    (void)SetConsoleOutputCP(CP_UTF8);
    (void)setvbuf(stdout, nullptr, _IONBF, 0);

    const HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);  // NOLINT

    DWORD dwMode = 0;
    if (GetConsoleMode(console, &dwMode) != 0) {
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        (void)SetConsoleMode(console, dwMode);
    }
#endif

    SetLogCallback(OnLogCallback);
}

void OnLogCallback(const Severity severity, std::string_view message) {
    LastMessage = message;
    switch (severity) {
        case Severity::Error:
            print(Red, "{}\n", message);
            break;
        case Severity::Warning:
            print(Yellow, "{}\n", message);
            break;
        case Severity::Info:
            print(White, "{}\n", message);
            break;
        case Severity::Trace:
            print(Gray, "{}\n", message);
            break;
    }
}

void ClearLastMessage() {
    LastMessage = "";
}

[[nodiscard]] std::string GetLastMessage() {
    return LastMessage;
}
