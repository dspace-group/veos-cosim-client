# Copyright dSPACE SE & Co. KG. All rights reserved.

cmake_minimum_required(VERSION 3.15)

project(DsVeosCoSim VERSION 1.1)

option(BUILD_SHARED_LIBS "Compile dSPACE VEOS CoSim as a shared library" OFF)
option(DSVEOSCOSIM_BUILD_TESTS "Create tests for dSPACE VEOS CoSim" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(DSVEOSCOSIM_TYPE STATIC)

if(MSVC)
  set(
    DSVEOSCOSIM_WARNINGS
    /permissive-
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
    /wd4865
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
    -Wall
    -Wextra
    -Wpedantic
  )
endif()

if(BUILD_SHARED_LIBS)
  set(DSVEOSCOSIM_TYPE SHARED)
endif()

find_package(fmt QUIET)

if(NOT fmt_FOUND)
  message(STATUS "fmt not found, using bundled version")
  add_subdirectory(third_party/fmt EXCLUDE_FROM_ALL)
endif()

add_subdirectory(src)

if(DSVEOSCOSIM_BUILD_TESTS)
  set(INSTALL_GTEST OFF)
  set(gtest_force_shared_crt ON)
  add_subdirectory(third_party/googletest EXCLUDE_FROM_ALL)

  set(BENCHMARK_ENABLE_INSTALL OFF)
  set(BENCHMARK_INSTALL_DOCS OFF)
  set(BENCHMARK_ENABLE_GTEST_TESTS OFF)
  set(BENCHMARK_USE_BUNDLED_GTEST OFF)
  add_subdirectory(third_party/benchmark EXCLUDE_FROM_ALL)

  add_subdirectory(tests)
endif()
