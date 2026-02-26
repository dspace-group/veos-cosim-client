// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <thread>

#include <benchmark/benchmark.h>

#include "CoSimClient.hpp"
#include "CoSimServer.hpp"
#include "Event.hpp"
#include "Helper.hpp"
#include "OsUtilities.hpp"
#include "PerformanceTestHelper.hpp"

using namespace std::chrono;
using namespace DsVeosCoSim;

namespace {

void HandleClient(CoSimClient& coSimClient, ConnectionKind connectionKind, Event& connectedEvent) {
    ConnectConfig connectConfig{};
    connectConfig.clientName = "PerformanceTestClient";
    connectConfig.serverName = CoSimServerName;
    if (connectionKind == ConnectionKind::Remote) {
        connectConfig.remoteIpAddress = "127.0.0.1";
        connectConfig.remotePort = CoSimPort;
    }

    MustBeOk(coSimClient.Connect(connectConfig));

    connectedEvent.Set();

    Callbacks callbacks{};
    MustBeNotConnected(coSimClient.RunCallbackBasedCoSimulation(callbacks));
}

void RunTest(benchmark::State& state, ConnectionKind connectionKind) {
    CoSimServerConfig config;
    config.port = CoSimPort;
    config.enableRemoteAccess = true;
    config.serverName = CoSimServerName;
    config.startPortMapper = false;
    config.registerAtPortMapper = false;

    std::unique_ptr<CoSimServer> server;
    MustBeOk(CreateServer(server));
    MustBeOk(server->Load(config));

    std::unique_ptr<CoSimClient> client;
    MustBeOk(CreateClient(client));

    Event connectedEvent;

    std::thread thread(HandleClient, std::ref(*client), connectionKind, std::ref(connectedEvent));

    SimulationTime simulationTime{};
    MustBeOk(server->Start(simulationTime));

    (void)connectedEvent.Wait(Infinite);

    for (auto _ : state) {
        SimulationTime nextSimulationTime{};
        MustBeOk(server->Step(simulationTime, nextSimulationTime));

        if (nextSimulationTime.nanoseconds > simulationTime.nanoseconds) {
            simulationTime = nextSimulationTime;
        } else {
            simulationTime.nanoseconds++;
        }
    }

    client->Disconnect();
    thread.join();
    client.reset();
}

void Remote(benchmark::State& state) {
    RunTest(state, ConnectionKind::Remote);
}

void Local(benchmark::State& state) {
    RunTest(state, ConnectionKind::Local);
}

}  // namespace

//BENCHMARK(Remote);
BENCHMARK(Local);
