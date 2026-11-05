# Copyright dSPACE SE & Co. KG. All rights reserved.

if(TARGET GTest::gmock AND TARGET GTest::gtest)
  return()
endif()

if(TARGET GTest::gmock OR TARGET GTest::gtest)
  message(FATAL_ERROR "DsVeosCoSim tests require both GTest::gmock and GTest::gtest. Provide both targets before including DsVeosCoSim.")
endif()

if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/../third_party/googletest/CMakeLists.txt")
  message(FATAL_ERROR "DsVeosCoSim tests require GoogleTest. Provide GTest::gmock and GTest::gtest or initialize third_party/googletest.")
endif()

set(INSTALL_GTEST OFF)
set(BUILD_GMOCK ON)
set(gtest_force_shared_crt ON)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../third_party/googletest" "${CMAKE_CURRENT_BINARY_DIR}/third_party/googletest" EXCLUDE_FROM_ALL)

if(CMAKE_EXPORT_COMPILE_COMMANDS)
  foreach(target gmock gmock_main gtest gtest_main)
    if(TARGET ${target})
      set_property(TARGET ${target} PROPERTY EXPORT_COMPILE_COMMANDS OFF)
    endif()
  endforeach()
endif()
