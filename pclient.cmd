:: Copyright dSPACE GmbH. All rights reserved.

@echo off

setlocal enabledelayedexpansion

set currentDir=%~dp0

set config=%1
set platformToUse=%2

if "%config%"=="" set config=Release
if /i "%config%"=="debug" set config=Debug
if /i "%config%"=="release" set config=Release

if "%platformToUse%"=="" set platformToUse=x64
if /i "%platformToUse%"=="x64" set platformToUse=x64
if /i "%platformToUse%"=="x86" set platformToUse=x86

echo Running performance test client for %config% %platformToUse% ...

set filePath=%currentDir%tmpwin\%config%\%platformToUse%\tests\PerformanceTestClient\PerformanceTestClient.exe

if not exist "%filePath%" (
    echo Could not find file "%filePath%".
    exit /b 1
)

call "%filePath%" || exit /b 1

echo Running performance test client for %config% %platformToUse% finished successfully.
exit /b 0
