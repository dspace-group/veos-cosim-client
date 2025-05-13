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

if(MSVC)
  target_compile_options(PerformanceTestClient PRIVATE /Wall /wd4061 /wd4324 /wd4514 /wd4582 /wd4623 /wd4625 /wd4626 /wd4710 /wd4711 /wd4820 /wd5026 /wd5027 /wd5039 /wd5045)
else()
  target_compile_options(PerformanceTestClient PRIVATE -Wall -Wextra -Wpedantic)
endif()
