// Copyright dSPACE GmbH. All rights reserved.

#include <string>

#include "Communication.h"
#include "Generator.h"
#include "Logger.h"
#include "Protocol.h"
#include "TestHelper.h"

using namespace DsVeosCoSim;

class TestProtocol : public testing::Test {
protected:
    Channel _senderChannel;
    Channel _receiverChannel;

    void SetUp() override {
        SetLogCallback(OnLogCallback);

        Server server;
        uint16_t port = 0;
        ASSERT_OK(server.Start(port, true));

        ASSERT_OK(ConnectToServer("127.0.0.1", port, 0, _senderChannel));

        ASSERT_OK(server.Accept(_receiverChannel));
    }

    void TearDown() override {
        _senderChannel.Disconnect();
        _receiverChannel.Disconnect();
    }

    void AssertFrame(FrameKind expected) {
        FrameKind frameKind = FrameKind::Unknown;
        ASSERT_OK(Protocol::ReceiveHeader(_receiverChannel, frameKind));
        ASSERT_EQ((int)expected, (int)frameKind);
    }
};

TEST_F(TestProtocol, SendAndReceiveOk) {
    // Arrange

    // Act
    ASSERT_OK(Protocol::SendOk(_senderChannel));

    // Assert
    AssertFrame(FrameKind::Ok);
}

TEST_F(TestProtocol, SendAndReceivePing) {
    // Arrange

    // Act
    ASSERT_OK(Protocol::SendPing(_senderChannel));

    // Assert
    AssertFrame(FrameKind::Ping);
}

TEST_F(TestProtocol, SendAndReceiveError) {
    // Arrange
    const std::string sendErrorMessage = GenerateString("ErrorMessage");

    // Act
    ASSERT_OK(Protocol::SendError(_senderChannel, sendErrorMessage));

    // Assert
    AssertFrame(FrameKind::Error);

    std::string receiveErrorMessage;
    ASSERT_OK(Protocol::ReadError(_receiverChannel, receiveErrorMessage));
    AssertEq(sendErrorMessage, receiveErrorMessage);
}

TEST_F(TestProtocol, SendAndReceiveConnect) {
    // Arrange
    const uint32_t sendVersion = GenerateU32();
    const Mode sendMode = GenerateRandom(Mode::None, Mode::Responder);
    const std::string sendServerName = GenerateString("ServerName");
    const std::string sendClientName = GenerateString("ClientName");

    // Act
    ASSERT_OK(Protocol::SendConnect(_senderChannel, sendVersion, sendMode, sendServerName, sendClientName));

    // Assert
    AssertFrame(FrameKind::Connect);

    uint32_t receiveVersion{};
    Mode receiveMode{};
    std::string receiveServerName;
    std::string receiveClientName;
    ASSERT_OK(Protocol::ReadConnect(_receiverChannel, receiveVersion, receiveMode, receiveServerName, receiveClientName));
    ASSERT_EQ(sendVersion, receiveVersion);
    ASSERT_EQ((int)sendMode, (int)receiveMode);
    AssertEq(sendServerName, receiveServerName);
    AssertEq(sendClientName, receiveClientName);
}

TEST_F(TestProtocol, SendAndReceiveAccepted) {
    // Arrange
    const uint32_t sendProtocolVersion = GenerateU32();
    const Mode sendMode = GenerateRandom(Mode::None, Mode::Responder);
    const std::vector<IoSignalContainer> sendIncomingSignals = CreateSignals(2);
    const std::vector<IoSignalContainer> sendOutgoingSignals = CreateSignals(3);
    const std::vector<CanControllerContainer> sendCanControllers = CreateCanControllers(4);
    const std::vector<EthControllerContainer> sendEthControllers = CreateEthControllers(5);
    const std::vector<LinControllerContainer> sendLinControllers = CreateLinControllers(6);

    // Act
    ASSERT_OK(Protocol::SendAccepted(_senderChannel,
                                     sendProtocolVersion,
                                     sendMode,
                                     sendIncomingSignals,
                                     sendOutgoingSignals,
                                     sendCanControllers,
                                     sendEthControllers,
                                     sendLinControllers));

    // Assert
    AssertFrame(FrameKind::Accepted);

    uint32_t receiveProtocolVersion{};
    Mode receiveMode{};
    std::vector<IoSignalContainer> receiveIncomingSignals;
    std::vector<IoSignalContainer> receiveOutgoingSignals;
    std::vector<CanControllerContainer> receiveCanControllers;
    std::vector<EthControllerContainer> receiveEthControllers;
    std::vector<LinControllerContainer> receiveLinControllers;
    ASSERT_OK(Protocol::ReadAccepted(_receiverChannel,
                                     receiveProtocolVersion,
                                     receiveMode,
                                     receiveIncomingSignals,
                                     receiveOutgoingSignals,
                                     receiveCanControllers,
                                     receiveEthControllers,
                                     receiveLinControllers));
    ASSERT_EQ(sendProtocolVersion, receiveProtocolVersion);
    ASSERT_EQ((int)sendMode, (int)receiveMode);
    AssertEq(sendIncomingSignals, receiveIncomingSignals);
    AssertEq(sendOutgoingSignals, receiveOutgoingSignals);
    AssertEq(sendCanControllers, receiveCanControllers);
    AssertEq(sendEthControllers, receiveEthControllers);
    AssertEq(sendLinControllers, receiveLinControllers);
}

TEST_F(TestProtocol, SendAndReceiveStart) {
    // Arrange
    const SimulationTime sendSimulationTime = GenerateI64();

    // Act
    ASSERT_OK(Protocol::SendStart(_senderChannel, sendSimulationTime));

    // Assert
    AssertFrame(FrameKind::Start);

    SimulationTime receiveSimulationTime{};
    ASSERT_OK(Protocol::ReadStart(_receiverChannel, receiveSimulationTime));
    ASSERT_EQ(sendSimulationTime, receiveSimulationTime);
}

TEST_F(TestProtocol, SendAndReceiveStop) {
    // Arrange
    const SimulationTime sendSimulationTime = GenerateI64();

    // Act
    ASSERT_OK(Protocol::SendStop(_senderChannel, sendSimulationTime));

    // Assert
    AssertFrame(FrameKind::Stop);

    SimulationTime receiveSimulationTime{};
    ASSERT_OK(Protocol::ReadStop(_receiverChannel, receiveSimulationTime));
    ASSERT_EQ(sendSimulationTime, receiveSimulationTime);
}

TEST_F(TestProtocol, SendAndReceiveTerminate) {
    // Arrange
    const SimulationTime sendSimulationTime = GenerateI64();
    const TerminateReason sendTerminateReason = GenerateRandom(TerminateReason::Finished, TerminateReason::Error);

    // Act
    ASSERT_OK(Protocol::SendTerminate(_senderChannel, sendSimulationTime, sendTerminateReason));

    // Assert
    AssertFrame(FrameKind::Terminate);

    SimulationTime receiveSimulationTime{};
    TerminateReason receiveTerminateReason{};
    ASSERT_OK(Protocol::ReadTerminate(_receiverChannel, receiveSimulationTime, receiveTerminateReason));
    ASSERT_EQ(sendSimulationTime, receiveSimulationTime);
    ASSERT_EQ(sendTerminateReason, receiveTerminateReason);
}

TEST_F(TestProtocol, SendAndReceivePause) {
    // Arrange
    const SimulationTime sendSimulationTime = GenerateI64();

    // Act
    ASSERT_OK(Protocol::SendPause(_senderChannel, sendSimulationTime));

    // Assert
    AssertFrame(FrameKind::Pause);

    SimulationTime receiveSimulationTime{};
    ASSERT_OK(Protocol::ReadPause(_receiverChannel, receiveSimulationTime));
    ASSERT_EQ(sendSimulationTime, receiveSimulationTime);
}

TEST_F(TestProtocol, SendAndReceiveContinue) {
    // Arrange
    const SimulationTime sendSimulationTime = GenerateI64();

    // Act
    ASSERT_OK(Protocol::SendContinue(_senderChannel, sendSimulationTime));

    // Assert
    AssertFrame(FrameKind::Continue);

    SimulationTime receiveSimulationTime{};
    ASSERT_OK(Protocol::ReadContinue(_receiverChannel, receiveSimulationTime));
    ASSERT_EQ(sendSimulationTime, receiveSimulationTime);
}

TEST_F(TestProtocol, SendAndReceiveGetPort) {
    // Arrange
    const std::string sendServerName = GenerateString("ServerName");

    // Act
    ASSERT_OK(Protocol::SendGetPort(_senderChannel, sendServerName));

    // Assert
    AssertFrame(FrameKind::GetPort);

    std::string receiveServerName;
    ASSERT_OK(Protocol::ReadGetPort(_receiverChannel, receiveServerName));
    AssertEq(sendServerName, receiveServerName);
}

TEST_F(TestProtocol, SendAndReceiveGetPortResponse) {
    // Arrange
    const uint16_t sendPort = GenerateU16();

    // Act
    ASSERT_OK(Protocol::SendGetPortResponse(_senderChannel, sendPort));

    // Assert
    AssertFrame(FrameKind::GetPortResponse);

    uint16_t receivePort{};
    ASSERT_OK(Protocol::ReadGetPortResponse(_receiverChannel, receivePort));
    ASSERT_EQ(sendPort, receivePort);
}

TEST_F(TestProtocol, SendAndReceiveSetPort) {
    // Arrange
    const std::string sendServerName = GenerateString("ServerName");
    const uint16_t sendPort = GenerateU16();

    // Act
    ASSERT_OK(Protocol::SendSetPort(_senderChannel, sendServerName, sendPort));

    // Assert
    AssertFrame(FrameKind::SetPort);

    std::string receiveServerName;
    uint16_t receivePort{};
    ASSERT_OK(Protocol::ReadSetPort(_receiverChannel, receiveServerName, receivePort));
    AssertEq(sendServerName, receiveServerName);
    ASSERT_EQ(sendPort, receivePort);
}

TEST_F(TestProtocol, SendAndReceiveUnsetPort) {
    // Arrange
    const std::string sendServerName = GenerateString("ServerName");

    // Act
    ASSERT_OK(Protocol::SendUnsetPort(_senderChannel, sendServerName));

    // Assert
    AssertFrame(FrameKind::UnsetPort);

    std::string receiveServerName;
    ASSERT_OK(Protocol::ReadUnsetPort(_receiverChannel, receiveServerName));
    AssertEq(sendServerName, receiveServerName);
}
