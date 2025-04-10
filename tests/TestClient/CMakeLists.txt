# Copyright dSPACE GmbH. All rights reserved.

add_executable(
  TestClient
)

target_sources(
  TestClient
  PRIVATE
  Program.cpp
)

target_include_directories(
  TestClient
  PRIVATE
  ../../src
)

target_link_libraries(
  TestClient
  PRIVATE
  DsVeosCoSim
  fmt::fmt
  shared
)
