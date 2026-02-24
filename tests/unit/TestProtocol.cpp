// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <memory>
#include <string>

#include <gmock/gmock.h>

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

    void CustomSetUp(ConnectionKind connectionKind) {
        if (connectionKind == ConnectionKind::Remote) {
            std::unique_ptr<ChannelServer> server;
            AssertOk(CreateTcpChannelServer(0, true, server));
            uint16_t port = server->GetLocalPort();

            AssertOk(TryConnectToTcpChannel("127.0.0.1", port, 0, DefaultTimeout, _senderChannel));
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
    CustomSetUp(GetParam());

    size_t sendSize = GenerateU32();

    // Act
    AssertOk(_protocol->WriteSize(_senderChannel->GetWriter(), sendSize));
    AssertOk(_senderChannel->GetWriter().EndWrite());

    // Assert
    size_t receiveSize{};
    AssertOk(_protocol->ReadSize(_receiverChannel->GetReader(), receiveSize));
    AssertOk(_receiverChannel->GetReader().EndRead());
    ASSERT_EQ(sendSize, receiveSize);
}

TEST_P(TestProtocol, SendAndReceiveLength) {
    // Arrange
    CustomSetUp(GetParam());

    uint32_t sendLength = GenerateU32();

    // Act
    AssertOk(_protocol->WriteLength(_senderChannel->GetWriter(), sendLength));
    AssertOk(_senderChannel->GetWriter().EndWrite());

    // Assert
    uint32_t receiveLength{};
    AssertOk(_protocol->ReadLength(_receiverChannel->GetReader(), receiveLength));
    AssertOk(_receiverChannel->GetReader().EndRead());
    ASSERT_EQ(sendLength, receiveLength);
}

TEST_P(TestProtocol, SendAndReceiveSignalId) {
    // Arrange
    CustomSetUp(GetParam());

    IoSignalId sendSignalId = GenerateIoSignalId();

    // Act
    AssertOk(_protocol->WriteSignalId(_senderChannel->GetWriter(), sendSignalId));
    AssertOk(_senderChannel->GetWriter().EndWrite());

    // Assert
    IoSignalId receiveSignalId{};
    AssertOk(_protocol->ReadSignalId(_receiverChannel->GetReader(), receiveSignalId));
    AssertOk(_receiverChannel->GetReader().EndRead());
    ASSERT_EQ(sendSignalId, receiveSignalId);
}

TEST_P(TestProtocol, SendAndReceiveCanMessageContainer) {
    // Arrange
    CustomSetUp(GetParam());

    CanMessageContainer sendCanMessageContainer;
    FillWithRandom(sendCanMessageContainer, GenerateBusControllerId());

    // Act
    AssertOk(_protocol->WriteMessage(_senderChannel->GetWriter(), sendCanMessageContainer));
    AssertOk(_senderChannel->GetWriter().EndWrite());

    // Assert
    CanMessageContainer receiveCanMessageContainer;
    AssertOk(_protocol->ReadMessage(_receiverChannel->GetReader(), receiveCanMessageContainer));
    AssertOk(_receiverChannel->GetReader().EndRead());
    ASSERT_EQ(sendCanMessageContainer, receiveCanMessageContainer);
}

TEST_P(TestProtocol, SendAndReceiveEthMessageContainer) {
    // Arrange
    CustomSetUp(GetParam());

    EthMessageContainer sendEthMessageContainer;
    FillWithRandom(sendEthMessageContainer, GenerateBusControllerId());

    // Act
    AssertOk(_protocol->WriteMessage(_senderChannel->GetWriter(), sendEthMessageContainer));
    AssertOk(_senderChannel->GetWriter().EndWrite());

    // Assert
    EthMessageContainer receiveEthMessageContainer;
    AssertOk(_protocol->ReadMessage(_receiverChannel->GetReader(), receiveEthMessageContainer));
    AssertOk(_receiverChannel->GetReader().EndRead());
    ASSERT_EQ(sendEthMessageContainer, receiveEthMessageContainer);
}

TEST_P(TestProtocol, SendAndReceiveLinMessageContainer) {
    // Arrange
    CustomSetUp(GetParam());

    LinMessageContainer sendLinMessageContainer;
    FillWithRandom(sendLinMessageContainer, GenerateBusControllerId());

    // Act
    AssertOk(_protocol->WriteMessage(_senderChannel->GetWriter(), sendLinMessageContainer));
    AssertOk(_senderChannel->GetWriter().EndWrite());

    // Assert
    LinMessageContainer receiveLinMessageContainer;
    AssertOk(_protocol->ReadMessage(_receiverChannel->GetReader(), receiveLinMessageContainer));
    AssertOk(_receiverChannel->GetReader().EndRead());
    ASSERT_EQ(sendLinMessageContainer, receiveLinMessageContainer);
}

TEST_P(TestProtocol, SendAndReceiveFrMessageContainer) {
    // Arrange
    CustomSetUp(GetParam());

    FrMessageContainer sendFrMessageContainer;
    FillWithRandom(sendFrMessageContainer, GenerateBusControllerId());

    // Act
    AssertOk(_protocol->WriteMessage(_senderChannel->GetWriter(), sendFrMessageContainer));
    AssertOk(_senderChannel->GetWriter().EndWrite());

    // Assert
    FrMessageContainer receiveFrMessageContainer;
    AssertOk(_protocol->ReadMessage(_receiverChannel->GetReader(), receiveFrMessageContainer));
    AssertOk(_receiverChannel->GetReader().EndRead());
    ASSERT_EQ(sendFrMessageContainer, receiveFrMessageContainer);
}

TEST_P(TestProtocol, SendAndReceiveOk) {
    // Arrange
    CustomSetUp(GetParam());

    // Act
    AssertOk(_protocol->SendOk(_senderChannel->GetWriter()));

    // Assert
    AssertFrame(FrameKind::Ok);
    AssertOk(_protocol->ReadOk(_receiverChannel->GetReader()));
}

TEST_P(TestProtocol, SendTwoFramesAtOnce) {
    // Arrange
    CustomSetUp(GetParam());

    // Act
    AssertOk(_protocol->SendOk(_senderChannel->GetWriter()));
    AssertOk(_protocol->SendOk(_senderChannel->GetWriter()));

    // Assert
    AssertFrame(FrameKind::Ok);
    AssertOk(_protocol->ReadOk(_receiverChannel->GetReader()));
    AssertFrame(FrameKind::Ok);
    AssertOk(_protocol->ReadOk(_receiverChannel->GetReader()));
}

TEST_P(TestProtocol, SendAndReceiveError) {
    // Arrange
    CustomSetUp(GetParam());

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
    CustomSetUp(GetParam());

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
    CustomSetUp(GetParam());

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
    CustomSetUp(GetParam());

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
    ASSERT_EQ(sendVersion, receiveVersion);
    ASSERT_EQ(sendMode, receiveMode);
    ASSERT_EQ(sendServerName, receiveServerName);
    ASSERT_EQ(sendClientName, receiveClientName);
}

TEST_P(TestProtocol, SendAndReceiveConnectOk) {
    // Arrange
    CustomSetUp(GetParam());

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
    ASSERT_EQ(sendProtocolVersion, receiveProtocolVersion);
    ASSERT_EQ(sendMode, receiveMode);
    ASSERT_EQ(sendStepSize, receiveStepSize);
    ASSERT_THAT(receiveIncomingSignals, ContainerEq(sendIncomingSignals));
    ASSERT_THAT(receiveOutgoingSignals, ContainerEq(sendOutgoingSignals));
    ASSERT_THAT(receiveCanControllers, ContainerEq(sendCanControllers));
    ASSERT_THAT(receiveEthControllers, ContainerEq(sendEthControllers));
    ASSERT_THAT(receiveLinControllers, ContainerEq(sendLinControllers));
    ASSERT_THAT(receiveFrControllers, ContainerEq(sendFrControllers));
}

TEST_P(TestProtocol, SendAndReceiveStart) {
    // Arrange
    CustomSetUp(GetParam());

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
    CustomSetUp(GetParam());

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
    CustomSetUp(GetParam());

    SimulationTime sendSimulationTime = GenerateSimulationTime();
    TerminateReason sendTerminateReason = GenerateRandom(TerminateReason::Finished, TerminateReason::Error);

    // Act
    AssertOk(_protocol->SendTerminate(_senderChannel->GetWriter(), sendSimulationTime, sendTerminateReason));

    // Assert
    AssertFrame(FrameKind::Terminate);

    SimulationTime receiveSimulationTime{};
    TerminateReason receiveTerminateReason{};
    AssertOk(_protocol->ReadTerminate(_receiverChannel->GetReader(), receiveSimulationTime, receiveTerminateReason));
    ASSERT_EQ(sendSimulationTime, receiveSimulationTime);
    ASSERT_EQ(sendTerminateReason, receiveTerminateReason);
}

TEST_P(TestProtocol, SendAndReceivePause) {
    // Arrange
    CustomSetUp(GetParam());

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
    CustomSetUp(GetParam());

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
    ConnectionKind connectionKind = GetParam();
    CustomSetUp(connectionKind);

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
    ConnectionKind connectionKind = GetParam();
    CustomSetUp(connectionKind);

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
    CustomSetUp(GetParam());

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
    CustomSetUp(GetParam());

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
    CustomSetUp(GetParam());

    std::string sendServerName = GenerateString("Server名前");
    uint16_t sendPort = GenerateU16();

    // Act
    AssertOk(_protocol->SendSetPort(_senderChannel->GetWriter(), sendServerName, sendPort));

    // Assert
    AssertFrame(FrameKind::SetPort);

    std::string receiveServerName;
    uint16_t receivePort{};
    AssertOk(_protocol->ReadSetPort(_receiverChannel->GetReader(), receiveServerName, receivePort));
    ASSERT_EQ(sendServerName, receiveServerName);
    ASSERT_EQ(sendPort, receivePort);
}

TEST_P(TestProtocol, SendAndReceiveUnsetPort) {
    // Arrange
    CustomSetUp(GetParam());

    std::string sendServerName = GenerateString("Server名前");

    // Act
    AssertOk(_protocol->SendUnsetPort(_senderChannel->GetWriter(), sendServerName));

    // Assert
    AssertFrame(FrameKind::UnsetPort);

    std::string receiveServerName;
    AssertOk(_protocol->ReadUnsetPort(_receiverChannel->GetReader(), receiveServerName));
    ASSERT_EQ(sendServerName, receiveServerName);
}

}  // namespace
