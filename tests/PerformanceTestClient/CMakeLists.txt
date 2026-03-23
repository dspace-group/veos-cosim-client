# Copyright dSPACE SE & Co. KG. All rights reserved.

add_executable(
  PerformanceTestClient
)

target_sources(
  PerformanceTestClient
  PRIVATE
  ClientCoSimCallbackBased.cpp
  ClientCoSimPollingBased.cpp
  ClientLocalCommunication.cpp
  ClientLocalSocket.cpp
  ClientPipe.cpp
  ClientRemoteCommunication.cpp
  ClientTcpSocket.cpp
  ClientUdpSocket.cpp
  PerformanceTestClient.cpp
  Program.cpp
)

if(WIN32)
  target_sources(
    PerformanceTestClient
    PRIVATE
    ClientEvents.cpp
    ClientShmPipe.cpp
  )
endif()

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

target_link_libraries(
  PerformanceTestClient
  PRIVATE
  DsVeosCoSim
  shared
)
