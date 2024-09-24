# Copyright dSPACE GmbH. All rights reserved.

cmake_minimum_required(VERSION 3.13)

project(DsVeosCoSim VERSION 1.0)

option(BUILD_SHARED_LIBS "Compile dSPACE VEOS CoSim as a shared library" OFF)
option(DSVEOSCOSIM_BUILD_TESTS "Create tests for dSPACE VEOS CoSim" OFF)
option(DSVEOSCOSIM_BUILD_BENCHMARKS "Create benchmarks for dSPACE VEOS CoSim" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DWIN32 /D_WINDOWS /Zm1000 /EHsc /GR /W4 /wd4324 /wd4459")
  string(REGEX REPLACE "/W3" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  add_compile_options(/Zi /external:anglebrackets /external:W0)
else()
  add_compile_options(-Wall -Wextra -pedantic)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-stringop-overflow)
  endif()
endif()

set(DSVEOSCOSIM_TYPE STATIC)
if(BUILD_SHARED_LIBS)
  set(DSVEOSCOSIM_TYPE SHARED)
endif()

add_subdirectory(src)
if(DSVEOSCOSIM_BUILD_TESTS OR DSVEOSCOSIM_BUILD_BENCHMARKS)
add_subdirectory(shared)
add_subdirectory(third_party)
add_subdirectory(third_party/fmt)
add_subdirectory(utilities/PerformanceTestClient)
add_subdirectory(utilities/PerformanceTestServer)
add_subdirectory(utilities/TestClient)
add_subdirectory(utilities/TestServer)
if(DSVEOSCOSIM_BUILD_TESTS)
  set(INSTALL_GTEST OFF)
  set(gtest_force_shared_crt ON)
  add_subdirectory(third_party/googletest)
  add_subdirectory(test)
endif()
if(DSVEOSCOSIM_BUILD_BENCHMARKS)
  set(BENCHMARK_ENABLE_INSTALL OFF)
  set(BENCHMARK_INSTALL_DOCS OFF)
  set(BENCHMARK_ENABLE_GTEST_TESTS OFF)
  set(BENCHMARK_USE_BUNDLED_GTEST OFF)
  add_subdirectory(third_party/benchmark)
  add_subdirectory(benchmark)
endif()
endif()

install(EXPORT DsVeosCoSimTargets
  FILE DsVeosCoSimTargets.cmake
  DESTINATION lib/cmake/DsVeosCoSim
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/DsVeosCoSimConfig.cmake"
  INSTALL_DESTINATION "lib/cmake/example"
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/DsVeosCoSimConfigVersion.cmake"
  VERSION "${DsVeosCoSim_VERSION_MAJOR}.${DsVeosCoSim_VERSION_MINOR}"
  COMPATIBILITY AnyNewerVersion
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/DsVeosCoSimConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/DsVeosCoSimConfigVersion.cmake
  DESTINATION lib/cmake/DsVeosCoSim
)
