// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <functional>
#include <string>

#include "Event.h"

using PerformanceTestFunc = std::function<void(const std::string& host, DsVeosCoSim::Event& connectedEvent, uint64_t& counter, const bool& isStopped)>;

void RunPerformanceTest(const PerformanceTestFunc& function, const std::string& host);

void RunTcpTest(const std::string& host);
void RunUdpTest(const std::string& host);
void RunUdsTest();
void RunPipeTest();
void RunEventsTest();
void RunRemoteCommunicationTest(const std::string& host);
void RunLocalCommunicationTest();
void RunCoSimCallbackTest(const std::string& host);
void RunCoSimPollingTest(const std::string& host);
