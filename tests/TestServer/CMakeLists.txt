# Copyright dSPACE GmbH. All rights reserved.

add_executable(
  TestServer
)

target_sources(
  TestServer
  PRIVATE
  Program.cpp
)

target_include_directories(
  TestServer
  PRIVATE
  ../../src
)

target_link_libraries(
  TestServer
  PRIVATE
  DsVeosCoSim
  fmt::fmt
  shared
)

if(MSVC)
  target_compile_options(TestServer PRIVATE /Wall /wd4061 /wd4324 /wd4514 /wd4582 /wd4623 /wd4625 /wd4626 /wd4710 /wd4711 /wd4820 /wd5026 /wd5027 /wd5039 /wd5045)
else()
  target_compile_options(TestServer PRIVATE -Wall -Wextra -Wpedantic)
endif()
