# Copyright dSPACE SE & Co. KG. All rights reserved.

add_executable(
  DsVeosCoSimTest
)

target_sources(
  DsVeosCoSimTest
  PRIVATE
  Communication/TestLocalChannel.cpp
  Communication/TestTcpChannel.cpp
  OsAbstraction/TestLocalSocket.cpp
  OsAbstraction/TestNamedEvent.cpp
  OsAbstraction/TestNamedLock.cpp
  OsAbstraction/TestSharedMemory.cpp
  OsAbstraction/TestShmPipe.cpp
  OsAbstraction/TestTcpSocket.cpp
  Helpers/TestHelper.cpp
  Program.cpp
  TestBusBuffer.cpp
  TestCoSim.cpp
  TestIoBuffer.cpp
  TestPortMapper.cpp
  TestProtocol.cpp
  TestTypes.cpp
)

target_include_directories(
  DsVeosCoSimTest
  PRIVATE
  Helpers
)

target_compile_options(
  DsVeosCoSimTest
  PRIVATE
  ${DSVEOSCOSIM_WARNINGS}
)

target_link_libraries(
  DsVeosCoSimTest
  DsVeosCoSim
  shared
  GTest::gmock
  GTest::gtest
)
