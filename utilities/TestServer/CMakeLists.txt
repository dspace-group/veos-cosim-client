# Copyright dSPACE GmbH. All rights reserved.

add_executable(
  TestServer
  Program.cpp
)

target_link_libraries(
  TestServer
  ${PROJECT_NAME}
  shared
)
