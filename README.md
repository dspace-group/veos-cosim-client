# veos-cosim-client

Implementation of a shared library used to create co-simulation clients for dSPACE VEOS.

## Documentation

- [Documentation entry point](doc/documentation.md)
- [Quickstart](doc/guides/quickstart.md)
- [Tutorial](doc/tutorial/tutorial.md)
- [API reference](doc/api-reference/api-reference.md)

## Build

### As a standalone project

DsVeosCoSim requires `fmt` for internal string formatting. A standalone build picks up the bundled copy from `third_party/fmt` automatically, so no extra CMake arguments are required as long as that directory is present.

The repository now provides cross-platform CMake presets and wrapper scripts:

- `build.ps1` for native Windows
- `build.sh` for native Linux and WSL

Defaults:

- Configuration: `Debug`
- Tests: `ON`
- WSL uses dedicated output directories (`build/wsl-*`) to avoid interference with native Windows builds.

#### Build with scripts (recommended)

Windows (PowerShell):

```powershell
./build.ps1
```

Linux:

```bash
./build.sh
```

WSL:

```bash
./build.sh
```

Useful options:

- `--config Release` for release builds
- `--no-test` to skip tests
- `--clean` to remove the preset-specific build directory before configuring
- `--preset <name>` to force a specific preset

Examples:

```powershell
./build.ps1 -Config Release -Clean
./build.ps1 -Preset win-debug
```

```bash
./build.sh --config Release --clean
./build.sh --no-test
```

#### Build with CMake presets directly

List available presets:

```console
cmake --list-presets
```

Configure/build/test with a preset:

```console
cmake --preset win-debug
cmake --build --preset win-debug --config Debug
./build/win-debug/tests/unit/Debug/DsVeosCoSimTest.exe
```

Preset families:

- Windows native: `win-debug`, `win-release`
- Linux native: `linux-debug`, `linux-release`
- WSL: `wsl-debug`, `wsl-release`

WSL note:

- Use only `wsl-*` presets in WSL. Do not reuse `win-*` or `linux-*` output directories from WSL.

```console
cmake -S . -B build
cmake --build build
```

### As a subdirectory in another CMake project

When you embed DsVeosCoSim into a parent project, CMake uses an existing `fmt::fmt` or `fmt::fmt-header-only` target if your project already defines one. Otherwise it falls back to the bundled copy in `third_party/fmt`.

```cmake
find_package(fmt CONFIG REQUIRED) # Optional if you want to use your own fmt package.
add_subdirectory(veos-cosim-client)
target_link_libraries(your_target PRIVATE DsVeosCoSim)
```

If you do not provide `fmt` from the parent project, keep `third_party/fmt` in the `veos-cosim-client` source tree so `add_subdirectory(veos-cosim-client)` can add the bundled copy.

The public header is `include/DsVeosCoSim/DsVeosCoSim.h`.

## Integration Choices

- Use the project as a CMake subdirectory if you want to build the client library together with your application.
- Build it as a shared library if you want to access the API from another language runtime.
- Build it as a static library if you want to link it directly into your executable.

For more detailed setup guidance, refer to [Basics on CoSim Clients](doc/basics/basics-clients.md) and [How to Prepare the CoSim Demo](doc/tutorial/prepare.md).
