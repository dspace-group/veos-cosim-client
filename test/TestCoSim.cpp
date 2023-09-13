// Copyright dSPACE GmbH. All rights reserved.

#include <chrono>
#include <string>

#include "CoSimClient.h"
#include "CoSimServer.h"
#include "CoSimServerWrapper.h"
#include "Generator.h"
#include "Logger.h"
#include "TestHelper.h"

using namespace std::chrono_literals;
using namespace DsVeosCoSim;

namespace {

struct ConnectConfigContainer {
    ConnectConfig config{};
    std::string serverName;
    std::string clientName;
};

CoSimServerConfig CreateServerConfig(bool isClientOptional = false) {
    CoSimServerConfig config{};
    config.serverName = GenerateString("ServerName");
    config.startPortMapper = true;
    config.isClientOptional = isClientOptional;
    config.logCallback = OnLogCallback;
    return config;
}

void CreateConnectConfig(const std::string& serverName, ConnectConfigContainer& container) {
    container.serverName = serverName;
    container.clientName = GenerateString("ClientName");
    container.config.serverName = container.serverName.c_str();
    container.config.clientName = container.clientName.c_str();
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
    CoSimServerConfig config = CreateServerConfig();

    CoSimServerWrapper server;

    // Act
    const Result result = server.Load(config);

    // Assert
    ASSERT_OK(result);
}

TEST_F(TestCoSim, StartServerWithoutOptionalClient) {
    // Arrange
    CoSimServerConfig config = CreateServerConfig(true);

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
    CoSimServerConfig config = CreateServerConfig(true);

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
    CoSimServerConfig config = CreateServerConfig(true);

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
    CoSimServerConfig config = CreateServerConfig(true);

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
    CoSimServerConfig config = CreateServerConfig(true);

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
    CoSimServerConfig config = CreateServerConfig(true);

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
    ConnectConfigContainer container{};
    CreateConnectConfig(GenerateString("ServerName"), container);

    CoSimClient client;

    // Act
    const Result result = client.Connect(container.config);

    // Assert
    ASSERT_ERROR(result);
}

TEST_F(TestCoSim, ConnectWithoutServerNameAndPort) {
    // Arrange
    ConnectConfigContainer container{};

    CoSimClient client;

    // Act
    const Result result = client.Connect(container.config);

    // Assert
    ASSERT_INVALID_ARGUMENT(result);
}

TEST_F(TestCoSim, ConnectToServer) {
    // Arrange
    CoSimServerConfig config = CreateServerConfig(true);
    ConnectConfigContainer container{};
    CreateConnectConfig(config.serverName, container);

    CoSimServerWrapper server;
    ASSERT_OK(server.Load(config));

    CoSimClient client;

    // Act
    const Result result = client.Connect(container.config);

    // Assert
    ASSERT_OK(result);
}

// Add more tests
