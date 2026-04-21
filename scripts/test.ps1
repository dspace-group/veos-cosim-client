# Copyright dSPACE SE & Co. KG. All rights reserved.

param(
  [Parameter(Position = 0)]
  [ValidateSet("debug", "release")]
  [string]$Config = "debug"
)

$Preset = "win-$($Config.ToLowerInvariant())"

Write-Host "==> Running build before test" -ForegroundColor Cyan
& "$PSScriptRoot\build.ps1" $Config
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "==> Test ($Preset)" -ForegroundColor Cyan
ctest --preset $Preset --output-on-failure
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
