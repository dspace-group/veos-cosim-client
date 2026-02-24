// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

namespace DsVeosCoSim {

void StartEventsServer();
void StartLocalSocketServer();
void StartTcpSocketServer();
void StartUdpSocketServer();
void StartPipeServer();
void StartShmPipeServer();
void StartLocalCommunicationServer();
void StartRemoteCommunicationServer();
void StartCoSimServer();

}  // namespace DsVeosCoSim
