:: Copyright dSPACE GmbH. All rights reserved.

@echo off

setlocal enabledelayedexpansion

set currentDir=%~dp0

echo Cleaning ...

rmdir /s /q "%currentDir%obj" >nul 2>&1
rmdir /s /q "%currentDir%tmpwin" >nul 2>&1

echo Cleaning finished successfully.
exit /b 0
