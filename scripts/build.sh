#!/usr/bin/env bash
# Copyright dSPACE SE & Co. KG. All rights reserved.
set -euo pipefail

CONFIG="${1:-debug}"
if [[ "$CONFIG" != "debug" && "$CONFIG" != "release" ]]; then
  echo "Usage: $0 [debug|release]" >&2
  exit 1
fi

PRESET="linux-$CONFIG"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"

echo "==> Configure ($PRESET)"
cmake --preset "$PRESET" -S "$ROOT"

echo "==> Build ($PRESET)"
cmake --build --preset "$PRESET" --parallel
