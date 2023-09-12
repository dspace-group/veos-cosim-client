// Copyright dSPACE GmbH. All rights reserved.

#include "CoSimTypes.h"
#include "Socket.h"
#include "TestHelper.h"

using namespace DsVeosCoSim;

int main(int argc, char** argv) {
    if (StartupNetwork() != Result::Ok) {
        return 1;
    }

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
