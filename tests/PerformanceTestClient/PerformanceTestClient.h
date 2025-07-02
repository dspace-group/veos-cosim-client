// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <functional>
#include <string_view>

#include "Event.h"

using PerformanceTestFunc = std::function<
    void(std::string_view host, DsVeosCoSim::Event& connectedEvent, uint64_t& counter, const bool& isStopped)>;

void RunPerformanceTest(const PerformanceTestFunc& function, std::string_view host);

void RunTcpTest(std::string_view host);
void RunUdpTest(std::string_view host);
void RunUdsTest();
void RunPipeTest();
void RunEventsTest();
void RunRemoteCommunicationTest(std::string_view host);
void RunLocalCommunicationTest();
void RunCoSimCallbackTest(std::string_view host);
void RunCoSimPollingTest(std::string_view host);
