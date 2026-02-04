# Copyright dSPACE SE & Co. KG. All rights reserved.

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
  ClientLocal.cpp
  PerformanceTestClient.cpp
  Program.cpp
)

target_include_directories(
  PerformanceTestClient
  PRIVATE
  ./
)

target_compile_options(
  PerformanceTestClient
  PRIVATE
  ${DSVEOSCOSIM_WARNINGS}
)

target_compile_definitions(
  PerformanceTestClient
  PRIVATE
  ALL_COMMUNICATION_TESTS
)

target_link_libraries(
  PerformanceTestClient
  PRIVATE
  DsVeosCoSim
  shared
)
