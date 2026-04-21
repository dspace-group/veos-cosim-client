# AGENTS.md

## Scope

- This repository builds a C++17/CMake VEOS co-simulation library.
- Keep comments and identifiers in English.
- Prefer minimal, surgical changes that match the existing style.

## Start Here

- [README.md](README.md) for supported build and test entry points.
- [CLAUDE.md](CLAUDE.md) for model-agnostic workflow and verification heuristics used by tools that read `CLAUDE.md`.
- [doc/documentation.md](doc/documentation.md) for the documentation entry point.
- [doc/guides/lifecycle.md](doc/guides/lifecycle.md) for connection and execution-mode rules.
- [doc/guides/callbacks-and-buffering.md](doc/guides/callbacks-and-buffering.md) for callback precedence and receive semantics.
- [doc/guides/messages-vs-containers.md](doc/guides/messages-vs-containers.md) when changing bus-message APIs or docs.

## Code Style

- clang-format (Chromium base, 160-char column limit, LF line endings). Run `clang-format -i <file>` before committing edited files.
- clang-tidy is enabled with a broad check set; see [.clang-tidy](.clang-tidy) for suppressed checks and naming rules.
- Naming: classes and methods are `PascalCase`; private member fields are `_camelCase` (prefix `_`, camelBack). Enforced by `readability-identifier-naming`.
- Every new source file must begin with `// Copyright dSPACE SE & Co. KG. All rights reserved.`
- Apply `[[nodiscard]]` to every function that returns `Result` or `bool`.
- When adding a new `enum class`, add a `format_as(EnumName)` free function so `{fmt}` can format it directly.
- Use anonymous namespaces (`namespace { ... }`) in `.cpp` files for file-local implementation classes.

## Build And Test

- Preferred wrappers:
  - Windows: `./scripts/build.ps1`, `./scripts/test.ps1`
  - Linux/WSL: `./scripts/build.sh`, `./scripts/test.sh`
  - Optional wrappers: `just build`, `just test`, `just clean`
- Preferred direct presets:
  - Windows: `cmake --preset win-debug`, `cmake --build --preset win-debug`, `ctest --preset win-debug`
  - Linux/WSL: `cmake --preset linux-debug`, `cmake --build --preset linux-debug`, `ctest --preset linux-debug`
- Tests are only built when `DSVEOSCOSIM_BUILD_TESTS=ON`.
- Always run tests through CTest, not by invoking test binaries directly.
- On Windows, PowerShell build scripts expect a Visual Studio Developer PowerShell or Developer Command Prompt.

## Repo Map

- [include/DsVeosCoSim/DsVeosCoSim.h](include/DsVeosCoSim/DsVeosCoSim.h): public C API. Treat it as stable unless the user explicitly asks for an API change.
- [src/DsVeosCoSim.cpp](src/DsVeosCoSim.cpp): exported C API shim over the internal C++ implementation.
- [src/CoSimClient.hpp](src/CoSimClient.hpp) and matching `.cpp`: client connection lifecycle, command loop, and public operations.
- [src/CoSimServer.hpp](src/CoSimServer.hpp) and matching `.cpp`: server-side test/demo harness and background service flow.
- [src/Protocol.hpp](src/Protocol.hpp) and matching `.cpp`: frame protocol, version negotiation, and serialization contract.
- [src/SignalExchange.hpp](src/SignalExchange.hpp) and [src/BusExchange.hpp](src/BusExchange.hpp): signal and bus data exchange layers.
- [src/Communication](src/Communication): local (ShmPipe, Windows-only) and TCP socket transport implementations; transport is selected by `ConnectionKind` (`Local` vs `Remote`).
- [src/Helpers](src/Helpers): `Result.hpp` (result enum + predicates), `Logger.hpp` (severity, thread-safe, fmt-backed), `RingBuffer.hpp`, `Event.hpp`, `Environment.hpp` (debug/trace env-var helpers).
- [src/OsAbstraction](src/OsAbstraction): platform-specific helpers (`NamedEvent`, `SharedMemory`, `ShmPipe*` — all Windows-only; `Socket` — cross-platform). Keep new platform-specific code here; platform branching is `#ifdef _WIN32` within shared files, not filename-level splits.
- [tests/unit](tests/unit): GoogleTest unit coverage for the library internals; many tests are parameterised on `ConnectionKind` and/or `CoSimType`.
- [tests/shared](tests/shared): shared test helpers used by all test targets — random data generators, signal/bus factories, `StartUp()`, logging utilities.
- [tests/TestClient](tests/TestClient) and [tests/TestServer](tests/TestServer): executable test programs built only when tests are enabled.

## Working Rules

- Do not change the public API or the structure of [CMakePresets.json](CMakePresets.json) without an explicit request.
- Keep cross-platform code on the standard library path when possible; avoid platform-specific APIs outside [src/OsAbstraction](src/OsAbstraction) and the existing platform-specialized files.
- Error handling rule of thumb:
  - Use `std::runtime_error` where an internal invariant/assert-style failure would otherwise be appropriate.
  - Use `DsVeosCoSim::Result` for normal API and transport outcomes.
  - Prefer the predicate helpers (`IsOk`, `IsError`, `IsTimeout`, etc. from `Helpers/Result.hpp`) over direct enum comparisons.
- Callbacks run on the polling thread. Do not add internal synchronization unless the requested change requires it.
- Remove only the dead code your change creates; do not refactor unrelated areas opportunistically.

## Simulation Pitfalls

- After connecting, choose one execution mode per connection: callback-based or polling-based. Reconnect before switching modes.
- In polling mode, every successful `PollCommand` must be followed by `FinishCommand`.
- For the same bus/data kind on the same connection, do not mix push-style callbacks with pull-style receive APIs.
- Message-container callbacks take precedence over plain message callbacks for the same bus type.
- After disconnect/reconnect, reacquire dynamic lists such as signals and controllers.

## Documentation Changes

- Link to existing docs instead of copying long explanations into instructions or comments.
- If you edit files under [doc](doc), preserve relative Markdown links and validate any local links you touch.
