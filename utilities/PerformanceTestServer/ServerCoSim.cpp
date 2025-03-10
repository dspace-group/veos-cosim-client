// Copyright dSPACE GmbH. All rights reserved.

#include <memory>
#include <thread>

#include "CoSimHelper.h"
#include "CoSimServer.h"
#include "LogHelper.h"
#include "PerformanceTestHelper.h"

using namespace DsVeosCoSim;

namespace {

void CoSimServerRun() {
    try {
        LogTrace("dSPACE VEOS CoSim server is listening ...");

        bool stopSimulation = false;

        CoSimServerConfig config;
        config.port = CoSimPort;
        config.enableRemoteAccess = true;
        config.serverName = CoSimServerName;
        config.logCallback = OnLogCallback;
        config.startPortMapper = false;
        config.registerAtPortMapper = false;
        config.simulationStoppedCallback = [&stopSimulation](SimulationTime) {
            stopSimulation = true;
        };

        std::unique_ptr<CoSimServer> server = CreateServer();
        server->Load(config);

        while (true) {
            SimulationTime simulationTime{};
            server->Start(simulationTime);

            stopSimulation = false;

            while (!stopSimulation) {
                SimulationTime nextSimulationTime{};
                server->Step(simulationTime, nextSimulationTime);

                ++simulationTime;
            }
        }
    } catch (const std::exception& e) {
        LogError("Error in CoSim server thread: {}", e.what());
    }
}

}  // namespace

void StartCoSimServer() {  // NOLINT
    std::thread(CoSimServerRun).detach();
}
