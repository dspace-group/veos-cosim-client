# Copyright dSPACE SE & Co. KG. All rights reserved.

cmake_minimum_required(VERSION 3.19)

project(DsVeosCoSim VERSION 1.1)

option(BUILD_SHARED_LIBS "Build all libraries (DsVeosCoSim and its dependencies) as shared libraries" OFF)
option(DSVEOSCOSIM_BUILD_SHARED "Build only DsVeosCoSim as a shared library and bundle its dependencies statically" OFF)
option(DSVEOSCOSIM_BUILD_TESTS "Create tests for dSPACE VEOS CoSim" OFF)
option(DSVEOSCOSIM_SUPPRESS_VENDORED_FMT_WARNINGS "Disable compiler warnings for the vendored fmt target" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(DSVEOSCOSIM_TYPE STATIC)

if(MSVC)
  set(
    DSVEOSCOSIM_WARNINGS
    /permissive-
    /EHsc
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
    /wd4868
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

if(BUILD_SHARED_LIBS OR DSVEOSCOSIM_BUILD_SHARED)
  set(DSVEOSCOSIM_TYPE SHARED)
endif()

# Bundle mode: DsVeosCoSim is shared while its dependencies stay static and are linked into it.
set(DSVEOSCOSIM_BUNDLE_DEPS OFF)
if(DSVEOSCOSIM_BUILD_SHARED AND NOT BUILD_SHARED_LIBS)
  set(DSVEOSCOSIM_BUNDLE_DEPS ON)
endif()

if(TARGET fmt::fmt)
  set(DSVEOSCOSIM_FMT_TARGET fmt::fmt)
elseif(TARGET fmt::fmt-header-only)
  set(DSVEOSCOSIM_FMT_TARGET fmt::fmt-header-only)
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/fmt/CMakeLists.txt")
  if(NOT DEFINED FMT_SYSTEM_HEADERS)
    set(FMT_SYSTEM_HEADERS ON CACHE BOOL "Expose vendored fmt headers as system headers")
  endif()

  add_subdirectory(third_party/fmt EXCLUDE_FROM_ALL)

  if(DSVEOSCOSIM_SUPPRESS_VENDORED_FMT_WARNINGS AND TARGET fmt)
    if(MSVC)
      target_compile_options(fmt PRIVATE /W0)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
      target_compile_options(fmt PRIVATE -w)
    endif()
  endif()

  # Static fmt must be position-independent to be linked into the shared DsVeosCoSim,
  # and hidden so its symbols are not re-exported from the bundled library.
  if(DSVEOSCOSIM_BUNDLE_DEPS AND TARGET fmt)
    set_target_properties(
      fmt
      PROPERTIES
      POSITION_INDEPENDENT_CODE ON
      C_VISIBILITY_PRESET hidden
      CXX_VISIBILITY_PRESET hidden
      VISIBILITY_INLINES_HIDDEN ON
    )
  endif()

  if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.20 AND CMAKE_EXPORT_COMPILE_COMMANDS AND TARGET fmt)
    set_property(TARGET fmt PROPERTY EXPORT_COMPILE_COMMANDS OFF)
  endif()

  if(TARGET fmt::fmt)
    set(DSVEOSCOSIM_FMT_TARGET fmt::fmt)
  elseif(TARGET fmt::fmt-header-only)
    set(DSVEOSCOSIM_FMT_TARGET fmt::fmt-header-only)
  endif()
endif()

if(NOT DEFINED DSVEOSCOSIM_FMT_TARGET)
  message(FATAL_ERROR "DsVeosCoSim requires fmt. Provide fmt::fmt from the parent project or include third_party/fmt.")
endif()

add_subdirectory(src)

if(DSVEOSCOSIM_BUILD_TESTS)
  set(INSTALL_GTEST OFF)
  set(BUILD_GMOCK ON)
  set(gtest_force_shared_crt ON)
  add_subdirectory(third_party/googletest EXCLUDE_FROM_ALL)

  if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.20 AND CMAKE_EXPORT_COMPILE_COMMANDS)
    foreach(target gmock gmock_main gtest gtest_main)
      if(TARGET ${target})
        set_property(TARGET ${target} PROPERTY EXPORT_COMPILE_COMMANDS OFF)
      endif()
    endforeach()
  endif()

  add_subdirectory(tests/shared)
  add_subdirectory(tests/TestClient)
  add_subdirectory(tests/TestServer)

  include(CTest)
  add_subdirectory(tests/unit)
endif()
