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

if(MSVC)
  target_compile_options(PerformanceTestServer PRIVATE /Wall /wd4061 /wd4324 /wd4355 /wd4514 /wd4582 /wd4623 /wd4625 /wd4626 /wd4710 /wd4711 /wd4820 /wd5026 /wd5027 /wd5039 /wd5045 /wd5204 /wd5220)
else()
  target_compile_options(PerformanceTestServer PRIVATE -Wall -Wextra -Wpedantic)
endif()
