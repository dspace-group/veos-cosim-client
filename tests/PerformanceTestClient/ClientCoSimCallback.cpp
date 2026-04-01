// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <string>

#include "CoSimClient.hpp"
#include "CoSimTypes.hpp"
#include "Logger.hpp"
#include "PerformanceTestClient.hpp"
#include "PerformanceTestHelper.hpp"
#include "Result.hpp"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] Result RunClientCoSimCallbackInternal(const std::string& host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    std::unique_ptr<CoSimClient> coSimClient = CreateClient();
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

void RunClientCoSimCallback(const std::string& host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    Result result = RunClientCoSimCallbackInternal(host, connectedEvent, counter, isStopped);
    if (!IsNotConnected(result)) {
        LogError("Could not run CoSim callback client.");
    }
}

}  // namespace

void ClientCoSimCallback(const std::string& host) {  // NOLINT(misc-use-internal-linkage)
    if (host.empty()) {
        LogTrace("Local dSPACE VEOS CoSim Callback:");
    } else {
        LogTrace("Remote dSPACE VEOS CoSim Callback:");
    }

    RunPerformanceTest(RunClientCoSimCallback, host);
    LogTrace("");
}
