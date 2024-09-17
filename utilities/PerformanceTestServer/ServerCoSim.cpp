// Copyright dSPACE GmbH. All rights reserved.

#include <thread>

#include "CoSimServer.h"
#include "LogHelper.h"
#include "Logger.h"
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
        config.simulationStoppedCallback = [&stopSimulation](DsVeosCoSim_SimulationTime) {
            stopSimulation = true;
        };

        CoSimServer server;
        server.Load(config);

        while (true) {
            DsVeosCoSim_SimulationTime simulationTime{};
            server.Start(simulationTime);

            stopSimulation = false;

            while (!stopSimulation) {
                DsVeosCoSim_SimulationTime nextSimulationTime{};
                server.Step(simulationTime, nextSimulationTime);

                simulationTime++;
            }
        }
    } catch (const std::exception& e) {
        LogError("Error in CoSim server thread: {}", e.what());
    }
}

}  // namespace

void StartCoSimServer() {
    std::thread(CoSimServerRun).detach();
}