// Copyright dSPACE GmbH. All rights reserved.

#include <gtest/gtest.h>

#include <string_view>
#include <thread>

#include "DsVeosCoSim/CoSimClient.h"
#include "DsVeosCoSim/CoSimServer.h"
#include "DsVeosCoSim/CoSimTypes.h"
#include "Event.h"
#include "Generator.h"
#include "LogHelper.h"

using namespace std::chrono;
using namespace DsVeosCoSim;

namespace {

class BackgroundThread final {
public:
    explicit BackgroundThread(CoSimServer& coSimServer) : _coSimServer(coSimServer) {
        _thread = std::thread([this] {
            while (!_stopEvent.Wait(1)) {
                try {
                    _coSimServer.BackgroundService();
                } catch (const std::exception& e) {
                    LogError(e.what());
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

    std::unique_ptr<CoSimServer> server = CreateServer();

    // Act and assert
    ASSERT_NO_THROW(server->Load(config));
}

TEST_F(TestCoSim, StartServerWithoutOptionalClient) {
    // Arrange
    CoSimServerConfig config = CreateServerConfig(true);

    std::unique_ptr<CoSimServer> server = CreateServer();
    server->Load(config);

    SimulationTime simulationTime = GenerateSimulationTime();

    // Act and assert
    ASSERT_NO_THROW(server->Start(simulationTime));
}

TEST_F(TestCoSim, StopServerWithoutOptionalClient) {
    // Arrange
    CoSimServerConfig config = CreateServerConfig(true);

    std::unique_ptr<CoSimServer> server = CreateServer();
    server->Load(config);
    server->Start(GenerateSimulationTime());

    SimulationTime simulationTime = GenerateSimulationTime();

    // Act and assert
    ASSERT_NO_THROW(server->Stop(simulationTime));
}

TEST_F(TestCoSim, PauseServerWithoutOptionalClient) {
    // Arrange
    CoSimServerConfig config = CreateServerConfig(true);

    std::unique_ptr<CoSimServer> server = CreateServer();
    server->Load(config);
    server->Start(GenerateSimulationTime());

    SimulationTime simulationTime = GenerateSimulationTime();

    // Act and assert
    ASSERT_NO_THROW(server->Pause(simulationTime));
}

TEST_F(TestCoSim, ContinueServerWithoutOptionalClient) {
    // Arrange
    CoSimServerConfig config = CreateServerConfig(true);

    std::unique_ptr<CoSimServer> server = CreateServer();
    server->Load(config);
    server->Start(GenerateSimulationTime());
    server->Pause(GenerateSimulationTime());

    SimulationTime simulationTime = GenerateSimulationTime();

    // Act and assert
    ASSERT_NO_THROW(server->Continue(simulationTime));
}

TEST_F(TestCoSim, TerminateServerWithoutOptionalClient) {
    // Arrange
    CoSimServerConfig config = CreateServerConfig(true);

    std::unique_ptr<CoSimServer> server = CreateServer();
    server->Load(config);
    server->Start(GenerateSimulationTime());

    SimulationTime simulationTime = GenerateSimulationTime();
    TerminateReason reason = GenerateRandom(TerminateReason::Finished, TerminateReason::Error);

    // Act and assert
    ASSERT_NO_THROW(server->Terminate(simulationTime, reason));
}

TEST_F(TestCoSim, StepServerWithoutOptionalClient) {
    // Arrange
    CoSimServerConfig config = CreateServerConfig(true);

    std::unique_ptr<CoSimServer> server = CreateServer();
    server->Load(config);
    server->Start(GenerateSimulationTime());

    SimulationTime simulationTime = GenerateSimulationTime();
    SimulationTime nextSimulationTime{};

    // Act and assert
    ASSERT_NO_THROW(nextSimulationTime = server->Step(simulationTime));

    // Assert
    ASSERT_EQ(0s, nextSimulationTime);
}

TEST_P(TestCoSim, ConnectWithoutServer) {
    // Arrange
    ConnectionKind connectionKind = GetParam();

    ConnectConfig connectConfig = CreateConnectConfig(connectionKind, GenerateString("Server名前"), 0);

    std::unique_ptr<CoSimClient> client = CreateClient();

    // Act and assert
    ASSERT_FALSE(client->Connect(connectConfig));
}

TEST_P(TestCoSim, ConnectToServerWithOptionalClient) {
    // Arrange
    ConnectionKind connectionKind = GetParam();

    CoSimServerConfig config = CreateServerConfig(true);

    std::unique_ptr<CoSimServer> server = CreateServer();
    server->Load(config);

    BackgroundThread backgroundThread(*server);

    uint16_t port = server->GetLocalPort();

    ConnectConfig connectConfig = CreateConnectConfig(connectionKind, config.serverName, port);
    std::unique_ptr<CoSimClient> client = CreateClient();

    // Act and assert
    ASSERT_TRUE(client->Connect(connectConfig));
}

TEST_P(TestCoSim, ConnectToServerWithMandatoryClient) {
    // Arrange
    ConnectionKind connectionKind = GetParam();

    CoSimServerConfig config = CreateServerConfig(false);

    std::unique_ptr<CoSimServer> server = CreateServer();
    server->Load(config);

    BackgroundThread backgroundThread(*server);

    uint16_t port = server->GetLocalPort();

    ConnectConfig connectConfig = CreateConnectConfig(connectionKind, config.serverName, port);
    std::unique_ptr<CoSimClient> client = CreateClient();

    // Act and assert
    ASSERT_TRUE(client->Connect(connectConfig));
}

TEST_P(TestCoSim, DisconnectFromServerWithMandatoryClient) {
    // Arrange
    ConnectionKind connectionKind = GetParam();

    Event stoppedEvent;

    CoSimServerConfig config = CreateServerConfig(false);
    config.simulationStoppedCallback = [&](SimulationTime) {
        stoppedEvent.Set();
    };

    std::unique_ptr<CoSimServer> server = CreateServer();
    server->Load(config);

    BackgroundThread backgroundThread(*server);

    uint16_t port = server->GetLocalPort();

    ConnectConfig connectConfig = CreateConnectConfig(connectionKind, config.serverName, port);
    std::unique_ptr<CoSimClient> client = CreateClient();
    ASSERT_TRUE(client->Connect(connectConfig));

    // Act
    client->Disconnect();

    // Assert
    ASSERT_TRUE(stoppedEvent.Wait(1000));
}

// Add more tests

}  // namespace
