# Copyright dSPACE GmbH. All rights reserved.

add_executable(
  ${PROJECT_NAME}Test
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
  ${PROJECT_NAME}Test
  PRIVATE
  Helpers
)

if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
  target_compile_definitions(
    ${PROJECT_NAME}Test
    PRIVATE
    EXCEPTION_TESTS
  )
endif()

target_link_libraries(
  ${PROJECT_NAME}Test
  ${PROJECT_NAME}
  shared
  GTest::gtest
  GTest::gmock
)
