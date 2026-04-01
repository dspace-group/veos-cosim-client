# Copyright dSPACE SE & Co. KG. All rights reserved.

param(
  [ValidateSet("debug", "release")]
  [string]$Config = "debug"
)

$Preset = "win-$Config"

Write-Host "==> Running build before test" -ForegroundColor Cyan
& "$PSScriptRoot\build.ps1" -Config $Config
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "==> Test ($Preset)" -ForegroundColor Cyan
ctest --preset $Preset
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
