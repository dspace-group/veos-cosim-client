@echo off

setlocal

set currentDir=%~dp0.

echo Building ...

if not exist %currentDir%\tmpwin\Debug mkdir %currentDir%\tmpwin\Debug || exit /b 1
cd %currentDir%\tmpwin\Debug
cmake ..\.. -GNinja -DDSVEOSCOSIM_BUILD_TESTS=ON || exit /b 1
cmake --build . --config Debug || exit /b 1

if not exist %currentDir%\tmpwin\Release mkdir %currentDir%\tmpwin\Release || exit /b 1
cd %currentDir%\tmpwin\Release
cmake ..\.. -GNinja -DDSVEOSCOSIM_BUILD_TESTS=ON -DDSVEOSCOSIM_BUILD_BENCHMARKS=ON -DCMAKE_BUILD_TYPE=Release || exit /b 1
cmake --build . --config Release || exit /b 1

echo Build finished successfully.
exit /b 0
