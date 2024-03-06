// Copyright dSPACE GmbH. All rights reserved.

#include <string>

#include "CoSimClient.h"
#include "CoSimServer.h"
#include "CoSimServerWrapper.h"
#include "Generator.h"
#include "Logger.h"
#include "TestHelper.h"

using namespace DsVeosCoSim;

namespace {

CoSimServerConfig CreateServerConfig(bool isClientOptional = false) {
    CoSimServerConfig config{};
    config.serverName = GenerateString("ServerName日本語");
    config.startPortMapper = true;
    config.isClientOptional = isClientOptional;
    config.logCallback = OnLogCallback;
    return config;
}

void CreateConnectConfig(const std::string& serverName, ConnectConfig& connectConfig) {
    connectConfig.serverName = serverName;
    connectConfig.clientName = GenerateString("ClientName日本語");
}

}  // namespace

class TestCoSim : public testing::Test {
protected:
    void SetUp() override {
        SetLogCallback(OnLogCallback);
    }
};

TEST_F(TestCoSim, LoadServer) {
    // Arrange
    const CoSimServerConfig config = CreateServerConfig();

    CoSimServerWrapper server;

    // Act
    const Result result = server.Load(config);

    // Assert
    ASSERT_OK(result);
}

TEST_F(TestCoSim, StartServerWithoutOptionalClient) {
    // Arrange
    const CoSimServerConfig config = CreateServerConfig(true);

    CoSimServerWrapper server;
    ASSERT_OK(server.Load(config));

    const SimulationTime simulationTime = GenerateI64();

    // Act
    const Result result = server.Start(simulationTime);

    // Assert
    ASSERT_OK(result);
}

TEST_F(TestCoSim, StopServerWithoutOptionalClient) {
    // Arrange
    const CoSimServerConfig config = CreateServerConfig(true);

    CoSimServerWrapper server;
    ASSERT_OK(server.Load(config));
    ASSERT_OK(server.Start(GenerateI64()));

    const SimulationTime simulationTime = GenerateI64();

    // Act
    const Result result = server.Stop(simulationTime);

    // Assert
    ASSERT_OK(result);
}

TEST_F(TestCoSim, PauseServerWithoutOptionalClient) {
    // Arrange
    const CoSimServerConfig config = CreateServerConfig(true);

    CoSimServerWrapper server;
    ASSERT_OK(server.Load(config));
    ASSERT_OK(server.Start(GenerateI64()));

    const SimulationTime simulationTime = GenerateI64();

    // Act
    const Result result = server.Pause(simulationTime);

    // Assert
    ASSERT_OK(result);
}

TEST_F(TestCoSim, ContinueServerWithoutOptionalClient) {
    // Arrange
    const CoSimServerConfig config = CreateServerConfig(true);

    CoSimServerWrapper server;
    ASSERT_OK(server.Load(config));
    ASSERT_OK(server.Start(GenerateI64()));
    ASSERT_OK(server.Pause(GenerateI64()));

    const SimulationTime simulationTime = GenerateI64();

    // Act
    const Result result = server.Continue(simulationTime);

    // Assert
    ASSERT_OK(result);
}

TEST_F(TestCoSim, TerminateServerWithoutOptionalClient) {
    // Arrange
    const CoSimServerConfig config = CreateServerConfig(true);

    CoSimServerWrapper server;
    ASSERT_OK(server.Load(config));
    ASSERT_OK(server.Start(GenerateI64()));

    const SimulationTime simulationTime = GenerateI64();
    const TerminateReason reason = GenerateRandom(TerminateReason::Finished, TerminateReason::Error);

    // Act
    const Result result = server.Terminate(simulationTime, reason);

    // Assert
    ASSERT_OK(result);
}

TEST_F(TestCoSim, StepServerWithoutOptionalClient) {
    // Arrange
    const CoSimServerConfig config = CreateServerConfig(true);

    CoSimServerWrapper server;
    ASSERT_OK(server.Load(config));
    ASSERT_OK(server.Start(GenerateI64()));

    const SimulationTime simulationTime = GenerateI64();
    SimulationTime nextSimulationTime{};

    // Act
    const Result result = server.Step(simulationTime, nextSimulationTime);

    // Assert
    ASSERT_OK(result);
    ASSERT_EQ(0, nextSimulationTime);
}

TEST_F(TestCoSim, ConnectWithoutServer) {
    // Arrange
    ConnectConfig connectConfig{};
    CreateConnectConfig(GenerateString("ServerName日本語"), connectConfig);

    CoSimClient client;

    // Act
    const Result result = client.Connect(connectConfig);

    // Assert
    ASSERT_ERROR(result);
}

TEST_F(TestCoSim, ConnectWithoutServerNameAndPort) {
    // Arrange
    const ConnectConfig connectConfig{};

    CoSimClient client;

    // Act
    const Result result = client.Connect(connectConfig);

    // Assert
    ASSERT_INVALID_ARGUMENT(result);
}

TEST_F(TestCoSim, ConnectToServer) {
    // Arrange
    const CoSimServerConfig config = CreateServerConfig(true);
    ConnectConfig connectConfig{};
    CreateConnectConfig(config.serverName, connectConfig);

    CoSimServerWrapper server;
    ASSERT_OK(server.Load(config));

    CoSimClient client;

    // Act
    const Result result = client.Connect(connectConfig);

    // Assert
    ASSERT_OK(result);
}

// Add more tests
