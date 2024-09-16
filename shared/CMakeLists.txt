# Copyright dSPACE GmbH. All rights reserved.

add_library(
  shared
  STATIC
  BackgroundService.cpp
  ClientServerTestHelper.cpp
  CoSimServerWrapper.cpp
  Generator.cpp
  Helper.cpp
  LogHelper.cpp
  OsAbstractionTestHelper.cpp
)

target_include_directories(
  shared
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/.>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src/Communication>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src/Helpers>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src/OsAbstraction>
)

target_compile_definitions(
  shared
  PUBLIC
  _CRT_SECURE_NO_WARNINGS
  WIN32_LEAN_AND_MEAN
)

target_link_libraries(
  shared
  ${PROJECT_NAME}
  fmt::fmt
)
