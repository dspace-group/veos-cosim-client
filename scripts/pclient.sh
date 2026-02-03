#!/bin/bash

# Copyright dSPACE SE & Co. KG. All rights reserved.

scriptFile=$(readlink -f "$0")
baseDir=$(dirname "$scriptFile")/..

config=$1
[ -z "$config" ] && config=Release
[ "${config,,}" == "debug" ] && config=Debug
[ "${config,,}" == "release" ] && config=Release

echo Running performance test client for $config ...

filePath=$baseDir/tmplin/$config/tests/PerformanceTestClient/PerformanceTestClient
if [ -z "$filePath" ]; then
    echo Could not find file "$filePath".
    exit 1
fi

$filePath || exit 1

echo Running performance test client for $config finished successfully.
exit 0
