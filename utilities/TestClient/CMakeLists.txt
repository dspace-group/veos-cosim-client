# Copyright dSPACE GmbH. All rights reserved.

add_executable(
  TestClient
  Program.cpp
)

target_link_libraries(
  TestClient
  ${PROJECT_NAME}
  shared
)
