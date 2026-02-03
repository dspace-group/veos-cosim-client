// Copyright dSPACE SE & Co. KG. All rights reserved.

#if 0

#include <benchmark/benchmark.h>

#include <thread>

#include "DsVeosCoSim/CoSimClient.h"
#include "DsVeosCoSim/CoSimServer.h"
#include "Event.h"
#include "Helper.h"
#include "PerformanceTestHelper.h"

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
    MustBeDisconnected(coSimClient.RunCallbackBasedCoSimulation(callbacks));
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

    MustBeTrue(connectedEvent.Wait(100000000));

    for (auto _ : state) {
        SimulationTime nextSimulationTime{};
        MustBeOk(server->Step(simulationTime, nextSimulationTime));

        ++simulationTime;
    }

    client->Disconnect();
    client.reset();

    thread.join();
}

void Remote(benchmark::State& state) {
    RunTest(state, ConnectionKind::Remote);
}

void Local(benchmark::State& state) {
    RunTest(state, ConnectionKind::Local);
}

}  // namespace

BENCHMARK(Remote);
BENCHMARK(Local);

#endif
