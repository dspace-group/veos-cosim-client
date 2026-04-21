// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <chrono>
#include <cstring>
#include <future>
#include <memory>
#include <string>
#include <thread>

#include <fmt/format.h>

#include <gtest/gtest.h>

#include "Channel.hpp"
#include "CoSimClient.hpp"
#include "CoSimServer.hpp"
#include "CoSimTypes.hpp"
#include "Helper.hpp"
#include "OsUtilities.hpp"
#include "Protocol.hpp"
#include "TestHelper.hpp"

using namespace DsVeosCoSim;

namespace {

class TestCoSimClient : public testing::TestWithParam<ConnectionKind> {
protected:
    void CustomSetUp(ConnectionKind connectionKind) {
        _serverName = GenerateString("CoSimServer名前");

        if (connectionKind == ConnectionKind::Remote) {
            AssertOk(CreateTcpChannelServer(0, true, _server));
            _serverPort = _server->GetLocalPort();
        } else {
            AssertOk(CreateLocalChannelServer(_serverName, _server));
        }

        AssertOk(CreateProtocol(ProtocolVersion1, _serverProtocol));
        _client = std::make_unique<CoSimClient>();
    }

    [[nodiscard]] ConnectConfig MakeConfig(ConnectionKind connectionKind) const {
        ConnectConfig config;
        config.serverName = _serverName;
        config.clientName = "TestClient";
        if (connectionKind == ConnectionKind::Remote) {
            config.remoteIpAddress = "127.0.0.1";
            config.remotePort = _serverPort;
        }

        return config;
    }

    void ConnectAndStartPolling(ConnectionKind connectionKind, CoSimServerConfig serverConfig = {}) {
        _serverName = GenerateString("CoSimServer名前");
        serverConfig.serverName = _serverName;
        serverConfig.enableRemoteAccess = (connectionKind == ConnectionKind::Remote);
        serverConfig.port = 0;
        serverConfig.isClientOptional = false;
        serverConfig.registerAtPortMapper = false;

        _coSimServer = std::make_unique<CoSimServer>();
        AssertOk(_coSimServer->Load(serverConfig));
        uint16_t port{};
        AssertOk(_coSimServer->GetLocalPort(port));
        _serverPort = port;

        _client = std::make_unique<CoSimClient>();

        auto serverStartTask = std::async(std::launch::async, [this] {
            return _coSimServer->Start(SimulationTime{});
        });

        AssertOk(_client->Connect(MakeConfig(connectionKind)));
        AssertOk(_client->StartPollingBasedCoSimulation({}));

        SimulationTime simulationTime{};
        Command command{};
        AssertOk(_client->PollCommand(simulationTime, command, Infinite));
        AssertOk(_client->FinishCommand());
        AssertOk(serverStartTask.get());
    }

    template <typename ServerFunction>
    [[nodiscard]] std::future<Result> RunServer(const ServerFunction& serverFunction) {
        return std::async(std::launch::async, [this, serverFunction]() {
            std::unique_ptr<Channel> channel;
            CheckResult(WaitForAccept(channel));
            return serverFunction(*channel, *_serverProtocol);
        });
    }

    void ConnectClientAndServer(ConnectionKind connectionKind) {
        CustomSetUp(connectionKind);
        auto serverTask = std::async(std::launch::async, [this]() {
            CheckResult(WaitForAccept(_serverChannel));
            return AcceptAndSendConnectOk(*_serverChannel, *_serverProtocol);
        });
        AssertOk(_client->Connect(MakeConfig(connectionKind)));
        AssertOk(serverTask.get());
    }

    [[nodiscard]] static Result AcceptAndSendConnectOk(Channel& channel, IProtocol& protocol) {
        return AcceptAndSendConnectOk(channel, protocol, ProtocolVersion1, SimulationTime{}, SimulationState::Stopped);
    }

    [[nodiscard]] static Result AcceptAndSendConnectOk(Channel& channel, IProtocol& protocol, SimulationTime stepSize, SimulationState simulationState) {
        return AcceptAndSendConnectOk(channel, protocol, ProtocolVersion1, stepSize, simulationState, {}, {}, {}, {}, {}, {});
    }

    [[nodiscard]] static Result AcceptAndSendConnectOk(Channel& channel,
                                                       IProtocol& protocol,
                                                       uint32_t protocolVersion,
                                                       SimulationTime stepSize,
                                                       SimulationState simulationState,
                                                       const std::vector<IoSignalContainer>& incomingSignals = {},
                                                       const std::vector<IoSignalContainer>& outgoingSignals = {},
                                                       const std::vector<CanControllerContainer>& canControllers = {},
                                                       const std::vector<EthControllerContainer>& ethControllers = {},
                                                       const std::vector<LinControllerContainer>& linControllers = {},
                                                       const std::vector<FrControllerContainer>& frControllers = {}) {
        FrameKind frameKind{};
        CheckResult(protocol.ReceiveHeader(channel.GetReader(), frameKind));
        if (frameKind != FrameKind::Connect) {
            return CreateError();
        }

        uint32_t version{};
        Mode mode{};
        std::string serverName;
        std::string clientName;
        CheckResult(protocol.ReadConnect(channel.GetReader(), version, mode, serverName, clientName));

        CheckResult(protocol.SendConnectOk(channel.GetWriter(),
                                           protocolVersion,
                                           {},
                                           stepSize,
                                           simulationState,
                                           incomingSignals,
                                           outgoingSignals,
                                           canControllers,
                                           ethControllers,
                                           linControllers,
                                           frControllers));
        return CreateOk();
    }

    [[nodiscard]] static Result AcceptAndSendError(Channel& channel, IProtocol& protocol, std::string_view errorMessage) {
        FrameKind frameKind{};
        CheckResult(protocol.ReceiveHeader(channel.GetReader(), frameKind));
        if (frameKind != FrameKind::Connect) {
            return CreateError();
        }

        uint32_t version{};
        Mode mode{};
        std::string serverName;
        std::string clientName;
        CheckResult(protocol.ReadConnect(channel.GetReader(), version, mode, serverName, clientName));

        CheckResult(protocol.SendError(channel.GetWriter(), errorMessage));
        return CreateOk();
    }

    std::unique_ptr<ChannelServer> _server;
    std::unique_ptr<Channel> _serverChannel;
    std::unique_ptr<IProtocol> _serverProtocol;
    std::unique_ptr<CoSimServer> _coSimServer;
    std::unique_ptr<CoSimClient> _client;
    std::string _serverName;
    uint16_t _serverPort{};

    [[nodiscard]] Result WaitForAccept(std::unique_ptr<Channel>& channel) const {
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(DefaultTimeoutInMilliseconds);

        while (std::chrono::steady_clock::now() < deadline) {
            Result result = _server->TryAccept(channel);
            if (IsOk(result)) {
                return result;
            }

            if (!IsNotConnected(result)) {
                return result;
            }

            std::this_thread::yield();
        }

        return CreateTimeout();
    }
};

INSTANTIATE_TEST_SUITE_P(,
                         TestCoSimClient,
                         testing::Values(ConnectionKind::Local, ConnectionKind::Remote),
                         [](const testing::TestParamInfo<ConnectionKind>& info) {
                             return fmt::format("{}", info.param);
                         });

// --- Connect ---

TEST_P(TestCoSimClient, ConnectWithEmptyConfigShouldReturnInvalidArgument) {
    // Arrange
    _client = std::make_unique<CoSimClient>();
    ConnectConfig config;

    // Act
    Result result = _client->Connect(config);

    // Assert
    AssertInvalidArgument(result);
}

TEST_P(TestCoSimClient, Connect) {
    // Arrange
    CustomSetUp(GetParam());
    auto serverTask = RunServer([](Channel& channel, IProtocol& protocol) {
        return AcceptAndSendConnectOk(channel, protocol);
    });

    // Act
    Result result = _client->Connect(MakeConfig(GetParam()));

    // Assert
    AssertOk(result);
    AssertOk(serverTask.get());
    ASSERT_EQ(ConnectionState::Connected, _client->GetConnectionState());
}

TEST_P(TestCoSimClient, ConnectAlreadyConnectedShouldReturnOk) {
    // Arrange
    CustomSetUp(GetParam());
    auto serverTask = RunServer([](Channel& channel, IProtocol& protocol) {
        return AcceptAndSendConnectOk(channel, protocol);
    });
    AssertOk(_client->Connect(MakeConfig(GetParam())));
    AssertOk(serverTask.get());

    // Act
    Result result = _client->Connect(MakeConfig(GetParam()));

    // Assert
    AssertOk(result);
}

TEST_P(TestCoSimClient, ConnectServerSendsErrorShouldFail) {
    // Arrange
    CustomSetUp(GetParam());
    auto serverTask = RunServer([](Channel& channel, IProtocol& protocol) {
        return AcceptAndSendError(channel, protocol, "Connection rejected.");
    });

    // Act
    Result result = _client->Connect(MakeConfig(GetParam()));

    // Assert
    AssertError(result);
    AssertOk(serverTask.get());
    ASSERT_EQ(ConnectionState::Disconnected, _client->GetConnectionState());
}

TEST_F(TestCoSimClient, ConnectToNonExistentServerShouldFail) {
    // Arrange
    // Server accepts TCP connection but drops it without protocol response.
    // This avoids the 1s TCP connect timeout of a truly dead port.
    CustomSetUp(ConnectionKind::Remote);
    auto serverTask = RunServer([]([[maybe_unused]] Channel& channel, [[maybe_unused]] IProtocol& protocol) {
        return CreateOk();
    });

    // Act
    Result result = _client->Connect(MakeConfig(ConnectionKind::Remote));

    // Assert
    AssertNotConnected(result);
    serverTask.wait();
}

// --- Disconnect ---

TEST_P(TestCoSimClient, DisconnectAfterConnect) {
    // Arrange
    CustomSetUp(GetParam());
    auto serverTask = RunServer([](Channel& channel, IProtocol& protocol) {
        return AcceptAndSendConnectOk(channel, protocol);
    });
    AssertOk(_client->Connect(MakeConfig(GetParam())));
    AssertOk(serverTask.get());

    // Act
    _client->Disconnect();

    // Assert
    ASSERT_EQ(ConnectionState::Disconnected, _client->GetConnectionState());
}

TEST_F(TestCoSimClient, DisconnectWithoutConnectShouldNotCrash) {
    // Arrange
    _client = std::make_unique<CoSimClient>();

    // Act
    _client->Disconnect();

    // Assert
    ASSERT_EQ(ConnectionState::Disconnected, _client->GetConnectionState());
}

TEST_P(TestCoSimClient, DoubleDisconnectShouldNotCrash) {
    // Arrange
    CustomSetUp(GetParam());
    auto serverTask = RunServer([](Channel& channel, IProtocol& protocol) {
        return AcceptAndSendConnectOk(channel, protocol);
    });
    AssertOk(_client->Connect(MakeConfig(GetParam())));
    AssertOk(serverTask.get());
    _client->Disconnect();

    // Act
    _client->Disconnect();

    // Assert
    ASSERT_EQ(ConnectionState::Disconnected, _client->GetConnectionState());
}

// --- GetConnectionState ---

TEST_F(TestCoSimClient, GetConnectionStateInitialShouldBeDisconnected) {
    // Arrange
    _client = std::make_unique<CoSimClient>();

    // Act
    ConnectionState state = _client->GetConnectionState();

    // Assert
    ASSERT_EQ(ConnectionState::Disconnected, state);
}

TEST_F(TestCoSimClient, GetConnectionStateAfterFailedConnectShouldBeDisconnected) {
    // Arrange
    // Server accepts TCP connection but drops it without protocol response.
    CustomSetUp(ConnectionKind::Remote);
    auto serverTask = RunServer([]([[maybe_unused]] Channel& channel, [[maybe_unused]] IProtocol& protocol) {
        return CreateOk();
    });
    AssertNotConnected(_client->Connect(MakeConfig(ConnectionKind::Remote)));
    serverTask.wait();

    // Act
    ConnectionState state = _client->GetConnectionState();

    // Assert
    ASSERT_EQ(ConnectionState::Disconnected, state);
}

// --- Reconnect ---

TEST_P(TestCoSimClient, ReconnectAfterDisconnect) {
    // Arrange
    CustomSetUp(GetParam());

    // First connection
    auto serverTask1 = RunServer([](Channel& channel, IProtocol& protocol) {
        return AcceptAndSendConnectOk(channel, protocol);
    });
    AssertOk(_client->Connect(MakeConfig(GetParam())));
    AssertOk(serverTask1.get());
    _client->Disconnect();

    // Second connection needs a fresh server
    _serverName = GenerateString("CoSimServer名前");
    if (GetParam() == ConnectionKind::Local) {
        AssertOk(CreateLocalChannelServer(_serverName, _server));
    } else {
        AssertOk(CreateTcpChannelServer(0, true, _server));
        _serverPort = _server->GetLocalPort();
    }

    auto serverTask2 = RunServer([](Channel& channel, IProtocol& protocol) {
        return AcceptAndSendConnectOk(channel, protocol);
    });

    // Act
    Result result = _client->Connect(MakeConfig(GetParam()));

    // Assert
    AssertOk(result);
    AssertOk(serverTask2.get());
    ASSERT_EQ(ConnectionState::Connected, _client->GetConnectionState());
}

// --- GetStepSize ---

TEST_F(TestCoSimClient, GetStepSizeWhenNotConnectedShouldFail) {
    // Arrange
    _client = std::make_unique<CoSimClient>();

    // Act
    SimulationTime stepSize{};
    Result result = _client->GetStepSize(stepSize);

    // Assert
    AssertNotConnected(result);
}

TEST_P(TestCoSimClient, GetStepSize) {
    // Arrange
    CustomSetUp(GetParam());
    SimulationTime expectedStepSize = GenerateSimulationTime();
    auto serverTask = RunServer([expectedStepSize](Channel& channel, IProtocol& protocol) {
        return AcceptAndSendConnectOk(channel, protocol, expectedStepSize, SimulationState::Stopped);
    });
    AssertOk(_client->Connect(MakeConfig(GetParam())));
    AssertOk(serverTask.get());

    // Act
    SimulationTime stepSize{};
    Result result = _client->GetStepSize(stepSize);

    // Assert
    AssertOk(result);
    ASSERT_EQ(expectedStepSize, stepSize);
}

// --- GetCurrentSimulationTime ---

TEST_F(TestCoSimClient, GetCurrentSimulationTimeWhenNotConnectedShouldFail) {
    // Arrange
    _client = std::make_unique<CoSimClient>();

    // Act
    SimulationTime simulationTime{};
    Result result = _client->GetCurrentSimulationTime(simulationTime);

    // Assert
    AssertNotConnected(result);
}

TEST_P(TestCoSimClient, GetCurrentSimulationTimeAfterConnect) {
    // Arrange
    CustomSetUp(GetParam());
    auto serverTask = RunServer([](Channel& channel, IProtocol& protocol) {
        return AcceptAndSendConnectOk(channel, protocol);
    });
    AssertOk(_client->Connect(MakeConfig(GetParam())));
    AssertOk(serverTask.get());

    // Act
    SimulationTime simulationTime{};
    Result result = _client->GetCurrentSimulationTime(simulationTime);

    // Assert
    AssertOk(result);
    ASSERT_EQ(SimulationTime{}, simulationTime);
}

// --- GetSimulationState ---

TEST_F(TestCoSimClient, GetSimulationStateWhenNotConnectedShouldFail) {
    // Arrange
    _client = std::make_unique<CoSimClient>();

    // Act
    SimulationState simulationState{};
    Result result = _client->GetSimulationState(simulationState);

    // Assert
    AssertNotConnected(result);
}

TEST_P(TestCoSimClient, GetSimulationState) {
    // Arrange
    CustomSetUp(GetParam());
    SimulationState expectedState = SimulationState::Running;
    auto serverTask = RunServer([expectedState](Channel& channel, IProtocol& protocol) {
        return AcceptAndSendConnectOk(channel, protocol, SimulationTime{}, expectedState);
    });
    AssertOk(_client->Connect(MakeConfig(GetParam())));
    AssertOk(serverTask.get());

    // Act
    SimulationState simulationState{};
    Result result = _client->GetSimulationState(simulationState);

    // Assert
    AssertOk(result);
    ASSERT_EQ(expectedState, simulationState);
}

// --- RunCallbackBasedCoSimulation ---

TEST_F(TestCoSimClient, RunCallbackBasedCoSimulationWhenNotConnectedShouldFail) {
    // Arrange
    _client = std::make_unique<CoSimClient>();

    // Act
    Result result = _client->RunCallbackBasedCoSimulation({});

    // Assert
    AssertNotConnected(result);
}

TEST_P(TestCoSimClient, RunCallbackBasedCoSimulationReceivesStartAndTerminate) {
    // Arrange
    ConnectClientAndServer(GetParam());

    SimulationTime startTime(std::chrono::nanoseconds(1000));
    SimulationTime terminateTime(std::chrono::nanoseconds(2000));
    bool startCalled = false;
    bool terminateCalled = false;

    Callbacks callbacks{};
    callbacks.simulationStartedCallback = [&](SimulationTime) {
        startCalled = true;
    };
    callbacks.simulationTerminatedCallback = [&](SimulationTime, TerminateReason) {
        terminateCalled = true;
        _client->Disconnect();
    };

    auto serverTask = std::async(std::launch::async, [&]() -> Result {
        // Send Start
        CheckResult(_serverProtocol->SendStart(_serverChannel->GetWriter(), startTime));
        FrameKind frameKind{};
        CheckResult(_serverProtocol->ReceiveHeader(_serverChannel->GetReader(), frameKind));
        CheckResult(_serverProtocol->ReadOk(_serverChannel->GetReader()));

        // Send Terminate — client will disconnect in the callback, so no Ok response expected.
        CheckResult(_serverProtocol->SendTerminate(_serverChannel->GetWriter(), terminateTime, TerminateReason::Finished));
        return CreateOk();
    });

    // Act
    Result result = _client->RunCallbackBasedCoSimulation(callbacks);

    // Assert
    AssertNotConnected(result);
    AssertOk(serverTask.get());
    ASSERT_TRUE(startCalled);
    ASSERT_TRUE(terminateCalled);
}

TEST_P(TestCoSimClient, RunCallbackBasedCoSimulationAfterPollingModeShouldFail) {
    // Arrange
    ConnectClientAndServer(GetParam());
    AssertOk(_client->StartPollingBasedCoSimulation({}));

    // Act
    Result result = _client->RunCallbackBasedCoSimulation({});

    // Assert
    AssertError(result);
}

// --- StartPollingBasedCoSimulation ---

TEST_F(TestCoSimClient, StartPollingBasedCoSimulationWhenNotConnectedShouldFail) {
    // Arrange
    _client = std::make_unique<CoSimClient>();

    // Act
    Result result = _client->StartPollingBasedCoSimulation({});

    // Assert
    AssertNotConnected(result);
}

TEST_P(TestCoSimClient, StartPollingBasedCoSimulation) {
    // Arrange
    ConnectClientAndServer(GetParam());

    // Act
    Result result = _client->StartPollingBasedCoSimulation({});

    // Assert
    AssertOk(result);
}

TEST_P(TestCoSimClient, StartPollingBasedCoSimulationAfterCallbackModeShouldSucceed) {
    // Arrange
    ConnectClientAndServer(GetParam());

    // Force blocking mode by running callback-based co-sim that immediately disconnects.
    Callbacks callbacks{};
    callbacks.simulationTerminatedCallback = [&](SimulationTime, TerminateReason) {
        _client->Disconnect();
    };

    auto serverTask = std::async(std::launch::async, [&]() -> Result {
        // Client will disconnect in the callback, so no Ok response expected.
        CheckResult(_serverProtocol->SendTerminate(_serverChannel->GetWriter(), SimulationTime{}, TerminateReason::Finished));
        return CreateOk();
    });
    AssertNotConnected(_client->RunCallbackBasedCoSimulation(callbacks));
    AssertOk(serverTask.get());

    // Reconnect
    _serverName = GenerateString("CoSimServer名前");
    if (GetParam() == ConnectionKind::Local) {
        AssertOk(CreateLocalChannelServer(_serverName, _server));
    } else {
        AssertOk(CreateTcpChannelServer(0, true, _server));
        _serverPort = _server->GetLocalPort();
    }

    auto connectServerTask = std::async(std::launch::async, [this]() {
        CheckResult(WaitForAccept(_serverChannel));
        return AcceptAndSendConnectOk(*_serverChannel, *_serverProtocol);
    });
    AssertOk(_client->Connect(MakeConfig(GetParam())));
    AssertOk(connectServerTask.get());

    // Act
    Result result = _client->StartPollingBasedCoSimulation({});

    // Assert — reconnect resets responder mode, so this should succeed
    AssertOk(result);
}

// --- PollCommand ---

TEST_F(TestCoSimClient, PollCommandWhenNotConnectedShouldFail) {
    // Arrange
    _client = std::make_unique<CoSimClient>();

    // Act
    SimulationTime simulationTime{};
    Command command{};
    Result result = _client->PollCommand(simulationTime, command, Infinite);

    // Assert
    AssertNotConnected(result);
}

TEST_P(TestCoSimClient, PollCommandReceivesStart) {
    // Arrange
    ConnectClientAndServer(GetParam());
    AssertOk(_client->StartPollingBasedCoSimulation({}));

    SimulationTime expectedTime(std::chrono::nanoseconds(5000));
    auto serverTask = std::async(std::launch::async, [&]() -> Result {
        CheckResult(_serverProtocol->SendStart(_serverChannel->GetWriter(), expectedTime));
        return CreateOk();
    });

    // Act
    SimulationTime simulationTime{};
    Command command{};
    Result result = _client->PollCommand(simulationTime, command, Infinite);

    // Assert
    AssertOk(result);
    AssertOk(serverTask.get());
    ASSERT_EQ(Command::Start, command);
    ASSERT_EQ(expectedTime, simulationTime);
}

TEST_P(TestCoSimClient, PollCommandReceivesStop) {
    // Arrange
    ConnectClientAndServer(GetParam());
    AssertOk(_client->StartPollingBasedCoSimulation({}));

    SimulationTime expectedTime(std::chrono::nanoseconds(3000));
    auto serverTask = std::async(std::launch::async, [&]() -> Result {
        CheckResult(_serverProtocol->SendStop(_serverChannel->GetWriter(), expectedTime));
        return CreateOk();
    });

    // Act
    SimulationTime simulationTime{};
    Command command{};
    Result result = _client->PollCommand(simulationTime, command, Infinite);

    // Assert
    AssertOk(result);
    AssertOk(serverTask.get());
    ASSERT_EQ(Command::Stop, command);
    ASSERT_EQ(expectedTime, simulationTime);
}

TEST_P(TestCoSimClient, PollCommandReceivesTerminate) {
    // Arrange
    ConnectClientAndServer(GetParam());
    AssertOk(_client->StartPollingBasedCoSimulation({}));

    SimulationTime expectedTime(std::chrono::nanoseconds(7000));
    auto serverTask = std::async(std::launch::async, [&]() -> Result {
        CheckResult(_serverProtocol->SendTerminate(_serverChannel->GetWriter(), expectedTime, TerminateReason::Error));
        return CreateOk();
    });

    // Act
    SimulationTime simulationTime{};
    Command command{};
    Result result = _client->PollCommand(simulationTime, command, Infinite);

    // Assert
    AssertOk(result);
    AssertOk(serverTask.get());
    ASSERT_EQ(Command::Terminate, command);
    ASSERT_EQ(expectedTime, simulationTime);
}

TEST_P(TestCoSimClient, PollCommandReceivesPause) {
    // Arrange
    ConnectClientAndServer(GetParam());
    AssertOk(_client->StartPollingBasedCoSimulation({}));

    SimulationTime expectedTime(std::chrono::nanoseconds(4000));
    auto serverTask = std::async(std::launch::async, [&]() -> Result {
        CheckResult(_serverProtocol->SendPause(_serverChannel->GetWriter(), expectedTime));
        return CreateOk();
    });

    // Act
    SimulationTime simulationTime{};
    Command command{};
    Result result = _client->PollCommand(simulationTime, command, Infinite);

    // Assert
    AssertOk(result);
    AssertOk(serverTask.get());
    ASSERT_EQ(Command::Pause, command);
    ASSERT_EQ(expectedTime, simulationTime);
}

TEST_P(TestCoSimClient, PollCommandReceivesContinue) {
    // Arrange
    ConnectClientAndServer(GetParam());
    AssertOk(_client->StartPollingBasedCoSimulation({}));

    SimulationTime expectedTime(std::chrono::nanoseconds(6000));
    auto serverTask = std::async(std::launch::async, [&]() -> Result {
        CheckResult(_serverProtocol->SendContinue(_serverChannel->GetWriter(), expectedTime));
        return CreateOk();
    });

    // Act
    SimulationTime simulationTime{};
    Command command{};
    Result result = _client->PollCommand(simulationTime, command, Infinite);

    // Assert
    AssertOk(result);
    AssertOk(serverTask.get());
    ASSERT_EQ(Command::Continue, command);
    ASSERT_EQ(expectedTime, simulationTime);
}

TEST_P(TestCoSimClient, PollCommandReceivesStep) {
    // Arrange
    ConnectAndStartPolling(GetParam());

    SimulationTime expectedTime(std::chrono::nanoseconds(8000));

    auto serverTask = std::async(std::launch::async, [this, expectedTime] {
        SimulationTime nextTime{};
        return _coSimServer->Step(expectedTime, nextTime);
    });

    // Act
    SimulationTime simulationTime{};
    Command command{};
    Result result = _client->PollCommand(simulationTime, command, Infinite);

    // Assert
    AssertOk(result);
    ASSERT_EQ(Command::Step, command);
    ASSERT_EQ(expectedTime, simulationTime);
    AssertOk(_client->FinishCommand());
    AssertOk(serverTask.get());
}

TEST_P(TestCoSimClient, PollCommandSkipsPingAndReturnsNextCommand) {
    // Arrange
    ConnectClientAndServer(GetParam());
    AssertOk(_client->StartPollingBasedCoSimulation({}));

    SimulationTime startTime(std::chrono::nanoseconds(9000));

    auto serverTask = std::async(std::launch::async, [&]() -> Result {
        // Send Ping first
        CheckResult(_serverProtocol->SendPing(_serverChannel->GetWriter(), SimulationTime(std::chrono::nanoseconds(100))));
        // Read PingOk
        FrameKind frameKind{};
        CheckResult(_serverProtocol->ReceiveHeader(_serverChannel->GetReader(), frameKind));
        if (frameKind != FrameKind::PingOk) {
            return CreateError();
        }

        Command command{};
        CheckResult(_serverProtocol->ReadPingOk(_serverChannel->GetReader(), command));

        // Now send Start
        CheckResult(_serverProtocol->SendStart(_serverChannel->GetWriter(), startTime));
        return CreateOk();
    });

    // Act
    SimulationTime simulationTime{};
    Command command{};
    Result result = _client->PollCommand(simulationTime, command, Infinite);

    // Assert
    AssertOk(result);
    AssertOk(serverTask.get());
    ASSERT_EQ(Command::Start, command);
    ASSERT_EQ(startTime, simulationTime);
}

TEST_P(TestCoSimClient, PollCommandWithoutFinishCommandShouldFail) {
    // Arrange
    ConnectClientAndServer(GetParam());
    AssertOk(_client->StartPollingBasedCoSimulation({}));

    auto serverTask = std::async(std::launch::async, [&]() -> Result {
        CheckResult(_serverProtocol->SendStart(_serverChannel->GetWriter(), SimulationTime{}));
        return CreateOk();
    });

    SimulationTime simulationTime{};
    Command command{};
    AssertOk(_client->PollCommand(simulationTime, command, Infinite));
    AssertOk(serverTask.get());

    // Act — second PollCommand without FinishCommand
    Result result = _client->PollCommand(simulationTime, command, Infinite);

    // Assert
    AssertError(result);
}

TEST_P(TestCoSimClient, PollCommandTimesOutWhenNoCommandArrives) {
    // Arrange
    ConnectClientAndServer(GetParam());
    AssertOk(_client->StartPollingBasedCoSimulation({}));

    SimulationTime simulationTime{};
    Command command{};

    // Act
    Result result = _client->PollCommand(simulationTime, command, 50);

    // Assert
    AssertTimeout(result);
}

// --- FinishCommand ---

TEST_F(TestCoSimClient, FinishCommandWhenNotConnectedShouldFail) {
    // Arrange
    _client = std::make_unique<CoSimClient>();

    // Act
    Result result = _client->FinishCommand();

    // Assert
    AssertNotConnected(result);
}

TEST_P(TestCoSimClient, FinishCommandWithoutPollCommandShouldFail) {
    // Arrange
    ConnectClientAndServer(GetParam());
    AssertOk(_client->StartPollingBasedCoSimulation({}));

    // Act
    Result result = _client->FinishCommand();

    // Assert
    AssertError(result);
}

TEST_P(TestCoSimClient, FinishCommandAfterStart) {
    // Arrange
    ConnectClientAndServer(GetParam());
    AssertOk(_client->StartPollingBasedCoSimulation({}));

    auto serverTask = std::async(std::launch::async, [&]() -> Result {
        CheckResult(_serverProtocol->SendStart(_serverChannel->GetWriter(), SimulationTime{}));

        FrameKind frameKind{};
        CheckResult(_serverProtocol->ReceiveHeader(_serverChannel->GetReader(), frameKind));
        if (frameKind != FrameKind::Ok) {
            return CreateError();
        }

        CheckResult(_serverProtocol->ReadOk(_serverChannel->GetReader()));
        return CreateOk();
    });

    SimulationTime simulationTime{};
    Command command{};
    AssertOk(_client->PollCommand(simulationTime, command, Infinite));

    // Act
    Result result = _client->FinishCommand();

    // Assert
    AssertOk(result);
    AssertOk(serverTask.get());
}

TEST_P(TestCoSimClient, FinishCommandAfterStep) {
    // Arrange
    ConnectAndStartPolling(GetParam());

    auto serverTask = std::async(std::launch::async, [this] {
        SimulationTime nextTime{};
        return _coSimServer->Step(SimulationTime{}, nextTime);
    });

    SimulationTime simulationTime{};
    Command command{};
    AssertOk(_client->PollCommand(simulationTime, command, Infinite));

    // Act
    Result result = _client->FinishCommand();

    // Assert
    AssertOk(result);
    AssertOk(serverTask.get());
}

// --- SetNextSimulationTime ---

TEST_F(TestCoSimClient, SetNextSimulationTimeWhenNotConnectedShouldFail) {
    // Arrange
    _client = std::make_unique<CoSimClient>();

    // Act
    Result result = _client->SetNextSimulationTime(SimulationTime{});

    // Assert
    AssertNotConnected(result);
}

TEST_P(TestCoSimClient, SetNextSimulationTime) {
    // Arrange
    ConnectClientAndServer(GetParam());

    // Act
    Result result = _client->SetNextSimulationTime(SimulationTime(std::chrono::nanoseconds(42000)));

    // Assert
    AssertOk(result);
}

TEST_P(TestCoSimClient, SetNextSimulationTimeIsSentInStepOk) {
    // Arrange
    ConnectAndStartPolling(GetParam());
    SimulationTime expectedNextTime(std::chrono::nanoseconds(42000));
    AssertOk(_client->SetNextSimulationTime(expectedNextTime));

    SimulationTime receivedNextTime{};
    auto serverTask = std::async(std::launch::async, [this, &receivedNextTime] {
        return _coSimServer->Step(SimulationTime{}, receivedNextTime);
    });

    SimulationTime simulationTime{};
    Command command{};
    AssertOk(_client->PollCommand(simulationTime, command, Infinite));
    AssertOk(_client->FinishCommand());

    // Assert
    AssertOk(serverTask.get());
    ASSERT_EQ(expectedNextTime, receivedNextTime);
}

// --- GetRoundTripTime ---

TEST_F(TestCoSimClient, GetRoundTripTimeWhenNotConnectedShouldFail) {
    // Arrange
    _client = std::make_unique<CoSimClient>();

    // Act
    SimulationTime roundTripTime{};
    Result result = _client->GetRoundTripTime(roundTripTime);

    // Assert
    AssertNotConnected(result);
}

TEST_P(TestCoSimClient, GetRoundTripTime) {
    // Arrange
    ConnectClientAndServer(GetParam());

    // Act
    SimulationTime roundTripTime{};
    Result result = _client->GetRoundTripTime(roundTripTime);

    // Assert
    AssertOk(result);
}

// --- Start ---

TEST_F(TestCoSimClient, StartWhenNotConnectedShouldFail) {
    // Arrange
    _client = std::make_unique<CoSimClient>();

    // Act
    Result result = _client->Start();

    // Assert
    AssertNotConnected(result);
}

TEST_P(TestCoSimClient, StartSetsNextCommand) {
    // Arrange
    bool received = false;
    CoSimServerConfig config{};
    config.simulationStartedCallback = [&](SimulationTime) {
        received = true;
    };
    ConnectAndStartPolling(GetParam(), config);
    AssertOk(_client->Start());

    auto serverTask = std::async(std::launch::async, [this] {
        SimulationTime nextTime{};
        return _coSimServer->Step(SimulationTime{}, nextTime);
    });

    SimulationTime simulationTime{};
    Command command{};
    AssertOk(_client->PollCommand(simulationTime, command, Infinite));
    AssertOk(_client->FinishCommand());

    // Assert
    AssertOk(serverTask.get());
    EXPECT_TRUE(received);
}

// --- Stop ---

TEST_F(TestCoSimClient, StopWhenNotConnectedShouldFail) {
    // Arrange
    _client = std::make_unique<CoSimClient>();

    // Act
    Result result = _client->Stop();

    // Assert
    AssertNotConnected(result);
}

TEST_P(TestCoSimClient, StopSetsNextCommand) {
    // Arrange
    bool received = false;
    CoSimServerConfig config{};
    config.simulationStoppedCallback = [&](SimulationTime) {
        received = true;
    };
    ConnectAndStartPolling(GetParam(), config);
    AssertOk(_client->Stop());

    auto serverTask = std::async(std::launch::async, [this] {
        SimulationTime nextTime{};
        return _coSimServer->Step(SimulationTime{}, nextTime);
    });

    SimulationTime simulationTime{};
    Command command{};
    AssertOk(_client->PollCommand(simulationTime, command, Infinite));
    AssertOk(_client->FinishCommand());

    // Assert
    AssertOk(serverTask.get());
    EXPECT_TRUE(received);
}

// --- Pause ---

TEST_F(TestCoSimClient, PauseWhenNotConnectedShouldFail) {
    // Arrange
    _client = std::make_unique<CoSimClient>();

    // Act
    Result result = _client->Pause();

    // Assert
    AssertNotConnected(result);
}

TEST_P(TestCoSimClient, PauseSetsNextCommand) {
    // Arrange
    bool received = false;
    CoSimServerConfig config{};
    config.simulationPausedCallback = [&](SimulationTime) {
        received = true;
    };
    ConnectAndStartPolling(GetParam(), config);
    AssertOk(_client->Pause());

    auto serverTask = std::async(std::launch::async, [this] {
        SimulationTime nextTime{};
        return _coSimServer->Step(SimulationTime{}, nextTime);
    });

    SimulationTime simulationTime{};
    Command command{};
    AssertOk(_client->PollCommand(simulationTime, command, Infinite));
    AssertOk(_client->FinishCommand());

    // Assert
    AssertOk(serverTask.get());
    EXPECT_TRUE(received);
}

// --- Continue ---

TEST_F(TestCoSimClient, ContinueWhenNotConnectedShouldFail) {
    // Arrange
    _client = std::make_unique<CoSimClient>();

    // Act
    Result result = _client->Continue();

    // Assert
    AssertNotConnected(result);
}

TEST_P(TestCoSimClient, ContinueSetsNextCommand) {
    // Arrange
    bool received = false;
    CoSimServerConfig config{};
    config.simulationContinuedCallback = [&](SimulationTime) {
        received = true;
    };
    ConnectAndStartPolling(GetParam(), config);
    AssertOk(_client->Continue());

    auto serverTask = std::async(std::launch::async, [this] {
        SimulationTime nextTime{};
        return _coSimServer->Step(SimulationTime{}, nextTime);
    });

    SimulationTime simulationTime{};
    Command command{};
    AssertOk(_client->PollCommand(simulationTime, command, Infinite));
    AssertOk(_client->FinishCommand());

    // Assert
    AssertOk(serverTask.get());
    EXPECT_TRUE(received);
}

// --- Terminate ---

TEST_F(TestCoSimClient, TerminateWhenNotConnectedShouldFail) {
    // Arrange
    _client = std::make_unique<CoSimClient>();

    // Act
    Result result = _client->Terminate(TerminateReason::Finished);

    // Assert
    AssertNotConnected(result);
}

TEST_P(TestCoSimClient, TerminateFinishedSetsNextCommand) {
    // Arrange
    bool received = false;
    CoSimServerConfig config{};
    config.simulationTerminatedCallback = [&](SimulationTime, TerminateReason) {
        received = true;
    };
    ConnectAndStartPolling(GetParam(), config);
    AssertOk(_client->Terminate(TerminateReason::Finished));

    auto serverTask = std::async(std::launch::async, [this] {
        SimulationTime nextTime{};
        return _coSimServer->Step(SimulationTime{}, nextTime);
    });

    SimulationTime simulationTime{};
    Command command{};
    AssertOk(_client->PollCommand(simulationTime, command, Infinite));
    AssertOk(_client->FinishCommand());

    // Assert
    AssertOk(serverTask.get());
    EXPECT_TRUE(received);
}

TEST_P(TestCoSimClient, TerminateErrorSetsNextCommand) {
    // Arrange
    bool received = false;
    CoSimServerConfig config{};
    config.simulationTerminatedCallback = [&](SimulationTime, TerminateReason) {
        received = true;
    };
    ConnectAndStartPolling(GetParam(), config);
    AssertOk(_client->Terminate(TerminateReason::Error));

    auto serverTask = std::async(std::launch::async, [this] {
        SimulationTime nextTime{};
        return _coSimServer->Step(SimulationTime{}, nextTime);
    });

    SimulationTime simulationTime{};
    Command command{};
    AssertOk(_client->PollCommand(simulationTime, command, Infinite));
    AssertOk(_client->FinishCommand());

    // Assert
    AssertOk(serverTask.get());
    EXPECT_TRUE(received);
}

// --- GetIncomingSignals ---

TEST_F(TestCoSimClient, GetIncomingSignalsWhenNotConnectedShouldFail) {
    // Arrange
    _client = std::make_unique<CoSimClient>();

    // Act
    uint32_t count{};
    const IoSignal* signals{};
    Result result = _client->GetIncomingSignals(count, signals);

    // Assert
    AssertNotConnected(result);
}

TEST_P(TestCoSimClient, GetIncomingSignalsReturnsSignalsFromConnectOk) {
    // Arrange
    auto signal1 = CreateSignal(DataType::Float64, SizeKind::Fixed);
    auto signal2 = CreateSignal(DataType::Int32, SizeKind::Fixed);
    CoSimServerConfig config{};
    config.incomingSignals = {signal1, signal2};
    ConnectAndStartPolling(GetParam(), config);

    // Act
    uint32_t count{};
    const IoSignal* signals{};
    Result result = _client->GetIncomingSignals(count, signals);

    // Assert
    AssertOk(result);
    ASSERT_EQ(2U, count);
    ASSERT_EQ(signal1.id, signals[0].id);
    ASSERT_EQ(signal2.id, signals[1].id);
}

TEST_P(TestCoSimClient, GetIncomingSignalsVectorReturnsSignalsFromConnectOk) {
    // Arrange
    auto signal1 = CreateSignal(DataType::Float64, SizeKind::Fixed);
    CoSimServerConfig config{};
    config.incomingSignals = {signal1};
    ConnectAndStartPolling(GetParam(), config);

    // Act
    std::vector<IoSignal> signals;
    Result result = _client->GetIncomingSignals(signals);

    // Assert
    AssertOk(result);
    ASSERT_EQ(1U, signals.size());
    ASSERT_EQ(signal1.id, signals[0].id);
}

// --- GetOutgoingSignals ---

TEST_F(TestCoSimClient, GetOutgoingSignalsWhenNotConnectedShouldFail) {
    // Arrange
    _client = std::make_unique<CoSimClient>();

    // Act
    uint32_t count{};
    const IoSignal* signals{};
    Result result = _client->GetOutgoingSignals(count, signals);

    // Assert
    AssertNotConnected(result);
}

TEST_P(TestCoSimClient, GetOutgoingSignalsReturnsSignalsFromConnectOk) {
    // Arrange
    auto signal1 = CreateSignal(DataType::UInt16, SizeKind::Fixed);
    CoSimServerConfig config{};
    config.outgoingSignals = {signal1};
    ConnectAndStartPolling(GetParam(), config);

    // Act
    uint32_t count{};
    const IoSignal* signals{};
    Result result = _client->GetOutgoingSignals(count, signals);

    // Assert
    AssertOk(result);
    ASSERT_EQ(1U, count);
    ASSERT_EQ(signal1.id, signals[0].id);
}

TEST_P(TestCoSimClient, GetOutgoingSignalsVectorReturnsSignalsFromConnectOk) {
    // Arrange
    auto signal1 = CreateSignal(DataType::Float32, SizeKind::Fixed);
    CoSimServerConfig config{};
    config.outgoingSignals = {signal1};
    ConnectAndStartPolling(GetParam(), config);

    // Act
    std::vector<IoSignal> signals;
    Result result = _client->GetOutgoingSignals(signals);

    // Assert
    AssertOk(result);
    ASSERT_EQ(1U, signals.size());
    ASSERT_EQ(signal1.id, signals[0].id);
}

// --- Write ---

TEST_F(TestCoSimClient, WriteWhenNotConnectedShouldFail) {
    // Arrange
    _client = std::make_unique<CoSimClient>();
    uint8_t value = 42;

    // Act
    Result result = _client->Write(IoSignalId{}, 1, &value);

    // Assert
    AssertNotConnected(result);
}

TEST_F(TestCoSimClient, WriteSignal) {
    // Arrange — Remote only: Local uses shared memory signal exchange not supported by mock server.
    auto outSignal = CreateSignal(DataType::Float64, SizeKind::Fixed);
    CoSimServerConfig config{};
    config.outgoingSignals = {outSignal};
    ConnectAndStartPolling(ConnectionKind::Remote, config);

    auto ioData = GenerateIoData(outSignal);
    AssertOk(_client->Write(outSignal.id, outSignal.length, ioData.data()));

    auto serverTask = std::async(std::launch::async, [this] {
        SimulationTime nextTime{};
        return _coSimServer->Step(SimulationTime{}, nextTime);
    });

    SimulationTime simulationTime{};
    Command command{};
    AssertOk(_client->PollCommand(simulationTime, command, Infinite));
    AssertOk(_client->FinishCommand());
    AssertOk(serverTask.get());

    // Assert — server reads the signal the client wrote
    uint32_t length = outSignal.length;
    const void* value{};
    bool valueRead{};
    AssertOk(_coSimServer->Read(outSignal.id, length, &value, valueRead));
    ASSERT_TRUE(valueRead);
    ASSERT_EQ(0, std::memcmp(ioData.data(), value, ioData.size()));
}

// --- Read ---

TEST_F(TestCoSimClient, ReadWhenNotConnectedShouldFail) {
    // Arrange
    _client = std::make_unique<CoSimClient>();
    uint8_t value{};
    uint32_t length{};

    // Act
    Result result = _client->Read(IoSignalId{}, length, &value);

    // Assert
    AssertNotConnected(result);
}

TEST_F(TestCoSimClient, ReadSignal) {
    // Arrange — Remote only: Local uses shared memory signal exchange not supported by mock server.
    auto inSignal = CreateSignal(DataType::Float64, SizeKind::Fixed);
    CoSimServerConfig config{};
    config.incomingSignals = {inSignal};
    ConnectAndStartPolling(ConnectionKind::Remote, config);

    auto expectedData = GenerateIoData(inSignal);
    AssertOk(_coSimServer->Write(inSignal.id, inSignal.length, expectedData.data()));

    auto serverTask = std::async(std::launch::async, [this] {
        SimulationTime nextTime{};
        return _coSimServer->Step(SimulationTime{}, nextTime);
    });

    SimulationTime simulationTime{};
    Command command{};
    AssertOk(_client->PollCommand(simulationTime, command, Infinite));

    // Act
    size_t dataTypeSize = GetDataTypeSize(inSignal.dataType);
    std::vector<uint8_t> readData(dataTypeSize * inSignal.length);
    uint32_t length = inSignal.length;
    Result result = _client->Read(inSignal.id, length, readData.data());

    // Assert
    AssertOk(result);
    ASSERT_EQ(expectedData, readData);

    AssertOk(_client->FinishCommand());
    AssertOk(serverTask.get());
}

TEST_F(TestCoSimClient, ReadSignalRef) {
    // Arrange — Remote only: Local uses shared memory signal exchange not supported by mock server.
    auto inSignal = CreateSignal(DataType::Float64, SizeKind::Fixed);
    CoSimServerConfig config{};
    config.incomingSignals = {inSignal};
    ConnectAndStartPolling(ConnectionKind::Remote, config);

    auto expectedData = GenerateIoData(inSignal);
    size_t dataTypeSize = GetDataTypeSize(inSignal.dataType);
    AssertOk(_coSimServer->Write(inSignal.id, inSignal.length, expectedData.data()));

    auto serverTask = std::async(std::launch::async, [this] {
        SimulationTime nextTime{};
        return _coSimServer->Step(SimulationTime{}, nextTime);
    });

    SimulationTime simulationTime{};
    Command command{};
    AssertOk(_client->PollCommand(simulationTime, command, Infinite));

    // Act
    const void* dataPtr{};
    uint32_t length = inSignal.length;
    Result result = _client->Read(inSignal.id, length, &dataPtr);

    // Assert
    AssertOk(result);
    ASSERT_NE(nullptr, dataPtr);
    ASSERT_EQ(0, std::memcmp(expectedData.data(), dataPtr, dataTypeSize * inSignal.length));

    AssertOk(_client->FinishCommand());
    AssertOk(serverTask.get());
}

// --- GetCanControllers ---

TEST_F(TestCoSimClient, GetCanControllersWhenNotConnectedShouldFail) {
    // Arrange
    _client = std::make_unique<CoSimClient>();

    // Act
    uint32_t count{};
    const CanController* controllers{};
    Result result = _client->GetCanControllers(count, controllers);

    // Assert
    AssertNotConnected(result);
}

TEST_P(TestCoSimClient, GetCanControllersReturnsControllersFromConnectOk) {
    // Arrange
    auto controllers = CreateCanControllers(2);
    CoSimServerConfig config{};
    config.canControllers = controllers;
    ConnectAndStartPolling(GetParam(), config);

    // Act
    uint32_t count{};
    const CanController* result_controllers{};
    Result result = _client->GetCanControllers(count, result_controllers);

    // Assert
    AssertOk(result);
    ASSERT_EQ(2U, count);
    ASSERT_EQ(controllers[0].id, result_controllers[0].id);
    ASSERT_EQ(controllers[1].id, result_controllers[1].id);
}

TEST_P(TestCoSimClient, GetCanControllersVectorReturnsControllersFromConnectOk) {
    // Arrange
    auto controllers = CreateCanControllers(1);
    CoSimServerConfig config{};
    config.canControllers = controllers;
    ConnectAndStartPolling(GetParam(), config);

    // Act
    std::vector<CanController> result_controllers;
    Result result = _client->GetCanControllers(result_controllers);

    // Assert
    AssertOk(result);
    ASSERT_EQ(1U, result_controllers.size());
    ASSERT_EQ(controllers[0].id, result_controllers[0].id);
}

// --- GetEthControllers ---

TEST_F(TestCoSimClient, GetEthControllersWhenNotConnectedShouldFail) {
    // Arrange
    _client = std::make_unique<CoSimClient>();

    // Act
    uint32_t count{};
    const EthController* controllers{};
    Result result = _client->GetEthControllers(count, controllers);

    // Assert
    AssertNotConnected(result);
}

TEST_P(TestCoSimClient, GetEthControllersReturnsControllersFromConnectOk) {
    // Arrange
    auto controllers = CreateEthControllers(2);
    CoSimServerConfig config{};
    config.ethControllers = controllers;
    ConnectAndStartPolling(GetParam(), config);

    // Act
    uint32_t count{};
    const EthController* result_controllers{};
    Result result = _client->GetEthControllers(count, result_controllers);

    // Assert
    AssertOk(result);
    ASSERT_EQ(2U, count);
    ASSERT_EQ(controllers[0].id, result_controllers[0].id);
    ASSERT_EQ(controllers[1].id, result_controllers[1].id);
}

TEST_P(TestCoSimClient, GetEthControllersVectorReturnsControllersFromConnectOk) {
    // Arrange
    auto controllers = CreateEthControllers(1);
    CoSimServerConfig config{};
    config.ethControllers = controllers;
    ConnectAndStartPolling(GetParam(), config);

    // Act
    std::vector<EthController> result_controllers;
    Result result = _client->GetEthControllers(result_controllers);

    // Assert
    AssertOk(result);
    ASSERT_EQ(1U, result_controllers.size());
    ASSERT_EQ(controllers[0].id, result_controllers[0].id);
}

// --- GetLinControllers ---

TEST_F(TestCoSimClient, GetLinControllersWhenNotConnectedShouldFail) {
    // Arrange
    _client = std::make_unique<CoSimClient>();

    // Act
    uint32_t count{};
    const LinController* controllers{};
    Result result = _client->GetLinControllers(count, controllers);

    // Assert
    AssertNotConnected(result);
}

TEST_P(TestCoSimClient, GetLinControllersReturnsControllersFromConnectOk) {
    // Arrange
    auto controllers = CreateLinControllers(2);
    CoSimServerConfig config{};
    config.linControllers = controllers;
    ConnectAndStartPolling(GetParam(), config);

    // Act
    uint32_t count{};
    const LinController* result_controllers{};
    Result result = _client->GetLinControllers(count, result_controllers);

    // Assert
    AssertOk(result);
    ASSERT_EQ(2U, count);
    ASSERT_EQ(controllers[0].id, result_controllers[0].id);
    ASSERT_EQ(controllers[1].id, result_controllers[1].id);
}

TEST_P(TestCoSimClient, GetLinControllersVectorReturnsControllersFromConnectOk) {
    // Arrange
    auto controllers = CreateLinControllers(1);
    CoSimServerConfig config{};
    config.linControllers = controllers;
    ConnectAndStartPolling(GetParam(), config);

    // Act
    std::vector<LinController> result_controllers;
    Result result = _client->GetLinControllers(result_controllers);

    // Assert
    AssertOk(result);
    ASSERT_EQ(1U, result_controllers.size());
    ASSERT_EQ(controllers[0].id, result_controllers[0].id);
}

// --- GetFrControllers ---

TEST_F(TestCoSimClient, GetFrControllersWhenNotConnectedShouldFail) {
    // Arrange
    _client = std::make_unique<CoSimClient>();

    // Act
    uint32_t count{};
    const FrController* controllers{};
    Result result = _client->GetFrControllers(count, controllers);

    // Assert
    AssertNotConnected(result);
}

TEST_F(TestCoSimClient, GetFrControllersReturnsControllersFromConnectOk) {
    // Arrange - Remote only, Protocol v2 required for FR
    auto controllers = CreateFrControllers(2);
    CoSimServerConfig config{};
    config.frControllers = controllers;
    ConnectAndStartPolling(ConnectionKind::Remote, config);

    // Act
    uint32_t count{};
    const FrController* result_controllers{};
    Result result = _client->GetFrControllers(count, result_controllers);

    // Assert
    AssertOk(result);
    ASSERT_EQ(2U, count);
    ASSERT_EQ(controllers[0].id, result_controllers[0].id);
    ASSERT_EQ(controllers[1].id, result_controllers[1].id);
}

TEST_F(TestCoSimClient, GetFrControllersVectorReturnsControllersFromConnectOk) {
    // Arrange - Remote only, Protocol v2 required for FR
    auto controllers = CreateFrControllers(1);
    CoSimServerConfig config{};
    config.frControllers = controllers;
    ConnectAndStartPolling(ConnectionKind::Remote, config);

    // Act
    std::vector<FrController> result_controllers;
    Result result = _client->GetFrControllers(result_controllers);

    // Assert
    AssertOk(result);
    ASSERT_EQ(1U, result_controllers.size());
    ASSERT_EQ(controllers[0].id, result_controllers[0].id);
}

// --- CAN Message Validation ---

TEST_F(TestCoSimClient, TransmitCanMessageWhenNotConnectedShouldFail) {
    // Arrange
    _client = std::make_unique<CoSimClient>();
    CanMessageContainer msg{};

    // Act
    Result result = _client->Transmit(msg);

    // Assert
    AssertNotConnected(result);
}

TEST_F(TestCoSimClient, TransmitCanMessageExceedingMaxLengthShouldFail) {
    // Arrange
    auto canControllers = CreateCanControllers(1);
    CoSimServerConfig config{};
    config.canControllers = canControllers;
    ConnectAndStartPolling(ConnectionKind::Remote, config);

    CanMessageContainer msg{};
    msg.controllerId = canControllers[0].id;
    msg.length = CanMessageMaxLength + 1;

    // Act
    Result result = _client->Transmit(msg);

    // Assert
    AssertInvalidArgument(result);
}

TEST_F(TestCoSimClient, TransmitCanMessageDlcAbove8WithoutFdFlagShouldFail) {
    // Arrange
    auto canControllers = CreateCanControllers(1);
    CoSimServerConfig config{};
    config.canControllers = canControllers;
    ConnectAndStartPolling(ConnectionKind::Remote, config);

    CanMessageContainer msg{};
    msg.controllerId = canControllers[0].id;
    msg.length = 12;
    msg.flags = {};

    // Act
    Result result = _client->Transmit(msg);

    // Assert
    AssertInvalidArgument(result);
}

TEST_F(TestCoSimClient, TransmitCanMessageBitRateSwitchWithoutFdFlagShouldFail) {
    // Arrange
    auto canControllers = CreateCanControllers(1);
    CoSimServerConfig config{};
    config.canControllers = canControllers;
    ConnectAndStartPolling(ConnectionKind::Remote, config);

    CanMessageContainer msg{};
    msg.controllerId = canControllers[0].id;
    msg.length = 8;
    msg.flags = CanMessageFlags::BitRateSwitch;

    // Act
    Result result = _client->Transmit(msg);

    // Assert
    AssertInvalidArgument(result);
}

TEST_F(TestCoSimClient, TransmitCanMessageWithFdFlagAndLargeDlcShouldSucceed) {
    // Arrange
    auto canControllers = CreateCanControllers(1);
    CoSimServerConfig config{};
    config.canControllers = canControllers;
    ConnectAndStartPolling(ConnectionKind::Remote, config);

    CanMessageContainer msg{};
    msg.controllerId = canControllers[0].id;
    msg.length = 64;
    msg.flags = CanMessageFlags::FlexibleDataRateFormat;
    FillWithRandomData(msg.data.data(), msg.length);

    // Act
    Result result = _client->Transmit(msg);

    // Assert
    AssertOk(result);
}

// --- CAN Transmit + Receive ---

TEST_F(TestCoSimClient, TransmitCanMessage) {
    // Arrange — Remote only
    auto controllers = CreateCanControllers(1);
    CanMessageContainer receivedMsg{};
    CoSimServerConfig config{};
    config.canControllers = controllers;
    config.canMessageContainerReceivedCallback = [&](SimulationTime, const CanController&, const CanMessageContainer& msg) {
        receivedMsg = msg;
    };
    ConnectAndStartPolling(ConnectionKind::Remote, config);

    CanMessageContainer sentMsg{};
    FillWithRandom(sentMsg, controllers[0].id);
    AssertOk(_client->Transmit(sentMsg));

    auto serverTask = std::async(std::launch::async, [this] {
        SimulationTime nextTime{};
        return _coSimServer->Step(SimulationTime{}, nextTime);
    });

    SimulationTime simulationTime{};
    Command command{};
    AssertOk(_client->PollCommand(simulationTime, command, Infinite));
    AssertOk(_client->FinishCommand());
    AssertOk(serverTask.get());

    // Assert
    ASSERT_EQ(sentMsg.controllerId, receivedMsg.controllerId);
    ASSERT_EQ(sentMsg.length, receivedMsg.length);
    ASSERT_EQ(0, std::memcmp(sentMsg.data.data(), receivedMsg.data.data(), sentMsg.length));
}

TEST_F(TestCoSimClient, ReceiveCanMessage) {
    // Arrange — Remote only
    auto controllers = CreateCanControllers(1);
    CoSimServerConfig config{};
    config.canControllers = controllers;
    ConnectAndStartPolling(ConnectionKind::Remote, config);

    CanMessageContainer sentMsg{};
    FillWithRandom(sentMsg, controllers[0].id);
    AssertOk(_coSimServer->Transmit(sentMsg));

    auto serverTask = std::async(std::launch::async, [this] {
        SimulationTime nextTime{};
        return _coSimServer->Step(SimulationTime{}, nextTime);
    });

    SimulationTime simulationTime{};
    Command command{};
    AssertOk(_client->PollCommand(simulationTime, command, Infinite));

    // Act
    CanMessageContainer receivedMsg{};
    Result result = _client->Receive(receivedMsg);

    // Assert
    AssertOk(result);
    ASSERT_EQ(sentMsg.controllerId, receivedMsg.controllerId);
    ASSERT_EQ(sentMsg.length, receivedMsg.length);
    ASSERT_EQ(0, std::memcmp(sentMsg.data.data(), receivedMsg.data.data(), sentMsg.length));

    AssertOk(_client->FinishCommand());
    AssertOk(serverTask.get());
}

// --- ETH Transmit + Receive ---

TEST_F(TestCoSimClient, TransmitEthMessage) {
    // Arrange — Remote only
    auto controllers = CreateEthControllers(1);
    EthMessageContainer receivedMsg{};
    CoSimServerConfig config{};
    config.ethControllers = controllers;
    config.ethMessageContainerReceivedCallback = [&](SimulationTime, const EthController&, const EthMessageContainer& msg) {
        receivedMsg = msg;
    };
    ConnectAndStartPolling(ConnectionKind::Remote, config);

    EthMessageContainer sentMsg{};
    FillWithRandom(sentMsg, controllers[0].id);
    AssertOk(_client->Transmit(sentMsg));

    auto serverTask = std::async(std::launch::async, [this] {
        SimulationTime nextTime{};
        return _coSimServer->Step(SimulationTime{}, nextTime);
    });

    SimulationTime simulationTime{};
    Command command{};
    AssertOk(_client->PollCommand(simulationTime, command, Infinite));
    AssertOk(_client->FinishCommand());
    AssertOk(serverTask.get());

    // Assert
    ASSERT_EQ(sentMsg.controllerId, receivedMsg.controllerId);
    ASSERT_EQ(sentMsg.length, receivedMsg.length);
    ASSERT_EQ(0, std::memcmp(sentMsg.data.data(), receivedMsg.data.data(), sentMsg.length));
}

TEST_F(TestCoSimClient, ReceiveEthMessage) {
    // Arrange — Remote only
    auto controllers = CreateEthControllers(1);
    CoSimServerConfig config{};
    config.ethControllers = controllers;
    ConnectAndStartPolling(ConnectionKind::Remote, config);

    EthMessageContainer sentMsg{};
    FillWithRandom(sentMsg, controllers[0].id);
    AssertOk(_coSimServer->Transmit(sentMsg));

    auto serverTask = std::async(std::launch::async, [this] {
        SimulationTime nextTime{};
        return _coSimServer->Step(SimulationTime{}, nextTime);
    });

    SimulationTime simulationTime{};
    Command command{};
    AssertOk(_client->PollCommand(simulationTime, command, Infinite));

    // Act
    EthMessageContainer receivedMsg{};
    Result result = _client->Receive(receivedMsg);

    // Assert
    AssertOk(result);
    ASSERT_EQ(sentMsg.controllerId, receivedMsg.controllerId);
    ASSERT_EQ(sentMsg.length, receivedMsg.length);
    ASSERT_EQ(0, std::memcmp(sentMsg.data.data(), receivedMsg.data.data(), sentMsg.length));

    AssertOk(_client->FinishCommand());
    AssertOk(serverTask.get());
}

// --- LIN Transmit + Receive ---

TEST_F(TestCoSimClient, TransmitLinMessage) {
    // Arrange — Remote only
    auto controllers = CreateLinControllers(1);
    LinMessageContainer receivedMsg{};
    CoSimServerConfig config{};
    config.linControllers = controllers;
    config.linMessageContainerReceivedCallback = [&](SimulationTime, const LinController&, const LinMessageContainer& msg) {
        receivedMsg = msg;
    };
    ConnectAndStartPolling(ConnectionKind::Remote, config);

    LinMessageContainer sentMsg{};
    FillWithRandom(sentMsg, controllers[0].id);
    AssertOk(_client->Transmit(sentMsg));

    auto serverTask = std::async(std::launch::async, [this] {
        SimulationTime nextTime{};
        return _coSimServer->Step(SimulationTime{}, nextTime);
    });

    SimulationTime simulationTime{};
    Command command{};
    AssertOk(_client->PollCommand(simulationTime, command, Infinite));
    AssertOk(_client->FinishCommand());
    AssertOk(serverTask.get());

    // Assert
    ASSERT_EQ(sentMsg.controllerId, receivedMsg.controllerId);
    ASSERT_EQ(sentMsg.length, receivedMsg.length);
    ASSERT_EQ(0, std::memcmp(sentMsg.data.data(), receivedMsg.data.data(), sentMsg.length));
}

TEST_F(TestCoSimClient, ReceiveLinMessage) {
    // Arrange — Remote only
    auto controllers = CreateLinControllers(1);
    CoSimServerConfig config{};
    config.linControllers = controllers;
    ConnectAndStartPolling(ConnectionKind::Remote, config);

    LinMessageContainer sentMsg{};
    FillWithRandom(sentMsg, controllers[0].id);
    AssertOk(_coSimServer->Transmit(sentMsg));

    auto serverTask = std::async(std::launch::async, [this] {
        SimulationTime nextTime{};
        return _coSimServer->Step(SimulationTime{}, nextTime);
    });

    SimulationTime simulationTime{};
    Command command{};
    AssertOk(_client->PollCommand(simulationTime, command, Infinite));

    // Act
    LinMessageContainer receivedMsg{};
    Result result = _client->Receive(receivedMsg);

    // Assert
    AssertOk(result);
    ASSERT_EQ(sentMsg.controllerId, receivedMsg.controllerId);
    ASSERT_EQ(sentMsg.length, receivedMsg.length);
    ASSERT_EQ(0, std::memcmp(sentMsg.data.data(), receivedMsg.data.data(), sentMsg.length));

    AssertOk(_client->FinishCommand());
    AssertOk(serverTask.get());
}

// --- FR Transmit + Receive (Protocol v2, Remote only) ---

TEST_F(TestCoSimClient, TransmitFrMessage) {
    // Arrange — Remote only
    auto controllers = CreateFrControllers(1);
    FrMessageContainer receivedMsg{};
    CoSimServerConfig config{};
    config.frControllers = controllers;
    config.frMessageContainerReceivedCallback = [&](SimulationTime, const FrController&, const FrMessageContainer& msg) {
        receivedMsg = msg;
    };
    ConnectAndStartPolling(ConnectionKind::Remote, config);

    FrMessageContainer sentMsg{};
    FillWithRandom(sentMsg, controllers[0].id);
    AssertOk(_client->Transmit(sentMsg));

    auto serverTask = std::async(std::launch::async, [this] {
        SimulationTime nextTime{};
        return _coSimServer->Step(SimulationTime{}, nextTime);
    });

    SimulationTime simulationTime{};
    Command command{};
    AssertOk(_client->PollCommand(simulationTime, command, Infinite));
    AssertOk(_client->FinishCommand());
    AssertOk(serverTask.get());

    // Assert
    ASSERT_EQ(sentMsg.controllerId, receivedMsg.controllerId);
    ASSERT_EQ(sentMsg.length, receivedMsg.length);
    ASSERT_EQ(0, std::memcmp(sentMsg.data.data(), receivedMsg.data.data(), sentMsg.length));
}

TEST_F(TestCoSimClient, ReceiveFrMessage) {
    // Arrange — Remote only
    auto controllers = CreateFrControllers(1);
    CoSimServerConfig config{};
    config.frControllers = controllers;
    ConnectAndStartPolling(ConnectionKind::Remote, config);

    FrMessageContainer sentMsg{};
    FillWithRandom(sentMsg, controllers[0].id);
    AssertOk(_coSimServer->Transmit(sentMsg));

    auto serverTask = std::async(std::launch::async, [this] {
        SimulationTime nextTime{};
        return _coSimServer->Step(SimulationTime{}, nextTime);
    });

    SimulationTime simulationTime{};
    Command command{};
    AssertOk(_client->PollCommand(simulationTime, command, Infinite));

    // Act
    FrMessageContainer receivedMsg{};
    Result result = _client->Receive(receivedMsg);

    // Assert
    AssertOk(result);
    ASSERT_EQ(sentMsg.controllerId, receivedMsg.controllerId);
    ASSERT_EQ(sentMsg.length, receivedMsg.length);
    ASSERT_EQ(0, std::memcmp(sentMsg.data.data(), receivedMsg.data.data(), sentMsg.length));

    AssertOk(_client->FinishCommand());
    AssertOk(serverTask.get());
}

}  // namespace
