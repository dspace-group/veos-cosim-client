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
add_subdirectory(shared)
add_subdirectory(third_party)
endif()

if(DSVEOSCOSIM_BUILD_TESTS)
  add_subdirectory(test)
  add_subdirectory(utilities/TestClient)
  add_subdirectory(utilities/TestServer)
  add_subdirectory(utilities/PerformanceTestClient)
  add_subdirectory(utilities/PerformanceTestServer)
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
