// Copyright dSPACE GmbH. All rights reserved.

#include <stdexcept>
#include <string>
#include <string_view>

#include "CoSimClient.h"
#include "CoSimTypes.h"
#include "Helper.h"
#include "LogHelper.h"
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

        coSimClient.StartPollingBasedCoSimulation({});

        while (!isStopped) {
            DsVeosCoSim_SimulationTime simulationTime{};
            Command command{};
            MUST_BE_TRUE(coSimClient.PollCommand(simulationTime, command, false));

            switch (command) {  // NOLINT(clang-diagnostic-switch-enum)
                case Command::Step:
                    counter++;
                    break;
                case Command::Start:
                case Command::Stop:
                case Command::Terminate:
                case Command::Pause:
                case Command::Continue:
                    break;
                default:
                    throw std::runtime_error("Invalid command.");
            }

            MUST_BE_TRUE(coSimClient.FinishCommand());
        }

        coSimClient.Disconnect();
    } catch (const std::exception& e) {
        LogError("Exception in CoSim polling client thread: {}", e.what());
        connectedEvent.Set();
    }
}

}  // namespace

void RunCoSimPollingTest(std::string_view host) {
    if (host.empty()) {
        LogTrace("Local dSPACE VEOS CoSim Polling:");
    } else {
        LogTrace("Remote dSPACE VEOS CoSim Polling:");
    }

    RunPerformanceTest(CoSimClientRun, host);
    LogTrace("");
}
