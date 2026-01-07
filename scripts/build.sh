#!/bin/bash

# Copyright dSPACE GmbH. All rights reserved.

scriptFile=$(readlink -f "$0")
baseDir=$(dirname "$scriptFile")/..

config=$1
[ -z "$config" ] && config=Debug
[ "${config,,}" == "debug" ] && config=Debug
[ "${config,,}" == "release" ] && config=Release

echo Building $config ...

mkdir -p "$baseDir/tmplin/$config" || exit 1
cd "$baseDir/tmplin/$config"

cmake ../.. -GNinja -DCMAKE_BUILD_TYPE=$config -DDSVEOSCOSIM_BUILD_TESTS=ON || exit 1
cmake --build . || exit 1

echo Building $config finished successfully.
exit 0
