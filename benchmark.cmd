:: Copyright dSPACE GmbH. All rights reserved.

@echo off

setlocal enabledelayedexpansion

set currentDir=%~dp0.

set config=%1
set platformToUse=%2

if "%config%"=="" (
    set config=Release
)

if "%platformToUse%"=="" (
    set platformToUse=x64
)

echo Running benchmarks for %config% %platformToUse% ...

set filePath=%currentDir%\tmpwin\%config%\%platformToUse%\benchmark\DsVeosCoSimBenchmark.exe

if not exist "%filePath%" (
    echo Could not find file "%filePath%".
    exit /b 1
)

call "%filePath%" || exit /b 1

echo Running benchmarks for %config% %platformToUse% finished successfully.
exit /b 0
