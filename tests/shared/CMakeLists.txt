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

target_link_libraries(
  shared
  PUBLIC
  DsVeosCoSim
  fmt::fmt
)

if(MSVC)
  target_compile_options(shared PRIVATE /Wall /wd4061 /wd4324 /wd4514 /wd4582 /wd4625 /wd4623 /wd4626 /wd4668 /wd4710 /wd4711 /wd4820 /wd5026 /wd5027 /wd5039 /wd5045)
else()
  target_compile_options(shared PRIVATE -Wall -Wextra -Wpedantic)
endif()
