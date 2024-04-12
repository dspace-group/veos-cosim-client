@echo off

setlocal

if not exist tmpwin\Debug mkdir tmpwin\Debug || exit /b 1
cd tmpwin\Debug
cmake ..\.. -DDSVEOSCOSIM_BUILD_TESTS=ON || exit /b 1
cmake --build . --config Debug || exit /b 1

if not exist tmpwin\Release mkdir tmpwin\Release || exit /b 1
cd tmpwin\Release
cmake ..\.. -DDSVEOSCOSIM_BUILD_TESTS=ON || exit /b 1
cmake --build . --config Release || exit /b 1

echo Build successful
exit /b 0
