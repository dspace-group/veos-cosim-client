:: Copyright dSPACE GmbH. All rights reserved.

@echo off

setlocal enabledelayedexpansion

set currentDir=%~dp0.

set config=%1
set platformToUse=%2

if "%config%"=="" (
    set config=Debug
)

if "%platformToUse%"=="" (
    if "%platform%"=="" (
        set platformToUse=x64
        call :DevEnv || exit /b 1
    ) else (
        set platformToUse=%platform%
    )
) else (
    if "%platform%" == "" (
        call :DevEnv || exit /b 1
    ) else (
        if "%platform%" neq "%platformToUse%" (
            echo Environment is already set up for %platform%
            exit /b 1
        )
    )
)

echo Building %config% %platformToUse% ...

set buildDir=%currentDir%\tmpwin\%config%\%platformToUse%
if not exist "%buildDir%" mkdir "%buildDir%" || exit /b 1
cd "%buildDir%"
cmake ..\..\.. -GNinja -DCMAKE_BUILD_TYPE=%config% -DDSVEOSCOSIM_BUILD_TESTS=ON || exit /b 1
cmake --build . --config %config% || exit /b 1

echo Building %config% %platformToUse% finished successfully.
exit /b 0

:DevEnv
set fileName=vcvars32.bat
if "%platformToUse%"=="x64" (
    set fileName=vcvars64.bat
)

call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\%fileName%" || exit /b 1
set DevEnvDir=C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE\
exit /b 0
