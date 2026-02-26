// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

namespace DsVeosCoSim {

void StartLocalSocketServer();
void StartTcpSocketServer();
void StartUdpSocketServer();
void StartPipeServer();
void StartLocalCommunicationServer();
void StartRemoteCommunicationServer();
void StartCoSimServer();

#ifdef _WIN32

void StartEventsServer();
void StartShmPipeServer();

#endif

}  // namespace DsVeosCoSim
