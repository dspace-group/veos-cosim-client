# Copyright dSPACE GmbH. All rights reserved.

add_executable(
  TestServer
)

target_sources(
  TestServer
  PRIVATE
  Program.cpp
)

target_link_libraries(
  TestServer
  DsVeosCoSim
  shared
)
