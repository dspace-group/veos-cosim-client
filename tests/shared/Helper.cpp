// Copyright dSPACE GmbH. All rights reserved.

#include "Helper.h"

#include <string>
#include <string_view>

#include "CoSimHelper.h"
#include "LogHelper.h"
#include "Socket.h"

#ifdef _WIN32
#include <Windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>

#include <cstdlib>
#endif

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] uint16_t GetNextFreeDynamicPort() {
    const Socket socket(AddressFamily::Ipv4);
    socket.Bind(0, false);
    return socket.GetLocalPort();
}

}  // namespace

[[nodiscard]] int32_t GetChar() {
#ifdef _WIN32
    return _getch();
#else
    termios oldt{};
    tcgetattr(STDIN_FILENO, &oldt);

    termios newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);

    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    int32_t character = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return character;
#endif
}

[[nodiscard]] bool StartUp() {
    InitializeOutput();

    try {
        StartupNetwork();
    } catch (const std::exception& e) {
        LogError(e.what());
        return false;
    }

    try {
        const uint16_t portMapperPort = GetNextFreeDynamicPort();
        const std::string portMapperPortString = std::to_string(portMapperPort);
#ifdef _WIN32
        const std::string environmentString = "VEOS_COSIM_PORTMAPPER_PORT=" + portMapperPortString;
        (void)_putenv(environmentString.c_str());
#else
        (void)setenv("VEOS_COSIM_PORTMAPPER_PORT", portMapperPortString.c_str(), 1);
#endif
    } catch (const std::exception& e) {
        LogError(e.what());
        return false;
    }

    return true;
}

[[nodiscard]] std::string_view GetLoopBackAddress(const AddressFamily addressFamily) {
    if (addressFamily == AddressFamily::Ipv4) {
        return "127.0.0.1";
    }

    return "::1";
}

[[nodiscard]] bool SendComplete(const Socket& socket, const void* buffer, size_t length) {
    const auto* bufferPointer = static_cast<const uint8_t*>(buffer);
    while (length > 0) {
        int32_t sentSize{};
        CheckResult(socket.Send(bufferPointer, static_cast<int32_t>(length), sentSize));

        length -= sentSize;
        bufferPointer += sentSize;
    }

    return true;
}

[[nodiscard]] bool ReceiveComplete(const Socket& socket, void* buffer, size_t length) {
    auto* bufferPointer = static_cast<uint8_t*>(buffer);
    while (length > 0) {
        int32_t receivedSize{};
        CheckResult(socket.Receive(bufferPointer, static_cast<int32_t>(length), receivedSize));

        length -= receivedSize;
        bufferPointer += receivedSize;
    }

    return true;
}
