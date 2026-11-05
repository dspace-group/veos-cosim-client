# Copyright dSPACE SE & Co. KG. All rights reserved.

cmake_minimum_required(VERSION 3.20)

project(DsVeosCoSim VERSION 1.1.0)

option(BUILD_SHARED_LIBS "Build all libraries (DsVeosCoSim and its dependencies) as shared libraries" OFF)
option(DSVEOSCOSIM_BUILD_SHARED "Build only DsVeosCoSim as a shared library and bundle its dependencies statically" OFF)
option(DSVEOSCOSIM_BUILD_TESTS "Create tests for dSPACE VEOS CoSim" OFF)
option(DSVEOSCOSIM_USE_BUNDLED_FMT "Use third_party/fmt when no fmt target or package is available" ON)
option(DSVEOSCOSIM_SUPPRESS_VENDORED_FMT_WARNINGS "Disable compiler warnings for the vendored fmt target" ON)

set(DSVEOSCOSIM_TYPE STATIC)

if(BUILD_SHARED_LIBS OR DSVEOSCOSIM_BUILD_SHARED)
  set(DSVEOSCOSIM_TYPE SHARED)
endif()

# Bundle mode: DsVeosCoSim is shared while its dependencies stay static and are linked into it.
set(DSVEOSCOSIM_BUNDLE_DEPS OFF)
if(DSVEOSCOSIM_BUILD_SHARED AND NOT BUILD_SHARED_LIBS)
  set(DSVEOSCOSIM_BUNDLE_DEPS ON)
endif()

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/CompilerWarnings.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Fmt.cmake")

add_subdirectory(src)

if(DSVEOSCOSIM_BUILD_TESTS)
  enable_testing()
  include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/GoogleTest.cmake")
  add_subdirectory(tests)
endif()
