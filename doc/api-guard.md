<!-- Copyright dSPACE SE & Co. KG. All rights reserved. -->

# API & ABI Guard

Every pull request runs an automated API/ABI guard in CI to protect external
co-simulation clients from accidental breaking changes.

## What the guard checks

The public API surface is the single header
[`include/DsVeosCoSim/DsVeosCoSim.h`](../include/DsVeosCoSim/DsVeosCoSim.h).
Everything under `src/` is internal.

| Layer | Job | Detects | Platform |
|-------|-----|---------|----------|
| 1 API snapshot | `api-snapshot` | function signatures, struct field types/order, enum values | Cross-platform |
| 2 ABI diff | `abi-diff` | struct layout, symbol removal/change, enum size | Linux |

Both compare against a committed baseline:
- Layer 1: `api/DsVeosCoSim.api.txt`
- Layer 2: `api/DsVeosCoSim.baseline.abi`

## Prerequisites

### Layer 1 (API snapshot)

- `clang`
- `python3`

```bash
sudo apt-get update && sudo apt-get install -y clang python3
```

### Layer 2 (ABI diff)

`abidw` and `abidiff` ship as part of **libabigail**. On Debian/Ubuntu they are
provided by the `abigail-tools` package:

```bash
sudo apt-get update && sudo apt-get install -y \
  abigail-tools \   # provides abidw / abidiff
  cmake \
  ninja-build \     # the linux presets use the Ninja generator
  clang             # compiler
```

Other platforms:

| Platform | Command | Package |
|----------|---------|---------|
| Fedora / RHEL / CentOS | `sudo dnf install libabigail` | `libabigail` |
| openSUSE | `sudo zypper install libabigail-tools` | `libabigail-tools` |
| Arch (AUR) | build from AUR | `libabigail` |

Verify the tools are available:

```bash
abidw --version
abidiff --version
```

> **Debug info is required.** `abidw` reads the ABI from a binary's DWARF debug
> information, so the shared library must be built with `-g` (the ABI build uses
> `-DCMAKE_CXX_FLAGS="-g -Og" -DCMAKE_C_FLAGS="-g -Og"`). A stripped/release
> binary without debug info will not produce a meaningful ABI dump.

> **Tests must be disabled for the ABI build.** In shared mode the library
> exports only the public `DsVeosCoSim_*` symbols with hidden visibility, so the
> test suite (which links internal symbols) cannot be built at the same time.
> The ABI build therefore uses `-DDSVEOSCOSIM_BUILD_TESTS=OFF` and builds only
> the library target.

## Running the checks locally

### Layer 1 — API snapshot

```bash
./tools/dump-api.sh > /tmp/current.api.txt
diff -u api/DsVeosCoSim.api.txt /tmp/current.api.txt
```

No output means no public API change.

### Layer 2 — ABI diff

Use the helper script, which builds the shared library (tests off, with debug
info), dumps its ABI, and compares against the baseline:

```bash
./tools/abi-diff.sh            # compare against the committed baseline
./tools/abi-diff.sh --update   # regenerate the baseline (intentional break)
```

Equivalent manual steps:

```bash
CC=clang CXX=clang++ cmake --preset debug \
  -DDSVEOSCOSIM_BUILD_SHARED=ON -DDSVEOSCOSIM_BUILD_TESTS=OFF \
  -DCMAKE_CXX_FLAGS="-g -Og" -DCMAKE_C_FLAGS="-g -Og"
cmake --build --preset debug --target DsVeosCoSim

SO="$(find build -name 'libDsVeosCoSim*.so' -print -quit)"
abidw --headers-dir include --out-file /tmp/current.abi "$SO"
abidiff --suppressions api/abi-suppressions.txt \
        --headers-dir1 include --headers-dir2 include \
        api/DsVeosCoSim.baseline.abi /tmp/current.abi
```

## Intentional-break workflow

When you deliberately change the public API/ABI, in the same PR:

1. Regenerate baselines:
   ```bash
   ./tools/dump-api.sh > api/DsVeosCoSim.api.txt
   ./tools/abi-diff.sh --update
   ```
2. Commit the updated `api/` files (the diff makes the change reviewable).
3. Bump the version per the semver policy below.
4. Describe the break in the changelog with a migration note.

## Semantic versioning policy

| Change | Guard result | Version bump |
|--------|--------------|--------------|
| Breaking: removed/renamed function, changed signature, changed struct layout/field type, changed enum value | fails / incompatible | MAJOR (1.2.0 -> 2.0.0) |
| Additive: new function/struct/enum, appended optional field | baseline update / compatible | MINOR (1.2.0 -> 1.3.0) |
| No public API change | passes | PATCH (1.2.0 -> 1.2.1) |

The v1.2.0 size-type widening (`int32_t` -> `size_t`) is a textbook MAJOR change:
under this policy it would have been released as 2.0.0.

## Suppressing known noise (Layer 2)

Internal/bundled dependencies (such as the hidden-visibility `fmt`) are not part
of the public contract. Suppress them in
[`api/abi-suppressions.txt`](../api/abi-suppressions.txt) so the guard only
reacts to genuine public API changes. Keep suppressions narrow and documented.

## Making the guard required

Add the `api-snapshot` and `abi-diff` jobs as required status checks on `main`.
