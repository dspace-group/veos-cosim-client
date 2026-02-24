// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "PerformanceTestClient.hpp"

#include <string>

#include "CoSimClient.hpp"
#include "CoSimTypes.hpp"
#include "Helper.hpp"
#include "PerformanceTestHelper.hpp"

namespace DsVeosCoSim {

namespace {

[[nodiscard]] Result Run(const std::string& host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
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
        CheckResult(coSimClient->PollCommand(simulationTime, command));

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
                return CreateError("Invalid command.");
        }

        CheckResult(coSimClient->FinishCommand());
    }

    coSimClient->Disconnect();
    return CreateOk();
}

void CoSimClientRun(const std::string& host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    if (!IsOk(Run(host, connectedEvent, counter, isStopped))) {
        LogError("Could not run CoSim polling client.");
    }
}

}  // namespace

void RunCoSimPollingTest(const std::string& host) {  // NOLINT(misc-use-internal-linkage)
    if (host.empty()) {
        LogTrace("Local dSPACE VEOS CoSim Polling:");
    } else {
        LogTrace("Remote dSPACE VEOS CoSim Polling:");
    }

    RunPerformanceTest(CoSimClientRun, host);
    LogTrace("");
}

}  // namespace DsVeosCoSim
