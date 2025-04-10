# Copyright dSPACE GmbH. All rights reserved.

add_executable(
  TestServer
)

target_sources(
  TestServer
  PRIVATE
  Program.cpp
)

target_include_directories(
  TestServer
  PRIVATE
  ../../src
)

target_link_libraries(
  TestServer
  PRIVATE
  DsVeosCoSim
  fmt::fmt
  shared
)
