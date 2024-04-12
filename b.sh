#!/bin/bash

# Copyright dSPACE GmbH. All rights reserved.

scriptFile=$(readlink -f "$0")
currentDir=$(dirname "$scriptFile")

mkdir -p "$currentDir/tmplin/Debug" || exit 1
cd "$currentDir/tmplin/Debug"
cmake ../.. -GNinja -DDSVEOSCOSIM_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug || exit 1
cmake --build . || exit 1

mkdir -p "$currentDir/tmplin/Release" || exit 1
cd "$currentDir/tmplin/Release"
cmake ../.. -GNinja -DDSVEOSCOSIM_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release || exit 1
cmake --build . || exit 1
