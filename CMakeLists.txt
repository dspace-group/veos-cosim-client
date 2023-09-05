# Copyright dSPACE GmbH. All rights reserved.

cmake_minimum_required(VERSION 3.13)

project(DsVeosCoSim VERSION 1.0)

option(BUILD_SHARED_LIBS "Compile dSPACE VEOS CoSim as a shared library" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

if(MSVC)
  add_compile_options(/Zi)
else()
  add_compile_options(-Wall -Wextra -pedantic)
endif()

set(DSVEOSCOSIM_TYPE STATIC)
if(BUILD_SHARED_LIBS)
  set(DSVEOSCOSIM_TYPE SHARED)
endif()

add_subdirectory(src)

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
