:: Copyright dSPACE GmbH. All rights reserved.

@echo off

setlocal

echo Cleaning ...

set currentDir=%~dp0
set baseDir=%currentDir%.

del /s /q "%baseDir%\*.user" >nul 2>&1
del /s /q "%baseDir%\launchSettings.json" >nul 2>&1
rmdir /s /q "%baseDir%\tmpwin" >nul 2>&1
rmdir /s /q "%baseDir%\tmplin" >nul 2>&1

echo Cleaning finished successfully.
