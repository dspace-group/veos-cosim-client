# Copyright dSPACE GmbH. All rights reserved.

if(DSVEOSCOSIM_BUILD_TESTS)
  add_subdirectory(fmt)
endif()

if(DSVEOSCOSIM_BUILD_TESTS)
  set(INSTALL_GTEST OFF)
  set(gtest_force_shared_crt ON)
  add_subdirectory(googletest)
endif()
