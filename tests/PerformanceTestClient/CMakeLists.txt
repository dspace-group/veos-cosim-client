# Copyright dSPACE SE & Co. KG. All rights reserved.

add_executable(
  PerformanceTestClient
)

target_sources(
  PerformanceTestClient
  PRIVATE
  ClientCoSimCallback.cpp
  ClientCoSimPolling.cpp
  ClientLocalChannel.cpp
  ClientLocalSocket.cpp
  ClientPipe.cpp
  ClientRemoteChannel.cpp
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
