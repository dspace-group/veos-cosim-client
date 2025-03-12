// Copyright dSPACE GmbH. All rights reserved.

#include <memory>
#include <string>

#include "BusBuffer.h"
#include "Channel.h"
#include "DsVeosCoSim/CoSimTypes.h"
#include "Generator.h"
#include "Helper.h"
#include "IoBuffer.h"
#include "Protocol.h"
#include "TestHelper.h"

using namespace DsVeosCoSim;

namespace {

class TestProtocol : public testing::TestWithParam<ConnectionKind> {
protected:
    std::unique_ptr<Channel> _senderChannel;
    std::unique_ptr<Channel> _receiverChannel;

    void CustomSetUp(const ConnectionKind connectionKind) {
        if (connectionKind == ConnectionKind::Remote) {
            std::unique_ptr<ChannelServer> server = CreateTcpChannelServer(0, true);
            const uint16_t port = server->GetLocalPort();

            _senderChannel = ConnectToTcpChannel("127.0.0.1", port);
            _receiverChannel = Accept(*server);
        } else {
#ifdef _WIN32
            const std::string name = GenerateString("LocalChannel名前");
            std::unique_ptr<ChannelServer> server = CreateLocalChannelServer(name);

            _senderChannel = ConnectToLocalChannel(name);
            _receiverChannel = Accept(*server);
#else
            const std::string name = GenerateString("UdsChannel名前");
            std::unique_ptr<ChannelServer> server = CreateUdsChannelServer(name);

            _senderChannel = ConnectToUdsChannel(name);
            _receiverChannel = Accept(*server);
#endif
        }
    }

    void TearDown() override {
        _senderChannel->Disconnect();
        _receiverChannel->Disconnect();
    }

    void AssertFrame(FrameKind expected) const {
        FrameKind frameKind{};
        ASSERT_TRUE(Protocol::ReceiveHeader(_receiverChannel->GetReader(), frameKind));
        ASSERT_EQ(static_cast<int32_t>(expected), static_cast<int32_t>(frameKind));
    }
};

INSTANTIATE_TEST_SUITE_P(,
                         TestProtocol,
                         testing::Values(ConnectionKind::Local, ConnectionKind::Remote),
                         [](const testing::TestParamInfo<ConnectionKind>& info) {
                             return ToString(info.param);
                         });

TEST_P(TestProtocol, SendAndReceiveOk) {
    // Arrange
    CustomSetUp(GetParam());

    // Act
    ASSERT_TRUE(Protocol::SendOk(_senderChannel->GetWriter()));

    // Assert
    AssertFrame(FrameKind::Ok);
}

TEST_P(TestProtocol, SendAndReceiveError) {
    // Arrange
    CustomSetUp(GetParam());

    const std::string sendErrorMessage = GenerateString("Errorメッセージ");

    // Act
    ASSERT_TRUE(Protocol::SendError(_senderChannel->GetWriter(), sendErrorMessage));

    // Assert
    AssertFrame(FrameKind::Error);

    std::string receiveErrorMessage;
    ASSERT_TRUE(Protocol::ReadError(_receiverChannel->GetReader(), receiveErrorMessage));
    AssertEq(sendErrorMessage, receiveErrorMessage);
}

TEST_P(TestProtocol, SendAndReceivePing) {
    // Arrange
    CustomSetUp(GetParam());

    // Act
    ASSERT_TRUE(Protocol::SendPing(_senderChannel->GetWriter()));

    // Assert
    AssertFrame(FrameKind::Ping);
}

TEST_P(TestProtocol, SendAndReceivePingOk) {
    // Arrange
    CustomSetUp(GetParam());

    const auto sendCommand = static_cast<Command>(GenerateU32());

    // Act
    ASSERT_TRUE(Protocol::SendPingOk(_senderChannel->GetWriter(), sendCommand));

    // Assert
    AssertFrame(FrameKind::PingOk);

    Command receiveCommand{};
    ASSERT_TRUE(Protocol::ReadPingOk(_receiverChannel->GetReader(), receiveCommand));
    ASSERT_EQ(sendCommand, receiveCommand);
}

TEST_P(TestProtocol, SendAndReceiveConnect) {
    // Arrange
    CustomSetUp(GetParam());

    const uint32_t sendVersion = GenerateU32();
    constexpr Mode sendMode{};
    const std::string sendServerName = GenerateString("Server名前");
    const std::string sendClientName = GenerateString("Client名前");

    // Act
    ASSERT_TRUE(
        Protocol::SendConnect(_senderChannel->GetWriter(), sendVersion, sendMode, sendServerName, sendClientName));

    // Assert
    AssertFrame(FrameKind::Connect);

    uint32_t receiveVersion{};
    Mode receiveMode{};
    std::string receiveServerName;
    std::string receiveClientName;
    ASSERT_TRUE(Protocol::ReadConnect(_receiverChannel->GetReader(),
                                      receiveVersion,
                                      receiveMode,
                                      receiveServerName,
                                      receiveClientName));
    ASSERT_EQ(sendVersion, receiveVersion);
    ASSERT_EQ(static_cast<int32_t>(sendMode), static_cast<int32_t>(receiveMode));
    AssertEq(sendServerName, receiveServerName);
    AssertEq(sendClientName, receiveClientName);
}

TEST_P(TestProtocol, SendAndReceiveConnectOk) {
    // Arrange
    CustomSetUp(GetParam());

    const uint32_t sendProtocolVersion = GenerateU32();
    constexpr Mode sendMode{};
    const SimulationTime sendStepSize = GenerateSimulationTime();
    constexpr SimulationState sendSimulationState{};
    const std::vector<IoSignalContainer> sendIncomingSignals = CreateSignals(2);
    const std::vector<IoSignalContainer> sendOutgoingSignals = CreateSignals(3);
    const std::vector<CanControllerContainer> sendCanControllers = CreateCanControllers(4);
    const std::vector<EthControllerContainer> sendEthControllers = CreateEthControllers(5);
    const std::vector<LinControllerContainer> sendLinControllers = CreateLinControllers(6);

    // Act
    ASSERT_TRUE(Protocol::SendConnectOk(_senderChannel->GetWriter(),
                                        sendProtocolVersion,
                                        sendMode,
                                        sendStepSize,
                                        sendSimulationState,
                                        sendIncomingSignals,
                                        sendOutgoingSignals,
                                        sendCanControllers,
                                        sendEthControllers,
                                        sendLinControllers));

    // Assert
    AssertFrame(FrameKind::ConnectOk);

    uint32_t receiveProtocolVersion{};
    Mode receiveMode{};
    SimulationTime receiveStepSize{};
    SimulationState receiveSimulationState{};
    std::vector<IoSignalContainer> receiveIncomingSignals;
    std::vector<IoSignalContainer> receiveOutgoingSignals;
    std::vector<CanControllerContainer> receiveCanControllers;
    std::vector<EthControllerContainer> receiveEthControllers;
    std::vector<LinControllerContainer> receiveLinControllers;
    ASSERT_TRUE(Protocol::ReadConnectOk(_receiverChannel->GetReader(),
                                        receiveProtocolVersion,
                                        receiveMode,
                                        receiveStepSize,
                                        receiveSimulationState,
                                        receiveIncomingSignals,
                                        receiveOutgoingSignals,
                                        receiveCanControllers,
                                        receiveEthControllers,
                                        receiveLinControllers));
    ASSERT_EQ(sendProtocolVersion, receiveProtocolVersion);
    ASSERT_EQ(static_cast<int32_t>(sendMode), static_cast<int32_t>(receiveMode));
    ASSERT_EQ(sendStepSize, receiveStepSize);
    AssertEq<IoSignalContainer, IoSignal>(sendIncomingSignals, receiveIncomingSignals);
    AssertEq<IoSignalContainer, IoSignal>(sendOutgoingSignals, receiveOutgoingSignals);
    AssertEq<CanControllerContainer, CanController>(sendCanControllers, receiveCanControllers);
    AssertEq<EthControllerContainer, EthController>(sendEthControllers, receiveEthControllers);
    AssertEq<LinControllerContainer, LinController>(sendLinControllers, receiveLinControllers);
}

TEST_P(TestProtocol, SendAndReceiveStart) {
    // Arrange
    CustomSetUp(GetParam());

    const SimulationTime sendSimulationTime = GenerateSimulationTime();

    // Act
    ASSERT_TRUE(Protocol::SendStart(_senderChannel->GetWriter(), sendSimulationTime));

    // Assert
    AssertFrame(FrameKind::Start);

    SimulationTime receiveSimulationTime{};
    ASSERT_TRUE(Protocol::ReadStart(_receiverChannel->GetReader(), receiveSimulationTime));
    ASSERT_EQ(sendSimulationTime, receiveSimulationTime);
}

TEST_P(TestProtocol, SendAndReceiveStop) {
    // Arrange
    CustomSetUp(GetParam());

    const SimulationTime sendSimulationTime = GenerateSimulationTime();

    // Act
    ASSERT_TRUE(Protocol::SendStop(_senderChannel->GetWriter(), sendSimulationTime));

    // Assert
    AssertFrame(FrameKind::Stop);

    SimulationTime receiveSimulationTime{};
    ASSERT_TRUE(Protocol::ReadStop(_receiverChannel->GetReader(), receiveSimulationTime));
    ASSERT_EQ(sendSimulationTime, receiveSimulationTime);
}

TEST_P(TestProtocol, SendAndReceiveTerminate) {
    // Arrange
    CustomSetUp(GetParam());

    const SimulationTime sendSimulationTime = GenerateSimulationTime();
    const TerminateReason sendTerminateReason = GenerateRandom(TerminateReason::Finished, TerminateReason::Error);

    // Act
    ASSERT_TRUE(Protocol::SendTerminate(_senderChannel->GetWriter(), sendSimulationTime, sendTerminateReason));

    // Assert
    AssertFrame(FrameKind::Terminate);

    SimulationTime receiveSimulationTime{};
    TerminateReason receiveTerminateReason{};
    ASSERT_TRUE(Protocol::ReadTerminate(_receiverChannel->GetReader(), receiveSimulationTime, receiveTerminateReason));
    ASSERT_EQ(sendSimulationTime, receiveSimulationTime);
    ASSERT_EQ(sendTerminateReason, receiveTerminateReason);
}

TEST_P(TestProtocol, SendAndReceivePause) {
    // Arrange
    CustomSetUp(GetParam());

    const SimulationTime sendSimulationTime = GenerateSimulationTime();

    // Act
    ASSERT_TRUE(Protocol::SendPause(_senderChannel->GetWriter(), sendSimulationTime));

    // Assert
    AssertFrame(FrameKind::Pause);

    SimulationTime receiveSimulationTime{};
    ASSERT_TRUE(Protocol::ReadPause(_receiverChannel->GetReader(), receiveSimulationTime));
    ASSERT_EQ(sendSimulationTime, receiveSimulationTime);
}

TEST_P(TestProtocol, SendAndReceiveContinue) {
    // Arrange
    CustomSetUp(GetParam());

    const SimulationTime sendSimulationTime = GenerateSimulationTime();

    // Act
    ASSERT_TRUE(Protocol::SendContinue(_senderChannel->GetWriter(), sendSimulationTime));

    // Assert
    AssertFrame(FrameKind::Continue);

    SimulationTime receiveSimulationTime{};
    ASSERT_TRUE(Protocol::ReadContinue(_receiverChannel->GetReader(), receiveSimulationTime));
    ASSERT_EQ(sendSimulationTime, receiveSimulationTime);
}

TEST_P(TestProtocol, SendAndReceiveStep) {
    // Arrange
    const ConnectionKind connectionKind = GetParam();
    CustomSetUp(connectionKind);

    const SimulationTime sendSimulationTime = GenerateSimulationTime();

    const std::string ioBufferName = GenerateString("IoBuffer名前");
    const std::unique_ptr<IoBuffer> clientIoBuffer =
        CreateIoBuffer(CoSimType::Client, connectionKind, ioBufferName, {}, {});
    const std::unique_ptr<IoBuffer> serverIoBuffer =
        CreateIoBuffer(CoSimType::Server, connectionKind, ioBufferName, {}, {});

    const std::string busBufferName = GenerateString("BusBuffer名前");
    const std::unique_ptr<BusBuffer> clientBusBuffer =
        CreateBusBuffer(CoSimType::Client, connectionKind, busBufferName, {}, {}, {});
    const std::unique_ptr<BusBuffer> serverBusBuffer =
        CreateBusBuffer(CoSimType::Server, connectionKind, busBufferName, {}, {}, {});

    // Act
    ASSERT_TRUE(Protocol::SendStep(_senderChannel->GetWriter(), sendSimulationTime, *clientIoBuffer, *clientBusBuffer));

    // Assert
    AssertFrame(FrameKind::Step);

    SimulationTime receiveSimulationTime{};
    ASSERT_TRUE(Protocol::ReadStep(_receiverChannel->GetReader(),
                                   receiveSimulationTime,
                                   *serverIoBuffer,
                                   *serverBusBuffer,
                                   {}));
    ASSERT_EQ(sendSimulationTime, receiveSimulationTime);
}

TEST_P(TestProtocol, SendAndReceiveStepOk) {
    // Arrange
    const ConnectionKind connectionKind = GetParam();
    CustomSetUp(connectionKind);

    const SimulationTime sendSimulationTime = GenerateSimulationTime();

    const auto sendCommand = static_cast<Command>(GenerateU32());

    const std::string ioBufferName = GenerateString("IoBuffer名前");
    const std::unique_ptr<IoBuffer> clientIoBuffer =
        CreateIoBuffer(CoSimType::Client, connectionKind, ioBufferName, {}, {});
    const std::unique_ptr<IoBuffer> serverIoBuffer =
        CreateIoBuffer(CoSimType::Server, connectionKind, ioBufferName, {}, {});

    const std::string busBufferName = GenerateString("BusBuffer名前");
    const std::unique_ptr<BusBuffer> clientBusBuffer =
        CreateBusBuffer(CoSimType::Client, connectionKind, busBufferName, {}, {}, {});
    const std::unique_ptr<BusBuffer> serverBusBuffer =
        CreateBusBuffer(CoSimType::Server, connectionKind, busBufferName, {}, {}, {});

    // Act
    ASSERT_TRUE(Protocol::SendStepOk(_senderChannel->GetWriter(),
                                     sendSimulationTime,
                                     sendCommand,
                                     *clientIoBuffer,
                                     *clientBusBuffer));

    // Assert
    AssertFrame(FrameKind::StepOk);

    SimulationTime receiveSimulationTime{};
    Command receiveCommand{};
    ASSERT_TRUE(Protocol::ReadStepOk(_receiverChannel->GetReader(),
                                     receiveSimulationTime,
                                     receiveCommand,
                                     *serverIoBuffer,
                                     *serverBusBuffer,
                                     {}));
    ASSERT_EQ(sendSimulationTime, receiveSimulationTime);
}

TEST_P(TestProtocol, SendAndReceiveGetPort) {
    // Arrange
    CustomSetUp(GetParam());

    const std::string sendServerName = GenerateString("Server名前");

    // Act
    ASSERT_TRUE(Protocol::SendGetPort(_senderChannel->GetWriter(), sendServerName));

    // Assert
    AssertFrame(FrameKind::GetPort);

    std::string receiveServerName;
    ASSERT_TRUE(Protocol::ReadGetPort(_receiverChannel->GetReader(), receiveServerName));
    AssertEq(sendServerName, receiveServerName);
}

TEST_P(TestProtocol, SendAndReceiveGetPortOk) {
    // Arrange
    CustomSetUp(GetParam());

    const uint16_t sendPort = GenerateU16();

    // Act
    ASSERT_TRUE(Protocol::SendGetPortOk(_senderChannel->GetWriter(), sendPort));

    // Assert
    AssertFrame(FrameKind::GetPortOk);

    uint16_t receivePort{};
    ASSERT_TRUE(Protocol::ReadGetPortOk(_receiverChannel->GetReader(), receivePort));
    ASSERT_EQ(sendPort, receivePort);
}

TEST_P(TestProtocol, SendAndReceiveSetPort) {
    // Arrange
    CustomSetUp(GetParam());

    const std::string sendServerName = GenerateString("Server名前");
    const uint16_t sendPort = GenerateU16();

    // Act
    ASSERT_TRUE(Protocol::SendSetPort(_senderChannel->GetWriter(), sendServerName, sendPort));

    // Assert
    AssertFrame(FrameKind::SetPort);

    std::string receiveServerName;
    uint16_t receivePort{};
    ASSERT_TRUE(Protocol::ReadSetPort(_receiverChannel->GetReader(), receiveServerName, receivePort));
    AssertEq(sendServerName, receiveServerName);
    ASSERT_EQ(sendPort, receivePort);
}

TEST_P(TestProtocol, SendAndReceiveUnsetPort) {
    // Arrange
    CustomSetUp(GetParam());

    const std::string sendServerName = GenerateString("Server名前");

    // Act
    ASSERT_TRUE(Protocol::SendUnsetPort(_senderChannel->GetWriter(), sendServerName));

    // Assert
    AssertFrame(FrameKind::UnsetPort);

    std::string receiveServerName;
    ASSERT_TRUE(Protocol::ReadUnsetPort(_receiverChannel->GetReader(), receiveServerName));
    AssertEq(sendServerName, receiveServerName);
}

}  // namespace
