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
  PUBLIC
  ../../src
  ../../src/Communication
  ../../src/Helpers
  ../../src/OsAbstraction
)

target_link_libraries(
  TestServer
  DsVeosCoSim
  fmt::fmt
)
