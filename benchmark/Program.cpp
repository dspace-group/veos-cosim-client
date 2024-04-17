// Copyright dSPACE GmbH. All rights reserved.

#include <benchmark/benchmark.h>

#include "Logger.h"
#include "BenchmarkHelper.h"

using namespace DsVeosCoSim;

int main(int argc, char** argv) {
    SetLogCallback(OnLogCallback);

    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}
