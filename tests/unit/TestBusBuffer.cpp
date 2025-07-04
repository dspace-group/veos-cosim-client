// Copyright dSPACE GmbH. All rights reserved.

#include <fmt/format.h>

#include <gtest/gtest.h>

#include <deque>
#include <memory>
#include <optional>
#include <string>
#include <thread>

#include "BusBuffer.h"
#include "DsVeosCoSim/CoSimTypes.h"
#include "Generator.h"
#include "Helper.h"
#include "LogHelper.h"
#include "TestHelper.h"

using testing::Types;

using namespace DsVeosCoSim;
using namespace testing;

namespace {

template <typename Types>
class TestBusBuffer : public Test {
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
        ExpectOk(CreateTcpChannelServer(0, true, remoteServer));
        ExpectTrue(remoteServer);
        std::optional<uint16_t> port = remoteServer->GetLocalPort();
        ExpectTrue(port);

        ExpectOk(TryConnectToTcpChannel("127.0.0.1", *port, 0, DefaultTimeout, _remoteSenderChannel));
        ExpectTrue(_remoteSenderChannel);
        ExpectOk(remoteServer->TryAccept(_remoteReceiverChannel));
        ExpectTrue(_remoteReceiverChannel);

#ifdef _WIN32
        std::string name = GenerateString("LocalChannel名前");
        std::unique_ptr<ChannelServer> localServer;
        ExpectOk(CreateLocalChannelServer(name, localServer));
        ExpectTrue(localServer);

        ExpectOk(TryConnectToLocalChannel(name, _localSenderChannel));
        ExpectTrue(_localSenderChannel);
        ExpectOk(localServer->TryAccept(_localReceiverChannel));
        ExpectTrue(_localReceiverChannel);
#else
        std::string name = GenerateString("UdsChannel名前");
        std::unique_ptr<ChannelServer> localServer;
        ExpectOk(CreateUdsChannelServer(name, localServer));
        ExpectTrue(localServer);

        ExpectOk(TryConnectToUdsChannel(name, _localSenderChannel));
        ExpectTrue(_localSenderChannel);
        ExpectOk(localServer->TryAccept(_localReceiverChannel));
        ExpectTrue(_localReceiverChannel);
#endif
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

    void SetUp() override {
        ClearLastMessage();
    }

    void Transfer(ConnectionKind connectionKind, const BusBuffer& senderBusBuffer, const BusBuffer& receiverBusBuffer) {
        ChannelReader& reader = connectionKind == ConnectionKind::Remote ? _remoteReceiverChannel->GetReader()
                                                                         : _localReceiverChannel->GetReader();
        ChannelWriter& writer = connectionKind == ConnectionKind::Remote ? _remoteSenderChannel->GetWriter()
                                                                         : _localSenderChannel->GetWriter();
        std::thread thread([&] {
            ExpectOk(receiverBusBuffer.Deserialize(reader, {}, {}));
        });

        ExpectOk(senderBusBuffer.Serialize(writer));
        ExpectOk(writer.EndWrite());

        thread.join();
    }

    void Transfer(ConnectionKind connectionKind,
                  BusBuffer& senderBusBuffer,
                  BusBuffer& receiverBusBuffer,
                  std::deque<std::tuple<TController, TMessageContainer>>& expectedCallbacks) {
        ChannelReader& reader = connectionKind == ConnectionKind::Remote ? _remoteReceiverChannel->GetReader()
                                                                         : _localReceiverChannel->GetReader();
        ChannelWriter& writer = connectionKind == ConnectionKind::Remote ? _remoteSenderChannel->GetWriter()
                                                                         : _localSenderChannel->GetWriter();

        SimulationTime simulationTime = GenerateSimulationTime();

        Callbacks callbacks{};
        if constexpr (std::is_same_v<TController, CanController>) {
            callbacks.canMessageContainerReceivedCallback = [&](SimulationTime simTime,
                                                                const CanController& controller,
                                                                const CanMessageContainer& messageContainer) {
                AssertEq(simTime, simulationTime);
                AssertFalse(expectedCallbacks.empty());
                auto& [expectedController, expectedMessage] = expectedCallbacks.front();
                AssertEq(expectedController, controller);
                AssertEq(expectedMessage, messageContainer);
                expectedCallbacks.pop_front();
            };
        }

        if constexpr (std::is_same_v<TController, EthController>) {
            callbacks.ethMessageContainerReceivedCallback = [&](SimulationTime simTime,
                                                                const EthController& controller,
                                                                const EthMessageContainer& messageContainer) {
                AssertEq(simTime, simulationTime);
                AssertFalse(expectedCallbacks.empty());
                auto& [expectedController, expectedMessage] = expectedCallbacks.front();
                AssertEq(expectedController, controller);
                AssertEq(expectedMessage, messageContainer);
                expectedCallbacks.pop_front();
            };
        }

        if constexpr (std::is_same_v<TController, LinController>) {
            callbacks.linMessageContainerReceivedCallback = [&](SimulationTime simTime,
                                                                const LinController& controller,
                                                                const LinMessageContainer& messageContainer) {
                AssertEq(simTime, simulationTime);
                AssertFalse(expectedCallbacks.empty());
                auto& [expectedController, expectedMessage] = expectedCallbacks.front();
                AssertEq(expectedController, controller);
                AssertEq(expectedMessage, messageContainer);
                expectedCallbacks.pop_front();
            };
        }

        std::thread thread([&] {
            AssertOk(receiverBusBuffer.Deserialize(reader, simulationTime, callbacks));
        });

        AssertOk(senderBusBuffer.Serialize(writer));
        AssertOk(writer.EndWrite());

        thread.join();

        AssertTrue(expectedCallbacks.empty());
    }
};

template <typename Types>
std::unique_ptr<Channel> TestBusBuffer<Types>::_remoteSenderChannel;

template <typename Types>
std::unique_ptr<Channel> TestBusBuffer<Types>::_remoteReceiverChannel;

template <typename Types>
std::unique_ptr<Channel> TestBusBuffer<Types>::_localSenderChannel;

template <typename Types>
std::unique_ptr<Channel> TestBusBuffer<Types>::_localReceiverChannel;

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
using Parameters = Types<Param<CanControllerContainer,
                               CanController,
                               CanMessageContainer,
                               CanMessage,
                               CoSimType::Client,
                               ConnectionKind::Local>>;
#else
using Parameters = Types<Param<CanControllerContainer,
                               CanController,
                               CanMessageContainer,
                               CanMessage,
                               CoSimType::Client,
                               ConnectionKind::Local>,
                         Param<CanControllerContainer,
                               CanController,
                               CanMessageContainer,
                               CanMessage,
                               CoSimType::Client,
                               ConnectionKind::Remote>,
                         Param<CanControllerContainer,
                               CanController,
                               CanMessageContainer,
                               CanMessage,
                               CoSimType::Server,
                               ConnectionKind::Local>,
                         Param<CanControllerContainer,
                               CanController,
                               CanMessageContainer,
                               CanMessage,
                               CoSimType::Server,
                               ConnectionKind::Remote>,
                         Param<EthControllerContainer,
                               EthController,
                               EthMessageContainer,
                               EthMessage,
                               CoSimType::Client,
                               ConnectionKind::Local>,
                         Param<EthControllerContainer,
                               EthController,
                               EthMessageContainer,
                               EthMessage,
                               CoSimType::Client,
                               ConnectionKind::Remote>,
                         Param<EthControllerContainer,
                               EthController,
                               EthMessageContainer,
                               EthMessage,
                               CoSimType::Server,
                               ConnectionKind::Local>,
                         Param<EthControllerContainer,
                               EthController,
                               EthMessageContainer,
                               EthMessage,
                               CoSimType::Server,
                               ConnectionKind::Remote>,
                         Param<LinControllerContainer,
                               LinController,
                               LinMessageContainer,
                               LinMessage,
                               CoSimType::Client,
                               ConnectionKind::Local>,
                         Param<LinControllerContainer,
                               LinController,
                               LinMessageContainer,
                               LinMessage,
                               CoSimType::Client,
                               ConnectionKind::Remote>,
                         Param<LinControllerContainer,
                               LinController,
                               LinMessageContainer,
                               LinMessage,
                               CoSimType::Server,
                               ConnectionKind::Local>,
                         Param<LinControllerContainer,
                               LinController,
                               LinMessageContainer,
                               LinMessage,
                               CoSimType::Server,
                               ConnectionKind::Remote>>;
#endif

class NameGenerator {
public:
    template <typename T>
    static std::string GetName([[maybe_unused]] int32_t index) {
        using TControllerContainer = typename T::ControllerContainer;
        CoSimType coSimType = T::GetCoSimType();
        ConnectionKind connectionKind = T::GetConnectionKind();

        if constexpr (std::is_same_v<TControllerContainer, CanControllerContainer>) {
            return fmt::format("CAN_{}_{}", ToString(coSimType), ToString(connectionKind));
        }

        if constexpr (std::is_same_v<TControllerContainer, EthControllerContainer>) {
            return fmt::format("ETH_{}_{}", ToString(coSimType), ToString(connectionKind));
        }

        if constexpr (std::is_same_v<TControllerContainer, LinControllerContainer>) {
            return fmt::format("LIN_{}_{}", ToString(coSimType), ToString(connectionKind));
        }
    }
};

TYPED_TEST_SUITE(TestBusBuffer, Parameters, NameGenerator);

TYPED_TEST(TestBusBuffer, InitializeOneController) {
    using TControllerContainer = typename TypeParam::ControllerContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controller{};
    FillWithRandom(controller);

    std::unique_ptr<BusBuffer> busBuffer;

    // Act and assert
    AssertOk(CreateBusBuffer(coSimType, connectionKind, name, {Convert(controller)}, busBuffer));
}

TYPED_TEST(TestBusBuffer, InitializeMultipleControllers) {
    using TControllerContainer = typename TypeParam::ControllerContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controller1{};
    FillWithRandom(controller1);
    TControllerContainer controller2{};
    FillWithRandom(controller2);

    std::unique_ptr<BusBuffer> busBuffer;

    // Act and assert
    AssertOk(CreateBusBuffer(coSimType, connectionKind, name, {Convert(controller1), Convert(controller2)}, busBuffer));
}

TYPED_TEST(TestBusBuffer, TransmitMessageContainer) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controller{};
    FillWithRandom(controller);

    std::unique_ptr<BusBuffer> busBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {Convert(controller)}, busBuffer));

    TMessageContainer sendMessage{};
    FillWithRandom(sendMessage, controller.id);

    // Act and assert
    AssertOk(busBuffer->Transmit(sendMessage));
}

TYPED_TEST(TestBusBuffer, TransmitMessageContainerWithMove) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controller{};
    FillWithRandom(controller);

    std::unique_ptr<BusBuffer> busBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {Convert(controller)}, busBuffer));

    TMessageContainer sendMessage{};
    FillWithRandom(sendMessage, controller.id);

    // Act and assert
    AssertOk(busBuffer->Transmit(std::move(sendMessage)));
}

TYPED_TEST(TestBusBuffer, TransmitMessageWhenBufferIsFull) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controller{};
    FillWithRandom(controller);

    std::unique_ptr<BusBuffer> busBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {Convert(controller)}, busBuffer));

    // Fill queue
    for (uint32_t i = 0; i < controller.queueSize; i++) {
        TMessageContainer sendMessage{};
        FillWithRandom(sendMessage, controller.id);
        ExpectOk(busBuffer->Transmit(sendMessage));
    }

    TMessageContainer rejectedMessage{};
    FillWithRandom(rejectedMessage, controller.id);

    // Act
    AssertFull(busBuffer->Transmit(rejectedMessage));

    // Assert
    AssertLastMessage(
        fmt::format("Transmit buffer for controller '{}' is full. Messages are dropped.", controller.name));
}

TYPED_TEST(TestBusBuffer, TransmitMessageWhenBufferIsOnlyFullForSpecificController) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controller1{};
    FillWithRandom(controller1);
    TControllerContainer controller2{};
    FillWithRandom(controller2);

    std::unique_ptr<BusBuffer> busBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {Convert(controller1), Convert(controller2)}, busBuffer));

    // Fill queue
    for (uint32_t i = 0; i < controller1.queueSize; i++) {
        TMessageContainer sendMessage{};
        FillWithRandom(sendMessage, controller1.id);
        ExpectOk(busBuffer->Transmit(sendMessage));
    }

    TMessageContainer rejectedMessage{};
    FillWithRandom(rejectedMessage, controller1.id);

    // Act
    AssertFull(busBuffer->Transmit(rejectedMessage));

    // Assert
    AssertLastMessage(
        fmt::format("Transmit buffer for controller '{}' is full. Messages are dropped.", controller1.name));
}

TYPED_TEST(TestBusBuffer, TransmitMessageWhenBufferIsFullForOtherController) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controller1{};
    FillWithRandom(controller1);
    TControllerContainer controller2{};
    FillWithRandom(controller2);

    std::unique_ptr<BusBuffer> busBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {Convert(controller1), Convert(controller2)}, busBuffer));

    // Fill queue
    for (uint32_t i = 0; i < controller1.queueSize; i++) {
        TMessageContainer sendMessage{};
        FillWithRandom(sendMessage, controller1.id);
        ExpectOk(busBuffer->Transmit(sendMessage));
    }

    TMessageContainer acceptedMessage{};
    FillWithRandom(acceptedMessage, controller2.id);

    // Act and assert
    AssertOk(busBuffer->Transmit(acceptedMessage));
}

TYPED_TEST(TestBusBuffer, ReceiveMessageOnEmptyBuffer) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controller{};
    FillWithRandom(controller);

    std::unique_ptr<BusBuffer> senderBusBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {Convert(controller)}, senderBusBuffer));
    std::unique_ptr<BusBuffer> receiverBusBuffer;
    ExpectOk(CreateBusBuffer(GetCounterPart(coSimType),
                             connectionKind,
                             GetCounterPart(name, connectionKind),
                             {Convert(controller)},
                             receiverBusBuffer));

    TestBusBuffer<TypeParam>::Transfer(connectionKind, *senderBusBuffer, *receiverBusBuffer);

    TMessageContainer receivedMessage{};

    // Act and assert
    AssertEmpty(receiverBusBuffer->Receive(receivedMessage));
}

TYPED_TEST(TestBusBuffer, ReceiveMessageOnEmptyBufferByEvent) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controller{};
    FillWithRandom(controller);

    std::unique_ptr<BusBuffer> senderBusBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {Convert(controller)}, senderBusBuffer));
    std::unique_ptr<BusBuffer> receiverBusBuffer;
    ExpectOk(CreateBusBuffer(GetCounterPart(coSimType),
                             connectionKind,
                             GetCounterPart(name, connectionKind),
                             {Convert(controller)},
                             receiverBusBuffer));

    std::deque<std::tuple<TController, TMessageContainer>> expectedEvents;

    // Act and assert
    TestBusBuffer<TypeParam>::Transfer(connectionKind, *senderBusBuffer, *receiverBusBuffer, expectedEvents);
}

TYPED_TEST(TestBusBuffer, ReceiveTransmittedMessage) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controller{};
    FillWithRandom(controller);

    std::unique_ptr<BusBuffer> senderBusBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {Convert(controller)}, senderBusBuffer));
    std::unique_ptr<BusBuffer> receiverBusBuffer;
    ExpectOk(CreateBusBuffer(GetCounterPart(coSimType),
                             connectionKind,
                             GetCounterPart(name, connectionKind),
                             {Convert(controller)},
                             receiverBusBuffer));

    TMessageContainer sendMessage{};
    FillWithRandom(sendMessage, controller.id);
    ExpectOk(senderBusBuffer->Transmit(sendMessage));

    TestBusBuffer<TypeParam>::Transfer(connectionKind, *senderBusBuffer, *receiverBusBuffer);

    TMessageContainer receivedMessage{};

    // Act
    AssertOk(receiverBusBuffer->Receive(receivedMessage));

    // Assert
    AssertEq(sendMessage, receivedMessage);
}

TYPED_TEST(TestBusBuffer, ReceiveTransmittedMessageContainer) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controller{};
    FillWithRandom(controller);

    std::unique_ptr<BusBuffer> senderBusBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {Convert(controller)}, senderBusBuffer));
    std::unique_ptr<BusBuffer> receiverBusBuffer;
    ExpectOk(CreateBusBuffer(GetCounterPart(coSimType),
                             connectionKind,
                             GetCounterPart(name, connectionKind),
                             {Convert(controller)},
                             receiverBusBuffer));

    TMessageContainer sendMessage{};
    FillWithRandom(sendMessage, controller.id);
    ExpectOk(senderBusBuffer->Transmit(sendMessage));

    TestBusBuffer<TypeParam>::Transfer(connectionKind, *senderBusBuffer, *receiverBusBuffer);

    TMessageContainer receivedMessage{};

    // Act
    AssertOk(receiverBusBuffer->Receive(receivedMessage));

    // Assert
    AssertEq(sendMessage, receivedMessage);
}

TYPED_TEST(TestBusBuffer, ReceiveTransmittedMessageByEvent) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controller{};
    FillWithRandom(controller);

    std::unique_ptr<BusBuffer> senderBusBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {Convert(controller)}, senderBusBuffer));
    std::unique_ptr<BusBuffer> receiverBusBuffer;
    ExpectOk(CreateBusBuffer(GetCounterPart(coSimType),
                             connectionKind,
                             GetCounterPart(name, connectionKind),
                             {Convert(controller)},
                             receiverBusBuffer));

    std::deque<std::tuple<TController, TMessageContainer>> expectedEvents;

    TMessageContainer sendMessage{};
    FillWithRandom(sendMessage, controller.id);
    ExpectOk(senderBusBuffer->Transmit(sendMessage));
    expectedEvents.push_back({Convert(controller), sendMessage});

    // Act and assert
    TestBusBuffer<TypeParam>::Transfer(connectionKind, *senderBusBuffer, *receiverBusBuffer, expectedEvents);
}

TYPED_TEST(TestBusBuffer, ReceiveMultipleTransmittedMessages) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controller1{};
    FillWithRandom(controller1);
    TControllerContainer controller2{};
    FillWithRandom(controller2);

    std::unique_ptr<BusBuffer> senderBusBuffer;
    ExpectOk(CreateBusBuffer(coSimType,
                             connectionKind,
                             name,
                             {Convert(controller1), Convert(controller2)},
                             senderBusBuffer));
    std::unique_ptr<BusBuffer> receiverBusBuffer;
    ExpectOk(CreateBusBuffer(GetCounterPart(coSimType),
                             connectionKind,
                             GetCounterPart(name, connectionKind),
                             {Convert(controller1), Convert(controller2)},
                             receiverBusBuffer));

    std::deque<TMessageContainer> sendMessages;

    for (uint32_t i = 0; i < controller1.queueSize + controller2.queueSize; i++) {
        BusControllerId controllerId = (i % 2) == 0 ? controller1.id : controller2.id;
        TMessageContainer sendMessage{};
        FillWithRandom(sendMessage, controllerId);
        sendMessages.push_back(sendMessage);
        ExpectOk(senderBusBuffer->Transmit(sendMessage));
    }

    TestBusBuffer<TypeParam>::Transfer(connectionKind, *senderBusBuffer, *receiverBusBuffer);

    TMessageContainer receivedMessage{};

    // Act and Assert
    for (uint32_t i = 0; i < controller1.queueSize + controller2.queueSize; i++) {
        AssertOk(receiverBusBuffer->Receive(receivedMessage));
        AssertEq(sendMessages[i], receivedMessage);
    }

    AssertEmpty(receiverBusBuffer->Receive(receivedMessage));
}

TYPED_TEST(TestBusBuffer, ReceiveTransmittedMessagesByEventWithTransfer) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controller1{};
    FillWithRandom(controller1);
    TControllerContainer controller2{};
    FillWithRandom(controller2);

    std::unique_ptr<BusBuffer> senderBusBuffer;
    ExpectOk(CreateBusBuffer(coSimType,
                             connectionKind,
                             name,
                             {Convert(controller1), Convert(controller2)},
                             senderBusBuffer));
    std::unique_ptr<BusBuffer> receiverBusBuffer;
    ExpectOk(CreateBusBuffer(GetCounterPart(coSimType),
                             connectionKind,
                             GetCounterPart(name, connectionKind),
                             {Convert(controller1), Convert(controller2)},
                             receiverBusBuffer));

    std::deque<std::tuple<TController, TMessageContainer>> expectedEvents;

    for (uint32_t i = 0; i < controller1.queueSize + controller2.queueSize; i++) {
        TControllerContainer* controller = (i % 2) == 0 ? &controller1 : &controller2;
        TMessageContainer sendMessage{};
        FillWithRandom(sendMessage, controller->id);
        expectedEvents.push_back({Convert(*controller), sendMessage});
        AssertOk(senderBusBuffer->Transmit(sendMessage));
    }

    // Act and assert
    TestBusBuffer<TypeParam>::Transfer(connectionKind, *senderBusBuffer, *receiverBusBuffer, expectedEvents);
}

TYPED_TEST(TestBusBuffer, DoNotReceiveNotFullyTransmittedMessage) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");
    std::string fakeName = GenerateString("FakeBusBuffer名前");

    TControllerContainer controller{};
    FillWithRandom(controller);

    std::unique_ptr<BusBuffer> senderBusBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {Convert(controller)}, senderBusBuffer));
    std::unique_ptr<BusBuffer> fakeSenderBusBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, fakeName, {Convert(controller)}, fakeSenderBusBuffer));
    std::unique_ptr<BusBuffer> receiverBusBuffer;
    ExpectOk(CreateBusBuffer(GetCounterPart(coSimType),
                             connectionKind,
                             GetCounterPart(name, connectionKind),
                             {Convert(controller)},
                             receiverBusBuffer));

    TMessageContainer sendMessage{};
    FillWithRandom(sendMessage, controller.id);
    ExpectOk(senderBusBuffer->Transmit(sendMessage));

    // Should not transfer anything
    TestBusBuffer<TypeParam>::Transfer(connectionKind, *fakeSenderBusBuffer, *receiverBusBuffer);

    TMessageContainer receivedMessage{};

    // Act and assert
    AssertEmpty(receiverBusBuffer->Receive(receivedMessage));
}

TYPED_TEST(TestBusBuffer, DoNotReceiveNotFullyTransmittedMessageByEvent) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");
    std::string fakeName = GenerateString("FakeBusBuffer名前");

    TControllerContainer controller{};
    FillWithRandom(controller);

    std::unique_ptr<BusBuffer> senderBusBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {Convert(controller)}, senderBusBuffer));
    std::unique_ptr<BusBuffer> fakeSenderBusBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, fakeName, {Convert(controller)}, fakeSenderBusBuffer));
    std::unique_ptr<BusBuffer> receiverBusBuffer;
    ExpectOk(CreateBusBuffer(GetCounterPart(coSimType),
                             connectionKind,
                             GetCounterPart(name, connectionKind),
                             {Convert(controller)},
                             receiverBusBuffer));

    std::deque<std::tuple<TController, TMessageContainer>> expectedEvents;

    TMessageContainer sendMessage{};
    FillWithRandom(sendMessage, controller.id);
    ExpectOk(senderBusBuffer->Transmit(sendMessage));

    // Act and assert
    TestBusBuffer<TypeParam>::Transfer(connectionKind,
                                       *fakeSenderBusBuffer,
                                       *receiverBusBuffer,
                                       expectedEvents);  // Should not transfer anything
}

}  // namespace
