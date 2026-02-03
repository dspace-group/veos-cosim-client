# Copyright dSPACE SE & Co. KG. All rights reserved.

$baseDir = Split-Path $PSScriptRoot

Write-Host "Cleaning ..."

$dirToRemove = Join-Path $baseDir "tmpwin"

Remove-Item -Path $dirToRemove -Recurse -Force -ErrorAction SilentlyContinue

Write-Host "Cleaning finished successfully."
