#!/bin/bash

# Copyright dSPACE GmbH. All rights reserved.

scriptFile=$(readlink -f "$0")
currentDir=$(dirname "$scriptFile")/.

config=$1
[ -z "$config" ] && config=Release

echo Running benchmarks for $config ...

filePath=$currentDir/tmplin/$config/benchmark/DsVeosCoSimBenchmark
if [ -z "$filePath" ]; then
    echo Could not find file "$filePath".
    exit 1
fi

$filePath || exit 1

echo Running benchmarks for $config finished successfully.
exit 0
