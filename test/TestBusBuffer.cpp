// Copyright dSPACE GmbH. All rights reserved.

#include <fmt/format.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <deque>
#include <memory>
#include <string>
#include <thread>

#include "BusBuffer.h"
#include "CoSimTypes.h"
#include "DsVeosCoSim/DsVeosCoSim.h"
#include "Generator.h"
#include "Helper.h"
#include "LogHelper.h"
#include "TestHelper.h"

#ifdef _WIN32
#include "LocalChannel.h"
#endif

using ::testing::Types;

using namespace DsVeosCoSim;
using namespace testing;

namespace {

template <typename Types>
class TestBusBuffer : public testing::Test {
protected:
    void SetUp() override {
        ClearLastMessage();
    }
};

void Transfer(ConnectionKind connectionKind, BusBuffer& senderBusBuffer, BusBuffer& receiverBusBuffer) {  // NOLINT
    std::unique_ptr<Channel> senderChannel{};
    std::unique_ptr<Channel> receiverChannel{};
    if (connectionKind == ConnectionKind::Remote) {
        TcpChannelServer server(0, true);
        uint16_t port = server.GetLocalPort();

        senderChannel = std::make_unique<SocketChannel>(ConnectToTcpChannel("127.0.0.1", port));
        receiverChannel = std::make_unique<SocketChannel>(Accept(server));
    } else {
        std::string name = GenerateString("Local名前");
#ifdef _WIN32
        LocalChannelServer server(name);

        senderChannel = std::make_unique<LocalChannel>(ConnectToLocalChannel(name));
        receiverChannel = std::make_unique<LocalChannel>(Accept(server));
#else
        UdsChannelServer server(name);

        senderChannel = std::make_unique<SocketChannel>(ConnectToUdsChannel(name));
        receiverChannel = std::make_unique<SocketChannel>(Accept(server));
#endif
    }

    std::jthread thread([&] {
        ASSERT_TRUE(receiverBusBuffer.Deserialize(receiverChannel->GetReader(), {}, {}));
    });

    ASSERT_TRUE(senderBusBuffer.Serialize(senderChannel->GetWriter()));
    ASSERT_TRUE(senderChannel->GetWriter().EndWrite());
}

template <typename TControllerExtern, typename TMessage>
void Transfer(ConnectionKind connectionKind,
              BusBuffer& senderBusBuffer,
              BusBuffer& receiverBusBuffer,
              std::deque<std::tuple<TControllerExtern, TMessage>> expectedCallbacks) {
    std::unique_ptr<Channel> senderChannel{};
    std::unique_ptr<Channel> receiverChannel{};
    if (connectionKind == ConnectionKind::Remote) {
        TcpChannelServer server(0, true);
        uint16_t port = server.GetLocalPort();

        senderChannel = std::make_unique<SocketChannel>(ConnectToTcpChannel("127.0.0.1", port));
        receiverChannel = std::make_unique<SocketChannel>(Accept(server));
    } else {
        std::string name = GenerateString("Local名前");
#ifdef _WIN32
        LocalChannelServer server(name);

        senderChannel = std::make_unique<LocalChannel>(ConnectToLocalChannel(name));
        receiverChannel = std::make_unique<LocalChannel>(Accept(server));
#else
        UdsChannelServer server(name);

        senderChannel = std::make_unique<SocketChannel>(ConnectToUdsChannel(name));
        receiverChannel = std::make_unique<SocketChannel>(Accept(server));
#endif
    }

    DsVeosCoSim_SimulationTime simulationTime = GenerateI64();

    Callbacks callbacks{};
    if constexpr (std::is_same_v<TControllerExtern, DsVeosCoSim_CanController>) {
        callbacks.canMessageReceivedCallback = [&](DsVeosCoSim_SimulationTime simTime,
                                                   const DsVeosCoSim_CanController& controller,
                                                   const DsVeosCoSim_CanMessage& message) {
            ASSERT_EQ(simTime, simulationTime);
            ASSERT_FALSE(expectedCallbacks.empty());
            const auto [expectedController, expectedMessage] = expectedCallbacks.front();
            AssertEq(expectedController, controller);
            AssertEq(expectedMessage, message);
            expectedCallbacks.pop_front();
        };
    }

    if constexpr (std::is_same_v<TControllerExtern, DsVeosCoSim_EthController>) {
        callbacks.ethMessageReceivedCallback = [&](DsVeosCoSim_SimulationTime simTime,
                                                   const DsVeosCoSim_EthController& controller,
                                                   const DsVeosCoSim_EthMessage& message) {
            ASSERT_EQ(simTime, simulationTime);
            ASSERT_FALSE(expectedCallbacks.empty());
            const auto [expectedController, expectedMessage] = expectedCallbacks.front();
            AssertEq(expectedController, controller);
            AssertEq(expectedMessage, message);
            expectedCallbacks.pop_front();
        };
    }

    if constexpr (std::is_same_v<TControllerExtern, DsVeosCoSim_LinController>) {
        callbacks.linMessageReceivedCallback = [&](DsVeosCoSim_SimulationTime simTime,
                                                   const DsVeosCoSim_LinController& controller,
                                                   const DsVeosCoSim_LinMessage& message) {
            ASSERT_EQ(simTime, simulationTime);
            ASSERT_FALSE(expectedCallbacks.empty());
            const auto [expectedController, expectedMessage] = expectedCallbacks.front();
            AssertEq(expectedController, controller);
            AssertEq(expectedMessage, message);
            expectedCallbacks.pop_front();
        };
    }

    std::thread thread([&] {
        ASSERT_TRUE(receiverBusBuffer.Deserialize(receiverChannel->GetReader(), simulationTime, callbacks));
    });

    ASSERT_TRUE(senderBusBuffer.Serialize(senderChannel->GetWriter()));
    ASSERT_TRUE(senderChannel->GetWriter().EndWrite());

    thread.join();

    ASSERT_TRUE(expectedCallbacks.empty());
}

template <typename TController,
          typename TControllerExtern,
          typename TMessage,
          typename TMessageExtern,
          CoSimType coSimType,
          ConnectionKind connectionKind>
struct Param {
    using Controller = TController;
    using ControllerExtern = TControllerExtern;
    using Message = TMessage;
    using MessageExtern = TMessageExtern;

    static CoSimType GetCoSimType() {
        return coSimType;
    }

    static ConnectionKind GetConnectionKind() {
        return connectionKind;
    }
};

using Parameters = Types<Param<CanController,
                               DsVeosCoSim_CanController,
                               CanMessage,
                               DsVeosCoSim_CanMessage,
                               CoSimType::Client,
                               ConnectionKind::Local>,
                         Param<CanController,
                               DsVeosCoSim_CanController,
                               CanMessage,
                               DsVeosCoSim_CanMessage,
                               CoSimType::Client,
                               ConnectionKind::Remote>,
                         Param<CanController,
                               DsVeosCoSim_CanController,
                               CanMessage,
                               DsVeosCoSim_CanMessage,
                               CoSimType::Server,
                               ConnectionKind::Local>,
                         Param<CanController,
                               DsVeosCoSim_CanController,
                               CanMessage,
                               DsVeosCoSim_CanMessage,
                               CoSimType::Server,
                               ConnectionKind::Remote>,
                         Param<EthController,
                               DsVeosCoSim_EthController,
                               EthMessage,
                               DsVeosCoSim_EthMessage,
                               CoSimType::Client,
                               ConnectionKind::Local>,
                         Param<EthController,
                               DsVeosCoSim_EthController,
                               EthMessage,
                               DsVeosCoSim_EthMessage,
                               CoSimType::Client,
                               ConnectionKind::Remote>,
                         Param<EthController,
                               DsVeosCoSim_EthController,
                               EthMessage,
                               DsVeosCoSim_EthMessage,
                               CoSimType::Server,
                               ConnectionKind::Local>,
                         Param<EthController,
                               DsVeosCoSim_EthController,
                               EthMessage,
                               DsVeosCoSim_EthMessage,
                               CoSimType::Server,
                               ConnectionKind::Remote>,
                         Param<LinController,
                               DsVeosCoSim_LinController,
                               LinMessage,
                               DsVeosCoSim_LinMessage,
                               CoSimType::Client,
                               ConnectionKind::Local>,
                         Param<LinController,
                               DsVeosCoSim_LinController,
                               LinMessage,
                               DsVeosCoSim_LinMessage,
                               CoSimType::Client,
                               ConnectionKind::Remote>,
                         Param<LinController,
                               DsVeosCoSim_LinController,
                               LinMessage,
                               DsVeosCoSim_LinMessage,
                               CoSimType::Server,
                               ConnectionKind::Local>,
                         Param<LinController,
                               DsVeosCoSim_LinController,
                               LinMessage,
                               DsVeosCoSim_LinMessage,
                               CoSimType::Server,
                               ConnectionKind::Remote>>;

class NameGenerator {
public:
    template <typename T>
    static std::string GetName([[maybe_unused]] int32_t index) {  // NOLINT
        using TController = typename T::Controller;
        CoSimType coSimType = T::GetCoSimType();
        ConnectionKind connectionKind = T::GetConnectionKind();

        std::string suffix = fmt::format("{}_{}", ToString(coSimType), ToString(connectionKind));

        if constexpr (std::is_same_v<TController, CanController>) {
            return "CAN_" + suffix;
        }

        if constexpr (std::is_same_v<TController, EthController>) {
            return "ETH_" + suffix;
        }

        if constexpr (std::is_same_v<TController, LinController>) {
            return "LIN_" + suffix;
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

    // Act and assert
    ASSERT_NO_THROW(BusBuffer(coSimType, connectionKind, name, {controller}));
}

#ifdef EXCEPTION_TESTS
TYPED_TEST(TestBusBuffer, InitializeControllersWithDuplicatedIds) {
    using TController = typename TypeParam::Controller;
    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TController controller{};
    FillWithRandom(controller);

    // Act and assert
    ASSERT_THAT(
        [&]() {
            BusBuffer(coSimType, connectionKind, name, {controller, controller});
        },
        ThrowsMessage<CoSimException>(fmt::format("Duplicated controller id {}.", controller.id)));
}
#endif

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

    // Act and assert
    ASSERT_NO_THROW(BusBuffer(coSimType, connectionKind, name, {controller1, controller2}));
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

    BusBuffer busBuffer(coSimType, connectionKind, name, {controller});

    TMessage sendMessage{};
    FillWithRandom(sendMessage, controller.id);

    // Act
    const bool result = busBuffer.Transmit(sendMessage);

    // Assert
    ASSERT_TRUE(result);
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

    BusBuffer busBuffer(coSimType, connectionKind, name, {controller});

    // Fill queue
    for (uint32_t i = 0; i < controller.queueSize; i++) {
        TMessage sendMessage{};
        FillWithRandom(sendMessage, controller.id);
        ASSERT_TRUE(busBuffer.Transmit(sendMessage));
    }

    TMessage rejectedMessage{};
    FillWithRandom(rejectedMessage, controller.id);

    // Act
    const bool result = busBuffer.Transmit(rejectedMessage);

    // Assert
    ASSERT_FALSE(result);
    AssertLastMessage(fmt::format("Queue for controller '{}' is full. Messages are dropped.", controller.name));
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

    BusBuffer busBuffer(coSimType, connectionKind, name, {controller1, controller2});

    // Fill queue
    for (uint32_t i = 0; i < controller1.queueSize; i++) {
        TMessage sendMessage{};
        FillWithRandom(sendMessage, controller1.id);
        ASSERT_TRUE(busBuffer.Transmit(sendMessage));
    }

    TMessage rejectedMessage{};
    FillWithRandom(rejectedMessage, controller1.id);

    // Act
    const bool result = busBuffer.Transmit(rejectedMessage);

    // Assert
    ASSERT_FALSE(result);
    AssertLastMessage(fmt::format("Queue for controller '{}' is full. Messages are dropped.", controller1.name));
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

    BusBuffer busBuffer(coSimType, connectionKind, name, {controller1, controller2});

    // Fill queue
    for (uint32_t i = 0; i < controller1.queueSize; i++) {
        TMessage sendMessage{};
        FillWithRandom(sendMessage, controller1.id);
        ASSERT_TRUE(busBuffer.Transmit(sendMessage));
    }

    TMessage acceptedMessage{};
    FillWithRandom(acceptedMessage, controller2.id);

    // Act
    const bool result = busBuffer.Transmit(acceptedMessage);

    // Assert
    ASSERT_TRUE(result);
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

    BusBuffer senderBusBuffer(coSimType, connectionKind, name, {controller});
    BusBuffer receiverBusBuffer(GetCounterPart(coSimType),
                                connectionKind,
                                GetCounterPart(name, connectionKind),
                                {controller});

    Transfer(connectionKind, senderBusBuffer, receiverBusBuffer);

    TMessageExtern receivedMessage{};

    // Act
    const bool result = receiverBusBuffer.Receive(receivedMessage);

    // Assert
    ASSERT_FALSE(result);
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

    BusBuffer senderBusBuffer(coSimType, connectionKind, name, {controller});
    BusBuffer receiverBusBuffer(GetCounterPart(coSimType),
                                connectionKind,
                                GetCounterPart(name, connectionKind),
                                {controller});

    std::deque<std::tuple<TControllerExtern, TMessage>> expectedEvents;

    // Act and assert
    Transfer(connectionKind, senderBusBuffer, receiverBusBuffer, expectedEvents);
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

    BusBuffer senderBusBuffer(coSimType, connectionKind, name, {controller});
    BusBuffer receiverBusBuffer(GetCounterPart(coSimType),
                                connectionKind,
                                GetCounterPart(name, connectionKind),
                                {controller});

    TMessage sendMessage{};
    FillWithRandom(sendMessage, controller.id);
    ASSERT_TRUE(senderBusBuffer.Transmit(sendMessage));

    Transfer(connectionKind, senderBusBuffer, receiverBusBuffer);

    TMessageExtern receivedMessage{};

    // Act
    const bool result = receiverBusBuffer.Receive(receivedMessage);

    // Assert
    ASSERT_TRUE(result);
    AssertEq(sendMessage, receivedMessage);
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

    BusBuffer senderBusBuffer(coSimType, connectionKind, name, {controller});
    BusBuffer receiverBusBuffer(GetCounterPart(coSimType),
                                connectionKind,
                                GetCounterPart(name, connectionKind),
                                {controller});

    std::deque<std::tuple<TControllerExtern, TMessage>> expectedEvents;

    TMessage sendMessage{};
    FillWithRandom(sendMessage, controller.id);
    ASSERT_TRUE(senderBusBuffer.Transmit(sendMessage));
    expectedEvents.push_back({controller, sendMessage});

    // Act and assert
    Transfer(connectionKind, senderBusBuffer, receiverBusBuffer, expectedEvents);
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

    BusBuffer senderBusBuffer(coSimType, connectionKind, name, {controller1, controller2});
    BusBuffer receiverBusBuffer(GetCounterPart(coSimType),
                                connectionKind,
                                GetCounterPart(name, connectionKind),
                                {controller1, controller2});

    std::deque<TMessage> sendMessages;

    for (uint32_t i = 0; i < controller1.queueSize + controller2.queueSize; i++) {
        uint32_t controllerId = (i % 2) == 0 ? controller1.id : controller2.id;
        TMessage sendMessage{};
        FillWithRandom(sendMessage, controllerId);
        sendMessages.push_back(sendMessage);
        ASSERT_TRUE(senderBusBuffer.Transmit(sendMessage));
    }

    Transfer(connectionKind, senderBusBuffer, receiverBusBuffer);

    TMessageExtern receivedMessage{};

    // Act and Assert
    for (uint32_t i = 0; i < controller1.queueSize + controller2.queueSize; i++) {
        ASSERT_TRUE(receiverBusBuffer.Receive(receivedMessage));
        AssertEq(sendMessages[i], receivedMessage);
    }

    ASSERT_FALSE(receiverBusBuffer.Receive(receivedMessage));
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

    BusBuffer senderBusBuffer(coSimType, connectionKind, name, {controller1, controller2});
    BusBuffer receiverBusBuffer(GetCounterPart(coSimType),
                                connectionKind,
                                GetCounterPart(name, connectionKind),
                                {controller1, controller2});

    std::deque<std::tuple<TControllerExtern, TMessage>> expectedEvents;

    for (uint32_t i = 0; i < controller1.queueSize + controller2.queueSize; i++) {
        TController* controller = (i % 2) == 0 ? &controller1 : &controller2;
        TMessage sendMessage{};
        FillWithRandom(sendMessage, controller->id);
        expectedEvents.push_back({*controller, sendMessage});
        ASSERT_TRUE(senderBusBuffer.Transmit(sendMessage));
    }

    // Act and assert
    Transfer(connectionKind, senderBusBuffer, receiverBusBuffer, expectedEvents);
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

    BusBuffer senderBusBuffer(coSimType, connectionKind, name, {controller});
    BusBuffer fakeSenderBusBuffer(coSimType, connectionKind, fakeName, {controller});
    BusBuffer receiverBusBuffer(GetCounterPart(coSimType),
                                connectionKind,
                                GetCounterPart(name, connectionKind),
                                {controller});

    TMessage sendMessage{};
    FillWithRandom(sendMessage, controller.id);
    ASSERT_TRUE(senderBusBuffer.Transmit(sendMessage));

    // Should not transfer anything
    Transfer(connectionKind, fakeSenderBusBuffer, receiverBusBuffer);

    TMessageExtern receivedMessage{};

    // Act
    const bool result = receiverBusBuffer.Receive(receivedMessage);

    // Assert
    ASSERT_FALSE(result);
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

    BusBuffer senderBusBuffer(coSimType, connectionKind, name, {controller});
    BusBuffer fakeSenderBusBuffer(coSimType, connectionKind, fakeName, {controller});
    BusBuffer receiverBusBuffer(GetCounterPart(coSimType),
                                connectionKind,
                                GetCounterPart(name, connectionKind),
                                {controller});

    std::deque<std::tuple<TControllerExtern, TMessage>> expectedEvents;

    TMessage sendMessage{};
    FillWithRandom(sendMessage, controller.id);
    ASSERT_TRUE(senderBusBuffer.Transmit(sendMessage));

    // Act and assert
    Transfer(connectionKind, fakeSenderBusBuffer, receiverBusBuffer, expectedEvents);  // Should not transfer anything
}

}  // namespace
