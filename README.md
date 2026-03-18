# veos-cosim-client

Implementation of a shared library used to create co-simulation clients for dSPACE VEOS.

## Documentation

- [Documentation entry point](doc/documentation.md)
- [Quickstart](doc/guides/quickstart.md)
- [Tutorial](doc/tutorial/tutorial.md)
- [API reference](doc/api-reference/api-reference.md)

## Build

### As a standalone project

```console
cmake -S . -B build
cmake --build build
```

### As a subdirectory in another CMake project

```cmake
add_subdirectory(veos-cosim-client)
target_link_libraries(your_target PRIVATE DsVeosCoSim)
```

The public headers are located in `include/DsVeosCoSim`.

## Integration Choices

- Use the project as a CMake subdirectory if you want to build the client library together with your application.
- Build it as a shared library if you want to access the API from another language runtime.
- Build it as a static library if you want to link it directly into your executable.

For more detailed setup guidance, refer to [Basics on CoSim Clients](doc/basics/basics-clients.md) and [How to Prepare the CoSim Demo](doc/tutorial/prepare.md).
