:: Copyright dSPACE GmbH. All rights reserved.

@echo off

setlocal

echo Cleaning ...

set currentDir=%~dp0.

rmdir /s /q "%currentDir%\tmpwin" >nul 2>&1

echo Cleaning finished successfully.
