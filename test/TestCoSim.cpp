// Copyright dSPACE GmbH. All rights reserved.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string_view>  // IWYU pragma: keep

#include "CoSimClient.h"
#include "CoSimServer.h"
#include "CoSimTypes.h"
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

[[nodiscard]] CoSimServerConfig CreateServerConfig(const bool isClientOptional = false) {
    CoSimServerConfig config{};
    config.serverName = GenerateString("Server名前");
    config.startPortMapper = false;
    config.registerAtPortMapper = false;
    config.isClientOptional = isClientOptional;
    config.logCallback = OnLogCallback;
    return config;
}

[[nodiscard]] ConnectConfig CreateConnectConfig(const ConnectionKind connectionKind,
                                                const std::string_view serverName,
                                                const uint16_t port = 0) {
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
    return ToString(info.param);
});

TEST_F(TestCoSim, LoadServer) {
    // Arrange
    const CoSimServerConfig config = CreateServerConfig();

    CoSimServer server;

    // Act and assert
    ASSERT_NO_THROW(server.Load(config));
}

TEST_F(TestCoSim, StartServerWithoutOptionalClient) {
    // Arrange
    const CoSimServerConfig config = CreateServerConfig(true);

    CoSimServer server;
    server.Load(config);

    const SimulationTime simulationTime = GenerateSimulationTime();

    // Act and assert
    ASSERT_NO_THROW(server.Start(simulationTime));
}

TEST_F(TestCoSim, StopServerWithoutOptionalClient) {
    // Arrange
    const CoSimServerConfig config = CreateServerConfig(true);

    CoSimServer server;
    server.Load(config);
    server.Start(GenerateSimulationTime());

    const SimulationTime simulationTime = GenerateSimulationTime();

    // Act and assert
    ASSERT_NO_THROW(server.Stop(simulationTime));
}

TEST_F(TestCoSim, PauseServerWithoutOptionalClient) {
    // Arrange
    const CoSimServerConfig config = CreateServerConfig(true);

    CoSimServer server;
    server.Load(config);
    server.Start(GenerateSimulationTime());

    const SimulationTime simulationTime = GenerateSimulationTime();

    // Act and assert
    ASSERT_NO_THROW(server.Pause(simulationTime));
}

TEST_F(TestCoSim, ContinueServerWithoutOptionalClient) {
    // Arrange
    const CoSimServerConfig config = CreateServerConfig(true);

    CoSimServer server;
    server.Load(config);
    server.Start(GenerateSimulationTime());
    server.Pause(GenerateSimulationTime());

    const SimulationTime simulationTime = GenerateSimulationTime();

    // Act and assert
    ASSERT_NO_THROW(server.Continue(simulationTime));
}

TEST_F(TestCoSim, TerminateServerWithoutOptionalClient) {
    // Arrange
    const CoSimServerConfig config = CreateServerConfig(true);

    CoSimServer server;
    server.Load(config);
    server.Start(GenerateSimulationTime());

    const SimulationTime simulationTime = GenerateSimulationTime();
    const TerminateReason reason = GenerateRandom(TerminateReason::Finished, TerminateReason::Error);

    // Act and assert
    ASSERT_NO_THROW(server.Terminate(simulationTime, reason));
}

TEST_F(TestCoSim, StepServerWithoutOptionalClient) {
    // Arrange
    const CoSimServerConfig config = CreateServerConfig(true);

    CoSimServer server;
    server.Load(config);
    server.Start(GenerateSimulationTime());

    const SimulationTime simulationTime = GenerateSimulationTime();
    SimulationTime nextSimulationTime{};

    // Act and assert
    ASSERT_NO_THROW(server.Step(simulationTime, nextSimulationTime));

    // Assert
    ASSERT_EQ(0s, nextSimulationTime);
}

TEST_P(TestCoSim, ConnectWithoutServer) {
    // Arrange
    const ConnectionKind connectionKind = GetParam();

    const ConnectConfig connectConfig = CreateConnectConfig(connectionKind, GenerateString("Server名前"));

    CoSimClient client;

    // Act and assert
    ASSERT_FALSE(client.Connect(connectConfig));
}

#ifdef EXCEPTION_TESTS
TEST_P(TestCoSim, ConnectWithoutServerNameAndPort) {
    // Arrange
    const ConnectionKind connectionKind = GetParam();

    const ConnectConfig connectConfig = CreateConnectConfig(connectionKind, "");

    CoSimClient client;

    // Act and assert
    ASSERT_THROW((void)client.Connect(connectConfig), std::runtime_error);
}
#endif

TEST_P(TestCoSim, ConnectToServerWithOptionalClient) {
    // Arrange
    const ConnectionKind connectionKind = GetParam();

    const CoSimServerConfig config = CreateServerConfig(true);

    CoSimServer server;
    server.Load(config);

    BackgroundThread backgroundThread(server);  // NOLINT

    const uint16_t port = server.GetLocalPort();

    const ConnectConfig connectConfig = CreateConnectConfig(connectionKind, config.serverName, port);
    CoSimClient client;

    // Act and assert
    ASSERT_TRUE(client.Connect(connectConfig));
}

TEST_P(TestCoSim, ConnectToServerWithMandatoryClient) {
    // Arrange
    const ConnectionKind connectionKind = GetParam();

    const CoSimServerConfig config = CreateServerConfig();

    CoSimServer server;
    server.Load(config);

    BackgroundThread backgroundThread(server);  // NOLINT

    const uint16_t port = server.GetLocalPort();

    const ConnectConfig connectConfig = CreateConnectConfig(connectionKind, config.serverName, port);
    CoSimClient client;

    // Act and assert
    ASSERT_TRUE(client.Connect(connectConfig));
}

TEST_P(TestCoSim, DisconnectFromServerWithMandatoryClient) {
    // Arrange
    const ConnectionKind connectionKind = GetParam();

    Event stoppedEvent;

    CoSimServerConfig config = CreateServerConfig();
    config.simulationStoppedCallback = [&](SimulationTime) {
        stoppedEvent.Set();
    };

    CoSimServer server;
    server.Load(config);

    BackgroundThread backgroundThread(server);  // NOLINT

    const uint16_t port = server.GetLocalPort();

    const ConnectConfig connectConfig = CreateConnectConfig(connectionKind, config.serverName, port);
    CoSimClient client;
    ASSERT_TRUE(client.Connect(connectConfig));

    // Act
    client.Disconnect();

    // Assert
    ASSERT_TRUE(stoppedEvent.Wait(1000));
}

// Add more tests

}  // namespace
