#!/bin/bash

# Copyright dSPACE GmbH. All rights reserved.

scriptFile=$(readlink -f "$0")
currentDir=$(dirname "$scriptFile")

for executableFile in $(find "$currentDir" -name DsVeosCoSimTest -type f -executable | xargs realpath)
do
    echo Running tests in "$executableFile" ...
    "$executableFile" || exit 1
done

echo Running tests finished successfully.
