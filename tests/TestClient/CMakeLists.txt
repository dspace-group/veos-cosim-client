# Copyright dSPACE SE & Co. KG. All rights reserved.

add_executable(
  DsVeosCoSimTestClient
)

target_sources(
  DsVeosCoSimTestClient
  PRIVATE
  Program.cpp
)

target_include_directories(
  DsVeosCoSimTestClient
  PRIVATE
  ../../src
)

dsveoscosim_enable_warnings(DsVeosCoSimTestClient)

target_link_libraries(
  DsVeosCoSimTestClient
  PRIVATE
  DsVeosCoSim
  DsVeosCoSimTestSupport
)
