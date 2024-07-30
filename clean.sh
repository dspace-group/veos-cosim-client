#!/bin/bash

# Copyright dSPACE GmbH. All rights reserved.

scriptFile=$(readlink -f "$0")
currentDir=$(dirname "$scriptFile")/.

echo Cleaning ...

rm -rf "$currentDir/tmplin"

echo Cleaning finished successfully.
