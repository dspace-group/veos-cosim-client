# Copyright dSPACE SE & Co. KG. All rights reserved.

cmake_minimum_required(VERSION 3.19)

project(DsVeosCoSim VERSION 1.1)

option(BUILD_SHARED_LIBS "Compile dSPACE VEOS CoSim as a shared library" ON)
option(DSVEOSCOSIM_BUILD_TESTS "Create tests for dSPACE VEOS CoSim" OFF)
option(DSVEOSCOSIM_SUPPRESS_VENDORED_FMT_WARNINGS "Disable compiler warnings for the vendored fmt target" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(DSVEOSCOSIM_TYPE STATIC)

if(MSVC)
  set(
    DSVEOSCOSIM_WARNINGS
    /permissive-
    /Wall
    /wd4061 # enumerator '<enumerator>' in switch of enum '<enum>' is not explicitly handled by a case label
    /wd4324 # '<structure>': structure was padded due to alignment specifier
    /wd4514 # '<function>': unreferenced inline function has been removed
    /wd4625 # copy constructor was implicitly defined as deleted
    /wd4626 # assignment operator was implicitly defined as deleted
    /wd4820 # '<bytes>' bytes padding added after data member '<data member>'
    /wd5026 # move constructor was implicitly defined as deleted
    /wd5027 # move assignment operator was implicitly defined as deleted
    /wd5045 # Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
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

if(DSVEOSCOSIM_BUILD_TESTS)
  set(DSVEOSCOSIM_TYPE STATIC)
  set(BUILD_SHARED_LIBS OFF)
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

  if(DSVEOSCOSIM_SUPPRESS_VENDORED_FMT_WARNINGS)
    if(MSVC)
      if(TARGET fmt)
        target_compile_options(fmt PRIVATE /W0)
      endif()
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
      if(TARGET fmt)
        target_compile_options(fmt PRIVATE -w)
      endif()
    endif()
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

  add_subdirectory(tests/shared)
  add_subdirectory(tests/TestClient)
  add_subdirectory(tests/TestServer)

  include(CTest)
  add_subdirectory(tests/unit)
endif()
