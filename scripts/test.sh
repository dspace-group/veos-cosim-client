#!/usr/bin/env bash
# Copyright dSPACE SE & Co. KG. All rights reserved.
set -euo pipefail

CONFIG="${1:-debug}"
if [[ "$CONFIG" != "debug" && "$CONFIG" != "release" ]]; then
  echo "Usage: $0 [debug|release]" >&2
  exit 1
fi

PRESET="linux-$CONFIG"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

echo "==> Running build before test"
"$SCRIPT_DIR/build.sh" "$CONFIG"

echo "==> Test ($PRESET)"
ctest --preset "$PRESET"
