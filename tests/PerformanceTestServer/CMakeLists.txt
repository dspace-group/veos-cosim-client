# Copyright dSPACE SE & Co. KG. All rights reserved.

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

target_compile_options(
  PerformanceTestServer
  PRIVATE
  ${DSVEOSCOSIM_WARNINGS}
)

target_compile_definitions(
  PerformanceTestServer
  PRIVATE
  ALL_COMMUNICATION_TESTS
)

target_link_libraries(
  PerformanceTestServer
  PRIVATE
  DsVeosCoSim
  shared
)
