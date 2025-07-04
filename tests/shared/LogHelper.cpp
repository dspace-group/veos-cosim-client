// Copyright dSPACE GmbH. All rights reserved.

#include "LogHelper.h"

#include <string_view>

#include <fmt/color.h>

#include "DsVeosCoSim/CoSimTypes.h"

#ifdef _WIN32
#include <Windows.h>
#endif

using namespace DsVeosCoSim;

namespace {

std::string LastMessage;

}  // namespace

void InitializeOutput() {
#if _WIN32
    (void)SetConsoleOutputCP(CP_UTF8);
    (void)setvbuf(stdout, nullptr, _IONBF, 0);

    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

    DWORD dwMode = 0;
    BOOL result = GetConsoleMode(console, &dwMode);
    if (result != 0) {
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        (void)SetConsoleMode(console, dwMode);
    }
#endif

    SetLogCallback(OnLogCallback);
}

void OnLogCallback(Severity severity, std::string_view message) {
    LastMessage = message;
    switch (severity) {
        case Severity::Error:
            print(fg(fmt::color::red), "{}\n", message);
            break;
        case Severity::Warning:
            print(fg(fmt::color::yellow), "{}\n", message);
            break;
        case Severity::Info:
            print(fg(fmt::color::white), "{}\n", message);
            break;
        case Severity::Trace:
            print(fg(fmt::color::light_gray), "{}\n", message);
            break;
    }
}

void LogCanMessageContainer(const CanMessageContainer& messageContainer) {
    print(fg(fmt::color::dodger_blue), "{}\n", ToString(messageContainer));
}

void LogEthMessageContainer(const EthMessageContainer& messageContainer) {
    print(fg(fmt::color::cyan), "{}\n", ToString(messageContainer));
}

void LogLinMessageContainer(const LinMessageContainer& messageContainer) {
    print(fg(fmt::color::lime), "{}\n", ToString(messageContainer));
}

void LogIoData(const IoSignal& ioSignal, uint32_t length, const void* value) {
    print(fg(fmt::color::fuchsia), "{}\n", IoDataToString(ioSignal, length, value));
}

void ClearLastMessage() {
    LastMessage = "";
}

[[nodiscard]] std::string GetLastMessage() {
    return LastMessage;
}
