# Copyright dSPACE SE & Co. KG. All rights reserved.

set windows-shell := ["pwsh.exe", "-NoLogo", "-NoProfile", "-c"]

script_ext := if os() == "windows" { "ps1" } else { "sh" }

help:
    @just --list

build config="debug":
    scripts/build.{{ script_ext }} {{ if os() == "windows" { "-Config" } else { "" } }} {{ config }}

clean config="debug":
    scripts/clean.{{ script_ext }} {{ if os() == "windows" { "-Config" } else { "" } }} {{ config }}

test config="debug":
    scripts/test.{{ script_ext }} {{ if os() == "windows" { "-Config" } else { "" } }} {{ config }}
