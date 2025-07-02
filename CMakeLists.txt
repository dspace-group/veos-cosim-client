# Copyright dSPACE GmbH. All rights reserved.

cmake_minimum_required(VERSION 3.15)

project(DsVeosCoSim VERSION 1.1)

option(BUILD_SHARED_LIBS "Compile dSPACE VEOS CoSim as a shared library" OFF)
option(DSVEOSCOSIM_BUILD_TESTS "Create tests for dSPACE VEOS CoSim" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")

set(DSVEOSCOSIM_TYPE STATIC)

if(MSVC)
  set(
    DSVEOSCOSIM_WARNINGS
    /Wall
    /wd4061
    /wd4324
    /wd4355
    /wd4514
    /wd4574
    /wd4582
    /wd4623
    /wd4625
    /wd4626
    /wd4668
    /wd4710
    /wd4711
    /wd4820
    /wd5026
    /wd5027
    /wd5039
    /wd5045
    /wd5204
    /wd5220
  )
else()
  set(
    DSVEOSCOSIM_WARNINGS
    "-Wall
    -Wextra
    -Wpedantic"
  )
endif()

if(BUILD_SHARED_LIBS)
  set(DSVEOSCOSIM_TYPE SHARED)
endif()

add_subdirectory(src)

if(DSVEOSCOSIM_BUILD_TESTS)
  add_subdirectory(tests)
  add_subdirectory(third_party)
endif()

# add_definitions(-DALL_COMMUNICATION_TESTS)
add_definitions(-DALL_BENCHMARK_TESTS)
