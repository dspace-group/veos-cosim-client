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
    using TController = typename Types::Controller;
    using TControllerExtern = typename Types::ControllerExtern;
    using TMessage = typename Types::Message;
    using TMessageExtern = typename Types::MessageExtern;

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
                  std::deque<std::tuple<TControllerExtern, TMessage>> expectedCallbacks) {
        ChannelReader& reader = connectionKind == ConnectionKind::Remote ? _remoteReceiverChannel->GetReader()
                                                                         : _localReceiverChannel->GetReader();
        ChannelWriter& writer = connectionKind == ConnectionKind::Remote ? _remoteSenderChannel->GetWriter()
                                                                         : _localSenderChannel->GetWriter();

        SimulationTime simulationTime = GenerateSimulationTime();

        Callbacks callbacks{};
        if constexpr (std::is_same_v<TControllerExtern, CanController>) {
            callbacks.canMessageReceivedCallback =
                [&](SimulationTime simTime, const CanController& controller, const CanMessage& message) {
                    AssertEq(simTime, simulationTime);
                    AssertFalse(expectedCallbacks.empty());
                    auto [expectedController, expectedMessage] = expectedCallbacks.front();
                    AssertEq(expectedController, controller);
                    AssertEq(Convert(expectedMessage), message);
                    expectedCallbacks.pop_front();
                };
        }

        if constexpr (std::is_same_v<TControllerExtern, EthController>) {
            callbacks.ethMessageReceivedCallback =
                [&](SimulationTime simTime, const EthController& controller, const EthMessage& message) {
                    AssertEq(simTime, simulationTime);
                    AssertFalse(expectedCallbacks.empty());
                    auto [expectedController, expectedMessage] = expectedCallbacks.front();
                    AssertEq(expectedController, controller);
                    AssertEq(Convert(expectedMessage), message);
                    expectedCallbacks.pop_front();
                };
        }

        if constexpr (std::is_same_v<TControllerExtern, LinController>) {
            callbacks.linMessageReceivedCallback =
                [&](SimulationTime simTime, const LinController& controller, const LinMessage& message) {
                    AssertEq(simTime, simulationTime);
                    AssertFalse(expectedCallbacks.empty());
                    auto [expectedController, expectedMessage] = expectedCallbacks.front();
                    AssertEq(expectedController, controller);
                    AssertEq(Convert(expectedMessage), message);
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

template <typename TController,
          typename TControllerExtern,
          typename TMessage,
          typename TMessageExtern,
          CoSimType TheCoSimType,
          ConnectionKind TheConnectionKind>
struct Param {
    using Controller = TController;
    using ControllerExtern = TControllerExtern;
    using Message = TMessage;
    using MessageExtern = TMessageExtern;

    static CoSimType GetCoSimType() {
        return TheCoSimType;
    }

    static ConnectionKind GetConnectionKind() {
        return TheConnectionKind;
    }
};

// #define SINGLE_TEST

#ifdef SINGLE_TEST
using Parameters = Types<Param<LinControllerContainer,
                               LinController,
                               LinMessageContainer,
                               LinMessage,
                               CoSimType::Client,
                               ConnectionKind::Remote>>;
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
        using TController = typename T::Controller;
        CoSimType coSimType = T::GetCoSimType();
        ConnectionKind connectionKind = T::GetConnectionKind();

        if constexpr (std::is_same_v<TController, CanControllerContainer>) {
            return fmt::format("CAN_{}_{}", ToString(coSimType), ToString(connectionKind));
        }

        if constexpr (std::is_same_v<TController, EthControllerContainer>) {
            return fmt::format("ETH_{}_{}", ToString(coSimType), ToString(connectionKind));
        }

        if constexpr (std::is_same_v<TController, LinControllerContainer>) {
            return fmt::format("LIN_{}_{}", ToString(coSimType), ToString(connectionKind));
        }
    }
};

TYPED_TEST_SUITE(TestBusBuffer, Parameters, NameGenerator);

TYPED_TEST(TestBusBuffer, InitializeOneController) {
    using TController = typename TypeParam::Controller;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TController controller{};
    FillWithRandom(controller);

    std::unique_ptr<BusBuffer> busBuffer;

    // Act and assert
    AssertOk(CreateBusBuffer(coSimType, connectionKind, name, {Convert(controller)}, busBuffer));
}

TYPED_TEST(TestBusBuffer, InitializeMultipleControllers) {
    using TController = typename TypeParam::Controller;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TController controller1{};
    FillWithRandom(controller1);
    TController controller2{};
    FillWithRandom(controller2);

    std::unique_ptr<BusBuffer> busBuffer;

    // Act and assert
    AssertOk(CreateBusBuffer(coSimType, connectionKind, name, {Convert(controller1), Convert(controller2)}, busBuffer));
}

TYPED_TEST(TestBusBuffer, TransmitMessage) {
    using TController = typename TypeParam::Controller;
    using TMessage = typename TypeParam::Message;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TController controller{};
    FillWithRandom(controller);

    std::unique_ptr<BusBuffer> busBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {Convert(controller)}, busBuffer));

    TMessage sendMessage{};
    FillWithRandom(sendMessage, controller.id);

    // Act and assert
    AssertOk(busBuffer->Transmit(Convert(sendMessage)));
}

TYPED_TEST(TestBusBuffer, TransmitMessageWhenBufferIsFull) {
    using TController = typename TypeParam::Controller;
    using TMessage = typename TypeParam::Message;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TController controller{};
    FillWithRandom(controller);

    std::unique_ptr<BusBuffer> busBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {Convert(controller)}, busBuffer));

    // Fill queue
    for (uint32_t i = 0; i < controller.queueSize; i++) {
        TMessage sendMessage{};
        FillWithRandom(sendMessage, controller.id);
        ExpectOk(busBuffer->Transmit(Convert(sendMessage)));
    }

    TMessage rejectedMessage{};
    FillWithRandom(rejectedMessage, controller.id);

    // Act
    AssertFull(busBuffer->Transmit(Convert(rejectedMessage)));

    // Assert
    AssertLastMessage(
        fmt::format("Transmit buffer for controller '{}' is full. Messages are dropped.", controller.name));
}

TYPED_TEST(TestBusBuffer, TransmitMessageWhenBufferIsOnlyFullForSpecificController) {
    using TController = typename TypeParam::Controller;
    using TMessage = typename TypeParam::Message;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TController controller1{};
    FillWithRandom(controller1);
    TController controller2{};
    FillWithRandom(controller2);

    std::unique_ptr<BusBuffer> busBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {Convert(controller1), Convert(controller2)}, busBuffer));

    // Fill queue
    for (uint32_t i = 0; i < controller1.queueSize; i++) {
        TMessage sendMessage{};
        FillWithRandom(sendMessage, controller1.id);
        ExpectOk(busBuffer->Transmit(Convert(sendMessage)));
    }

    TMessage rejectedMessage{};
    FillWithRandom(rejectedMessage, controller1.id);

    // Act
    AssertFull(busBuffer->Transmit(Convert(rejectedMessage)));

    // Assert
    AssertLastMessage(
        fmt::format("Transmit buffer for controller '{}' is full. Messages are dropped.", controller1.name));
}

TYPED_TEST(TestBusBuffer, TransmitMessageWhenBufferIsFullForOtherController) {
    using TController = typename TypeParam::Controller;
    using TMessage = typename TypeParam::Message;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TController controller1{};
    FillWithRandom(controller1);
    TController controller2{};
    FillWithRandom(controller2);

    std::unique_ptr<BusBuffer> busBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {Convert(controller1), Convert(controller2)}, busBuffer));

    // Fill queue
    for (uint32_t i = 0; i < controller1.queueSize; i++) {
        TMessage sendMessage{};
        FillWithRandom(sendMessage, controller1.id);
        ExpectOk(busBuffer->Transmit(Convert(sendMessage)));
    }

    TMessage acceptedMessage{};
    FillWithRandom(acceptedMessage, controller2.id);

    // Act and assert
    AssertOk(busBuffer->Transmit(Convert(acceptedMessage)));
}

TYPED_TEST(TestBusBuffer, ReceiveMessageOnEmptyBuffer) {
    using TController = typename TypeParam::Controller;
    using TMessageExtern = typename TypeParam::MessageExtern;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TController controller{};
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

    TMessageExtern receivedMessage{};

    // Act and assert
    AssertEmpty(receiverBusBuffer->Receive(receivedMessage));
}

TYPED_TEST(TestBusBuffer, ReceiveMessageOnEmptyBufferByEvent) {
    using TController = typename TypeParam::Controller;
    using TControllerExtern = typename TypeParam::ControllerExtern;
    using TMessage = typename TypeParam::Message;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TController controller{};
    FillWithRandom(controller);

    std::unique_ptr<BusBuffer> senderBusBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {Convert(controller)}, senderBusBuffer));
    std::unique_ptr<BusBuffer> receiverBusBuffer;
    ExpectOk(CreateBusBuffer(GetCounterPart(coSimType),
                             connectionKind,
                             GetCounterPart(name, connectionKind),
                             {Convert(controller)},
                             receiverBusBuffer));

    std::deque<std::tuple<TControllerExtern, TMessage>> expectedEvents;

    // Act and assert
    TestBusBuffer<TypeParam>::Transfer(connectionKind, *senderBusBuffer, *receiverBusBuffer, expectedEvents);
}

TYPED_TEST(TestBusBuffer, ReceiveTransmittedMessage) {
    using TController = typename TypeParam::Controller;
    using TMessage = typename TypeParam::Message;
    using TMessageExtern = typename TypeParam::MessageExtern;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TController controller{};
    FillWithRandom(controller);

    std::unique_ptr<BusBuffer> senderBusBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {Convert(controller)}, senderBusBuffer));
    std::unique_ptr<BusBuffer> receiverBusBuffer;
    ExpectOk(CreateBusBuffer(GetCounterPart(coSimType),
                             connectionKind,
                             GetCounterPart(name, connectionKind),
                             {Convert(controller)},
                             receiverBusBuffer));

    TMessage sendMessage{};
    FillWithRandom(sendMessage, controller.id);
    ExpectOk(senderBusBuffer->Transmit(Convert(sendMessage)));

    TestBusBuffer<TypeParam>::Transfer(connectionKind, *senderBusBuffer, *receiverBusBuffer);

    TMessageExtern receivedMessage{};

    // Act
    AssertOk(receiverBusBuffer->Receive(receivedMessage));

    // Assert
    AssertEq(Convert(sendMessage), receivedMessage);
}

TYPED_TEST(TestBusBuffer, ReceiveTransmittedMessageByEvent) {
    using TController = typename TypeParam::Controller;
    using TControllerExtern = typename TypeParam::ControllerExtern;
    using TMessage = typename TypeParam::Message;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TController controller{};
    FillWithRandom(controller);

    std::unique_ptr<BusBuffer> senderBusBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {Convert(controller)}, senderBusBuffer));
    std::unique_ptr<BusBuffer> receiverBusBuffer;
    ExpectOk(CreateBusBuffer(GetCounterPart(coSimType),
                             connectionKind,
                             GetCounterPart(name, connectionKind),
                             {Convert(controller)},
                             receiverBusBuffer));

    std::deque<std::tuple<TControllerExtern, TMessage>> expectedEvents;

    TMessage sendMessage{};
    FillWithRandom(sendMessage, controller.id);
    ExpectOk(senderBusBuffer->Transmit(Convert(sendMessage)));
    expectedEvents.push_back({Convert(controller), sendMessage});

    // Act and assert
    TestBusBuffer<TypeParam>::Transfer(connectionKind, *senderBusBuffer, *receiverBusBuffer, expectedEvents);
}

TYPED_TEST(TestBusBuffer, ReceiveMultipleTransmittedMessages) {
    using TController = typename TypeParam::Controller;
    using TMessage = typename TypeParam::Message;
    using TMessageExtern = typename TypeParam::MessageExtern;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TController controller1{};
    FillWithRandom(controller1);
    TController controller2{};
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

    std::deque<TMessage> sendMessages;

    for (uint32_t i = 0; i < controller1.queueSize + controller2.queueSize; i++) {
        BusControllerId controllerId = (i % 2) == 0 ? controller1.id : controller2.id;
        TMessage sendMessage{};
        FillWithRandom(sendMessage, controllerId);
        sendMessages.push_back(sendMessage);
        ExpectOk(senderBusBuffer->Transmit(Convert(sendMessage)));
    }

    TestBusBuffer<TypeParam>::Transfer(connectionKind, *senderBusBuffer, *receiverBusBuffer);

    TMessageExtern receivedMessage{};

    // Act and Assert
    for (uint32_t i = 0; i < controller1.queueSize + controller2.queueSize; i++) {
        AssertOk(receiverBusBuffer->Receive(receivedMessage));
        AssertEq(Convert(sendMessages[i]), receivedMessage);
    }

    AssertEmpty(receiverBusBuffer->Receive(receivedMessage));
}

TYPED_TEST(TestBusBuffer, ReceiveTransmittedMessagesByEventWithTransfer) {
    using TController = typename TypeParam::Controller;
    using TControllerExtern = typename TypeParam::ControllerExtern;
    using TMessage = typename TypeParam::Message;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TController controller1{};
    FillWithRandom(controller1);
    TController controller2{};
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

    std::deque<std::tuple<TControllerExtern, TMessage>> expectedEvents;

    for (uint32_t i = 0; i < controller1.queueSize + controller2.queueSize; i++) {
        TController* controller = (i % 2) == 0 ? &controller1 : &controller2;
        TMessage sendMessage{};
        FillWithRandom(sendMessage, controller->id);
        expectedEvents.push_back({Convert(*controller), sendMessage});
        AssertOk(senderBusBuffer->Transmit(Convert(sendMessage)));
    }

    // Act and assert
    TestBusBuffer<TypeParam>::Transfer(connectionKind, *senderBusBuffer, *receiverBusBuffer, expectedEvents);
}

TYPED_TEST(TestBusBuffer, DoNotReceiveNotFullyTransmittedMessage) {
    using TController = typename TypeParam::Controller;
    using TMessage = typename TypeParam::Message;
    using TMessageExtern = typename TypeParam::MessageExtern;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");
    std::string fakeName = GenerateString("FakeBusBuffer名前");

    TController controller{};
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

    TMessage sendMessage{};
    FillWithRandom(sendMessage, controller.id);
    ExpectOk(senderBusBuffer->Transmit(Convert(sendMessage)));

    // Should not transfer anything
    TestBusBuffer<TypeParam>::Transfer(connectionKind, *fakeSenderBusBuffer, *receiverBusBuffer);

    TMessageExtern receivedMessage{};

    // Act and assert
    AssertEmpty(receiverBusBuffer->Receive(receivedMessage));
}

TYPED_TEST(TestBusBuffer, DoNotReceiveNotFullyTransmittedMessageByEvent) {
    using TController = typename TypeParam::Controller;
    using TControllerExtern = typename TypeParam::ControllerExtern;
    using TMessage = typename TypeParam::Message;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");
    std::string fakeName = GenerateString("FakeBusBuffer名前");

    TController controller{};
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

    std::deque<std::tuple<TControllerExtern, TMessage>> expectedEvents;

    TMessage sendMessage{};
    FillWithRandom(sendMessage, controller.id);
    ExpectOk(senderBusBuffer->Transmit(Convert(sendMessage)));

    // Act and assert
    TestBusBuffer<TypeParam>::Transfer(connectionKind,
                                       *fakeSenderBusBuffer,
                                       *receiverBusBuffer,
                                       expectedEvents);  // Should not transfer anything
}

}  // namespace
