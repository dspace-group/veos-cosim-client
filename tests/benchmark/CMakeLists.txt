# Copyright dSPACE SE & Co. KG. All rights reserved.

add_executable(
  DsVeosCoSimBenchmark
)

target_sources(
  DsVeosCoSimBenchmark
  PRIVATE
  Communication/BenchmarkChannel.cpp
  OsAbstraction/BenchmarkNamedEvent.cpp
  OsAbstraction/BenchmarkShmPipe.cpp
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

target_link_libraries(
  DsVeosCoSimBenchmark
  DsVeosCoSim
  shared
  benchmark::benchmark
)
