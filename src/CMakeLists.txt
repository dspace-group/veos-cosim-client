# Copyright dSPACE GmbH. All rights reserved.

add_library(
  ${PROJECT_NAME}
  ${DSVEOSCOSIM_TYPE}
  Communication/LocalChannel.cpp
  Communication/SocketChannel.cpp
  Helpers/CoSimHelper.cpp
  OsAbstraction/Handle.cpp
  OsAbstraction/NamedEvent.cpp
  OsAbstraction/NamedMutex.cpp
  OsAbstraction/SharedMemory.cpp
  OsAbstraction/Socket.cpp
  OsAbstraction/OsUtilities.cpp
  BusBuffer.cpp
  CoSimClient.cpp
  CoSimServer.cpp
  DsVeosCoSim.cpp
  IoBuffer.cpp
  PortMapper.cpp
  Protocol.cpp
)

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
)

target_include_directories(
  ${PROJECT_NAME}
  PRIVATE
  ./
  Communication/
  Helpers/
  OsAbstraction/
)

target_compile_definitions(
  ${PROJECT_NAME}
  PRIVATE
  _CRT_SECURE_NO_WARNINGS
  DSVEOSCOSIM_EXPORT
  WIN32_LEAN_AND_MEAN
)

target_link_libraries(
  ${PROJECT_NAME}
  fmt::fmt
)

install(
  TARGETS ${PROJECT_NAME}
  EXPORT DsVeosCoSimTargets
  DESTINATION lib)
install(FILES ../include/DsVeosCoSim/DsVeosCoSim.h DESTINATION include)
