# Copyright dSPACE GmbH. All rights reserved.

add_executable(
  TestClient
)

target_sources(
  TestClient
  PRIVATE
  Program.cpp
)

target_link_libraries(
  TestClient
  DsVeosCoSim
  shared
)
