# Copyright dSPACE GmbH. All rights reserved.

add_executable(
  ${PROJECT_NAME}Benchmark
  Communication/BenchmarkChannel.cpp
  OsAbstraction/BenchmarkNamedEvent.cpp
  OsAbstraction/BenchmarkSocket.cpp
  BenchmarkBusBuffer.cpp
  BenchmarkIoBuffer.cpp
  Program.cpp
)

target_link_libraries(
  ${PROJECT_NAME}Benchmark
  ${PROJECT_NAME}
  shared
  benchmark::benchmark
)
