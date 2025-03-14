# Copyright dSPACE GmbH. All rights reserved.

add_subdirectory(fmt EXCLUDE_FROM_ALL)

set(INSTALL_GTEST OFF)
set(BUILD_GMOCK OFF)
set(gtest_force_shared_crt ON)
add_subdirectory(googletest EXCLUDE_FROM_ALL)
