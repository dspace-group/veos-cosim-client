# Copyright dSPACE GmbH. All rights reserved.

add_library(
  shared
  STATIC
)

target_sources(
  shared
  PRIVATE
  Generator.cpp
  Helper.cpp
  LogHelper.cpp
  OsAbstractionTestHelper.cpp
)

target_include_directories(
  shared
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/.>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../src>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../src/Communication>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../src/Helpers>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../src/OsAbstraction>
)

if(WIN32)
  target_compile_definitions(
    shared
    PUBLIC
    _CRT_SECURE_NO_WARNINGS
    WIN32_LEAN_AND_MEAN
  )
endif()

target_compile_options(
  shared
  PRIVATE
  ${DSVEOSCOSIM_WARNINGS}
)

target_link_libraries(
  shared
  PUBLIC
  DsVeosCoSim
  fmt::fmt
)
