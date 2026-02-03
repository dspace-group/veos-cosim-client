#!/bin/bash

# Copyright dSPACE SE & Co. KG. All rights reserved.

scriptFile=$(readlink -f "$0")
baseDir=$(dirname "$scriptFile")/..

config=$1
[ -z "$config" ] && config=Release

echo Running benchmarks for $config ...

filePath=$baseDir/tmplin/$config/benchmark/DsVeosCoSimBenchmark
if [ -z "$filePath" ]; then
    echo Could not find file "$filePath".
    exit 1
fi

$filePath || exit 1

echo Running benchmarks for $config finished successfully.
exit 0
