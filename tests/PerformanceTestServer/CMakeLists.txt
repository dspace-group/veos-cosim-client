# Copyright dSPACE GmbH. All rights reserved.

add_executable(
  PerformanceTestServer
)

target_sources(
  PerformanceTestServer
  PRIVATE
  Program.cpp
  ServerCoSim.cpp
  ServerEvents.cpp
  ServerLocalCommunication.cpp
  ServerPipe.cpp
  ServerRemoteCommunication.cpp
  ServerTcp.cpp
  ServerUdp.cpp
  ServerUds.cpp
)

target_link_libraries(
  PerformanceTestServer
  PRIVATE
  DsVeosCoSim
  shared
)
