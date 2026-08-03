#!/usr/bin/env bash
# Copyright dSPACE SE & Co. KG. All rights reserved.
#
# Layer 2 ABI guard: build the shared library with debug info, dump its ABI,
# and compare against the committed baseline using libabigail.
#
# Usage:
#   tools/abi-diff.sh            # compare current build against baseline (default)
#   tools/abi-diff.sh --update   # regenerate and overwrite the baseline instead
#
# Requirements: abigail-tools (abidw, abidiff), cmake, ninja, clang.
#   Debian/Ubuntu: sudo apt-get install -y abigail-tools cmake ninja-build clang

set -euo pipefail

# --- Configuration ------------------------------------------------------------
PRESET="linux-debug"
BASELINE="api/DsVeosCoSim.baseline.abi"
SUPPRESSIONS="api/abi-suppressions.txt"
HEADERS_DIR="include"
CURRENT_ABI="$(mktemp -t current-abi.XXXXXX)"
trap 'rm -f "$CURRENT_ABI"' EXIT

UPDATE=0
[ "${1:-}" = "--update" ] && UPDATE=1

# Emit a GitHub Actions annotation when in CI, otherwise a plain message.
log_error()   { if [ -n "${GITHUB_ACTIONS:-}" ]; then echo "::error::$*"; else echo "ERROR: $*" >&2; fi; }
log_warning() { if [ -n "${GITHUB_ACTIONS:-}" ]; then echo "::warning::$*"; else echo "WARNING: $*" >&2; fi; }

# --- Build --------------------------------------------------------------------
echo ">> Configuring and building shared library (preset: $PRESET)"
CC="${CC:-clang}" CXX="${CXX:-clang++}" cmake --preset "$PRESET" \
  -DDSVEOSCOSIM_BUILD_SHARED=ON \
  -DDSVEOSCOSIM_BUILD_TESTS=OFF \
  -DCMAKE_CXX_FLAGS="-g -Og" \
  -DCMAKE_C_FLAGS="-g -Og"
cmake --build --preset "$PRESET"

# --- Locate the built .so -----------------------------------------------------
SO="$(find build -name 'libDsVeosCoSim*.so' -print -quit || true)"
if [ -z "$SO" ]; then
  log_error "Could not find libDsVeosCoSim*.so under build/. Did the shared build succeed?"
  exit 1
fi
echo ">> Found shared library: $SO"

# --- Dump the current ABI -----------------------------------------------------
echo ">> Generating ABI dump"
abidw --headers-dir "$HEADERS_DIR" --out-file "$CURRENT_ABI" "$SO"

# --- Update mode: overwrite the baseline and exit -----------------------------
if [ "$UPDATE" -eq 1 ]; then
  mkdir -p "$(dirname "$BASELINE")"
  cp "$CURRENT_ABI" "$BASELINE"
  echo ">> Baseline updated: $BASELINE"
  echo "   Commit this file and bump the version per doc/api-guard.md."
  exit 0
fi

# --- Compare mode -------------------------------------------------------------
if [ ! -f "$BASELINE" ]; then
  log_error "Missing baseline '$BASELINE'. Seed it once with: tools/abi-diff.sh --update"
  exit 1
fi

SUPP_ARG=()
[ -f "$SUPPRESSIONS" ] && SUPP_ARG=(--suppressions "$SUPPRESSIONS")

echo ">> Comparing current ABI against baseline"
set +e
abidiff \
  "${SUPP_ARG[@]}" \
  --headers-dir1 "$HEADERS_DIR" --headers-dir2 "$HEADERS_DIR" \
  "$BASELINE" "$CURRENT_ABI"
status=$?
set -e

# abidiff exit status is a bitmask:
#   0x00 = no change
#   0x04 = ABI changed (compatible)
#   0x08 = ABI changed INCOMPATIBLY
if [ $((status & 0x08)) -ne 0 ]; then
  log_error "Incompatible ABI change detected. If intentional, run 'tools/abi-diff.sh --update', commit $BASELINE, and bump the MAJOR version (see doc/api-guard.md)."
  exit 1
fi

if [ "$status" -ne 0 ]; then
  log_warning "ABI changed but remains backward-compatible (e.g. additions). Update the baseline and bump the MINOR version."
  # Non-fatal: additions don't break existing consumers.
  exit 0
fi

echo ">> ABI is unchanged. OK."
