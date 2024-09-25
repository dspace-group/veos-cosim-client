# Copyright dSPACE GmbH. All rights reserved.

if(DSVEOSCOSIM_BUILD_TESTS OR DSVEOSCOSIM_BUILD_BENCHMARKS)
  add_subdirectory(fmt)
endif()

if(DSVEOSCOSIM_BUILD_TESTS)
  set(INSTALL_GTEST OFF)
  set(gtest_force_shared_crt ON)
  add_subdirectory(googletest)
endif()

if(DSVEOSCOSIM_BUILD_BENCHMARKS)
add_library(
  asio
  STATIC
  asio/asio/src/asio.cpp
)

target_include_directories(
  asio
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/asio/asio/include>
)

target_compile_definitions(
  asio
  PUBLIC
  -DASIO_STANDALONE
  -DASIO_SEPARATE_COMPILATION
  -DASIO_NO_DEPRECATED
)

if(WIN32 AND CMAKE_SYSTEM_VERSION)
  set(ver ${CMAKE_SYSTEM_VERSION})
  string(REPLACE "." "" ver ${ver})
  string(REGEX REPLACE "([0-9])" "0\\1" ver ${ver})
  set(version "0x${ver}")
  target_compile_definitions(
    asio
    PUBLIC
    -D_WIN32_WINNT=${version}
  )
endif()

if(WIN32)
target_link_libraries(
  asio
  bcrypt
  mswsock
)
endif()

set(BENCHMARK_ENABLE_INSTALL OFF)
set(BENCHMARK_INSTALL_DOCS OFF)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF)
set(BENCHMARK_USE_BUNDLED_GTEST OFF)

add_subdirectory(benchmark)
endif()
