:: Copyright dSPACE GmbH. All rights reserved.

@echo off

setlocal

set currentDir=%~dp0.

echo Testing ...

for /f %%x in ('dir /s /b %currentDir%\DsVeosCoSimTest.exe') do (
    echo Running tests in "%%x" ...
    "%%x" || exit /b 1
)

for /f %%x in ('dir /s /b %currentDir%\DsVeosCoSimBenchmark.exe') do (
    echo Running tests in "%%x" ...
    "%%x" || exit /b 1
)

echo Testing finished successfully.
