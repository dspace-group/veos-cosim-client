#!/bin/bash

# Copyright dSPACE GmbH. All rights reserved.

scriptFile=$(readlink -f "$0")
currentDir=$(dirname "$scriptFile")

config=$1
[ -z "$config" ] && config=Debug

echo Building $config ...

mkdir -p "$currentDir/tmplin/$config" || exit 1
cd "$currentDir/tmplin/$config"

cmake ../.. -GNinja -DCMAKE_BUILD_TYPE=$config -DDSVEOSCOSIM_BUILD_TESTS=ON || exit 1
cmake --build . || exit 1

echo Building $config finished successfully.
exit 0
