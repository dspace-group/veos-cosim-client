// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <functional>
#include <string>

#include "Event.hpp"

namespace DsVeosCoSim {

using PerformanceTestFunc = std::function<void(const std::string& host, Event& connectedEvent, uint64_t& counter, const bool& isStopped)>;

void RunPerformanceTest(const PerformanceTestFunc& function, const std::string& host);

void RunTcpSocketTest(const std::string& host);
void RunUdpSocketTest(const std::string& host);
void RunLocalSocketTest();
void RunPipeTest();
void RunRemoteCommunicationTest(const std::string& host);
void RunLocalCommunicationTest();
void RunCoSimCallbackTest(const std::string& host);
void RunCoSimPollingTest(const std::string& host);

#ifdef _WIN32

void RunEventsTest();
void RunShmPipeTest();

#endif

}  // namespace DsVeosCoSim
