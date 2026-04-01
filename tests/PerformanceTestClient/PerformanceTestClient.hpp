// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <functional>
#include <string>

#include "Event.hpp"

using PerformanceTestFunc = std::function<void(const std::string& host, DsVeosCoSim::Event& connectedEvent, uint64_t& counter, const bool& isStopped)>;

void RunPerformanceTest(const PerformanceTestFunc& function, const std::string& host);

void ClientCoSimCallback(const std::string& host);
void ClientCoSimPolling(const std::string& host);

#ifdef _WIN32
void ClientEvents();
#endif

void ClientLocalChannel();
void ClientLocalSocket();
void ClientPipe();
void ClientRemoteChannel(const std::string& host);

#ifdef _WIN32
void ClientShmPipe();
#endif

void ClientTcpSocket(const std::string& host);
void ClientUdpSocket(const std::string& host);
