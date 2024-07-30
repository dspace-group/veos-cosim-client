# Copyright dSPACE GmbH. All rights reserved.

if(DSVEOSCOSIM_BUILD_TESTS)
  set(BUILD_GMOCK OFF)
  set(INSTALL_GTEST OFF)
  set(gtest_force_shared_crt ON)
  add_subdirectory(googletest)
endif()

if(DSVEOSCOSIM_BUILD_BENCHMARKS)
  set(BENCHMARK_ENABLE_INSTALL OFF)
  set(BENCHMARK_INSTALL_DOCS OFF)
  set(BENCHMARK_ENABLE_GTEST_TESTS OFF)
  set(BENCHMARK_USE_BUNDLED_GTEST OFF)
  add_subdirectory(benchmark)
endif()
