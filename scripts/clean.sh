#!/bin/bash

# Copyright dSPACE SE & Co. KG. All rights reserved.

scriptFile=$(readlink -f "$0")
baseDir=$(dirname "$scriptFile")/..

echo Cleaning ...

rm -rf "$baseDir/tmplin"

echo Cleaning finished successfully.
