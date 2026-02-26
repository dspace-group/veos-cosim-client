# Copyright dSPACE SE & Co. KG. All rights reserved.

param(
  [string]$config = "Debug"
)

$ErrorActionPreference = "Stop"

switch ($config.ToLower()) {
  'debug' { $config = "Debug" }
  'release' { $config = "Release" }
  default { $config = "Debug" }
}

$baseDir = Split-Path $PSScriptRoot

Write-Host "Running test client for $config ..." -ForegroundColor Blue

$filePath = Join-Path $baseDir "tmpwin/$config/tests/TestClient/TestClient.exe"

if (-not (Test-Path $filePath)) {
  Write-Error "Could not find file '$filePath'."
  exit 1
}

& "$filePath"
if ($LASTEXITCODE -ne 0) {
  Write-Error "Running test client for $config failed."
  exit 1
}

Write-Host "Running test client for $config finished successfully." -ForegroundColor Blue
