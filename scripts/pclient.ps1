# Copyright dSPACE SE & Co. KG. All rights reserved.

param(
  [string]$config = "Release"
)

$ErrorActionPreference = "Stop"

switch ($config.ToLower()) {
  'debug' { $config = "Debug" }
  'release' { $config = "Release" }
  default { $config = "Release" }
}

$baseDir = Split-Path $PSScriptRoot

Write-Host "Running performance test client for $config ..." -ForegroundColor Blue

$filePath = Join-Path $baseDir "tmpwin/$config/tests/PerformanceTestClient/PerformanceTestClient.exe"

if (-not (Test-Path $filePath)) {
  Write-Error "Could not find file '$filePath'."
  exit 1
}

& "$filePath"

Write-Host "Running performance test client for $config finished successfully." -ForegroundColor Blue
