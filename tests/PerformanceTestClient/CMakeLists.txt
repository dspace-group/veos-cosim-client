# Copyright dSPACE GmbH. All rights reserved.

add_executable(
  PerformanceTestClient
)

target_sources(
  PerformanceTestClient
  PRIVATE
  ClientCoSimCallbackBased.cpp
  ClientCoSimPollingBased.cpp
  ClientEvents.cpp
  ClientLocalCommunication.cpp
  ClientPipe.cpp
  ClientRemoteCommunication.cpp
  ClientTcp.cpp
  ClientUdp.cpp
  ClientUds.cpp
  Program.cpp
  RunPerformanceTest.cpp
)

target_include_directories(
  PerformanceTestClient
  PRIVATE
  ./
)

target_link_libraries(
  PerformanceTestClient
  PRIVATE
  DsVeosCoSim
  shared
)
