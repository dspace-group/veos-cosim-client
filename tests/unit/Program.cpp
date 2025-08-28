// Copyright dSPACE GmbH. All rights reserved.

#include <gtest/gtest.h>

#include "Helper.h"

using namespace DsVeosCoSim;

int main(int argc, char** argv) {
    if (!IsOk(StartUp())) {
        return 1;
    }

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
