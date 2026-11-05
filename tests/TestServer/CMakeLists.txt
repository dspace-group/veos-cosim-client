# Copyright dSPACE SE & Co. KG. All rights reserved.

add_executable(
  DsVeosCoSimTestServer
)

target_sources(
  DsVeosCoSimTestServer
  PRIVATE
  Program.cpp
)

target_include_directories(
  DsVeosCoSimTestServer
  PRIVATE
  ../../src
)

dsveoscosim_enable_warnings(DsVeosCoSimTestServer)

target_link_libraries(
  DsVeosCoSimTestServer
  PRIVATE
  DsVeosCoSim
  DsVeosCoSimTestSupport
)
