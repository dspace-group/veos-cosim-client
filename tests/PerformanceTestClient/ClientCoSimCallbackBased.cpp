// Copyright dSPACE GmbH. All rights reserved.

#include <string_view>

#include "CoSimHelper.h"
#include "DsVeosCoSim/CoSimClient.h"
#include "DsVeosCoSim/CoSimTypes.h"
#include "PerformanceTestClient.h"
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

    Callbacks callbacks{};
    callbacks.simulationEndStepCallback = [&](SimulationTime) {
        if (isStopped) {
            coSimClient->Disconnect();
        }

        counter++;
    };

    return coSimClient->RunCallbackBasedCoSimulation(callbacks);
}

void CoSimClientRun(std::string_view host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    if (!IsDisconnected(Run(host, connectedEvent, counter, isStopped))) {
        LogError("Could not run CoSim callback client.");
    }
}

}  // namespace

void RunCoSimCallbackTest(std::string_view host) {  // NOLINT(misc-use-internal-linkage)
    if (host.empty()) {
        LogTrace("Local dSPACE VEOS CoSim Callback:");
    } else {
        LogTrace("Remote dSPACE VEOS CoSim Callback:");
    }

    RunPerformanceTest(CoSimClientRun, host);
    LogTrace("");
}
