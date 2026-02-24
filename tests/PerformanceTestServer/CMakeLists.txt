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
  ServerLocalSocket.cpp
  ServerPipe.cpp
  ServerRemoteCommunication.cpp
  ServerShmPipe.cpp
  ServerTcpSocket.cpp
  ServerUdpSocket.cpp
)

target_compile_options(
  PerformanceTestServer
  PRIVATE
  ${DSVEOSCOSIM_WARNINGS}
)

target_link_libraries(
  PerformanceTestServer
  PRIVATE
  DsVeosCoSim
  shared
)
