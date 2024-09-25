# Copyright dSPACE GmbH. All rights reserved.

add_executable(
  DsVeosCoSimTest
)

target_sources(
  DsVeosCoSimTest
  PRIVATE
  Communication/TestLocalChannel.cpp
  Communication/TestTcpChannel.cpp
  Communication/TestUdsChannel.cpp
  OsAbstraction/TestNamedEvent.cpp
  OsAbstraction/TestNamedMutex.cpp
  OsAbstraction/TestSharedMemory.cpp
  OsAbstraction/TestTcpSocket.cpp
  OsAbstraction/TestUdsSocket.cpp
  Helpers/TestHelper.cpp
  Program.cpp
  TestBusBuffer.cpp
  TestCoSim.cpp
  TestIoBuffer.cpp
  TestPortMapper.cpp
  TestProtocol.cpp
)

target_include_directories(
  DsVeosCoSimTest
  PRIVATE
  Helpers
)

if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
  target_compile_definitions(
    DsVeosCoSimTest
    PRIVATE
    EXCEPTION_TESTS
  )
endif()

target_link_libraries(
  DsVeosCoSimTest
  DsVeosCoSim
  shared
  GTest::gtest
  GTest::gmock
)
