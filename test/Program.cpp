// Copyright dSPACE GmbH. All rights reserved.

#include <gtest/gtest.h>

#include "Helper.h"

int32_t main(int32_t argc, char** argv) {
    if (!StartUp()) {
        return 1;
    }

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
