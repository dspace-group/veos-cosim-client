# veos-cosim-client

Implementation of a C++ library used to create co-simulation clients for dSPACE VEOS.

## Documentation

- [Documentation entry point](doc/documentation.md)
- [Quickstart](doc/guides/quickstart.md)
- [Tutorial](doc/tutorial/tutorial.md)
- [API reference](doc/api-reference/api-reference.md)

## Build

### As a standalone project

DsVeosCoSim requires `fmt` for internal string formatting. A standalone build picks up the bundled copy from `third_party/fmt` automatically, so no extra CMake arguments are required as long as that directory is present.

The repository provides cross-platform CMake presets, wrapper scripts, and `just` recipes.

- `scripts/build.ps1` for native Windows builds
- `scripts/build.sh` for native Linux and WSL builds
- `scripts/test.ps1` and `scripts/test.sh` to build and run the test suite
- `just build`, `just test`, and `just clean` as optional wrappers around the scripts

Defaults:

- Configuration: `Debug`
- Tests: `ON`
- Output directories follow the active preset, for example `build/win-debug` or `build/linux-debug`

#### Build with scripts (recommended)

Windows (PowerShell):

```powershell
./scripts/build.ps1
```

Linux:

```bash
./scripts/build.sh
```

WSL:

```bash
./scripts/build.sh
```

The scripts accept the build configuration as a positional argument:

- PowerShell: `debug|release`
- Bash: `debug|release`

Examples:

```powershell
./scripts/build.ps1
./scripts/build.ps1 Release
```

```bash
./scripts/build.sh
./scripts/build.sh release
```

To build and run tests with the wrapper scripts:

```powershell
./scripts/test.ps1
./scripts/test.ps1 Release
```

```bash
./scripts/test.sh
./scripts/test.sh release
```

If you use MSVC on Windows, run the PowerShell scripts from a Visual Studio Developer PowerShell or Developer Command Prompt.

#### Build with `just`

```console
just build
just build config=release
just test
just clean
```

#### Build with CMake presets directly

List available presets:

```console
cmake --list-presets
```

Configure, build, and test with a preset:

```console
cmake --preset win-debug
cmake --build --preset win-debug
ctest --preset win-debug
```

Preset families:

- Windows native: `win-debug`, `win-release`
- Linux and WSL: `linux-debug`, `linux-release`

Linux example:

```console
cmake --preset linux-debug
cmake --build --preset linux-debug
ctest --preset linux-debug
```

WSL note:

- Use the `linux-*` presets in WSL.

#### Build without presets

If you prefer not to use presets, a plain CMake build works well with single-config generators such as Ninja on Linux or WSL:

```console
cmake -S . -B build -DDSVEOSCOSIM_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
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
