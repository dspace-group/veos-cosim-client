# Copyright dSPACE SE & Co. KG. All rights reserved.

if(NOT TARGET fmt::fmt AND NOT TARGET fmt::fmt-header-only)
  find_package(fmt CONFIG QUIET)
endif()

if(NOT TARGET fmt::fmt AND NOT TARGET fmt::fmt-header-only
   AND DSVEOSCOSIM_USE_BUNDLED_FMT AND EXISTS "${CMAKE_CURRENT_LIST_DIR}/../third_party/fmt/CMakeLists.txt")
  if(NOT DEFINED FMT_SYSTEM_HEADERS)
    set(FMT_SYSTEM_HEADERS ON CACHE BOOL "Expose vendored fmt headers as system headers")
  endif()

  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../third_party/fmt" "${CMAKE_CURRENT_BINARY_DIR}/third_party/fmt" EXCLUDE_FROM_ALL)

  if(DSVEOSCOSIM_SUPPRESS_VENDORED_FMT_WARNINGS AND TARGET fmt)
    if(MSVC)
      target_compile_options(fmt PRIVATE /W0)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
      target_compile_options(fmt PRIVATE -w)
    endif()
  endif()

  # Static fmt must be position-independent to be linked into the shared DsVeosCoSim,
  # and hidden so its symbols are not re-exported from the bundled library.
  if(DSVEOSCOSIM_BUNDLE_DEPS AND TARGET fmt)
    set_target_properties(
      fmt
      PROPERTIES
      POSITION_INDEPENDENT_CODE ON
      C_VISIBILITY_PRESET hidden
      CXX_VISIBILITY_PRESET hidden
      VISIBILITY_INLINES_HIDDEN ON
    )
  endif()

  if(CMAKE_EXPORT_COMPILE_COMMANDS AND TARGET fmt)
    set_property(TARGET fmt PROPERTY EXPORT_COMPILE_COMMANDS OFF)
  endif()
endif()

if(TARGET fmt::fmt)
  set(DSVEOSCOSIM_FMT_TARGET fmt::fmt)
elseif(TARGET fmt::fmt-header-only)
  set(DSVEOSCOSIM_FMT_TARGET fmt::fmt-header-only)
else()
  message(FATAL_ERROR "DsVeosCoSim requires fmt. Provide fmt::fmt or fmt::fmt-header-only, install a fmt CMake package, or initialize third_party/fmt and enable DSVEOSCOSIM_USE_BUNDLED_FMT.")
endif()
