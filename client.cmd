:: Copyright dSPACE GmbH. All rights reserved.

@echo off

setlocal enabledelayedexpansion

set currentDir=%~dp0

set config=%1

if "%config%"=="" set config=Debug
if /i "%config%"=="debug" set config=Debug
if /i "%config%"=="release" set config=Release

echo Running test client for %config% ...

set filePath=%currentDir%tmpwin\%config%\tests\TestClient\TestClient.exe

if not exist "%filePath%" (
    echo Could not find file "%filePath%".
    exit /b 1
)

call "%filePath%" || exit /b 1

echo Running test client for %config% finished successfully.
exit /b 0
