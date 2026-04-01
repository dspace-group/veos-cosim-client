# Copyright dSPACE SE & Co. KG. All rights reserved.

add_executable(
  PerformanceTestServer
)

target_sources(
  PerformanceTestServer
  PRIVATE
  Program.cpp
  ServerCoSim.cpp
  ServerLocalChannel.cpp
  ServerLocalSocket.cpp
  ServerPipe.cpp
  ServerRemoteChannel.cpp
  ServerTcpSocket.cpp
  ServerUdpSocket.cpp
)

if(WIN32)
  target_sources(PerformanceTestServer PRIVATE
    ServerEvents.cpp
    ServerShmPipe.cpp
  )
endif()

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
