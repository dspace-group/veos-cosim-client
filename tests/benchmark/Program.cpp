// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <benchmark/benchmark.h>

#include "Helper.h"

using namespace DsVeosCoSim;

int main(int argc, char** argv) {
    if (!IsOk(StartUp())) {
        return 1;
    }

    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}
