# Copyright dSPACE GmbH. All rights reserved.

add_executable(
  ${PROJECT_NAME}Test
  CoSimServerWrapper.cpp
  Program.cpp
  TestBusBuffer.cpp
  TestCommunication.cpp
  TestCoSim.cpp
  TestHelper.cpp
  TestIoBuffer.cpp
  TestPortMapper.cpp
  TestProtocol.cpp
)

target_include_directories(
  ${PROJECT_NAME}Test
  PUBLIC
  ./
  ../include
  ../src
  ../ThirdParty/googletest/googletest/include
)

target_compile_definitions(${PROJECT_NAME}Test PRIVATE _CRT_SECURE_NO_WARNINGS DSVEOSCOSIM_EXPORT WIN32_LEAN_AND_MEAN)

target_link_libraries(
  ${PROJECT_NAME}Test
  ${PROJECT_NAME}
  GTest::gtest
)
