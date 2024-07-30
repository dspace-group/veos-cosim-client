// Copyright dSPACE GmbH. All rights reserved.

#include <gtest/gtest.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <cstdlib>
#endif
#include "CoSimTypes.h"
#include "Socket.h"

using namespace DsVeosCoSim;

namespace {

Result GetNextFreeDynamicPort(uint16_t& port) {
    Socket socket;
    CheckResult(socket.Create(AddressFamily::Ipv4));
    CheckResult(socket.Bind(0, false));
    CheckResult(socket.GetLocalPort(port));
    return Result::Ok;
}

}  // namespace

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    setvbuf(stdout, nullptr, _IONBF, 0);
#endif

    if (StartupNetwork() != Result::Ok) {
        return 1;
    }

    uint16_t portMapperPort{};
    if (GetNextFreeDynamicPort(portMapperPort) == Result::Ok) {
        std::string portMapperPortString = std::to_string(portMapperPort);
#ifdef _WIN32
        (void)SetEnvironmentVariableA("VEOS_COSIM_PORTMAPPER_PORT", portMapperPortString.c_str());
#else
        (void)setenv("VEOS_COSIM_PORTMAPPER_PORT", portMapperPortString.c_str(), 1);
#endif
    }

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
