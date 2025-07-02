# Copyright dSPACE GmbH. All rights reserved.

add_subdirectory(fmt EXCLUDE_FROM_ALL)

set(INSTALL_GTEST OFF)
set(BUILD_GMOCK OFF)
set(gtest_force_shared_crt ON)
add_subdirectory(googletest EXCLUDE_FROM_ALL)

set(BENCHMARK_ENABLE_INSTALL OFF)
set(BENCHMARK_INSTALL_DOCS OFF)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF)
set(BENCHMARK_USE_BUNDLED_GTEST OFF)
add_subdirectory(benchmark EXCLUDE_FROM_ALL)
