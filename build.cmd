:: Copyright dSPACE GmbH. All rights reserved.

@echo off

setlocal enabledelayedexpansion

set currentDir=%~dp0

set config=%1

if "%config%"=="" set config=Debug
if /i "%config%"=="debug" set config=Debug
if /i "%config%"=="release" set config=Release

where /q cmake || (
    echo Could not find cmake in path
    exit /b 1
)

where /q ninja || (
    echo Could not find ninja in path
    exit /b 1
)

echo Building %config% ...

set buildDir=%currentDir%tmpwin\%config%
if not exist "%buildDir%" mkdir "%buildDir%" || exit /b 1
cd "%buildDir%"
cmake ..\.. -GNinja -DCMAKE_BUILD_TYPE=%config% -DDSVEOSCOSIM_BUILD_TESTS=ON || exit /b 1
cmake --build . --config %config% || exit /b 1

echo Building %config% finished successfully.
exit /b 0
