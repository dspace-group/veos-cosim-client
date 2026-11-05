# Copyright dSPACE SE & Co. KG. All rights reserved.

add_library(
  DsVeosCoSimTestSupport
  STATIC
)

target_sources(
  DsVeosCoSimTestSupport
  PRIVATE
  Helper.cpp
)

target_include_directories(
  DsVeosCoSimTestSupport
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/.>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../src>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../src/Communication>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../src/Helpers>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../src/OsAbstraction>
)

if(WIN32)
  target_compile_definitions(
    DsVeosCoSimTestSupport
    PUBLIC
    _CRT_SECURE_NO_WARNINGS
    WIN32_LEAN_AND_MEAN
  )
endif()

dsveoscosim_enable_warnings(DsVeosCoSimTestSupport)

target_link_libraries(
  DsVeosCoSimTestSupport
  PUBLIC
  DsVeosCoSim
  ${DSVEOSCOSIM_FMT_TARGET}
)
