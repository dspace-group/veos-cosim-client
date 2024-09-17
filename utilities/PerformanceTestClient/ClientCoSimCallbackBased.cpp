// Copyright dSPACE GmbH. All rights reserved.

#include <string>
#include <string_view>

#include "CoSimClient.h"
#include "CoSimTypes.h"
#include "Helper.h"
#include "PerformanceTestHelper.h"
#include "RunPerformanceTest.h"

using namespace DsVeosCoSim;

namespace {

void CoSimClientRun(std::string_view host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    try {
        CoSimClient coSimClient;
        ConnectConfig connectConfig{};
        connectConfig.clientName = "PerformanceTestClient";
        connectConfig.serverName = CoSimServerName;
        connectConfig.remoteIpAddress = host;
        if (!host.empty()) {
            connectConfig.remotePort = CoSimPort;
        }

        MUST_BE_TRUE(coSimClient.Connect(connectConfig));

        connectedEvent.Set();

        Callbacks callbacks{};
        callbacks.simulationEndStepCallback = [&](DsVeosCoSim_SimulationTime) {
            if (isStopped) {
                coSimClient.Disconnect();
            }

            counter++;
        };

        MUST_BE_TRUE(coSimClient.RunCallbackBasedCoSimulation(callbacks));
    } catch (const std::exception& e) {
        LogError("Exception in CoSim callback client thread: {}", e.what());
        connectedEvent.Set();
    }
}

}  // namespace

void RunCoSimCallbackTest(std::string_view host) {
    if (host.empty()) {
        LogTrace("Local dSPACE VEOS CoSim Callback:");
    } else {
        LogTrace("Remote dSPACE VEOS CoSim Callback:");
    }

    RunPerformanceTest(CoSimClientRun, host);
    LogTrace("");
}