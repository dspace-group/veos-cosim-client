# Copyright dSPACE GmbH. All rights reserved.

add_executable(
  ${PROJECT_NAME}Benchmark
  Program.cpp
  BenchmarkHelper.cpp
  BenchmarkLocalChannel.cpp
  BenchmarkRemoteChannel.cpp
  BenchmarkSocket.cpp
  ../test/Generator.cpp
)

target_include_directories(
  ${PROJECT_NAME}Benchmark
  PUBLIC
  ./
  ../include
  ../src
  ../test
  ../third_party/benchmark/include
)

target_compile_definitions(${PROJECT_NAME}Benchmark PRIVATE _CRT_SECURE_NO_WARNINGS DSVEOSCOSIM_EXPORT WIN32_LEAN_AND_MEAN)

target_link_libraries(
  ${PROJECT_NAME}Benchmark
  ${PROJECT_NAME}
  benchmark::benchmark
)
