# Copyright dSPACE GmbH. All rights reserved.

add_executable(
  PerformanceTestServer
  Program.cpp
  ServerAsioBlocking.cpp
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
  ${PROJECT_NAME}
  shared
  asio
)
