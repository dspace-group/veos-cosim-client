#!/bin/bash

# Copyright dSPACE GmbH. All rights reserved.

scriptFile=$(readlink -f "$0")
currentDir=$(dirname "$scriptFile")

config=$1
[ -z "$config" ] && config=Debug
[ "${config,,}" == "debug" ] && config=Debug
[ "${config,,}" == "release" ] && config=Release

echo Running test server for $config ...

filePath=$currentDir/tmplin/$config/utilities/TestServer/TestServer
if [ -z "$filePath" ]; then
    echo Could not find file "$filePath".
    exit 1
fi

$filePath || exit 1

echo Running test server for $config finished successfully.
exit 0
