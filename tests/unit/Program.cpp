// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <gtest/gtest.h>

#include <Result.hpp>

#include "Helper.hpp"
#include "TestHelper.hpp"

using namespace DsVeosCoSim;

int main(int argc, char** argv) {
    InitializeOutput();

    if (!IsOk(StartUp())) {
        return 1;
    }

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
