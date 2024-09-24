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
