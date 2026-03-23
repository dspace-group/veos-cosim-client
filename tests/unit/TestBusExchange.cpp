// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <deque>
#include <memory>
#include <string>

#include <fmt/format.h>

#include <gtest/gtest.h>

#include "BusExchange.hpp"
#include "CoSimTypes.hpp"
#include "Helper.hpp"
#include "TestHelper.hpp"

using testing::Types;

using namespace DsVeosCoSim;
using namespace testing;

namespace {

template <typename Types>
class TestBusExchange : public Test {
    using TControllerContainer = typename Types::ControllerContainer;
    using TController = typename Types::Controller;
    using TMessageContainer = typename Types::MessageContainer;
    using TMessage = typename Types::Message;

protected:
    static std::unique_ptr<Channel> _remoteSenderChannel;
    static std::unique_ptr<Channel> _remoteReceiverChannel;

    static std::unique_ptr<Channel> _localSenderChannel;
    static std::unique_ptr<Channel> _localReceiverChannel;

    static void SetUpTestSuite() {
        std::unique_ptr<ChannelServer> remoteServer;
        AssertOk(CreateTcpChannelServer(0, true, remoteServer));
        uint16_t port = remoteServer->GetLocalPort();

        AssertOk(TryConnectToTcpChannel("127.0.0.1", port, 0, DefaultTimeout, _remoteSenderChannel));
        AssertOk(remoteServer->TryAccept(_remoteReceiverChannel));

        std::string name = GenerateString("LocalChannel名前");
        std::unique_ptr<ChannelServer> localServer;
        AssertOk(CreateLocalChannelServer(name, localServer));

        AssertOk(TryConnectToLocalChannel(name, _localSenderChannel));
        AssertOk(localServer->TryAccept(_localReceiverChannel));
    }

    static void TearDownTestSuite() {
        _remoteSenderChannel->Disconnect();
        _remoteReceiverChannel->Disconnect();
        _localSenderChannel->Disconnect();
        _localReceiverChannel->Disconnect();

        _remoteSenderChannel.reset();
        _remoteReceiverChannel.reset();
        _localSenderChannel.reset();
        _localReceiverChannel.reset();
    }

    void Transfer(ConnectionKind connectionKind, const BusExchange& senderBusExchange, const BusExchange& receiverBusExchange) {
        ChannelReader& reader = connectionKind == ConnectionKind::Remote ? _remoteReceiverChannel->GetReader() : _localReceiverChannel->GetReader();
        ChannelWriter& writer = connectionKind == ConnectionKind::Remote ? _remoteSenderChannel->GetWriter() : _localSenderChannel->GetWriter();

        AssertOk(senderBusExchange.Serialize(writer));
        AssertOk(writer.EndWrite());

        AssertOk(receiverBusExchange.Deserialize(reader, {}, {}));
    }

    void Transfer(ConnectionKind connectionKind,
                  BusExchange& senderBusExchange,
                  BusExchange& receiverBusExchange,
                  std::deque<std::tuple<TController, TMessageContainer>>& expectedCallbacks) {
        ChannelReader& reader = connectionKind == ConnectionKind::Remote ? _remoteReceiverChannel->GetReader() : _localReceiverChannel->GetReader();
        ChannelWriter& writer = connectionKind == ConnectionKind::Remote ? _remoteSenderChannel->GetWriter() : _localSenderChannel->GetWriter();

        SimulationTime expectedSimulationTime = GenerateSimulationTime();

        Callbacks callbacks{};
        if constexpr (std::is_same_v<TController, CanController>) {
            callbacks.canMessageContainerReceivedCallback =
                [&](SimulationTime simulationTime, const CanController& controller, const CanMessageContainer& messageContainer) {
                    ASSERT_EQ(simulationTime, expectedSimulationTime);
                    ASSERT_FALSE(expectedCallbacks.empty());
                    auto& [expectedController, expectedMessageContainer] = expectedCallbacks.front();
                    ASSERT_EQ(expectedController, controller);
                    ASSERT_EQ(expectedMessageContainer, messageContainer);
                    expectedCallbacks.pop_front();
                };
        }

        if constexpr (std::is_same_v<TController, EthController>) {
            callbacks.ethMessageContainerReceivedCallback =
                [&](SimulationTime simulationTime, const EthController& controller, const EthMessageContainer& messageContainer) {
                    ASSERT_EQ(simulationTime, expectedSimulationTime);
                    ASSERT_FALSE(expectedCallbacks.empty());
                    auto& [expectedController, expectedMessageContainer] = expectedCallbacks.front();
                    ASSERT_EQ(expectedController, controller);
                    ASSERT_EQ(expectedMessageContainer, messageContainer);
                    expectedCallbacks.pop_front();
                };
        }

        if constexpr (std::is_same_v<TController, LinController>) {
            callbacks.linMessageContainerReceivedCallback =
                [&](SimulationTime simulationTime, const LinController& controller, const LinMessageContainer& messageContainer) {
                    ASSERT_EQ(simulationTime, expectedSimulationTime);
                    ASSERT_FALSE(expectedCallbacks.empty());
                    auto& [expectedController, expectedMessageContainer] = expectedCallbacks.front();
                    ASSERT_EQ(expectedController, controller);
                    ASSERT_EQ(expectedMessageContainer, messageContainer);
                    expectedCallbacks.pop_front();
                };
        }

        if constexpr (std::is_same_v<TController, FrController>) {
            callbacks.frMessageContainerReceivedCallback =
                [&](SimulationTime simulationTime, const FrController& controller, const FrMessageContainer& messageContainer) {
                    ASSERT_EQ(simulationTime, expectedSimulationTime);
                    ASSERT_FALSE(expectedCallbacks.empty());
                    auto& [expectedController, expectedMessageContainer] = expectedCallbacks.front();
                    ASSERT_EQ(expectedController, controller);
                    ASSERT_EQ(expectedMessageContainer, messageContainer);
                    expectedCallbacks.pop_front();
                };
        }

        AssertOk(senderBusExchange.Serialize(writer));
        AssertOk(writer.EndWrite());

        AssertOk(receiverBusExchange.Deserialize(reader, expectedSimulationTime, callbacks));

        ASSERT_TRUE(expectedCallbacks.empty());
    }

    void Transfer(ConnectionKind connectionKind,
                  BusExchange& senderBusExchange,
                  BusExchange& receiverBusExchange,
                  std::deque<std::tuple<TController, TMessage>>& expectedCallbacks) {
        ChannelReader& reader = connectionKind == ConnectionKind::Remote ? _remoteReceiverChannel->GetReader() : _localReceiverChannel->GetReader();
        ChannelWriter& writer = connectionKind == ConnectionKind::Remote ? _remoteSenderChannel->GetWriter() : _localSenderChannel->GetWriter();

        SimulationTime expectedSimulationTime = GenerateSimulationTime();

        Callbacks callbacks{};
        if constexpr (std::is_same_v<TController, CanController>) {
            callbacks.canMessageReceivedCallback = [&](SimulationTime simulationTime, const CanController& controller, const CanMessage& message) {
                ASSERT_EQ(simulationTime, expectedSimulationTime);
                ASSERT_FALSE(expectedCallbacks.empty());
                auto& [expectedController, expectedMessage] = expectedCallbacks.front();
                ASSERT_EQ(expectedController, controller);
                ASSERT_EQ(expectedMessage, message);
                expectedCallbacks.pop_front();
            };
        }

        if constexpr (std::is_same_v<TController, EthController>) {
            callbacks.ethMessageReceivedCallback = [&](SimulationTime simulationTime, const EthController& controller, const EthMessage& message) {
                ASSERT_EQ(simulationTime, expectedSimulationTime);
                ASSERT_FALSE(expectedCallbacks.empty());
                auto& [expectedController, expectedMessage] = expectedCallbacks.front();
                ASSERT_EQ(expectedController, controller);
                ASSERT_EQ(expectedMessage, message);
                expectedCallbacks.pop_front();
            };
        }

        if constexpr (std::is_same_v<TController, LinController>) {
            callbacks.linMessageReceivedCallback = [&](SimulationTime simulationTime, const LinController& controller, const LinMessage& message) {
                ASSERT_EQ(simulationTime, expectedSimulationTime);
                ASSERT_FALSE(expectedCallbacks.empty());
                auto& [expectedController, expectedMessage] = expectedCallbacks.front();
                ASSERT_EQ(expectedController, controller);
                ASSERT_EQ(expectedMessage, message);
                expectedCallbacks.pop_front();
            };
        }

        if constexpr (std::is_same_v<TController, FrController>) {
            callbacks.frMessageReceivedCallback = [&](SimulationTime simulationTime, const FrController& controller, const FrMessage& message) {
                ASSERT_EQ(simulationTime, expectedSimulationTime);
                ASSERT_FALSE(expectedCallbacks.empty());
                auto& [expectedController, expectedMessage] = expectedCallbacks.front();
                ASSERT_EQ(expectedController, controller);
                ASSERT_EQ(expectedMessage, message);
                expectedCallbacks.pop_front();
            };
        }

        AssertOk(senderBusExchange.Serialize(writer));
        AssertOk(writer.EndWrite());

        AssertOk(receiverBusExchange.Deserialize(reader, expectedSimulationTime, callbacks));

        ASSERT_TRUE(expectedCallbacks.empty());
    }
};

template <typename Types>
std::unique_ptr<Channel> TestBusExchange<Types>::_remoteSenderChannel;

template <typename Types>
std::unique_ptr<Channel> TestBusExchange<Types>::_remoteReceiverChannel;

template <typename Types>
std::unique_ptr<Channel> TestBusExchange<Types>::_localSenderChannel;

template <typename Types>
std::unique_ptr<Channel> TestBusExchange<Types>::_localReceiverChannel;

template <typename TControllerContainer,
          typename TController,
          typename TMessageContainer,
          typename TMessage,
          CoSimType TheCoSimType,
          ConnectionKind TheConnectionKind>
struct Param {
    using ControllerContainer = TControllerContainer;
    using Controller = TController;
    using MessageContainer = TMessageContainer;
    using Message = TMessage;

    static CoSimType GetCoSimType() {
        return TheCoSimType;
    }

    static ConnectionKind GetConnectionKind() {
        return TheConnectionKind;
    }
};

// #define SINGLE_TEST

#ifdef SINGLE_TEST
using Parameters = Types<Param<LinControllerContainer, LinController, LinMessageContainer, LinMessage, CoSimType::Server, ConnectionKind::Remote>>;
#else
using Parameters = Types<Param<CanControllerContainer, CanController, CanMessageContainer, CanMessage, CoSimType::Client, ConnectionKind::Local>,
                         Param<CanControllerContainer, CanController, CanMessageContainer, CanMessage, CoSimType::Server, ConnectionKind::Remote>,
                         Param<EthControllerContainer, EthController, EthMessageContainer, EthMessage, CoSimType::Client, ConnectionKind::Remote>,
                         Param<EthControllerContainer, EthController, EthMessageContainer, EthMessage, CoSimType::Server, ConnectionKind::Local>,
                         Param<LinControllerContainer, LinController, LinMessageContainer, LinMessage, CoSimType::Client, ConnectionKind::Local>,
                         Param<LinControllerContainer, LinController, LinMessageContainer, LinMessage, CoSimType::Server, ConnectionKind::Remote>,
                         Param<FrControllerContainer, FrController, FrMessageContainer, FrMessage, CoSimType::Client, ConnectionKind::Remote>,
                         Param<FrControllerContainer, FrController, FrMessageContainer, FrMessage, CoSimType::Server, ConnectionKind::Local>>;
#endif

class NameGenerator {
public:
    template <typename T>
    static std::string GetName([[maybe_unused]] int32_t index) {
        using TControllerContainer = typename T::ControllerContainer;
        CoSimType coSimType = T::GetCoSimType();
        ConnectionKind connectionKind = T::GetConnectionKind();

        if constexpr (std::is_same_v<TControllerContainer, CanControllerContainer>) {
            return fmt::format("CAN_{}_{}", coSimType, connectionKind);
        }

        if constexpr (std::is_same_v<TControllerContainer, EthControllerContainer>) {
            return fmt::format("ETH_{}_{}", coSimType, connectionKind);
        }

        if constexpr (std::is_same_v<TControllerContainer, LinControllerContainer>) {
            return fmt::format("LIN_{}_{}", coSimType, connectionKind);
        }

        if constexpr (std::is_same_v<TControllerContainer, FrControllerContainer>) {
            return fmt::format("FR_{}_{}", coSimType, connectionKind);
        }
    }
};

TYPED_TEST_SUITE(TestBusExchange, Parameters, NameGenerator);

TYPED_TEST(TestBusExchange, InitializeOneController) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusExchange名前");

    TControllerContainer controllerContainer{};
    FillWithRandom(controllerContainer);

    TController controller = controllerContainer.Convert();

    std::unique_ptr<BusExchange> busExchange;

    std::unique_ptr<IProtocol> protocol;
    AssertOk(CreateProtocol(ProtocolVersionLatest, protocol));

    // Act
    Result result = CreateBusExchange(coSimType, connectionKind, name, {controller}, *protocol, busExchange);

    // Assert
    AssertOk(result);
}

TYPED_TEST(TestBusExchange, InitializeMultipleControllers) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusExchange名前");

    TControllerContainer controllerContainer1{};
    FillWithRandom(controllerContainer1);

    TController controller1 = controllerContainer1.Convert();

    TControllerContainer controllerContainer2{};
    FillWithRandom(controllerContainer2);

    TController controller2 = controllerContainer2.Convert();

    std::unique_ptr<BusExchange> busExchange;

    std::unique_ptr<IProtocol> protocol;
    AssertOk(CreateProtocol(ProtocolVersionLatest, protocol));

    // Act and assert
    AssertOk(CreateBusExchange(coSimType, connectionKind, name, {controller1, controller2}, *protocol, busExchange));
}

TYPED_TEST(TestBusExchange, TransmitMessageContainer) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusExchange名前");

    TControllerContainer controllerContainer{};
    FillWithRandom(controllerContainer);

    TController controller = controllerContainer.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(CreateProtocol(ProtocolVersionLatest, protocol));

    std::unique_ptr<BusExchange> busExchange;
    AssertOk(CreateBusExchange(coSimType, connectionKind, name, {controller}, *protocol, busExchange));

    TMessageContainer sendMessageContainer{};
    FillWithRandom(sendMessageContainer, controller.id);

    // Act and assert
    AssertOk(busExchange->Transmit(sendMessageContainer));
}

TYPED_TEST(TestBusExchange, TransmitMessage) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;
    using TMessage = typename TypeParam::Message;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusExchange名前");

    TControllerContainer controllerContainer{};
    FillWithRandom(controllerContainer);

    TController controller = controllerContainer.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(CreateProtocol(ProtocolVersionLatest, protocol));

    std::unique_ptr<BusExchange> busExchange;
    AssertOk(CreateBusExchange(coSimType, connectionKind, name, {controller}, *protocol, busExchange));

    TMessageContainer sendMessageContainer{};
    FillWithRandom(sendMessageContainer, controller.id);

    TMessage sendMessage{};
    sendMessageContainer.WriteTo(sendMessage);

    // Act and assert
    AssertOk(busExchange->Transmit(sendMessage));
}

TYPED_TEST(TestBusExchange, TransmitMessageContainerWhenBufferIsFull) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusExchange名前");

    TControllerContainer controllerContainer{};
    FillWithRandom(controllerContainer);

    TController controller = controllerContainer.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(CreateProtocol(ProtocolVersionLatest, protocol));

    std::unique_ptr<BusExchange> busExchange;
    AssertOk(CreateBusExchange(coSimType, connectionKind, name, {controller}, *protocol, busExchange));

    // Fill queue
    for (uint32_t i = 0; i < controller.queueSize; i++) {
        TMessageContainer sendMessageContainer{};
        FillWithRandom(sendMessageContainer, controller.id);

        AssertOk(busExchange->Transmit(sendMessageContainer));
    }

    TMessageContainer rejectedMessageContainer{};
    FillWithRandom(rejectedMessageContainer, controller.id);

    // Act
    Result result = busExchange->Transmit(rejectedMessageContainer);

    // Assert
    AssertFull(result);
}

TYPED_TEST(TestBusExchange, TransmitMessageWhenBufferIsFull) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;
    using TMessage = typename TypeParam::Message;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusExchange名前");

    TControllerContainer controllerContainer{};
    FillWithRandom(controllerContainer);

    TController controller = controllerContainer.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(CreateProtocol(ProtocolVersionLatest, protocol));

    std::unique_ptr<BusExchange> busExchange;
    AssertOk(CreateBusExchange(coSimType, connectionKind, name, {controller}, *protocol, busExchange));

    // Fill queue
    for (uint32_t i = 0; i < controller.queueSize; i++) {
        TMessageContainer sendMessageContainer{};
        FillWithRandom(sendMessageContainer, controller.id);

        TMessage sendMessage{};
        sendMessageContainer.WriteTo(sendMessage);

        AssertOk(busExchange->Transmit(sendMessage));
    }

    TMessageContainer rejectedMessageContainer{};
    FillWithRandom(rejectedMessageContainer, controller.id);

    TMessage rejectedMessage{};
    rejectedMessageContainer.WriteTo(rejectedMessage);

    // Act
    Result result = busExchange->Transmit(rejectedMessage);

    // Assert
    AssertFull(result);
}

TYPED_TEST(TestBusExchange, TransmitMessageContainerWhenBufferIsOnlyFullForSpecificController) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusExchange名前");

    TControllerContainer controllerContainer1{};
    FillWithRandom(controllerContainer1);

    TController controller1 = controllerContainer1.Convert();

    TControllerContainer controllerContainer2{};
    FillWithRandom(controllerContainer2);

    TController controller2 = controllerContainer2.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(CreateProtocol(ProtocolVersionLatest, protocol));

    std::unique_ptr<BusExchange> busExchange;
    AssertOk(CreateBusExchange(coSimType, connectionKind, name, {controller1, controller2}, *protocol, busExchange));

    // Fill queue
    for (uint32_t i = 0; i < controller1.queueSize; i++) {
        TMessageContainer sendMessageContainer{};
        FillWithRandom(sendMessageContainer, controller1.id);

        AssertOk(busExchange->Transmit(sendMessageContainer));
    }

    TMessageContainer rejectedMessageContainer{};
    FillWithRandom(rejectedMessageContainer, controller1.id);

    // Act
    Result result = busExchange->Transmit(rejectedMessageContainer);

    // Assert
    AssertFull(result);
}

TYPED_TEST(TestBusExchange, TransmitMessageWhenBufferIsOnlyFullForSpecificController) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;
    using TMessage = typename TypeParam::Message;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusExchange名前");

    TControllerContainer controllerContainer1{};
    FillWithRandom(controllerContainer1);

    TController controller1 = controllerContainer1.Convert();

    TControllerContainer controllerContainer2{};
    FillWithRandom(controllerContainer2);

    TController controller2 = controllerContainer2.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(CreateProtocol(ProtocolVersionLatest, protocol));

    std::unique_ptr<BusExchange> busExchange;
    AssertOk(CreateBusExchange(coSimType, connectionKind, name, {controller1, controller2}, *protocol, busExchange));

    // Fill queue
    for (uint32_t i = 0; i < controller1.queueSize; i++) {
        TMessageContainer sendMessageContainer{};
        FillWithRandom(sendMessageContainer, controller1.id);

        TMessage sendMessage{};
        sendMessageContainer.WriteTo(sendMessage);

        AssertOk(busExchange->Transmit(sendMessage));
    }

    TMessageContainer rejectedMessageContainer{};
    FillWithRandom(rejectedMessageContainer, controller1.id);

    TMessage rejectedMessage{};
    rejectedMessageContainer.WriteTo(rejectedMessage);

    // Act
    Result result = busExchange->Transmit(rejectedMessage);

    // Assert
    AssertFull(result);
}

TYPED_TEST(TestBusExchange, TransmitMessageContainerWhenBufferIsFullForOtherController) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusExchange名前");

    TControllerContainer controllerContainer1{};
    FillWithRandom(controllerContainer1);

    TController controller1 = controllerContainer1.Convert();

    TControllerContainer controllerContainer2{};
    FillWithRandom(controllerContainer2);

    TController controller2 = controllerContainer2.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(CreateProtocol(ProtocolVersionLatest, protocol));

    std::unique_ptr<BusExchange> busExchange;
    AssertOk(CreateBusExchange(coSimType, connectionKind, name, {controller1, controller2}, *protocol, busExchange));

    // Fill queue
    for (uint32_t i = 0; i < controller1.queueSize; i++) {
        TMessageContainer sendMessageContainer{};
        FillWithRandom(sendMessageContainer, controller1.id);

        AssertOk(busExchange->Transmit(sendMessageContainer));
    }

    TMessageContainer acceptedMessageContainer{};
    FillWithRandom(acceptedMessageContainer, controller2.id);

    // Act and assert
    AssertOk(busExchange->Transmit(acceptedMessageContainer));
}

TYPED_TEST(TestBusExchange, TransmitMessageWhenBufferIsFullForOtherController) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;
    using TMessage = typename TypeParam::Message;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusExchange名前");

    TControllerContainer controllerContainer1{};
    FillWithRandom(controllerContainer1);

    TController controller1 = controllerContainer1.Convert();

    TControllerContainer controllerContainer2{};
    FillWithRandom(controllerContainer2);

    TController controller2 = controllerContainer2.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(CreateProtocol(ProtocolVersionLatest, protocol));

    std::unique_ptr<BusExchange> busExchange;
    AssertOk(CreateBusExchange(coSimType, connectionKind, name, {controller1, controller2}, *protocol, busExchange));

    // Fill queue
    for (uint32_t i = 0; i < controller1.queueSize; i++) {
        TMessageContainer sendMessageContainer{};
        FillWithRandom(sendMessageContainer, controller1.id);

        TMessage sendMessage{};
        sendMessageContainer.WriteTo(sendMessage);

        AssertOk(busExchange->Transmit(sendMessage));
    }

    TMessageContainer acceptedMessageContainer{};
    FillWithRandom(acceptedMessageContainer, controller2.id);

    TMessage acceptedMessage{};
    acceptedMessageContainer.WriteTo(acceptedMessage);

    // Act and assert
    AssertOk(busExchange->Transmit(acceptedMessage));
}

TYPED_TEST(TestBusExchange, ReceiveMessageContainerOnEmptyBuffer) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusExchange名前");

    TControllerContainer controllerContainer{};
    FillWithRandom(controllerContainer);

    TController controller = controllerContainer.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(CreateProtocol(ProtocolVersionLatest, protocol));

    std::unique_ptr<BusExchange> senderBusExchange;
    AssertOk(CreateBusExchange(coSimType, connectionKind, name, {controller}, *protocol, senderBusExchange));

    std::unique_ptr<BusExchange> receiverBusExchange;
    AssertOk(CreateBusExchange(GetCounterPart(coSimType), connectionKind, GetCounterPart(name, connectionKind), {controller}, *protocol, receiverBusExchange));

    TestBusExchange<TypeParam>::Transfer(connectionKind, *senderBusExchange, *receiverBusExchange);

    TMessageContainer receivedMessageContainer{};

    // Act and assert
    AssertEmpty(receiverBusExchange->Receive(receivedMessageContainer));
}

TYPED_TEST(TestBusExchange, ReceiveMessageOnEmptyBuffer) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessage = typename TypeParam::Message;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusExchange名前");

    TControllerContainer controllerContainer{};
    FillWithRandom(controllerContainer);

    TController controller = controllerContainer.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(CreateProtocol(ProtocolVersionLatest, protocol));

    std::unique_ptr<BusExchange> senderBusExchange;
    AssertOk(CreateBusExchange(coSimType, connectionKind, name, {controller}, *protocol, senderBusExchange));

    std::unique_ptr<BusExchange> receiverBusExchange;
    AssertOk(CreateBusExchange(GetCounterPart(coSimType), connectionKind, GetCounterPart(name, connectionKind), {controller}, *protocol, receiverBusExchange));

    TestBusExchange<TypeParam>::Transfer(connectionKind, *senderBusExchange, *receiverBusExchange);

    TMessage receivedMessage{};

    // Act and assert
    AssertEmpty(receiverBusExchange->Receive(receivedMessage));
}

TYPED_TEST(TestBusExchange, ReceiveMessageContainerOnEmptyBufferByEvent) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusExchange名前");

    TControllerContainer controllerContainer{};
    FillWithRandom(controllerContainer);

    TController controller = controllerContainer.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(CreateProtocol(ProtocolVersionLatest, protocol));

    std::unique_ptr<BusExchange> senderBusExchange;
    AssertOk(CreateBusExchange(coSimType, connectionKind, name, {controller}, *protocol, senderBusExchange));

    std::unique_ptr<BusExchange> receiverBusExchange;
    AssertOk(CreateBusExchange(GetCounterPart(coSimType), connectionKind, GetCounterPart(name, connectionKind), {controller}, *protocol, receiverBusExchange));

    std::deque<std::tuple<TController, TMessageContainer>> expectedEvents;

    // Act and assert
    TestBusExchange<TypeParam>::Transfer(connectionKind, *senderBusExchange, *receiverBusExchange, expectedEvents);
}

TYPED_TEST(TestBusExchange, ReceiveMessageOnEmptyBufferByEvent) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessage = typename TypeParam::Message;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusExchange名前");

    TControllerContainer controllerContainer{};
    FillWithRandom(controllerContainer);

    TController controller = controllerContainer.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(CreateProtocol(ProtocolVersionLatest, protocol));

    std::unique_ptr<BusExchange> senderBusExchange;
    AssertOk(CreateBusExchange(coSimType, connectionKind, name, {controller}, *protocol, senderBusExchange));

    std::unique_ptr<BusExchange> receiverBusExchange;
    AssertOk(CreateBusExchange(GetCounterPart(coSimType), connectionKind, GetCounterPart(name, connectionKind), {controller}, *protocol, receiverBusExchange));

    std::deque<std::tuple<TController, TMessage>> expectedEvents;

    // Act and assert
    TestBusExchange<TypeParam>::Transfer(connectionKind, *senderBusExchange, *receiverBusExchange, expectedEvents);
}

TYPED_TEST(TestBusExchange, ReceiveTransmittedMessageContainer) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusExchange名前");

    TControllerContainer controllerContainer{};
    FillWithRandom(controllerContainer);

    TController controller = controllerContainer.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(CreateProtocol(ProtocolVersionLatest, protocol));

    std::unique_ptr<BusExchange> senderBusExchange;
    AssertOk(CreateBusExchange(coSimType, connectionKind, name, {controller}, *protocol, senderBusExchange));

    std::unique_ptr<BusExchange> receiverBusExchange;
    AssertOk(CreateBusExchange(GetCounterPart(coSimType), connectionKind, GetCounterPart(name, connectionKind), {controller}, *protocol, receiverBusExchange));

    TMessageContainer sendMessageContainer{};
    FillWithRandom(sendMessageContainer, controller.id);

    AssertOk(senderBusExchange->Transmit(sendMessageContainer));

    TestBusExchange<TypeParam>::Transfer(connectionKind, *senderBusExchange, *receiverBusExchange);

    TMessageContainer receivedMessageContainer{};

    // Act
    AssertOk(receiverBusExchange->Receive(receivedMessageContainer));

    // Assert
    ASSERT_EQ(sendMessageContainer, receivedMessageContainer);
}

TYPED_TEST(TestBusExchange, TransmitAndReceiveMessageContainerSimultaniously) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusExchange名前");

    TControllerContainer controllerContainer{};
    FillWithRandom(controllerContainer);

    TController controller = controllerContainer.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(CreateProtocol(ProtocolVersionLatest, protocol));

    std::unique_ptr<BusExchange> busExchange1;
    AssertOk(CreateBusExchange(coSimType, connectionKind, name, {controller}, *protocol, busExchange1));

    std::unique_ptr<BusExchange> busExchange2;
    AssertOk(CreateBusExchange(GetCounterPart(coSimType), connectionKind, GetCounterPart(name, connectionKind), {controller}, *protocol, busExchange2));

    TMessageContainer sendMessageContainer1{};
    FillWithRandom(sendMessageContainer1, controller.id);

    AssertOk(busExchange1->Transmit(sendMessageContainer1));

    TMessageContainer sendMessageContainer2{};
    FillWithRandom(sendMessageContainer2, controller.id);

    AssertOk(busExchange2->Transmit(sendMessageContainer2));

    TestBusExchange<TypeParam>::Transfer(connectionKind, *busExchange1, *busExchange2);
    TestBusExchange<TypeParam>::Transfer(connectionKind, *busExchange2, *busExchange1);

    TMessageContainer receivedMessageContainer1{};
    TMessageContainer receivedMessageContainer2{};

    // Act
    AssertOk(busExchange2->Receive(receivedMessageContainer1));
    AssertOk(busExchange1->Receive(receivedMessageContainer2));

    // Assert
    ASSERT_EQ(sendMessageContainer1, receivedMessageContainer1);
    ASSERT_EQ(sendMessageContainer2, receivedMessageContainer2);
}

TYPED_TEST(TestBusExchange, ReceiveTransmittedMessage) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;
    using TMessage = typename TypeParam::Message;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusExchange名前");

    TControllerContainer controllerContainer{};
    FillWithRandom(controllerContainer);

    TController controller = controllerContainer.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(CreateProtocol(ProtocolVersionLatest, protocol));

    std::unique_ptr<BusExchange> senderBusExchange;
    AssertOk(CreateBusExchange(coSimType, connectionKind, name, {controller}, *protocol, senderBusExchange));

    std::unique_ptr<BusExchange> receiverBusExchange;
    AssertOk(CreateBusExchange(GetCounterPart(coSimType), connectionKind, GetCounterPart(name, connectionKind), {controller}, *protocol, receiverBusExchange));

    TMessageContainer sendMessageContainer{};
    FillWithRandom(sendMessageContainer, controller.id);

    TMessage sendMessage{};
    sendMessageContainer.WriteTo(sendMessage);

    AssertOk(senderBusExchange->Transmit(sendMessage));

    TestBusExchange<TypeParam>::Transfer(connectionKind, *senderBusExchange, *receiverBusExchange);

    TMessage receivedMessage{};

    // Act
    AssertOk(receiverBusExchange->Receive(receivedMessage));

    // Assert
    ASSERT_EQ(sendMessage, receivedMessage);
}

TYPED_TEST(TestBusExchange, ReceiveTransmittedMessageContainerByEvent) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusExchange名前");

    TControllerContainer controllerContainer{};
    FillWithRandom(controllerContainer);

    TController controller = controllerContainer.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(CreateProtocol(ProtocolVersionLatest, protocol));

    std::unique_ptr<BusExchange> senderBusExchange;
    AssertOk(CreateBusExchange(coSimType, connectionKind, name, {controller}, *protocol, senderBusExchange));

    std::unique_ptr<BusExchange> receiverBusExchange;
    AssertOk(CreateBusExchange(GetCounterPart(coSimType), connectionKind, GetCounterPart(name, connectionKind), {controller}, *protocol, receiverBusExchange));

    std::deque<std::tuple<TController, TMessageContainer>> expectedEvents;

    TMessageContainer sendMessageContainer{};
    FillWithRandom(sendMessageContainer, controller.id);

    AssertOk(senderBusExchange->Transmit(sendMessageContainer));
    expectedEvents.push_back({controller, sendMessageContainer});

    // Act and assert
    TestBusExchange<TypeParam>::Transfer(connectionKind, *senderBusExchange, *receiverBusExchange, expectedEvents);
}

TYPED_TEST(TestBusExchange, ReceiveTransmittedMessageByEvent) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;
    using TMessage = typename TypeParam::Message;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusExchange名前");

    TControllerContainer controllerContainer{};
    FillWithRandom(controllerContainer);

    TController controller = controllerContainer.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(CreateProtocol(ProtocolVersionLatest, protocol));

    std::unique_ptr<BusExchange> senderBusExchange;
    AssertOk(CreateBusExchange(coSimType, connectionKind, name, {controller}, *protocol, senderBusExchange));

    std::unique_ptr<BusExchange> receiverBusExchange;
    AssertOk(CreateBusExchange(GetCounterPart(coSimType), connectionKind, GetCounterPart(name, connectionKind), {controller}, *protocol, receiverBusExchange));

    std::deque<std::tuple<TController, TMessage>> expectedEvents;

    TMessageContainer sendMessageContainer{};
    FillWithRandom(sendMessageContainer, controller.id);

    TMessage sendMessage{};
    sendMessageContainer.WriteTo(sendMessage);

    AssertOk(senderBusExchange->Transmit(sendMessage));
    expectedEvents.push_back({controller, sendMessage});

    // Act and assert
    TestBusExchange<TypeParam>::Transfer(connectionKind, *senderBusExchange, *receiverBusExchange, expectedEvents);
}

TYPED_TEST(TestBusExchange, ReceiveMultipleTransmittedMessageContainers) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusExchange名前");

    TControllerContainer controllerContainer1{};
    FillWithRandom(controllerContainer1);

    TController controller1 = controllerContainer1.Convert();

    TControllerContainer controllerContainer2{};
    FillWithRandom(controllerContainer2);

    TController controller2 = controllerContainer2.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(CreateProtocol(ProtocolVersionLatest, protocol));

    std::unique_ptr<BusExchange> senderBusExchange;
    AssertOk(CreateBusExchange(coSimType, connectionKind, name, {controller1, controller2}, *protocol, senderBusExchange));

    std::unique_ptr<BusExchange> receiverBusExchange;
    AssertOk(CreateBusExchange(GetCounterPart(coSimType),
                             connectionKind,
                             GetCounterPart(name, connectionKind),
                             {controller1, controller2},
                             *protocol,
                             receiverBusExchange));

    std::deque<TMessageContainer> sendMessageContainers;

    for (uint32_t i = 0; i < controller1.queueSize + controller2.queueSize; i++) {
        BusControllerId controllerId = (i % 2) == 0 ? controller1.id : controller2.id;

        TMessageContainer sendMessageContainer{};
        FillWithRandom(sendMessageContainer, controllerId);

        sendMessageContainers.push_back(sendMessageContainer);
        AssertOk(senderBusExchange->Transmit(sendMessageContainer));
    }

    TestBusExchange<TypeParam>::Transfer(connectionKind, *senderBusExchange, *receiverBusExchange);

    TMessageContainer receivedMessageContainer{};

    // Act and Assert
    for (uint32_t i = 0; i < controller1.queueSize + controller2.queueSize; i++) {
        AssertOk(receiverBusExchange->Receive(receivedMessageContainer));
        ASSERT_EQ(sendMessageContainers[i], receivedMessageContainer);
    }

    AssertEmpty(receiverBusExchange->Receive(receivedMessageContainer));
}

TYPED_TEST(TestBusExchange, ReceiveMultipleTransmittedMessages) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;
    using TMessage = typename TypeParam::Message;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusExchange名前");

    TControllerContainer controllerContainer1{};
    FillWithRandom(controllerContainer1);

    TController controller1 = controllerContainer1.Convert();

    TControllerContainer controllerContainer2{};
    FillWithRandom(controllerContainer2);

    TController controller2 = controllerContainer2.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(CreateProtocol(ProtocolVersionLatest, protocol));

    std::unique_ptr<BusExchange> senderBusExchange;
    AssertOk(CreateBusExchange(coSimType, connectionKind, name, {controller1, controller2}, *protocol, senderBusExchange));

    std::unique_ptr<BusExchange> receiverBusExchange;
    AssertOk(CreateBusExchange(GetCounterPart(coSimType),
                             connectionKind,
                             GetCounterPart(name, connectionKind),
                             {controller1, controller2},
                             *protocol,
                             receiverBusExchange));

    std::vector<TMessageContainer> sendMessageContainers;
    sendMessageContainers.reserve(controller1.queueSize + controller2.queueSize);

    std::vector<TMessage> sendMessages;
    sendMessages.reserve(controller1.queueSize + controller2.queueSize);

    for (uint32_t i = 0; i < controller1.queueSize + controller2.queueSize; i++) {
        BusControllerId controllerId = (i % 2) == 0 ? controller1.id : controller2.id;

        TMessageContainer& sendMessageContainer = sendMessageContainers.emplace_back();
        FillWithRandom(sendMessageContainer, controllerId);

        TMessage sendMessage{};
        sendMessageContainer.WriteTo(sendMessage);

        sendMessages.push_back(sendMessage);
        AssertOk(senderBusExchange->Transmit(sendMessage));
    }

    TestBusExchange<TypeParam>::Transfer(connectionKind, *senderBusExchange, *receiverBusExchange);

    TMessage receivedMessage{};

    // Act and Assert
    for (uint32_t i = 0; i < controller1.queueSize + controller2.queueSize; i++) {
        AssertOk(receiverBusExchange->Receive(receivedMessage));
        ASSERT_EQ(sendMessages[i], receivedMessage);
    }

    AssertEmpty(receiverBusExchange->Receive(receivedMessage));
}

TYPED_TEST(TestBusExchange, ReceiveTransmittedMessageContainersByEventWithTransfer) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusExchange名前");

    TControllerContainer controllerContainer1{};
    FillWithRandom(controllerContainer1);

    TController controller1 = controllerContainer1.Convert();

    TControllerContainer controllerContainer2{};
    FillWithRandom(controllerContainer2);

    TController controller2 = controllerContainer2.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(CreateProtocol(ProtocolVersionLatest, protocol));

    std::unique_ptr<BusExchange> senderBusExchange;
    AssertOk(CreateBusExchange(coSimType, connectionKind, name, {controller1, controller2}, *protocol, senderBusExchange));

    std::unique_ptr<BusExchange> receiverBusExchange;
    AssertOk(CreateBusExchange(GetCounterPart(coSimType),
                             connectionKind,
                             GetCounterPart(name, connectionKind),
                             {controller1, controller2},
                             *protocol,
                             receiverBusExchange));

    std::deque<std::tuple<TController, TMessageContainer>> expectedEvents;

    for (uint32_t i = 0; i < controller1.queueSize + controller2.queueSize; i++) {
        TController* controller = (i % 2) == 0 ? &controller1 : &controller2;

        TMessageContainer sendMessageContainer{};
        FillWithRandom(sendMessageContainer, controller->id);

        expectedEvents.push_back({*controller, sendMessageContainer});
        AssertOk(senderBusExchange->Transmit(sendMessageContainer));
    }

    // Act and assert
    TestBusExchange<TypeParam>::Transfer(connectionKind, *senderBusExchange, *receiverBusExchange, expectedEvents);
}

TYPED_TEST(TestBusExchange, ReceiveTransmittedMessagesByEventWithTransfer) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;
    using TMessage = typename TypeParam::Message;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusExchange名前");

    TControllerContainer controllerContainer1{};
    FillWithRandom(controllerContainer1);

    TController controller1 = controllerContainer1.Convert();

    TControllerContainer controllerContainer2{};
    FillWithRandom(controllerContainer2);

    TController controller2 = controllerContainer2.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(CreateProtocol(ProtocolVersionLatest, protocol));

    std::unique_ptr<BusExchange> senderBusExchange;
    AssertOk(CreateBusExchange(coSimType, connectionKind, name, {controller1, controller2}, *protocol, senderBusExchange));

    std::unique_ptr<BusExchange> receiverBusExchange;
    AssertOk(CreateBusExchange(GetCounterPart(coSimType),
                             connectionKind,
                             GetCounterPart(name, connectionKind),
                             {controller1, controller2},
                             *protocol,
                             receiverBusExchange));

    std::vector<TMessageContainer> sendMessageContainers;
    sendMessageContainers.reserve(controller1.queueSize + controller2.queueSize);

    std::deque<std::tuple<TController, TMessage>> expectedEvents;

    for (uint32_t i = 0; i < controller1.queueSize + controller2.queueSize; i++) {
        TController* controller = (i % 2) == 0 ? &controller1 : &controller2;

        TMessageContainer& sendMessageContainer = sendMessageContainers.emplace_back();
        FillWithRandom(sendMessageContainer, controller->id);

        TMessage sendMessage{};
        sendMessageContainer.WriteTo(sendMessage);

        expectedEvents.push_back({*controller, sendMessage});
        AssertOk(senderBusExchange->Transmit(sendMessage));
    }

    // Act and assert
    TestBusExchange<TypeParam>::Transfer(connectionKind, *senderBusExchange, *receiverBusExchange, expectedEvents);
}

TYPED_TEST(TestBusExchange, DoNotReceiveNotFullyTransmittedMessageContainer) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusExchange名前");
    std::string fakeName = GenerateString("FakeBusExchange名前");

    TControllerContainer controllerContainer{};
    FillWithRandom(controllerContainer);

    TController controller = controllerContainer.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(CreateProtocol(ProtocolVersionLatest, protocol));

    std::unique_ptr<BusExchange> senderBusExchange;
    AssertOk(CreateBusExchange(coSimType, connectionKind, name, {controller}, *protocol, senderBusExchange));

    std::unique_ptr<BusExchange> fakeSenderBusExchange;
    AssertOk(CreateBusExchange(coSimType, connectionKind, fakeName, {controller}, *protocol, fakeSenderBusExchange));

    std::unique_ptr<BusExchange> receiverBusExchange;
    AssertOk(CreateBusExchange(GetCounterPart(coSimType), connectionKind, GetCounterPart(name, connectionKind), {controller}, *protocol, receiverBusExchange));

    TMessageContainer sendMessageContainer{};
    FillWithRandom(sendMessageContainer, controller.id);

    AssertOk(senderBusExchange->Transmit(sendMessageContainer));

    // Should not transfer anything
    TestBusExchange<TypeParam>::Transfer(connectionKind, *fakeSenderBusExchange, *receiverBusExchange);

    TMessageContainer receivedMessageContainer{};

    // Act and assert
    AssertEmpty(receiverBusExchange->Receive(receivedMessageContainer));
}

TYPED_TEST(TestBusExchange, DoNotReceiveNotFullyTransmittedMessage) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;
    using TMessage = typename TypeParam::Message;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusExchange名前");
    std::string fakeName = GenerateString("FakeBusExchange名前");

    TControllerContainer controllerContainer{};
    FillWithRandom(controllerContainer);

    TController controller = controllerContainer.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(CreateProtocol(ProtocolVersionLatest, protocol));

    std::unique_ptr<BusExchange> senderBusExchange;
    AssertOk(CreateBusExchange(coSimType, connectionKind, name, {controller}, *protocol, senderBusExchange));

    std::unique_ptr<BusExchange> fakeSenderBusExchange;
    AssertOk(CreateBusExchange(coSimType, connectionKind, fakeName, {controller}, *protocol, fakeSenderBusExchange));

    std::unique_ptr<BusExchange> receiverBusExchange;
    AssertOk(CreateBusExchange(GetCounterPart(coSimType), connectionKind, GetCounterPart(name, connectionKind), {controller}, *protocol, receiverBusExchange));

    TMessageContainer sendMessageContainer{};
    FillWithRandom(sendMessageContainer, controller.id);

    TMessage sendMessage{};
    sendMessageContainer.WriteTo(sendMessage);

    AssertOk(senderBusExchange->Transmit(sendMessage));

    // Should not transfer anything
    TestBusExchange<TypeParam>::Transfer(connectionKind, *fakeSenderBusExchange, *receiverBusExchange);

    TMessage receivedMessage{};

    // Act and assert
    AssertEmpty(receiverBusExchange->Receive(receivedMessage));
}

TYPED_TEST(TestBusExchange, DoNotReceiveNotFullyTransmittedMessageContainerByEvent) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusExchange名前");
    std::string fakeName = GenerateString("FakeBusExchange名前");

    TControllerContainer controllerContainer{};
    FillWithRandom(controllerContainer);

    TController controller = controllerContainer.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(CreateProtocol(ProtocolVersionLatest, protocol));

    std::unique_ptr<BusExchange> senderBusExchange;
    AssertOk(CreateBusExchange(coSimType, connectionKind, name, {controller}, *protocol, senderBusExchange));

    std::unique_ptr<BusExchange> fakeSenderBusExchange;
    AssertOk(CreateBusExchange(coSimType, connectionKind, fakeName, {controller}, *protocol, fakeSenderBusExchange));

    std::unique_ptr<BusExchange> receiverBusExchange;
    AssertOk(CreateBusExchange(GetCounterPart(coSimType), connectionKind, GetCounterPart(name, connectionKind), {controller}, *protocol, receiverBusExchange));

    std::deque<std::tuple<TController, TMessageContainer>> expectedEvents;

    TMessageContainer sendMessageContainer{};
    FillWithRandom(sendMessageContainer, controller.id);

    AssertOk(senderBusExchange->Transmit(sendMessageContainer));

    // Act and assert
    TestBusExchange<TypeParam>::Transfer(connectionKind, *fakeSenderBusExchange, *receiverBusExchange,
                                       expectedEvents);  // Should not transfer anything
}

TYPED_TEST(TestBusExchange, DoNotReceiveNotFullyTransmittedMessageByEvent) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;
    using TMessage = typename TypeParam::Message;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusExchange名前");
    std::string fakeName = GenerateString("FakeBusExchange名前");

    TControllerContainer controllerContainer{};
    FillWithRandom(controllerContainer);

    TController controller = controllerContainer.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(CreateProtocol(ProtocolVersionLatest, protocol));

    std::unique_ptr<BusExchange> senderBusExchange;
    AssertOk(CreateBusExchange(coSimType, connectionKind, name, {controller}, *protocol, senderBusExchange));

    std::unique_ptr<BusExchange> fakeSenderBusExchange;
    AssertOk(CreateBusExchange(coSimType, connectionKind, fakeName, {controller}, *protocol, fakeSenderBusExchange));

    std::unique_ptr<BusExchange> receiverBusExchange;
    AssertOk(CreateBusExchange(GetCounterPart(coSimType), connectionKind, GetCounterPart(name, connectionKind), {controller}, *protocol, receiverBusExchange));

    std::deque<std::tuple<TController, TMessage>> expectedEvents;

    TMessageContainer sendMessageContainer{};
    FillWithRandom(sendMessageContainer, controller.id);

    TMessage sendMessage{};
    sendMessageContainer.WriteTo(sendMessage);

    AssertOk(senderBusExchange->Transmit(sendMessage));

    // Act and assert
    TestBusExchange<TypeParam>::Transfer(connectionKind, *fakeSenderBusExchange, *receiverBusExchange,
                                       expectedEvents);  // Should not transfer anything
}

}  // namespace
