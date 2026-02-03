// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <memory>
#include <optional>
#include <string>

#include "Channel.h"
#include "DsVeosCoSim/CoSimTypes.h"
#include "Helper.h"
#include "Protocol.h"
#include "TestHelper.h"

using namespace DsVeosCoSim;

namespace {

class TestProtocol : public testing::TestWithParam<ConnectionKind> {
protected:
    std::unique_ptr<Channel> _senderChannel;
    std::unique_ptr<Channel> _receiverChannel;

    void CustomSetUp(ConnectionKind connectionKind) {
        if (connectionKind == ConnectionKind::Remote) {
            std::unique_ptr<ChannelServer> server;
            ExpectOk(CreateTcpChannelServer(0, true, server));
            ExpectTrue(server);
            std::optional<uint16_t> port = server->GetLocalPort();
            ExpectTrue(port);

            ExpectOk(TryConnectToTcpChannel("127.0.0.1", *port, 0, DefaultTimeout, _senderChannel));
            ExpectTrue(_senderChannel);
            ExpectOk(server->TryAccept(_receiverChannel));
            ExpectTrue(_receiverChannel);
        } else {
#ifdef _WIN32
            std::string name = GenerateString("LocalChannel名前");
            std::unique_ptr<ChannelServer> server;
            ExpectOk(CreateLocalChannelServer(name, server));

            ExpectOk(TryConnectToLocalChannel(name, _senderChannel));
            ExpectTrue(_senderChannel);
            ExpectOk(server->TryAccept(_receiverChannel));
            ExpectTrue(_receiverChannel);
#else
            std::string name = GenerateString("UdsChannel名前");
            std::unique_ptr<ChannelServer> server;
            ExpectOk(CreateUdsChannelServer(name, server));
            ExpectTrue(server);

            ExpectOk(TryConnectToUdsChannel(name, _senderChannel));
            ExpectTrue(_senderChannel);
            ExpectOk(server->TryAccept(_receiverChannel));
            ExpectTrue(_receiverChannel);
#endif
        }
    }

    void TearDown() override {
        _senderChannel->Disconnect();
        _receiverChannel->Disconnect();
    }

    void AssertFrame(FrameKind expected) const {
        FrameKind frameKind{};
        AssertOk(GetLatestProtocol()->ReceiveHeader(_receiverChannel->GetReader(), frameKind));
        AssertEq(expected, frameKind);
    }
};

INSTANTIATE_TEST_SUITE_P(,
                         TestProtocol,
                         testing::Values(ConnectionKind::Local, ConnectionKind::Remote),
                         [](const testing::TestParamInfo<ConnectionKind>& info) {
                             return std::string(ToString(info.param));
                         });

TEST_P(TestProtocol, SendAndReceiveSize) {
    // Arrange
    CustomSetUp(GetParam());

    size_t sendSize = GenerateU32();

    // Act
    AssertOk(GetLatestProtocol()->WriteSize(_senderChannel->GetWriter(), sendSize));
    AssertOk(_senderChannel->GetWriter().EndWrite());

    // Assert
    size_t receiveSize{};
    AssertOk(GetLatestProtocol()->ReadSize(_receiverChannel->GetReader(), receiveSize));
    AssertEq(sendSize, receiveSize);
}

TEST_P(TestProtocol, SendAndReceiveLength) {
    // Arrange
    CustomSetUp(GetParam());

    uint32_t sendLength = GenerateU32();

    // Act
    AssertOk(GetLatestProtocol()->WriteLength(_senderChannel->GetWriter(), sendLength));
    AssertOk(_senderChannel->GetWriter().EndWrite());

    // Assert
    uint32_t receiveLength{};
    AssertOk(GetLatestProtocol()->ReadLength(_receiverChannel->GetReader(), receiveLength));
    AssertEq(sendLength, receiveLength);
}

TEST_P(TestProtocol, SendAndReceiveSignalId) {
    // Arrange
    CustomSetUp(GetParam());

    IoSignalId sendSignalId = GenerateIoSignalId();

    // Act
    AssertOk(GetLatestProtocol()->WriteSignalId(_senderChannel->GetWriter(), sendSignalId));
    AssertOk(_senderChannel->GetWriter().EndWrite());

    // Assert
    IoSignalId receiveSignalId{};
    AssertOk(GetLatestProtocol()->ReadSignalId(_receiverChannel->GetReader(), receiveSignalId));
    AssertEq(sendSignalId, receiveSignalId);
}

TEST_P(TestProtocol, SendAndReceiveCanMessageContainer) {
    // Arrange
    CustomSetUp(GetParam());

    CanMessageContainer sendCanMessageContainer;
    FillWithRandom(sendCanMessageContainer, GenerateBusControllerId());

    // Act
    AssertOk(GetLatestProtocol()->WriteMessage(_senderChannel->GetWriter(), sendCanMessageContainer));
    AssertOk(_senderChannel->GetWriter().EndWrite());

    // Assert
    CanMessageContainer receiveCanMessageContainer;
    AssertOk(GetLatestProtocol()->ReadMessage(_receiverChannel->GetReader(), receiveCanMessageContainer));
    AssertEq(sendCanMessageContainer, receiveCanMessageContainer);
}

TEST_P(TestProtocol, SendAndReceiveEthMessageContainer) {
    // Arrange
    CustomSetUp(GetParam());

    EthMessageContainer sendEthMessageContainer;
    FillWithRandom(sendEthMessageContainer, GenerateBusControllerId());

    // Act
    AssertOk(GetLatestProtocol()->WriteMessage(_senderChannel->GetWriter(), sendEthMessageContainer));
    AssertOk(_senderChannel->GetWriter().EndWrite());

    // Assert
    EthMessageContainer receiveEthMessageContainer;
    AssertOk(GetLatestProtocol()->ReadMessage(_receiverChannel->GetReader(), receiveEthMessageContainer));
    AssertEq(sendEthMessageContainer, receiveEthMessageContainer);
}

TEST_P(TestProtocol, SendAndReceiveLinMessageContainer) {
    // Arrange
    CustomSetUp(GetParam());

    LinMessageContainer sendLinMessageContainer;
    FillWithRandom(sendLinMessageContainer, GenerateBusControllerId());

    // Act
    AssertOk(GetLatestProtocol()->WriteMessage(_senderChannel->GetWriter(), sendLinMessageContainer));
    AssertOk(_senderChannel->GetWriter().EndWrite());

    // Assert
    LinMessageContainer receiveLinMessageContainer;
    AssertOk(GetLatestProtocol()->ReadMessage(_receiverChannel->GetReader(), receiveLinMessageContainer));
    AssertEq(sendLinMessageContainer, receiveLinMessageContainer);
}

TEST_P(TestProtocol, SendAndReceiveFrMessageContainer) {
    // Arrange
    CustomSetUp(GetParam());

    FrMessageContainer sendFrMessageContainer;
    FillWithRandom(sendFrMessageContainer, GenerateBusControllerId());

    // Act
    AssertOk(GetLatestProtocol()->WriteMessage(_senderChannel->GetWriter(), sendFrMessageContainer));
    AssertOk(_senderChannel->GetWriter().EndWrite());

    // Assert
    FrMessageContainer receiveFrMessageContainer;
    AssertOk(GetLatestProtocol()->ReadMessage(_receiverChannel->GetReader(), receiveFrMessageContainer));
    AssertEq(sendFrMessageContainer, receiveFrMessageContainer);
}

TEST_P(TestProtocol, SendAndReceiveOk) {
    // Arrange
    CustomSetUp(GetParam());

    // Act
    AssertOk(GetLatestProtocol()->SendOk(_senderChannel->GetWriter()));

    // Assert
    AssertFrame(FrameKind::Ok);
}

TEST_P(TestProtocol, SendAndReceiveError) {
    // Arrange
    CustomSetUp(GetParam());

    std::string sendErrorMessage = GenerateString("Errorメッセージ");

    // Act
    AssertOk(GetLatestProtocol()->SendError(_senderChannel->GetWriter(), sendErrorMessage));

    // Assert
    AssertFrame(FrameKind::Error);

    std::string receiveErrorMessage;
    AssertOk(GetLatestProtocol()->ReadError(_receiverChannel->GetReader(), receiveErrorMessage));
    AssertEq(sendErrorMessage, receiveErrorMessage);
}

TEST_P(TestProtocol, SendAndReceivePing) {
    // Arrange
    CustomSetUp(GetParam());

    // Act
    AssertOk(GetLatestProtocol()->SendPing(_senderChannel->GetWriter()));

    // Assert
    AssertFrame(FrameKind::Ping);
}

TEST_P(TestProtocol, SendAndReceivePingOk) {
    // Arrange
    CustomSetUp(GetParam());

    auto sendCommand = static_cast<Command>(GenerateU32());

    // Act
    AssertOk(GetLatestProtocol()->SendPingOk(_senderChannel->GetWriter(), sendCommand));

    // Assert
    AssertFrame(FrameKind::PingOk);

    Command receiveCommand{};
    AssertOk(GetLatestProtocol()->ReadPingOk(_receiverChannel->GetReader(), receiveCommand));
    AssertEq(sendCommand, receiveCommand);
}

TEST_P(TestProtocol, SendAndReceiveConnect) {
    // Arrange
    CustomSetUp(GetParam());

    uint32_t sendVersion = GenerateU32();
    Mode sendMode{};
    std::string sendServerName = GenerateString("Server名前");
    std::string sendClientName = GenerateString("Client名前");

    // Act
    AssertOk(GetLatestProtocol()->SendConnect(_senderChannel->GetWriter(), sendVersion, sendMode, sendServerName, sendClientName));

    // Assert
    AssertFrame(FrameKind::Connect);

    uint32_t receiveVersion{};
    Mode receiveMode{};
    std::string receiveServerName;
    std::string receiveClientName;
    AssertOk(GetLatestProtocol()->ReadConnect(_receiverChannel->GetReader(), receiveVersion, receiveMode, receiveServerName, receiveClientName));
    AssertEq(sendVersion, receiveVersion);
    AssertEq(sendMode, receiveMode);
    AssertEq(sendServerName, receiveServerName);
    AssertEq(sendClientName, receiveClientName);
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
    AssertOk(GetLatestProtocol()->SendConnectOk(_senderChannel->GetWriter(),
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
    AssertOk(GetLatestProtocol()->ReadConnectOkVersion(_receiverChannel->GetReader(), receiveProtocolVersion));
    AssertOk(GetLatestProtocol()->ReadConnectOk(_receiverChannel->GetReader(),
                                                receiveMode,
                                                receiveStepSize,
                                                receiveSimulationState,
                                                receiveIncomingSignals,
                                                receiveOutgoingSignals,
                                                receiveCanControllers,
                                                receiveEthControllers,
                                                receiveLinControllers,
                                                receiveFrControllers));
    AssertEq(sendProtocolVersion, receiveProtocolVersion);
    AssertEq(sendMode, receiveMode);
    AssertEq(sendStepSize, receiveStepSize);
    AssertEqHelper<IoSignal>(Convert(sendIncomingSignals), Convert(receiveIncomingSignals));
    AssertEqHelper<IoSignal>(Convert(sendOutgoingSignals), Convert(receiveOutgoingSignals));
    AssertEqHelper<CanController>(Convert(sendCanControllers), Convert(receiveCanControllers));
    AssertEqHelper<EthController>(Convert(sendEthControllers), Convert(receiveEthControllers));
    AssertEqHelper<LinController>(Convert(sendLinControllers), Convert(receiveLinControllers));
    AssertEqHelper<FrController>(Convert(sendFrControllers), Convert(receiveFrControllers));
}

TEST_P(TestProtocol, SendAndReceiveStart) {
    // Arrange
    CustomSetUp(GetParam());

    SimulationTime sendSimulationTime = GenerateSimulationTime();

    // Act
    AssertOk(GetLatestProtocol()->SendStart(_senderChannel->GetWriter(), sendSimulationTime));

    // Assert
    AssertFrame(FrameKind::Start);

    SimulationTime receiveSimulationTime{};
    AssertOk(GetLatestProtocol()->ReadStart(_receiverChannel->GetReader(), receiveSimulationTime));
    AssertEq(sendSimulationTime, receiveSimulationTime);
}

TEST_P(TestProtocol, SendAndReceiveStop) {
    // Arrange
    CustomSetUp(GetParam());

    SimulationTime sendSimulationTime = GenerateSimulationTime();

    // Act
    AssertOk(GetLatestProtocol()->SendStop(_senderChannel->GetWriter(), sendSimulationTime));

    // Assert
    AssertFrame(FrameKind::Stop);

    SimulationTime receiveSimulationTime{};
    AssertOk(GetLatestProtocol()->ReadStop(_receiverChannel->GetReader(), receiveSimulationTime));
    AssertEq(sendSimulationTime, receiveSimulationTime);
}

TEST_P(TestProtocol, SendAndReceiveTerminate) {
    // Arrange
    CustomSetUp(GetParam());

    SimulationTime sendSimulationTime = GenerateSimulationTime();
    TerminateReason sendTerminateReason = GenerateRandom(TerminateReason::Finished, TerminateReason::Error);

    // Act
    AssertOk(GetLatestProtocol()->SendTerminate(_senderChannel->GetWriter(), sendSimulationTime, sendTerminateReason));

    // Assert
    AssertFrame(FrameKind::Terminate);

    SimulationTime receiveSimulationTime{};
    TerminateReason receiveTerminateReason{};
    AssertOk(GetLatestProtocol()->ReadTerminate(_receiverChannel->GetReader(), receiveSimulationTime, receiveTerminateReason));
    AssertEq(sendSimulationTime, receiveSimulationTime);
    AssertEq(sendTerminateReason, receiveTerminateReason);
}

TEST_P(TestProtocol, SendAndReceivePause) {
    // Arrange
    CustomSetUp(GetParam());

    SimulationTime sendSimulationTime = GenerateSimulationTime();

    // Act
    AssertOk(GetLatestProtocol()->SendPause(_senderChannel->GetWriter(), sendSimulationTime));

    // Assert
    AssertFrame(FrameKind::Pause);

    SimulationTime receiveSimulationTime{};
    AssertOk(GetLatestProtocol()->ReadPause(_receiverChannel->GetReader(), receiveSimulationTime));
    AssertEq(sendSimulationTime, receiveSimulationTime);
}

TEST_P(TestProtocol, SendAndReceiveContinue) {
    // Arrange
    CustomSetUp(GetParam());

    SimulationTime sendSimulationTime = GenerateSimulationTime();

    // Act
    AssertOk(GetLatestProtocol()->SendContinue(_senderChannel->GetWriter(), sendSimulationTime));

    // Assert
    AssertFrame(FrameKind::Continue);

    SimulationTime receiveSimulationTime{};
    AssertOk(GetLatestProtocol()->ReadContinue(_receiverChannel->GetReader(), receiveSimulationTime));
    AssertEq(sendSimulationTime, receiveSimulationTime);
}

TEST_P(TestProtocol, SendAndReceiveStep) {
    // Arrange
    ConnectionKind connectionKind = GetParam();
    CustomSetUp(connectionKind);

    SimulationTime sendSimulationTime = GenerateSimulationTime();

    SerializeFunction serializeFunction = [=]([[maybe_unused]] ChannelWriter& writer) {
        return Result::Ok;
    };

    DeserializeFunction deserializeFunction =
        [=]([[maybe_unused]] ChannelReader& reader, [[maybe_unused]] SimulationTime simulationTime, [[maybe_unused]] const Callbacks& callbacks) {
            return Result::Ok;
        };

    // Act
    AssertOk(GetLatestProtocol()->SendStep(_senderChannel->GetWriter(), sendSimulationTime, serializeFunction, serializeFunction));

    // Assert
    AssertFrame(FrameKind::Step);

    SimulationTime receiveSimulationTime{};
    AssertOk(GetLatestProtocol()->ReadStep(_receiverChannel->GetReader(), receiveSimulationTime, deserializeFunction, deserializeFunction, {}));
    AssertEq(sendSimulationTime, receiveSimulationTime);
}

TEST_P(TestProtocol, SendAndReceiveStepOk) {
    // Arrange
    ConnectionKind connectionKind = GetParam();
    CustomSetUp(connectionKind);

    SimulationTime sendSimulationTime = GenerateSimulationTime();

    auto sendCommand = static_cast<Command>(GenerateU32());

    SerializeFunction serializeFunction = [=]([[maybe_unused]] ChannelWriter& writer) {
        return Result::Ok;
    };

    DeserializeFunction deserializeFunction =
        [=]([[maybe_unused]] ChannelReader& reader, [[maybe_unused]] SimulationTime simulationTime, [[maybe_unused]] const Callbacks& callbacks) {
            return Result::Ok;
        };

    // Act
    AssertOk(GetLatestProtocol()->SendStepOk(_senderChannel->GetWriter(), sendSimulationTime, sendCommand, serializeFunction, serializeFunction));

    // Assert
    AssertFrame(FrameKind::StepOk);

    SimulationTime receiveSimulationTime{};
    Command receiveCommand{};
    AssertOk(
        GetLatestProtocol()->ReadStepOk(_receiverChannel->GetReader(), receiveSimulationTime, receiveCommand, deserializeFunction, deserializeFunction, {}));
    AssertEq(sendSimulationTime, receiveSimulationTime);
}

TEST_P(TestProtocol, SendAndReceiveGetPort) {
    // Arrange
    CustomSetUp(GetParam());

    std::string sendServerName = GenerateString("Server名前");

    // Act
    AssertOk(GetLatestProtocol()->SendGetPort(_senderChannel->GetWriter(), sendServerName));

    // Assert
    AssertFrame(FrameKind::GetPort);

    std::string receiveServerName;
    AssertOk(GetLatestProtocol()->ReadGetPort(_receiverChannel->GetReader(), receiveServerName));
    AssertEq(sendServerName, receiveServerName);
}

TEST_P(TestProtocol, SendAndReceiveGetPortOk) {
    // Arrange
    CustomSetUp(GetParam());

    uint16_t sendPort = GenerateU16();

    // Act
    AssertOk(GetLatestProtocol()->SendGetPortOk(_senderChannel->GetWriter(), sendPort));

    // Assert
    AssertFrame(FrameKind::GetPortOk);

    uint16_t receivePort{};
    AssertOk(GetLatestProtocol()->ReadGetPortOk(_receiverChannel->GetReader(), receivePort));
    AssertEq(sendPort, receivePort);
}

TEST_P(TestProtocol, SendAndReceiveSetPort) {
    // Arrange
    CustomSetUp(GetParam());

    std::string sendServerName = GenerateString("Server名前");
    uint16_t sendPort = GenerateU16();

    // Act
    AssertOk(GetLatestProtocol()->SendSetPort(_senderChannel->GetWriter(), sendServerName, sendPort));

    // Assert
    AssertFrame(FrameKind::SetPort);

    std::string receiveServerName;
    uint16_t receivePort{};
    AssertOk(GetLatestProtocol()->ReadSetPort(_receiverChannel->GetReader(), receiveServerName, receivePort));
    AssertEq(sendServerName, receiveServerName);
    AssertEq(sendPort, receivePort);
}

TEST_P(TestProtocol, SendAndReceiveUnsetPort) {
    // Arrange
    CustomSetUp(GetParam());

    std::string sendServerName = GenerateString("Server名前");

    // Act
    AssertOk(GetLatestProtocol()->SendUnsetPort(_senderChannel->GetWriter(), sendServerName));

    // Assert
    AssertFrame(FrameKind::UnsetPort);

    std::string receiveServerName;
    AssertOk(GetLatestProtocol()->ReadUnsetPort(_receiverChannel->GetReader(), receiveServerName));
    AssertEq(sendServerName, receiveServerName);
}

}  // namespace
