# Copyright dSPACE GmbH. All rights reserved.

add_library(
  ${PROJECT_NAME}
  ${DSVEOSCOSIM_TYPE}
  BusBuffer.cpp
  Communication.cpp
  CoSimClient.cpp
  DsVeosCoSim.cpp
  IoBuffer.cpp
  Logger.cpp
  PortMapper.cpp
  Protocol.cpp
  Socket.cpp
)

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  $<INSTALL_INTERFACE:include/DsVeosCoSim>
)

target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS DSVEOSCOSIM_EXPORT WIN32_LEAN_AND_MEAN)

install(
  TARGETS ${PROJECT_NAME}
  EXPORT DsVeosCoSimTargets
  DESTINATION lib)
install(FILES ../include/DsVeosCoSim/DsVeosCoSim.h DESTINATION include)
