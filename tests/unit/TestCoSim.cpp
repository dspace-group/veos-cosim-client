// Copyright dSPACE GmbH. All rights reserved.

#include <gtest/gtest.h>

#include <string_view>
#include <thread>

#include "DsVeosCoSim/CoSimClient.h"
#include "DsVeosCoSim/CoSimServer.h"
#include "DsVeosCoSim/CoSimTypes.h"
#include "Event.h"
#include "Helper.h"
#include "TestHelper.h"

using namespace std::chrono;
using namespace DsVeosCoSim;

namespace {

class BackgroundThread final {
public:
    explicit BackgroundThread(CoSimServer& coSimServer) : _coSimServer(coSimServer) {
        _thread = std::thread([this] {
            while (!_stopEvent.Wait(1)) {
                if (!IsOk(_coSimServer.BackgroundService())) {
                    LogError("Error in background service.");
                    return;
                }
            }
        });
    }

    ~BackgroundThread() noexcept {
        _stopEvent.Set();
        if (std::this_thread::get_id() == _thread.get_id()) {
            _thread.detach();
        } else {
            _thread.join();
        }
    }

    BackgroundThread(const BackgroundThread&) = delete;
    BackgroundThread& operator=(const BackgroundThread&) = delete;

    BackgroundThread(BackgroundThread&&) = delete;
    BackgroundThread& operator=(BackgroundThread&&) = delete;

private:
    CoSimServer& _coSimServer;
    Event _stopEvent;
    std::thread _thread;
};

auto ConnectionKinds = testing::Values(ConnectionKind::Local, ConnectionKind::Remote);

[[nodiscard]] CoSimServerConfig CreateServerConfig(bool isClientOptional) {
    CoSimServerConfig config{};
    config.serverName = GenerateString("Server名前");
    config.startPortMapper = false;
    config.registerAtPortMapper = false;
    config.isClientOptional = isClientOptional;
    return config;
}

[[nodiscard]] ConnectConfig CreateConnectConfig(ConnectionKind connectionKind,
                                                std::string_view serverName,
                                                uint16_t port) {
    ConnectConfig connectConfig{};
    connectConfig.serverName = serverName;
    connectConfig.clientName = GenerateString("Client名前");
    if (connectionKind == ConnectionKind::Remote) {
        connectConfig.remoteIpAddress = "127.0.0.1";
        connectConfig.remotePort = port;
    }

    return connectConfig;
}

class TestCoSim : public testing::TestWithParam<ConnectionKind> {
protected:
    void SetUp() override {
        ClearLastMessage();
    }
};

INSTANTIATE_TEST_SUITE_P(, TestCoSim, ConnectionKinds, [](const testing::TestParamInfo<ConnectionKind>& info) {
    return std::string(ToString(info.param));
});

TEST_F(TestCoSim, LoadServer) {
    // Arrange
    CoSimServerConfig config = CreateServerConfig(false);

    std::unique_ptr<CoSimServer> server;
    ExpectOk(CreateServer(server));

    // Act and assert
    AssertOk(server->Load(config));
}

TEST_F(TestCoSim, StartServerWithoutOptionalClient) {
    // Arrange
    CoSimServerConfig config = CreateServerConfig(true);

    std::unique_ptr<CoSimServer> server;
    ExpectOk(CreateServer(server));
    ExpectOk(server->Load(config));

    SimulationTime simulationTime = GenerateSimulationTime();

    // Act and assert
    AssertOk(server->Start(simulationTime));
}

TEST_F(TestCoSim, StopServerWithoutOptionalClient) {
    // Arrange
    CoSimServerConfig config = CreateServerConfig(true);

    std::unique_ptr<CoSimServer> server;
    ExpectOk(CreateServer(server));
    ExpectOk(server->Load(config));
    ExpectOk(server->Start(GenerateSimulationTime()));

    SimulationTime simulationTime = GenerateSimulationTime();

    // Act and assert
    AssertOk(server->Stop(simulationTime));
}

TEST_F(TestCoSim, PauseServerWithoutOptionalClient) {
    // Arrange
    CoSimServerConfig config = CreateServerConfig(true);

    std::unique_ptr<CoSimServer> server;
    ExpectOk(CreateServer(server));
    ExpectOk(server->Load(config));
    ExpectOk(server->Start(GenerateSimulationTime()));

    SimulationTime simulationTime = GenerateSimulationTime();

    // Act and assert
    AssertOk(server->Pause(simulationTime));
}

TEST_F(TestCoSim, ContinueServerWithoutOptionalClient) {
    // Arrange
    CoSimServerConfig config = CreateServerConfig(true);

    std::unique_ptr<CoSimServer> server;
    ExpectOk(CreateServer(server));
    ExpectOk(server->Load(config));
    ExpectOk(server->Start(GenerateSimulationTime()));
    ExpectOk(server->Pause(GenerateSimulationTime()));

    SimulationTime simulationTime = GenerateSimulationTime();

    // Act and assert
    AssertOk(server->Continue(simulationTime));
}

TEST_F(TestCoSim, TerminateServerWithoutOptionalClient) {
    // Arrange
    CoSimServerConfig config = CreateServerConfig(true);

    std::unique_ptr<CoSimServer> server;
    ExpectOk(CreateServer(server));
    ExpectOk(server->Load(config));
    ExpectOk(server->Start(GenerateSimulationTime()));

    SimulationTime simulationTime = GenerateSimulationTime();
    TerminateReason reason = GenerateRandom(TerminateReason::Finished, TerminateReason::Error);

    // Act and assert
    AssertOk(server->Terminate(simulationTime, reason));
}

TEST_F(TestCoSim, StepServerWithoutOptionalClient) {
    // Arrange
    CoSimServerConfig config = CreateServerConfig(true);

    std::unique_ptr<CoSimServer> server;
    ExpectOk(CreateServer(server));
    ExpectOk(server->Load(config));
    ExpectOk(server->Start(GenerateSimulationTime()));

    SimulationTime simulationTime = GenerateSimulationTime();
    SimulationTime nextSimulationTime{};

    // Act
    AssertOk(server->Step(simulationTime, nextSimulationTime));

    // Assert
    AssertEq(0ns, nextSimulationTime);
}

TEST_P(TestCoSim, ConnectWithoutServer) {
    // Arrange
    ConnectionKind connectionKind = GetParam();

    ConnectConfig connectConfig = CreateConnectConfig(connectionKind, GenerateString("Server名前"), 0);

    std::unique_ptr<CoSimClient> client;
    ExpectOk(CreateClient(client));

    // Act and assert
    AssertNotOk(client->Connect(connectConfig));
}

TEST_P(TestCoSim, ConnectToServerWithOptionalClient) {
    // Arrange
    ConnectionKind connectionKind = GetParam();

    CoSimServerConfig config = CreateServerConfig(true);

    std::unique_ptr<CoSimServer> server;
    ExpectOk(CreateServer(server));
    ExpectOk(server->Load(config));

    BackgroundThread backgroundThread(*server);

    uint16_t port{};
    ExpectOk(server->GetLocalPort(port));

    ConnectConfig connectConfig = CreateConnectConfig(connectionKind, config.serverName, port);
    std::unique_ptr<CoSimClient> client;
    ExpectOk(CreateClient(client));

    // Act and assert
    AssertOk(client->Connect(connectConfig));
}

TEST_P(TestCoSim, ConnectToServerWithMandatoryClient) {
    // Arrange
    ConnectionKind connectionKind = GetParam();

    CoSimServerConfig config = CreateServerConfig(false);

    std::unique_ptr<CoSimServer> server;
    ExpectOk(CreateServer(server));
    ExpectOk(server->Load(config));

    BackgroundThread backgroundThread(*server);

    uint16_t port{};
    ExpectOk(server->GetLocalPort(port));

    ConnectConfig connectConfig = CreateConnectConfig(connectionKind, config.serverName, port);
    std::unique_ptr<CoSimClient> client;
    ExpectOk(CreateClient(client));

    // Act and assert
    AssertOk(client->Connect(connectConfig));
}

TEST_P(TestCoSim, DisconnectFromServerWithMandatoryClient) {
    // Arrange
    ConnectionKind connectionKind = GetParam();

    Event stoppedEvent;

    CoSimServerConfig config = CreateServerConfig(false);
    config.simulationStoppedCallback = [&](SimulationTime) {
        stoppedEvent.Set();
    };

    std::unique_ptr<CoSimServer> server;
    ExpectOk(CreateServer(server));
    ExpectOk(server->Load(config));

    BackgroundThread backgroundThread(*server);

    uint16_t port{};
    ExpectOk(server->GetLocalPort(port));

    ConnectConfig connectConfig = CreateConnectConfig(connectionKind, config.serverName, port);
    std::unique_ptr<CoSimClient> client;
    ExpectOk(CreateClient(client));
    AssertOk(client->Connect(connectConfig));

    // Act
    client->Disconnect();

    // Assert
    AssertTrue(stoppedEvent.Wait(1000));
}

// Add more tests

}  // namespace
