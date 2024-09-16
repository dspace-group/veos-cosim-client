// Copyright dSPACE GmbH. All rights reserved.

#include <benchmark/benchmark.h>

#include "Helper.h"

int32_t main(int32_t argc, char** argv) {
    if (!StartUp()) {
        return 1;
    }

    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}
