#!/bin/bash

# Copyright dSPACE GmbH. All rights reserved.

scriptFile=$(readlink -f "$0")
currentDir=$(dirname "$scriptFile")

config=$1
[ -z "$config" ] && config=Release
[ "${config,,}" == "debug" ] && config=Debug
[ "${config,,}" == "release" ] && config=Release

echo Running performance test client for $config ...

filePath=$currentDir/tmplin/$config/tests/PerformanceTestClient/PerformanceTestClient
if [ -z "$filePath" ]; then
    echo Could not find file "$filePath".
    exit 1
fi

$filePath || exit 1

echo Running performance test client for $config finished successfully.
exit 0
