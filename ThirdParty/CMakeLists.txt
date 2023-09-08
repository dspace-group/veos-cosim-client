# Copyright dSPACE GmbH. All rights reserved.

if(DSVEOSCOSIM_BUILD_TESTS)
  set(BUILD_GMOCK OFF)
  set(INSTALL_GTEST OFF)
  set(gtest_force_shared_crt ON)
  add_subdirectory(googletest)
endif()
