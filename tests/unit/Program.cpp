// Copyright dSPACE GmbH. All rights reserved.

#include <gtest/gtest.h>

#include <cstdint>

#include "Helper.h"

using namespace DsVeosCoSim;

int32_t main(int32_t argc, char** argv) {
    if (!IsOk(StartUp())) {
        return 1;
    }

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
