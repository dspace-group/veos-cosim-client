#!/bin/bash

# Copyright dSPACE GmbH. All rights reserved.

scriptFile=$(readlink -f "$0")
currentDir=$(dirname "$scriptFile")

echo Building ...

mkdir -p "$currentDir/tmplin/Debug" || exit 1
cd "$currentDir/tmplin/Debug"
cmake ../.. -GNinja -DCMAKE_BUILD_TYPE=Debug -DDSVEOSCOSIM_BUILD_TESTS=ON || exit 1
cmake --build . --config Debug || exit 1

mkdir -p "$currentDir/tmplin/Release" || exit 1
cd "$currentDir/tmplin/Release"
cmake ../.. -GNinja -DCMAKE_BUILD_TYPE=Release -DDSVEOSCOSIM_BUILD_TESTS=ON -DDSVEOSCOSIM_BUILD_BENCHMARKS=ON || exit 1
cmake --build . --config Release || exit 1

echo Build finished successfully.
