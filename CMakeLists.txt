# Copyright dSPACE GmbH. All rights reserved.

cmake_minimum_required(VERSION 3.15)

project(DsVeosCoSim VERSION 1.1)

option(BUILD_SHARED_LIBS "Compile dSPACE VEOS CoSim as a shared library" OFF)
option(DSVEOSCOSIM_BUILD_TESTS "Create tests for dSPACE VEOS CoSim" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(DSVEOSCOSIM_TYPE STATIC)

if(BUILD_SHARED_LIBS)
  set(DSVEOSCOSIM_TYPE SHARED)
endif()

add_subdirectory(src)

if(DSVEOSCOSIM_BUILD_TESTS)
  add_subdirectory(tests)
  add_subdirectory(third_party)
endif()
