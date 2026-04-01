# Copyright dSPACE SE & Co. KG. All rights reserved.

param(
  [ValidateSet("debug", "release")]
  [string]$Config = "debug"
)

$Preset = "win-$Config"
$Root = Split-Path $PSScriptRoot -Parent

Write-Host "==> Configure ($Preset)" -ForegroundColor Cyan
cmake --preset $Preset -S $Root
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "==> Build ($Preset)" -ForegroundColor Cyan
cmake --build --preset $Preset
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
