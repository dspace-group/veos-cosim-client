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
  BenchmarkIoBuffer.cpp
  Program.cpp
)

target_link_libraries(
  DsVeosCoSimBenchmark
  DsVeosCoSim
  shared
  benchmark::benchmark
)
