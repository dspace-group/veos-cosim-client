# Copyright dSPACE GmbH. All rights reserved.

add_library(
  DsVeosCoSim
  ${DSVEOSCOSIM_TYPE}
)

set_target_properties(
  DsVeosCoSim
  PROPERTIES
  SOVERSION 1
  VERSION 1.1.0
)

target_sources(
  DsVeosCoSim
  PRIVATE
  Communication/LocalChannel.cpp
  Communication/SocketChannel.cpp
  Helpers/CoSimHelper.cpp
  OsAbstraction/Handle.cpp
  OsAbstraction/NamedEvent.cpp
  OsAbstraction/NamedMutex.cpp
  OsAbstraction/OsUtilities.cpp
  OsAbstraction/SharedMemory.cpp
  OsAbstraction/Socket.cpp
  BusBuffer.cpp
  CoSimClient.cpp
  CoSimServer.cpp
  DsVeosCoSim.cpp
  IoBuffer.cpp
  PortMapper.cpp
  Protocol.cpp
)

target_include_directories(
  DsVeosCoSim
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
)

target_include_directories(
  DsVeosCoSim
  PRIVATE
  ./
  Communication/
  Helpers/
  OsAbstraction/
)

target_compile_definitions(
  DsVeosCoSim
  PRIVATE
  DSVEOSCOSIM_EXPORT
)

if(WIN32)
target_compile_definitions(
  DsVeosCoSim
  PRIVATE
  _CRT_SECURE_NO_WARNINGS
  WIN32_LEAN_AND_MEAN
)
endif()

if(WIN32)
target_link_libraries(
  DsVeosCoSim
  WS2_32
)
endif()

install(
  TARGETS DsVeosCoSim
  EXPORT DsVeosCoSimTargets
  DESTINATION lib
)

install(
  FILES ../include/DsVeosCoSim/DsVeosCoSim.h
  DESTINATION include
)
