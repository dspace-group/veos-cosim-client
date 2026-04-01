#!/usr/bin/env bash
# Copyright dSPACE SE & Co. KG. All rights reserved.
set -euo pipefail

CONFIG="${1:-debug}"
if [[ "$CONFIG" != "debug" && "$CONFIG" != "release" ]]; then
  echo "Usage: $0 [debug|release]" >&2
  exit 1
fi

PRESET="linux-$CONFIG"
BUILD_DIR="$(cd "$(dirname "$0")/.." && pwd)/build/$PRESET"

if [ -d "$BUILD_DIR" ]; then
  echo "==> Removing $BUILD_DIR"
  rm -rf "$BUILD_DIR"
else
  echo "==> Nothing to clean ($BUILD_DIR does not exist)"
fi
