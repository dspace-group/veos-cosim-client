# veos-cosim-client

Implementation of a C++ library used to create co-simulation clients for dSPACE VEOS.

## Documentation

- [Documentation entry point](doc/documentation.md)
- [Quickstart](doc/guides/quickstart.md)
- [Tutorial](doc/tutorial/tutorial.md)
- [API reference](doc/api-reference/api-reference.md)

## Build

### As a standalone project

CMake 3.20 or newer is required, including for the provided configure, build, and test presets.

DsVeosCoSim requires `fmt` for internal string formatting. It reuses an existing `fmt::fmt` or `fmt::fmt-header-only` target first, then tries an installed CMake package. Otherwise it uses the initialized `third_party/fmt` submodule when available. If none is available, configuration fails; dependencies are not downloaded automatically. Set `DSVEOSCOSIM_USE_BUNDLED_FMT=OFF` to require an existing target or installed package.

The repository provides cross-platform CMake presets, wrapper scripts, and `just` recipes.

- `scripts/build.ps1` for native Windows builds
- `scripts/build.sh` for native Linux and WSL builds
- `scripts/test.ps1` and `scripts/test.sh` to build and run the test suite
- `just build`, `just test`, and `just clean` as optional wrappers around the scripts

Defaults:

- Configuration: `Debug`
- Tests: `ON`
- Output directories follow the active preset: `build/debug` or `build/release`

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
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

The cross-platform presets are `debug` and `release`. CMake uses the compiler
provided by the current Windows, Linux, or WSL environment.

#### Build without presets

If you prefer not to use presets, a plain CMake build works well with single-config generators such as Ninja on Linux or WSL:

```console
cmake -S . -B build -DDSVEOSCOSIM_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

### As a subdirectory in another CMake project

When you embed DsVeosCoSim into a parent project, CMake uses an existing `fmt::fmt` or `fmt::fmt-header-only` target if your project already defines one. It does not configure or compile a second fmt copy in that case.

```cmake
find_package(fmt CONFIG REQUIRED) # Optional if you want to use your own fmt package.
add_subdirectory(veos-cosim-client)
target_link_libraries(your_target PRIVATE DsVeosCoSim::DsVeosCoSim)
```

The original target name `DsVeosCoSim` remains supported. The namespaced alias `DsVeosCoSim::DsVeosCoSim` is recommended for consumers. C++17 is a target requirement; the parent project's `CMAKE_CXX_STANDARD` is not overwritten.

`DSVEOSCOSIM_BUILD_TESTS` controls both test compilation and CTest registration, independently of `BUILD_TESTING`. When embedding the project, enable testing in the top-level project as well so CTest can discover the nested tests. Existing `GTest::gmock` and `GTest::gtest` targets are reused without modification; otherwise initialize `third_party/googletest`. Both targets must be provided together. The test targets are `DsVeosCoSimTest`, `DsVeosCoSimTestSupport`, `DsVeosCoSimTestClient`, and `DsVeosCoSimTestServer`.

### With FetchContent

```cmake
include(FetchContent)
FetchContent_Declare(veos_cosim_client
	GIT_REPOSITORY https://github.com/dspace-group/veos-cosim-client.git
	GIT_TAG <full-commit-id>
	GIT_SUBMODULES "")
FetchContent_MakeAvailable(veos_cosim_client)
target_link_libraries(your_target PRIVATE DsVeosCoSim::DsVeosCoSim)
```

Replace `<full-commit-id>` with the revision you want to use. With `GIT_SUBMODULES ""`, provide fmt as an existing target or installed package. To use the bundled fmt instead, replace that setting with `GIT_SUBMODULES "third_party/fmt"`. Tests additionally require existing `GTest::gmock` and `GTest::gtest` targets or the GoogleTest submodule.

To share one fmt build with your application, create its target before including VEOS, for example through `find_package` or `FetchContent_MakeAvailable`. The application is responsible for selecting a fmt version compatible with all consumers.

The library is static by default. Keep `BUILD_SHARED_LIBS=OFF` and `DSVEOSCOSIM_BUILD_SHARED=OFF` for static linkage. Scope these settings to the dependency integration rather than forcing unrelated parent-project libraries to change linkage.

For bundle mode (`DSVEOSCOSIM_BUILD_SHARED=ON` and `BUILD_SHARED_LIBS=OFF`), the bundled fmt is configured with position-independent code and hidden symbols. External fmt targets and packages are not modified: when using a compiled external fmt, the provider must ensure static linkage, position-independent code where required, and suitable symbol visibility to preserve the single-library bundle.

The public header is `include/DsVeosCoSim/DsVeosCoSim.h`.

## Integration Choices

- Use the project as a CMake subdirectory if you want to build the client library together with your application.
- Build it as a shared library if you want to access the API from another language runtime.
- Build it as a static library if you want to link it directly into your executable.

For more detailed setup guidance, refer to [Basics on CoSim Clients](doc/basics/basics-clients.md) and [How to Prepare the CoSim Demo](doc/tutorial/prepare.md).

## API Stability

Changes to the public API in
[`include/DsVeosCoSim/DsVeosCoSim.h`](include/DsVeosCoSim/DsVeosCoSim.h) are
gated by an automated API/ABI guard. See **[doc/api-guard.md](doc/api-guard.md)**
before modifying it.
