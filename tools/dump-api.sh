#!/usr/bin/env bash
# Copyright dSPACE SE & Co. KG. All rights reserved.
# Emits a deterministic snapshot of the public API from DsVeosCoSim.h.
# Usage: tools/dump-api.sh [path/to/DsVeosCoSim.h]
set -euo pipefail
HEADER="${1:-include/DsVeosCoSim/DsVeosCoSim.h}"
clang -Xclang -ast-dump=json -fsyntax-only -x c++ -std=c++17 \
      -Iinclude "$HEADER" \
  | python3 tools/normalize_api.py
