# Copyright dSPACE GmbH. All rights reserved.

add_executable(
  DsVeosCoSimBenchmark
)

target_sources(
  DsVeosCoSimBenchmark
  PRIVATE
  Communication/BenchmarkChannel.cpp
  OsAbstraction/BenchmarkNamedEvent.cpp
  OsAbstraction/BenchmarkSocket.cpp
  BenchmarkBusBuffer.cpp
  BenchmarkCoSim.cpp
  BenchmarkIoBuffer.cpp
  Program.cpp
)

target_compile_options(
  DsVeosCoSimBenchmark
  PRIVATE
  ${DSVEOSCOSIM_WARNINGS}
)

target_compile_definitions(
  DsVeosCoSimBenchmark
  PRIVATE
  ALL_BENCHMARK_TESTS
)

target_link_libraries(
  DsVeosCoSimBenchmark
  DsVeosCoSim
  shared
  benchmark::benchmark
)
