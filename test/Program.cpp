// Copyright dSPACE GmbH. All rights reserved.

#include <gtest/gtest.h>

#ifdef _WIN32
#include <Windows.h>
#endif
#include "CoSimTypes.h"
#include "Socket.h"

using namespace DsVeosCoSim;

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    setvbuf(stdout, nullptr, _IONBF, 0);
#endif

    if (StartupNetwork() != Result::Ok) {
        return 1;
    }

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
