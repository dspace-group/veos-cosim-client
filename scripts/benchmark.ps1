# Copyright dSPACE GmbH. All rights reserved.

param(
  [string]$config = "Release"
)

$ErrorActionPreference = "Stop"

switch ($config.ToLower()) {
  'debug'   { $config = "Debug" }
  'release' { $config = "Release" }
  default   { $config = "Release" }
}

$baseDir = Split-Path $PSScriptRoot

Write-Host "Running benchmarks for $config ..."

$filePath = Join-Path $baseDir "tmpwin/$config/tests/benchmark/DsVeosCoSimBenchmark.exe"

if (-not (Test-Path $filePath)) {
  Write-Error "Could not find file '$filePath'."
  exit 1
}

& "$filePath"

Write-Host "Running benchmarks for $config finished successfully."
