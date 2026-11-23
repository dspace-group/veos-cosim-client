# Copyright dSPACE SE & Co. KG. All rights reserved.

function(dsveoscosim_enable_warnings target)
  if(MSVC)
    target_compile_options(
      ${target}
      PRIVATE
      /permissive-
      /EHsc
      /W4
      /WX
      $<$<CXX_COMPILER_ID:Clang>:/clang:-Werror=sign-compare>
    )
  else()
    target_compile_options(
      ${target}
      PRIVATE
      -Wall
      -Wextra
      -Wpedantic
      -Werror
      -Werror=sign-compare
    )
  endif()
endfunction()
