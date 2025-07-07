// Copyright dSPACE GmbH. All rights reserved.

#include "PerformanceTestClient.h"

#ifdef ALL_COMMUNICATION_TESTS

#include <string_view>

#include "CoSimHelper.h"
#include "DsVeosCoSim/CoSimClient.h"
#include "DsVeosCoSim/CoSimTypes.h"
#include "PerformanceTestHelper.h"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] Result Run(std::string_view host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    std::unique_ptr<CoSimClient> coSimClient;
    CheckResult(CreateClient(coSimClient));
    ConnectConfig connectConfig{};
    connectConfig.clientName = "PerformanceTestClient";
    connectConfig.serverName = CoSimServerName;
    connectConfig.remoteIpAddress = host;
    if (!host.empty()) {
        connectConfig.remotePort = CoSimPort;
    }

    CheckResult(coSimClient->Connect(connectConfig));

    connectedEvent.Set();

    CheckResult(coSimClient->StartPollingBasedCoSimulation({}));

    while (!isStopped) {
        SimulationTime simulationTime{};
        Command command{};
        CheckResult(coSimClient->PollCommand(simulationTime, command, false));

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
                LogError("Invalid command.");
                return Result::Error;
        }

        CheckResult(coSimClient->FinishCommand());
    }

    coSimClient->Disconnect();
    return Result::Ok;
}

void CoSimClientRun(std::string_view host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    if (!IsOk(Run(host, connectedEvent, counter, isStopped))) {
        LogError("Could not run CoSim polling client.");
    }
}

}  // namespace

void RunCoSimPollingTest(std::string_view host) {  // NOLINT(misc-use-internal-linkage)
    if (host.empty()) {
        LogTrace("Local dSPACE VEOS CoSim Polling:");
    } else {
        LogTrace("Remote dSPACE VEOS CoSim Polling:");
    }

    RunPerformanceTest(CoSimClientRun, host);
    LogTrace("");
}

#else

void RunCoSimPollingTest([[maybe_unused]] std::string_view host) {  // NOLINT(misc-use-internal-linkage)
}

#endif  // ALL_COMMUNICATION_TESTS
