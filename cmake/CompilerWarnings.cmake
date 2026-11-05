# Copyright dSPACE SE & Co. KG. All rights reserved.

function(dsveoscosim_enable_warnings target)
  if(MSVC)
    target_compile_options(
      ${target}
      PRIVATE
      /permissive-
      /EHsc
      /W4
    )
  else()
    target_compile_options(
      ${target}
      PRIVATE
      -Wall
      -Wextra
      -Wpedantic
    )
  endif()
endfunction()
