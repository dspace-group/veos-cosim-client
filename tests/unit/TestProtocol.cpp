// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <memory>
#include <string>

#include <fmt/format.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Channel.hpp"
#include "CoSimTypes.hpp"
#include "Helper.hpp"
#include "Protocol.hpp"
#include "TestHelper.hpp"

using namespace DsVeosCoSim;
using namespace testing;

namespace {

class TestProtocol : public testing::TestWithParam<ConnectionKind> {
protected:
    std::unique_ptr<Channel> _senderChannel;
    std::unique_ptr<Channel> _receiverChannel;

    std::unique_ptr<IProtocol> _protocol;

    void SetUp() override {
        CustomSetUp(GetParam());
    }

    void CustomSetUp(ConnectionKind connectionKind) {
        if (connectionKind == ConnectionKind::Remote) {
            std::unique_ptr<ChannelServer> server;
            AssertOk(CreateTcpChannelServer(0, true, server));
            uint16_t port = server->GetLocalPort();

            AssertOk(TryConnectToTcpChannel("127.0.0.1", port, 0, DefaultTimeoutInMilliseconds, _senderChannel));
            AssertOk(server->TryAccept(_receiverChannel));
        } else {
            std::string name = GenerateString("LocalChannel名前");
            std::unique_ptr<ChannelServer> server;
            AssertOk(CreateLocalChannelServer(name, server));

            AssertOk(TryConnectToLocalChannel(name, _senderChannel));
            AssertOk(server->TryAccept(_receiverChannel));
        }

        AssertOk(CreateProtocol(ProtocolVersionLatest, _protocol));
    }

    void TearDown() override {
        _senderChannel->Disconnect();
        _receiverChannel->Disconnect();
    }

    void AssertFrame(FrameKind expected) const {
        FrameKind frameKind{};
        AssertOk(_protocol->ReceiveHeader(_receiverChannel->GetReader(), frameKind));
        ASSERT_EQ(expected, frameKind);
    }
};

INSTANTIATE_TEST_SUITE_P(,
                         TestProtocol,
                         testing::Values(ConnectionKind::Local, ConnectionKind::Remote),
                         [](const testing::TestParamInfo<ConnectionKind>& info) {
                             return fmt::format("{}", info.param);
                         });

TEST_P(TestProtocol, SendAndReceiveSize) {
    // Arrange
    size_t sendSize = GenerateU32();

    // Act
    AssertOk(_protocol->WriteSize(_senderChannel->GetWriter(), sendSize));
    AssertOk(_senderChannel->GetWriter().EndWrite());

    // Assert
    size_t receiveSize{};
    AssertOk(_protocol->ReadSize(_receiverChannel->GetReader(), receiveSize));
    _receiverChannel->GetReader().EndRead();
    ASSERT_EQ(sendSize, receiveSize);
}

TEST_P(TestProtocol, SendAndReceiveLength) {
    // Arrange
    uint32_t sendLength = GenerateU32();

    // Act
    AssertOk(_protocol->WriteLength(_senderChannel->GetWriter(), sendLength));
    AssertOk(_senderChannel->GetWriter().EndWrite());

    // Assert
    uint32_t receiveLength{};
    AssertOk(_protocol->ReadLength(_receiverChannel->GetReader(), receiveLength));
    _receiverChannel->GetReader().EndRead();
    ASSERT_EQ(sendLength, receiveLength);
}

TEST_P(TestProtocol, SendAndReceiveSignalId) {
    // Arrange
    IoSignalId sendSignalId = GenerateIoSignalId();

    // Act
    AssertOk(_protocol->WriteSignalId(_senderChannel->GetWriter(), sendSignalId));
    AssertOk(_senderChannel->GetWriter().EndWrite());

    // Assert
    IoSignalId receiveSignalId{};
    AssertOk(_protocol->ReadSignalId(_receiverChannel->GetReader(), receiveSignalId));
    _receiverChannel->GetReader().EndRead();
    ASSERT_EQ(sendSignalId, receiveSignalId);
}

TEST_P(TestProtocol, SendAndReceiveCanMessageContainer) {
    // Arrange
    CanMessageContainer sendCanMessageContainer;
    FillWithRandom(sendCanMessageContainer, GenerateBusControllerId());

    // Act
    AssertOk(_protocol->WriteMessage(_senderChannel->GetWriter(), sendCanMessageContainer));
    AssertOk(_senderChannel->GetWriter().EndWrite());

    // Assert
    CanMessageContainer receiveCanMessageContainer;
    AssertOk(_protocol->ReadMessage(_receiverChannel->GetReader(), receiveCanMessageContainer));
    _receiverChannel->GetReader().EndRead();
    ASSERT_EQ(sendCanMessageContainer, receiveCanMessageContainer);
}

TEST_P(TestProtocol, SendAndReceiveEthMessageContainer) {
    // Arrange
    EthMessageContainer sendEthMessageContainer;
    FillWithRandom(sendEthMessageContainer, GenerateBusControllerId());

    // Act
    AssertOk(_protocol->WriteMessage(_senderChannel->GetWriter(), sendEthMessageContainer));
    AssertOk(_senderChannel->GetWriter().EndWrite());

    // Assert
    EthMessageContainer receiveEthMessageContainer;
    AssertOk(_protocol->ReadMessage(_receiverChannel->GetReader(), receiveEthMessageContainer));
    _receiverChannel->GetReader().EndRead();
    ASSERT_EQ(sendEthMessageContainer, receiveEthMessageContainer);
}

TEST_P(TestProtocol, SendAndReceiveLinMessageContainer) {
    // Arrange
    LinMessageContainer sendLinMessageContainer;
    FillWithRandom(sendLinMessageContainer, GenerateBusControllerId());

    // Act
    AssertOk(_protocol->WriteMessage(_senderChannel->GetWriter(), sendLinMessageContainer));
    AssertOk(_senderChannel->GetWriter().EndWrite());

    // Assert
    LinMessageContainer receiveLinMessageContainer;
    AssertOk(_protocol->ReadMessage(_receiverChannel->GetReader(), receiveLinMessageContainer));
    _receiverChannel->GetReader().EndRead();
    ASSERT_EQ(sendLinMessageContainer, receiveLinMessageContainer);
}

TEST_P(TestProtocol, SendAndReceiveFrMessageContainer) {
    // Arrange
    FrMessageContainer sendFrMessageContainer;
    FillWithRandom(sendFrMessageContainer, GenerateBusControllerId());

    // Act
    AssertOk(_protocol->WriteMessage(_senderChannel->GetWriter(), sendFrMessageContainer));
    AssertOk(_senderChannel->GetWriter().EndWrite());

    // Assert
    FrMessageContainer receiveFrMessageContainer;
    AssertOk(_protocol->ReadMessage(_receiverChannel->GetReader(), receiveFrMessageContainer));
    _receiverChannel->GetReader().EndRead();
    ASSERT_EQ(sendFrMessageContainer, receiveFrMessageContainer);
}

TEST_P(TestProtocol, SendAndReceiveOk) {
    // Act
    AssertOk(_protocol->SendOk(_senderChannel->GetWriter()));

    // Assert
    AssertFrame(FrameKind::Ok);
    AssertOk(_protocol->ReadOk(_receiverChannel->GetReader()));
}

TEST_P(TestProtocol, SendAndReceiveError) {
    // Arrange
    std::string sendErrorMessage = GenerateString("Errorメッセージ");

    // Act
    AssertOk(_protocol->SendError(_senderChannel->GetWriter(), sendErrorMessage));

    // Assert
    AssertFrame(FrameKind::Error);

    std::string receiveErrorMessage;
    AssertOk(_protocol->ReadError(_receiverChannel->GetReader(), receiveErrorMessage));
    ASSERT_EQ(sendErrorMessage, receiveErrorMessage);
}

TEST_P(TestProtocol, SendAndReceivePing) {
    // Arrange
    auto sendRoundTripTime = SimulationTime{GenerateI64()};

    // Act
    AssertOk(_protocol->SendPing(_senderChannel->GetWriter(), sendRoundTripTime));

    // Assert
    AssertFrame(FrameKind::Ping);
    SimulationTime receiveRoundTripTime;
    AssertOk(_protocol->ReadPing(_receiverChannel->GetReader(), receiveRoundTripTime));
    ASSERT_EQ(sendRoundTripTime, receiveRoundTripTime);
}

TEST_P(TestProtocol, SendAndReceivePingOk) {
    // Arrange
    auto sendCommand = static_cast<Command>(GenerateU32());

    // Act
    AssertOk(_protocol->SendPingOk(_senderChannel->GetWriter(), sendCommand));

    // Assert
    AssertFrame(FrameKind::PingOk);

    Command receiveCommand{};
    AssertOk(_protocol->ReadPingOk(_receiverChannel->GetReader(), receiveCommand));
    ASSERT_EQ(sendCommand, receiveCommand);
}

TEST_P(TestProtocol, SendAndReceiveConnect) {
    // Arrange
    uint32_t sendVersion = GenerateU32();
    Mode sendMode{};
    std::string sendServerName = GenerateString("Server名前");
    std::string sendClientName = GenerateString("Client名前");

    // Act
    AssertOk(_protocol->SendConnect(_senderChannel->GetWriter(), sendVersion, sendMode, sendServerName, sendClientName));

    // Assert
    AssertFrame(FrameKind::Connect);

    uint32_t receiveVersion{};
    Mode receiveMode{};
    std::string receiveServerName;
    std::string receiveClientName;
    AssertOk(_protocol->ReadConnect(_receiverChannel->GetReader(), receiveVersion, receiveMode, receiveServerName, receiveClientName));
    EXPECT_EQ(sendVersion, receiveVersion);
    EXPECT_EQ(sendMode, receiveMode);
    EXPECT_EQ(sendServerName, receiveServerName);
    EXPECT_EQ(sendClientName, receiveClientName);
}

TEST_P(TestProtocol, SendAndReceiveConnectOk) {
    // Arrange
    uint32_t sendProtocolVersion = GenerateU32();
    Mode sendMode{};
    SimulationTime sendStepSize = GenerateSimulationTime();
    constexpr SimulationState sendSimulationState{};
    std::vector<IoSignalContainer> sendIncomingSignals = CreateSignals(2);
    std::vector<IoSignalContainer> sendOutgoingSignals = CreateSignals(3);
    std::vector<CanControllerContainer> sendCanControllers = CreateCanControllers(4);
    std::vector<EthControllerContainer> sendEthControllers = CreateEthControllers(5);
    std::vector<LinControllerContainer> sendLinControllers = CreateLinControllers(6);
    std::vector<FrControllerContainer> sendFrControllers = CreateFrControllers(6);

    // Act
    AssertOk(_protocol->SendConnectOk(_senderChannel->GetWriter(),
                                      sendProtocolVersion,
                                      sendMode,
                                      sendStepSize,
                                      sendSimulationState,
                                      sendIncomingSignals,
                                      sendOutgoingSignals,
                                      sendCanControllers,
                                      sendEthControllers,
                                      sendLinControllers,
                                      sendFrControllers));

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
    std::vector<FrControllerContainer> receiveFrControllers;
    AssertOk(_protocol->ReadConnectOkVersion(_receiverChannel->GetReader(), receiveProtocolVersion));
    AssertOk(_protocol->ReadConnectOk(_receiverChannel->GetReader(),
                                      receiveMode,
                                      receiveStepSize,
                                      receiveSimulationState,
                                      receiveIncomingSignals,
                                      receiveOutgoingSignals,
                                      receiveCanControllers,
                                      receiveEthControllers,
                                      receiveLinControllers,
                                      receiveFrControllers));
    EXPECT_EQ(sendProtocolVersion, receiveProtocolVersion);
    EXPECT_EQ(sendMode, receiveMode);
    EXPECT_EQ(sendStepSize, receiveStepSize);
    EXPECT_THAT(receiveIncomingSignals, ContainerEq(sendIncomingSignals));
    EXPECT_THAT(receiveOutgoingSignals, ContainerEq(sendOutgoingSignals));
    EXPECT_THAT(receiveCanControllers, ContainerEq(sendCanControllers));
    EXPECT_THAT(receiveEthControllers, ContainerEq(sendEthControllers));
    EXPECT_THAT(receiveLinControllers, ContainerEq(sendLinControllers));
    EXPECT_THAT(receiveFrControllers, ContainerEq(sendFrControllers));
}

TEST_P(TestProtocol, SendAndReceiveStart) {
    // Arrange
    SimulationTime sendSimulationTime = GenerateSimulationTime();

    // Act
    AssertOk(_protocol->SendStart(_senderChannel->GetWriter(), sendSimulationTime));

    // Assert
    AssertFrame(FrameKind::Start);

    SimulationTime receiveSimulationTime{};
    AssertOk(_protocol->ReadStart(_receiverChannel->GetReader(), receiveSimulationTime));
    ASSERT_EQ(sendSimulationTime, receiveSimulationTime);
}

TEST_P(TestProtocol, SendAndReceiveStop) {
    // Arrange
    SimulationTime sendSimulationTime = GenerateSimulationTime();

    // Act
    AssertOk(_protocol->SendStop(_senderChannel->GetWriter(), sendSimulationTime));

    // Assert
    AssertFrame(FrameKind::Stop);

    SimulationTime receiveSimulationTime{};
    AssertOk(_protocol->ReadStop(_receiverChannel->GetReader(), receiveSimulationTime));
    ASSERT_EQ(sendSimulationTime, receiveSimulationTime);
}

TEST_P(TestProtocol, SendAndReceiveTerminate) {
    // Arrange
    SimulationTime sendSimulationTime = GenerateSimulationTime();
    TerminateReason sendTerminateReason = GenerateRandom(TerminateReason::Finished, TerminateReason::Error);

    // Act
    AssertOk(_protocol->SendTerminate(_senderChannel->GetWriter(), sendSimulationTime, sendTerminateReason));

    // Assert
    AssertFrame(FrameKind::Terminate);

    SimulationTime receiveSimulationTime{};
    TerminateReason receiveTerminateReason{};
    AssertOk(_protocol->ReadTerminate(_receiverChannel->GetReader(), receiveSimulationTime, receiveTerminateReason));
    EXPECT_EQ(sendSimulationTime, receiveSimulationTime);
    EXPECT_EQ(sendTerminateReason, receiveTerminateReason);
}

TEST_P(TestProtocol, SendAndReceivePause) {
    // Arrange
    SimulationTime sendSimulationTime = GenerateSimulationTime();

    // Act
    AssertOk(_protocol->SendPause(_senderChannel->GetWriter(), sendSimulationTime));

    // Assert
    AssertFrame(FrameKind::Pause);

    SimulationTime receiveSimulationTime{};
    AssertOk(_protocol->ReadPause(_receiverChannel->GetReader(), receiveSimulationTime));
    ASSERT_EQ(sendSimulationTime, receiveSimulationTime);
}

TEST_P(TestProtocol, SendAndReceiveContinue) {
    // Arrange
    SimulationTime sendSimulationTime = GenerateSimulationTime();

    // Act
    AssertOk(_protocol->SendContinue(_senderChannel->GetWriter(), sendSimulationTime));

    // Assert
    AssertFrame(FrameKind::Continue);

    SimulationTime receiveSimulationTime{};
    AssertOk(_protocol->ReadContinue(_receiverChannel->GetReader(), receiveSimulationTime));
    ASSERT_EQ(sendSimulationTime, receiveSimulationTime);
}

TEST_P(TestProtocol, SendAndReceiveStep) {
    // Arrange
    SimulationTime sendSimulationTime = GenerateSimulationTime();

    SerializeFunction serializeFunction = [=]([[maybe_unused]] ChannelWriter& writer) {
        return CreateOk();
    };

    DeserializeFunction deserializeFunction =
        [=]([[maybe_unused]] ChannelReader& reader, [[maybe_unused]] SimulationTime simulationTime, [[maybe_unused]] const Callbacks& callbacks) {
            return CreateOk();
        };

    // Act
    AssertOk(_protocol->SendStep(_senderChannel->GetWriter(), sendSimulationTime, serializeFunction, serializeFunction));

    // Assert
    AssertFrame(FrameKind::Step);

    SimulationTime receiveSimulationTime{};
    AssertOk(_protocol->ReadStep(_receiverChannel->GetReader(), receiveSimulationTime, deserializeFunction, deserializeFunction, {}));
    ASSERT_EQ(sendSimulationTime, receiveSimulationTime);
}

TEST_P(TestProtocol, SendAndReceiveStepOk) {
    // Arrange
    SimulationTime sendSimulationTime = GenerateSimulationTime();

    auto sendCommand = static_cast<Command>(GenerateU32());

    SerializeFunction serializeFunction = [=]([[maybe_unused]] ChannelWriter& writer) {
        return CreateOk();
    };

    DeserializeFunction deserializeFunction =
        [=]([[maybe_unused]] ChannelReader& reader, [[maybe_unused]] SimulationTime simulationTime, [[maybe_unused]] const Callbacks& callbacks) {
            return CreateOk();
        };

    // Act
    AssertOk(_protocol->SendStepOk(_senderChannel->GetWriter(), sendSimulationTime, sendCommand, serializeFunction, serializeFunction));

    // Assert
    AssertFrame(FrameKind::StepOk);

    SimulationTime receiveSimulationTime{};
    Command receiveCommand{};
    AssertOk(_protocol->ReadStepOk(_receiverChannel->GetReader(), receiveSimulationTime, receiveCommand, deserializeFunction, deserializeFunction, {}));
    ASSERT_EQ(sendSimulationTime, receiveSimulationTime);
}

TEST_P(TestProtocol, SendAndReceiveGetPort) {
    // Arrange
    std::string sendServerName = GenerateString("Server名前");

    // Act
    AssertOk(_protocol->SendGetPort(_senderChannel->GetWriter(), sendServerName));

    // Assert
    AssertFrame(FrameKind::GetPort);

    std::string receiveServerName;
    AssertOk(_protocol->ReadGetPort(_receiverChannel->GetReader(), receiveServerName));
    ASSERT_EQ(sendServerName, receiveServerName);
}

TEST_P(TestProtocol, SendAndReceiveGetPortOk) {
    // Arrange
    uint16_t sendPort = GenerateU16();

    // Act
    AssertOk(_protocol->SendGetPortOk(_senderChannel->GetWriter(), sendPort));

    // Assert
    AssertFrame(FrameKind::GetPortOk);

    uint16_t receivePort{};
    AssertOk(_protocol->ReadGetPortOk(_receiverChannel->GetReader(), receivePort));
    ASSERT_EQ(sendPort, receivePort);
}

TEST_P(TestProtocol, SendAndReceiveSetPort) {
    // Arrange
    std::string sendServerName = GenerateString("Server名前");
    uint16_t sendPort = GenerateU16();

    // Act
    AssertOk(_protocol->SendSetPort(_senderChannel->GetWriter(), sendServerName, sendPort));

    // Assert
    AssertFrame(FrameKind::SetPort);

    std::string receiveServerName;
    uint16_t receivePort{};
    AssertOk(_protocol->ReadSetPort(_receiverChannel->GetReader(), receiveServerName, receivePort));
    EXPECT_EQ(sendServerName, receiveServerName);
    EXPECT_EQ(sendPort, receivePort);
}

TEST_P(TestProtocol, SendAndReceiveUnsetPort) {
    // Arrange
    std::string sendServerName = GenerateString("Server名前");

    // Act
    AssertOk(_protocol->SendUnsetPort(_senderChannel->GetWriter(), sendServerName));

    // Assert
    AssertFrame(FrameKind::UnsetPort);

    std::string receiveServerName;
    AssertOk(_protocol->ReadUnsetPort(_receiverChannel->GetReader(), receiveServerName));
    ASSERT_EQ(sendServerName, receiveServerName);
}

TEST_P(TestProtocol, SendAndReceiveConnectWithEmptyNames) {
    // Arrange
    uint32_t sendVersion = GenerateU32();
    Mode sendMode{};
    std::string sendServerName;
    std::string sendClientName;

    // Act
    AssertOk(_protocol->SendConnect(_senderChannel->GetWriter(), sendVersion, sendMode, sendServerName, sendClientName));

    // Assert
    AssertFrame(FrameKind::Connect);

    uint32_t receiveVersion{};
    Mode receiveMode{};
    std::string receiveServerName;
    std::string receiveClientName;
    AssertOk(_protocol->ReadConnect(_receiverChannel->GetReader(), receiveVersion, receiveMode, receiveServerName, receiveClientName));
    EXPECT_EQ(sendVersion, receiveVersion);
    EXPECT_EQ(sendMode, receiveMode);
    EXPECT_EQ(sendServerName, receiveServerName);
    EXPECT_EQ(sendClientName, receiveClientName);
}

TEST_P(TestProtocol, SendAndReceiveErrorWithEmptyMessage) {
    // Arrange
    std::string sendErrorMessage;

    // Act
    AssertOk(_protocol->SendError(_senderChannel->GetWriter(), sendErrorMessage));

    // Assert
    AssertFrame(FrameKind::Error);

    std::string receiveErrorMessage;
    AssertOk(_protocol->ReadError(_receiverChannel->GetReader(), receiveErrorMessage));
    ASSERT_EQ(sendErrorMessage, receiveErrorMessage);
}

TEST_P(TestProtocol, SendAndReceiveConnectOkWithEmptyLists) {
    // Arrange
    uint32_t sendProtocolVersion = GenerateU32();
    Mode sendMode{};
    SimulationTime sendStepSize = GenerateSimulationTime();
    constexpr SimulationState sendSimulationState{};

    // Act
    AssertOk(_protocol->SendConnectOk(_senderChannel->GetWriter(), sendProtocolVersion, sendMode, sendStepSize, sendSimulationState, {}, {}, {}, {}, {}, {}));

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
    std::vector<FrControllerContainer> receiveFrControllers;
    AssertOk(_protocol->ReadConnectOkVersion(_receiverChannel->GetReader(), receiveProtocolVersion));
    AssertOk(_protocol->ReadConnectOk(_receiverChannel->GetReader(),
                                      receiveMode,
                                      receiveStepSize,
                                      receiveSimulationState,
                                      receiveIncomingSignals,
                                      receiveOutgoingSignals,
                                      receiveCanControllers,
                                      receiveEthControllers,
                                      receiveLinControllers,
                                      receiveFrControllers));
    EXPECT_EQ(sendProtocolVersion, receiveProtocolVersion);
    EXPECT_EQ(sendMode, receiveMode);
    EXPECT_EQ(sendStepSize, receiveStepSize);
    EXPECT_TRUE(receiveIncomingSignals.empty());
    EXPECT_TRUE(receiveOutgoingSignals.empty());
    EXPECT_TRUE(receiveCanControllers.empty());
    EXPECT_TRUE(receiveEthControllers.empty());
    EXPECT_TRUE(receiveLinControllers.empty());
    EXPECT_TRUE(receiveFrControllers.empty());
}

TEST_P(TestProtocol, SendAndReceiveConnectOkWithManyControllers) {
    // Arrange
    uint32_t sendProtocolVersion = GenerateU32();
    Mode sendMode{};
    SimulationTime sendStepSize = GenerateSimulationTime();
    constexpr SimulationState sendSimulationState{};
    std::vector<IoSignalContainer> sendIncomingSignals = CreateSignals(20);
    std::vector<IoSignalContainer> sendOutgoingSignals = CreateSignals(20);
    std::vector<CanControllerContainer> sendCanControllers = CreateCanControllers(20);
    std::vector<EthControllerContainer> sendEthControllers = CreateEthControllers(20);
    std::vector<LinControllerContainer> sendLinControllers = CreateLinControllers(20);
    std::vector<FrControllerContainer> sendFrControllers = CreateFrControllers(20);

    // Act
    AssertOk(_protocol->SendConnectOk(_senderChannel->GetWriter(),
                                      sendProtocolVersion,
                                      sendMode,
                                      sendStepSize,
                                      sendSimulationState,
                                      sendIncomingSignals,
                                      sendOutgoingSignals,
                                      sendCanControllers,
                                      sendEthControllers,
                                      sendLinControllers,
                                      sendFrControllers));

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
    std::vector<FrControllerContainer> receiveFrControllers;
    AssertOk(_protocol->ReadConnectOkVersion(_receiverChannel->GetReader(), receiveProtocolVersion));
    AssertOk(_protocol->ReadConnectOk(_receiverChannel->GetReader(),
                                      receiveMode,
                                      receiveStepSize,
                                      receiveSimulationState,
                                      receiveIncomingSignals,
                                      receiveOutgoingSignals,
                                      receiveCanControllers,
                                      receiveEthControllers,
                                      receiveLinControllers,
                                      receiveFrControllers));
    EXPECT_EQ(sendProtocolVersion, receiveProtocolVersion);
    EXPECT_EQ(sendMode, receiveMode);
    EXPECT_EQ(sendStepSize, receiveStepSize);
    EXPECT_THAT(receiveIncomingSignals, ContainerEq(sendIncomingSignals));
    EXPECT_THAT(receiveOutgoingSignals, ContainerEq(sendOutgoingSignals));
    EXPECT_THAT(receiveCanControllers, ContainerEq(sendCanControllers));
    EXPECT_THAT(receiveEthControllers, ContainerEq(sendEthControllers));
    EXPECT_THAT(receiveLinControllers, ContainerEq(sendLinControllers));
    EXPECT_THAT(receiveFrControllers, ContainerEq(sendFrControllers));
}

TEST_P(TestProtocol, ReceiveHeaderOnEmptyChannelShouldFail) {
    // Arrange
    // Act - disconnect both sides so the channel is fully torn down
    _senderChannel->Disconnect();
    _receiverChannel->Disconnect();

    FrameKind frameKind{};
    Result result = _protocol->ReceiveHeader(_receiverChannel->GetReader(), frameKind);

    // Assert
    AssertNotConnected(result);
}

TEST_P(TestProtocol, SendAndReceiveData) {
    // Arrange
    std::vector<uint8_t> sendData(64);
    for (size_t i = 0; i < sendData.size(); i++) {
        sendData[i] = static_cast<uint8_t>(i);
    }

    // Act
    AssertOk(_protocol->WriteData(_senderChannel->GetWriter(), sendData.data(), sendData.size()));
    AssertOk(_senderChannel->GetWriter().EndWrite());

    // Assert
    std::vector<uint8_t> receiveData(sendData.size());
    AssertOk(_protocol->ReadData(_receiverChannel->GetReader(), receiveData.data(), receiveData.size()));
    _receiverChannel->GetReader().EndRead();
    ASSERT_THAT(receiveData, ContainerEq(sendData));
}

}  // namespace
