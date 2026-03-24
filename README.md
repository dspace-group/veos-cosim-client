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
