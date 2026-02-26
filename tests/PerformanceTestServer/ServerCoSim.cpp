// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <memory>
#include <thread>

#include "CoSimServer.hpp"
#include "Logger.hpp"
#include "PerformanceTestHelper.hpp"
#include "PerformanceTestServer.hpp"
#include "Result.hpp"

namespace DsVeosCoSim {

namespace {

[[nodiscard]] Result Run() {
    LogTrace("dSPACE VEOS CoSim server is listening ...");

    bool stopSimulation = false;

    CoSimServerConfig config;
    config.port = CoSimPort;
    config.enableRemoteAccess = true;
    config.serverName = CoSimServerName;
    config.startPortMapper = false;
    config.registerAtPortMapper = false;
    config.simulationStoppedCallback = [&stopSimulation](SimulationTime) {
        stopSimulation = true;
    };

    std::unique_ptr<CoSimServer> server;
    CheckResult(CreateServer(server));
    CheckResult(server->Load(config));

    while (true) {
        SimulationTime simulationTime{};
        CheckResult(server->Start(simulationTime));

        stopSimulation = false;

        while (!stopSimulation) {
            SimulationTime nextSimulationTime{};
            CheckResult(server->Step(simulationTime, nextSimulationTime));

            ++simulationTime.nanoseconds;
        }
    }

    return CreateOk();
}

void CoSimServerRun() {
    if (!IsOk(Run())) {
        LogError("Could not run CoSim server.");
    }
}

}  // namespace

void StartCoSimServer() {
    std::thread(CoSimServerRun).detach();
}

}  // namespace DsVeosCoSim
