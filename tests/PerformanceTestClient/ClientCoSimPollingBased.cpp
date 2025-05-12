// Copyright dSPACE GmbH. All rights reserved.

#ifdef ALL_COMMUNICATION_TESTS

#include <stdexcept>
#include <string_view>

#include "DsVeosCoSim/CoSimClient.h"
#include "DsVeosCoSim/CoSimTypes.h"
#include "Helper.h"
#include "LogHelper.h"
#include "PerformanceTestHelper.h"
#include "RunPerformanceTest.h"

using namespace DsVeosCoSim;

namespace {

void CoSimClientRun(const std::string_view host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    try {
        std::unique_ptr<CoSimClient> coSimClient = CreateClient();
        ConnectConfig connectConfig{};
        connectConfig.clientName = "PerformanceTestClient";
        connectConfig.serverName = CoSimServerName;
        connectConfig.remoteIpAddress = host;
        if (!host.empty()) {
            connectConfig.remotePort = CoSimPort;
        }

        MUST_BE_TRUE(coSimClient->Connect(connectConfig));

        connectedEvent.Set();

        coSimClient->StartPollingBasedCoSimulation({});

        while (!isStopped) {
            SimulationTime simulationTime{};
            Command command{};
            MUST_BE_TRUE(coSimClient->PollCommand(simulationTime, command, false));

            switch (command) {
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

            MUST_BE_TRUE(coSimClient->FinishCommand());
        }

        coSimClient->Disconnect();
    } catch (const std::exception& e) {
        LogError("Exception in CoSim polling client thread: {}", e.what());
        connectedEvent.Set();
    }
}

}  // namespace

void RunCoSimPollingTest(const std::string_view host) {  // NOLINT
    if (host.empty()) {
        LogTrace("Local dSPACE VEOS CoSim Polling:");
    } else {
        LogTrace("Remote dSPACE VEOS CoSim Polling:");
    }

    RunPerformanceTest(CoSimClientRun, host);
    LogTrace("");
}

#else

#include <string_view>

void RunCoSimPollingTest(const std::string_view host) {  // NOLINT
}

#endif  // ALL_COMMUNICATION_TESTS
