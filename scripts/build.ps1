# Copyright dSPACE SE & Co. KG. All rights reserved.

param(
  [string]$config = "Debug"
)

switch ($config.ToLower()) {
  'debug' { $config = "Debug" }
  'release' { $config = "Release" }
  default { $config = "Debug" }
}

$baseDir = Split-Path $PSScriptRoot

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
  Write-Error "Could not find cmake in PATH."
  exit 1
}

if (-not (Get-Command ninja -ErrorAction SilentlyContinue)) {
  Write-Error "Could not find ninja in PATH."
  exit 1
}

Write-Host "Building for $config ..." -ForegroundColor Blue

$buildDir = Join-Path $baseDir "tmpwin/$config"

if (-not (Test-Path $buildDir)) {
  New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
}

& cmake . -B "$buildDir" -GNinja -DCMAKE_BUILD_TYPE="$config" -DDSVEOSCOSIM_BUILD_TESTS=ON
if ($LASTEXITCODE -ne 0) {
  Write-Error "Building for $config failed."
  exit 1
}

& cmake --build "$buildDir" --config $config
if ($LASTEXITCODE -ne 0) {
  Write-Error "Building for $config failed."
  exit 1
}

Write-Host "Building for $config finished successfully." -ForegroundColor Blue
