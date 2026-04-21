# Copyright dSPACE SE & Co. KG. All rights reserved.

param(
  [Parameter(Position = 0)]
  [ValidateSet("debug", "release")]
  [string]$Config = "debug"
)

$Preset = "win-$($Config.ToLowerInvariant())"
$Root = Split-Path $PSScriptRoot -Parent
$BuildDir = Join-Path $Root "build" $Preset

if (Test-Path $BuildDir) {
  Write-Host "==> Removing $BuildDir" -ForegroundColor Cyan
  Remove-Item -Recurse -Force $BuildDir
}
else {
  Write-Host "==> Nothing to clean ($BuildDir does not exist)" -ForegroundColor Yellow
}
