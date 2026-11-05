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
  TestBusExchange.cpp
  TestCoSimClient.cpp
  TestDsVeosCoSim.cpp
  TestSignalExchange.cpp
  TestPortMapper.cpp
  TestProtocol.cpp
  TestTypes.cpp
)

target_include_directories(
  DsVeosCoSimTest
  PRIVATE
  Helpers
)

dsveoscosim_enable_warnings(DsVeosCoSimTest)

target_link_libraries(
  DsVeosCoSimTest
  PRIVATE
  DsVeosCoSim
  DsVeosCoSimTestSupport
  GTest::gmock
  GTest::gtest
)

add_test(
  NAME
  DsVeosCoSimTest
  COMMAND
  DsVeosCoSimTest
)
