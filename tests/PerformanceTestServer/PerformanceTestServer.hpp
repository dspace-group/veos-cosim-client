// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

void ServerCoSim();
void ServerLocalChannel();
void ServerLocalSocket();
void ServerPipe();
void ServerRemoteChannel();
void ServerTcpSocket();
void ServerUdpSocket();

#ifdef _WIN32
void ServerEvents();
void ServerShmPipe();
#endif
