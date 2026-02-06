// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <fmt/format.h>

#include <gtest/gtest.h>

#include <deque>
#include <memory>
#include <optional>
#include <string>
#include <thread>

#include "BusBuffer.h"
#include "DsVeosCoSim/CoSimTypes.h"
#include "Helper.h"
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

        std::string name = GenerateString("LocalChannel名前");
        std::unique_ptr<ChannelServer> localServer;
        ExpectOk(CreateLocalChannelServer(name, localServer));
        ExpectTrue(localServer);

        ExpectOk(TryConnectToLocalChannel(name, _localSenderChannel));
        ExpectTrue(_localSenderChannel);
        ExpectOk(localServer->TryAccept(_localReceiverChannel));
        ExpectTrue(_localReceiverChannel);
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
        ChannelReader& reader = connectionKind == ConnectionKind::Remote ? _remoteReceiverChannel->GetReader() : _localReceiverChannel->GetReader();
        ChannelWriter& writer = connectionKind == ConnectionKind::Remote ? _remoteSenderChannel->GetWriter() : _localSenderChannel->GetWriter();
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
        ChannelReader& reader = connectionKind == ConnectionKind::Remote ? _remoteReceiverChannel->GetReader() : _localReceiverChannel->GetReader();
        ChannelWriter& writer = connectionKind == ConnectionKind::Remote ? _remoteSenderChannel->GetWriter() : _localSenderChannel->GetWriter();

        SimulationTime expectedSimulationTime = GenerateSimulationTime();

        Callbacks callbacks{};
        if constexpr (std::is_same_v<TController, CanController>) {
            callbacks.canMessageContainerReceivedCallback =
                [&](SimulationTime simulationTime, const CanController& controller, const CanMessageContainer& messageContainer) {
                    AssertEq(simulationTime, expectedSimulationTime);
                    AssertFalse(expectedCallbacks.empty());
                    auto& [expectedController, expectedMessageContainer] = expectedCallbacks.front();
                    AssertEq(expectedController, controller);
                    AssertEq(expectedMessageContainer, messageContainer);
                    expectedCallbacks.pop_front();
                };
        }

        if constexpr (std::is_same_v<TController, EthController>) {
            callbacks.ethMessageContainerReceivedCallback =
                [&](SimulationTime simulationTime, const EthController& controller, const EthMessageContainer& messageContainer) {
                    AssertEq(simulationTime, expectedSimulationTime);
                    AssertFalse(expectedCallbacks.empty());
                    auto& [expectedController, expectedMessageContainer] = expectedCallbacks.front();
                    AssertEq(expectedController, controller);
                    AssertEq(expectedMessageContainer, messageContainer);
                    expectedCallbacks.pop_front();
                };
        }

        if constexpr (std::is_same_v<TController, LinController>) {
            callbacks.linMessageContainerReceivedCallback =
                [&](SimulationTime simulationTime, const LinController& controller, const LinMessageContainer& messageContainer) {
                    AssertEq(simulationTime, expectedSimulationTime);
                    AssertFalse(expectedCallbacks.empty());
                    auto& [expectedController, expectedMessageContainer] = expectedCallbacks.front();
                    AssertEq(expectedController, controller);
                    AssertEq(expectedMessageContainer, messageContainer);
                    expectedCallbacks.pop_front();
                };
        }

        if constexpr (std::is_same_v<TController, FrController>) {
            callbacks.frMessageContainerReceivedCallback =
                [&](SimulationTime simulationTime, const FrController& controller, const FrMessageContainer& messageContainer) {
                    AssertEq(simulationTime, expectedSimulationTime);
                    AssertFalse(expectedCallbacks.empty());
                    auto& [expectedController, expectedMessageContainer] = expectedCallbacks.front();
                    AssertEq(expectedController, controller);
                    AssertEq(expectedMessageContainer, messageContainer);
                    expectedCallbacks.pop_front();
                };
        }

        std::thread thread([&] {
            AssertOk(receiverBusBuffer.Deserialize(reader, expectedSimulationTime, callbacks));
        });

        AssertOk(senderBusBuffer.Serialize(writer));
        AssertOk(writer.EndWrite());

        thread.join();

        AssertTrue(expectedCallbacks.empty());
    }

    void Transfer(ConnectionKind connectionKind,
                  BusBuffer& senderBusBuffer,
                  BusBuffer& receiverBusBuffer,
                  std::deque<std::tuple<TController, TMessage>>& expectedCallbacks) {
        ChannelReader& reader = connectionKind == ConnectionKind::Remote ? _remoteReceiverChannel->GetReader() : _localReceiverChannel->GetReader();
        ChannelWriter& writer = connectionKind == ConnectionKind::Remote ? _remoteSenderChannel->GetWriter() : _localSenderChannel->GetWriter();

        SimulationTime expectedSimulationTime = GenerateSimulationTime();

        Callbacks callbacks{};
        if constexpr (std::is_same_v<TController, CanController>) {
            callbacks.canMessageReceivedCallback = [&](SimulationTime simulationTime, const CanController& controller, const CanMessage& message) {
                AssertEq(simulationTime, expectedSimulationTime);
                AssertFalse(expectedCallbacks.empty());
                auto& [expectedController, expectedMessage] = expectedCallbacks.front();
                AssertEq(expectedController, controller);
                AssertEq(expectedMessage, message);
                expectedCallbacks.pop_front();
            };
        }

        if constexpr (std::is_same_v<TController, EthController>) {
            callbacks.ethMessageReceivedCallback = [&](SimulationTime simulationTime, const EthController& controller, const EthMessage& message) {
                AssertEq(simulationTime, expectedSimulationTime);
                AssertFalse(expectedCallbacks.empty());
                auto& [expectedController, expectedMessage] = expectedCallbacks.front();
                AssertEq(expectedController, controller);
                AssertEq(expectedMessage, message);
                expectedCallbacks.pop_front();
            };
        }

        if constexpr (std::is_same_v<TController, LinController>) {
            callbacks.linMessageReceivedCallback = [&](SimulationTime simulationTime, const LinController& controller, const LinMessage& message) {
                AssertEq(simulationTime, expectedSimulationTime);
                AssertFalse(expectedCallbacks.empty());
                auto& [expectedController, expectedMessage] = expectedCallbacks.front();
                AssertEq(expectedController, controller);
                AssertEq(expectedMessage, message);
                expectedCallbacks.pop_front();
            };
        }

        if constexpr (std::is_same_v<TController, FrController>) {
            callbacks.frMessageReceivedCallback = [&](SimulationTime simulationTime, const FrController& controller, const FrMessage& message) {
                AssertEq(simulationTime, expectedSimulationTime);
                AssertFalse(expectedCallbacks.empty());
                auto& [expectedController, expectedMessage] = expectedCallbacks.front();
                AssertEq(expectedController, controller);
                AssertEq(expectedMessage, message);
                expectedCallbacks.pop_front();
            };
        }

        std::thread thread([&] {
            AssertOk(receiverBusBuffer.Deserialize(reader, expectedSimulationTime, callbacks));
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
using Parameters = Types<Param<CanControllerContainer, CanController, CanMessageContainer, CanMessage, CoSimType::Client, ConnectionKind::Local>>;
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
            return fmt::format("CAN_{}_{}", ToString(coSimType), ToString(connectionKind));
        }

        if constexpr (std::is_same_v<TControllerContainer, EthControllerContainer>) {
            return fmt::format("ETH_{}_{}", ToString(coSimType), ToString(connectionKind));
        }

        if constexpr (std::is_same_v<TControllerContainer, LinControllerContainer>) {
            return fmt::format("LIN_{}_{}", ToString(coSimType), ToString(connectionKind));
        }

        if constexpr (std::is_same_v<TControllerContainer, FrControllerContainer>) {
            return fmt::format("FR_{}_{}", ToString(coSimType), ToString(connectionKind));
        }
    }
};

TYPED_TEST_SUITE(TestBusBuffer, Parameters, NameGenerator);

TYPED_TEST(TestBusBuffer, InitializeOneController) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controllerContainer{};
    FillWithRandom(controllerContainer);

    TController controller = controllerContainer.Convert();

    std::unique_ptr<BusBuffer> busBuffer;

    std::unique_ptr<IProtocol> protocol;
    AssertOk(MakeProtocol(LATEST_VERSION, protocol));

    // Act and assert
    AssertOk(CreateBusBuffer(coSimType, connectionKind, name, {controller}, *protocol, busBuffer));
}

TYPED_TEST(TestBusBuffer, InitializeMultipleControllers) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controllerContainer1{};
    FillWithRandom(controllerContainer1);

    TController controller1 = controllerContainer1.Convert();

    TControllerContainer controllerContainer2{};
    FillWithRandom(controllerContainer2);

    TController controller2 = controllerContainer2.Convert();

    std::unique_ptr<BusBuffer> busBuffer;

    std::unique_ptr<IProtocol> protocol;
    AssertOk(MakeProtocol(LATEST_VERSION, protocol));

    // Act and assert
    AssertOk(CreateBusBuffer(coSimType, connectionKind, name, {controller1, controller2}, *protocol, busBuffer));
}

TYPED_TEST(TestBusBuffer, TransmitMessageContainer) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controllerContainer{};
    FillWithRandom(controllerContainer);

    TController controller = controllerContainer.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(MakeProtocol(LATEST_VERSION, protocol));

    std::unique_ptr<BusBuffer> busBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {controller}, *protocol, busBuffer));

    TMessageContainer sendMessageContainer{};
    FillWithRandom(sendMessageContainer, controller.id);

    // Act and assert
    AssertOk(busBuffer->Transmit(sendMessageContainer));
}

TYPED_TEST(TestBusBuffer, TransmitMessage) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;
    using TMessage = typename TypeParam::Message;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controllerContainer{};
    FillWithRandom(controllerContainer);

    TController controller = controllerContainer.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(MakeProtocol(LATEST_VERSION, protocol));

    std::unique_ptr<BusBuffer> busBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {controller}, *protocol, busBuffer));

    TMessageContainer sendMessageContainer{};
    FillWithRandom(sendMessageContainer, controller.id);

    TMessage sendMessage{};
    sendMessageContainer.WriteTo(sendMessage);

    // Act and assert
    AssertOk(busBuffer->Transmit(sendMessage));
}

TYPED_TEST(TestBusBuffer, TransmitMessageContainerWhenBufferIsFull) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controllerContainer{};
    FillWithRandom(controllerContainer);

    TController controller = controllerContainer.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(MakeProtocol(LATEST_VERSION, protocol));

    std::unique_ptr<BusBuffer> busBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {controller}, *protocol, busBuffer));

    // Fill queue
    for (uint32_t i = 0; i < controller.queueSize; i++) {
        TMessageContainer sendMessageContainer{};
        FillWithRandom(sendMessageContainer, controller.id);

        ExpectOk(busBuffer->Transmit(sendMessageContainer));
    }

    TMessageContainer rejectedMessageContainer{};
    FillWithRandom(rejectedMessageContainer, controller.id);

    // Act
    AssertFull(busBuffer->Transmit(rejectedMessageContainer));

    // Assert
    AssertLastMessage(fmt::format("Transmit buffer for controller '{}' is full. Messages are dropped.", controller.name));
}

TYPED_TEST(TestBusBuffer, TransmitMessageWhenBufferIsFull) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;
    using TMessage = typename TypeParam::Message;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controllerContainer{};
    FillWithRandom(controllerContainer);

    TController controller = controllerContainer.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(MakeProtocol(LATEST_VERSION, protocol));

    std::unique_ptr<BusBuffer> busBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {controller}, *protocol, busBuffer));

    // Fill queue
    for (uint32_t i = 0; i < controller.queueSize; i++) {
        TMessageContainer sendMessageContainer{};
        FillWithRandom(sendMessageContainer, controller.id);

        TMessage sendMessage{};
        sendMessageContainer.WriteTo(sendMessage);

        ExpectOk(busBuffer->Transmit(sendMessage));
    }

    TMessageContainer rejectedMessageContainer{};
    FillWithRandom(rejectedMessageContainer, controller.id);

    TMessage rejectedMessage{};
    rejectedMessageContainer.WriteTo(rejectedMessage);

    // Act
    AssertFull(busBuffer->Transmit(rejectedMessage));

    // Assert
    AssertLastMessage(fmt::format("Transmit buffer for controller '{}' is full. Messages are dropped.", controller.name));
}

TYPED_TEST(TestBusBuffer, TransmitMessageContainerWhenBufferIsOnlyFullForSpecificController) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controllerContainer1{};
    FillWithRandom(controllerContainer1);

    TController controller1 = controllerContainer1.Convert();

    TControllerContainer controllerContainer2{};
    FillWithRandom(controllerContainer2);

    TController controller2 = controllerContainer2.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(MakeProtocol(LATEST_VERSION, protocol));

    std::unique_ptr<BusBuffer> busBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {controller1, controller2}, *protocol, busBuffer));

    // Fill queue
    for (uint32_t i = 0; i < controller1.queueSize; i++) {
        TMessageContainer sendMessageContainer{};
        FillWithRandom(sendMessageContainer, controller1.id);

        ExpectOk(busBuffer->Transmit(sendMessageContainer));
    }

    TMessageContainer rejectedMessageContainer{};
    FillWithRandom(rejectedMessageContainer, controller1.id);

    // Act
    AssertFull(busBuffer->Transmit(rejectedMessageContainer));

    // Assert
    AssertLastMessage(fmt::format("Transmit buffer for controller '{}' is full. Messages are dropped.", controller1.name));
}

TYPED_TEST(TestBusBuffer, TransmitMessageWhenBufferIsOnlyFullForSpecificController) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;
    using TMessage = typename TypeParam::Message;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controllerContainer1{};
    FillWithRandom(controllerContainer1);

    TController controller1 = controllerContainer1.Convert();

    TControllerContainer controllerContainer2{};
    FillWithRandom(controllerContainer2);

    TController controller2 = controllerContainer2.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(MakeProtocol(LATEST_VERSION, protocol));

    std::unique_ptr<BusBuffer> busBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {controller1, controller2}, *protocol, busBuffer));

    // Fill queue
    for (uint32_t i = 0; i < controller1.queueSize; i++) {
        TMessageContainer sendMessageContainer{};
        FillWithRandom(sendMessageContainer, controller1.id);

        TMessage sendMessage{};
        sendMessageContainer.WriteTo(sendMessage);

        ExpectOk(busBuffer->Transmit(sendMessage));
    }

    TMessageContainer rejectedMessageContainer{};
    FillWithRandom(rejectedMessageContainer, controller1.id);

    TMessage rejectedMessage{};
    rejectedMessageContainer.WriteTo(rejectedMessage);

    // Act
    AssertFull(busBuffer->Transmit(rejectedMessage));

    // Assert
    AssertLastMessage(fmt::format("Transmit buffer for controller '{}' is full. Messages are dropped.", controller1.name));
}

TYPED_TEST(TestBusBuffer, TransmitMessageContainerWhenBufferIsFullForOtherController) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controllerContainer1{};
    FillWithRandom(controllerContainer1);

    TController controller1 = controllerContainer1.Convert();

    TControllerContainer controllerContainer2{};
    FillWithRandom(controllerContainer2);

    TController controller2 = controllerContainer2.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(MakeProtocol(LATEST_VERSION, protocol));

    std::unique_ptr<BusBuffer> busBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {controller1, controller2}, *protocol, busBuffer));

    // Fill queue
    for (uint32_t i = 0; i < controller1.queueSize; i++) {
        TMessageContainer sendMessageContainer{};
        FillWithRandom(sendMessageContainer, controller1.id);

        ExpectOk(busBuffer->Transmit(sendMessageContainer));
    }

    TMessageContainer acceptedMessageContainer{};
    FillWithRandom(acceptedMessageContainer, controller2.id);

    // Act and assert
    AssertOk(busBuffer->Transmit(acceptedMessageContainer));
}

TYPED_TEST(TestBusBuffer, TransmitMessageWhenBufferIsFullForOtherController) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;
    using TMessage = typename TypeParam::Message;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controllerContainer1{};
    FillWithRandom(controllerContainer1);

    TController controller1 = controllerContainer1.Convert();

    TControllerContainer controllerContainer2{};
    FillWithRandom(controllerContainer2);

    TController controller2 = controllerContainer2.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(MakeProtocol(LATEST_VERSION, protocol));

    std::unique_ptr<BusBuffer> busBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {controller1, controller2}, *protocol, busBuffer));

    // Fill queue
    for (uint32_t i = 0; i < controller1.queueSize; i++) {
        TMessageContainer sendMessageContainer{};
        FillWithRandom(sendMessageContainer, controller1.id);

        TMessage sendMessage{};
        sendMessageContainer.WriteTo(sendMessage);

        ExpectOk(busBuffer->Transmit(sendMessage));
    }

    TMessageContainer acceptedMessageContainer{};
    FillWithRandom(acceptedMessageContainer, controller2.id);

    TMessage acceptedMessage{};
    acceptedMessageContainer.WriteTo(acceptedMessage);

    // Act and assert
    AssertOk(busBuffer->Transmit(acceptedMessage));
}

TYPED_TEST(TestBusBuffer, ReceiveMessageContainerOnEmptyBuffer) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controllerContainer{};
    FillWithRandom(controllerContainer);

    TController controller = controllerContainer.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(MakeProtocol(LATEST_VERSION, protocol));

    std::unique_ptr<BusBuffer> senderBusBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {controller}, *protocol, senderBusBuffer));

    std::unique_ptr<BusBuffer> receiverBusBuffer;
    ExpectOk(CreateBusBuffer(GetCounterPart(coSimType), connectionKind, GetCounterPart(name, connectionKind), {controller}, *protocol, receiverBusBuffer));

    TestBusBuffer<TypeParam>::Transfer(connectionKind, *senderBusBuffer, *receiverBusBuffer);

    TMessageContainer receivedMessageContainer{};

    // Act and assert
    AssertEmpty(receiverBusBuffer->Receive(receivedMessageContainer));
}

TYPED_TEST(TestBusBuffer, ReceiveMessageOnEmptyBuffer) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessage = typename TypeParam::Message;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controllerContainer{};
    FillWithRandom(controllerContainer);

    TController controller = controllerContainer.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(MakeProtocol(LATEST_VERSION, protocol));

    std::unique_ptr<BusBuffer> senderBusBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {controller}, *protocol, senderBusBuffer));

    std::unique_ptr<BusBuffer> receiverBusBuffer;
    ExpectOk(CreateBusBuffer(GetCounterPart(coSimType), connectionKind, GetCounterPart(name, connectionKind), {controller}, *protocol, receiverBusBuffer));

    TestBusBuffer<TypeParam>::Transfer(connectionKind, *senderBusBuffer, *receiverBusBuffer);

    TMessage receivedMessage{};

    // Act and assert
    AssertEmpty(receiverBusBuffer->Receive(receivedMessage));
}

TYPED_TEST(TestBusBuffer, ReceiveMessageContainerOnEmptyBufferByEvent) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controllerContainer{};
    FillWithRandom(controllerContainer);

    TController controller = controllerContainer.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(MakeProtocol(LATEST_VERSION, protocol));

    std::unique_ptr<BusBuffer> senderBusBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {controller}, *protocol, senderBusBuffer));

    std::unique_ptr<BusBuffer> receiverBusBuffer;
    ExpectOk(CreateBusBuffer(GetCounterPart(coSimType), connectionKind, GetCounterPart(name, connectionKind), {controller}, *protocol, receiverBusBuffer));

    std::deque<std::tuple<TController, TMessageContainer>> expectedEvents;

    // Act and assert
    TestBusBuffer<TypeParam>::Transfer(connectionKind, *senderBusBuffer, *receiverBusBuffer, expectedEvents);
}

TYPED_TEST(TestBusBuffer, ReceiveMessageOnEmptyBufferByEvent) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessage = typename TypeParam::Message;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controllerContainer{};
    FillWithRandom(controllerContainer);

    TController controller = controllerContainer.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(MakeProtocol(LATEST_VERSION, protocol));

    std::unique_ptr<BusBuffer> senderBusBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {controller}, *protocol, senderBusBuffer));

    std::unique_ptr<BusBuffer> receiverBusBuffer;
    ExpectOk(CreateBusBuffer(GetCounterPart(coSimType), connectionKind, GetCounterPart(name, connectionKind), {controller}, *protocol, receiverBusBuffer));

    std::deque<std::tuple<TController, TMessage>> expectedEvents;

    // Act and assert
    TestBusBuffer<TypeParam>::Transfer(connectionKind, *senderBusBuffer, *receiverBusBuffer, expectedEvents);
}

TYPED_TEST(TestBusBuffer, ReceiveTransmittedMessageContainer) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controllerContainer{};
    FillWithRandom(controllerContainer);

    TController controller = controllerContainer.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(MakeProtocol(LATEST_VERSION, protocol));

    std::unique_ptr<BusBuffer> senderBusBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {controller}, *protocol, senderBusBuffer));

    std::unique_ptr<BusBuffer> receiverBusBuffer;
    ExpectOk(CreateBusBuffer(GetCounterPart(coSimType), connectionKind, GetCounterPart(name, connectionKind), {controller}, *protocol, receiverBusBuffer));

    TMessageContainer sendMessageContainer{};
    FillWithRandom(sendMessageContainer, controller.id);

    ExpectOk(senderBusBuffer->Transmit(sendMessageContainer));

    TestBusBuffer<TypeParam>::Transfer(connectionKind, *senderBusBuffer, *receiverBusBuffer);

    TMessageContainer receivedMessageContainer{};

    // Act
    AssertOk(receiverBusBuffer->Receive(receivedMessageContainer));

    // Assert
    AssertEq(sendMessageContainer, receivedMessageContainer);
}

TYPED_TEST(TestBusBuffer, ReceiveTransmittedMessage) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;
    using TMessage = typename TypeParam::Message;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controllerContainer{};
    FillWithRandom(controllerContainer);

    TController controller = controllerContainer.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(MakeProtocol(LATEST_VERSION, protocol));

    std::unique_ptr<BusBuffer> senderBusBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {controller}, *protocol, senderBusBuffer));

    std::unique_ptr<BusBuffer> receiverBusBuffer;
    ExpectOk(CreateBusBuffer(GetCounterPart(coSimType), connectionKind, GetCounterPart(name, connectionKind), {controller}, *protocol, receiverBusBuffer));

    TMessageContainer sendMessageContainer{};
    FillWithRandom(sendMessageContainer, controller.id);

    TMessage sendMessage{};
    sendMessageContainer.WriteTo(sendMessage);

    ExpectOk(senderBusBuffer->Transmit(sendMessage));

    TestBusBuffer<TypeParam>::Transfer(connectionKind, *senderBusBuffer, *receiverBusBuffer);

    TMessage receivedMessage{};

    // Act
    AssertOk(receiverBusBuffer->Receive(receivedMessage));

    // Assert
    AssertEq(sendMessage, receivedMessage);
}

TYPED_TEST(TestBusBuffer, ReceiveTransmittedMessageContainerByEvent) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controllerContainer{};
    FillWithRandom(controllerContainer);

    TController controller = controllerContainer.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(MakeProtocol(LATEST_VERSION, protocol));

    std::unique_ptr<BusBuffer> senderBusBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {controller}, *protocol, senderBusBuffer));

    std::unique_ptr<BusBuffer> receiverBusBuffer;
    ExpectOk(CreateBusBuffer(GetCounterPart(coSimType), connectionKind, GetCounterPart(name, connectionKind), {controller}, *protocol, receiverBusBuffer));

    std::deque<std::tuple<TController, TMessageContainer>> expectedEvents;

    TMessageContainer sendMessageContainer{};
    FillWithRandom(sendMessageContainer, controller.id);

    ExpectOk(senderBusBuffer->Transmit(sendMessageContainer));
    expectedEvents.push_back({controller, sendMessageContainer});

    // Act and assert
    TestBusBuffer<TypeParam>::Transfer(connectionKind, *senderBusBuffer, *receiverBusBuffer, expectedEvents);
}

TYPED_TEST(TestBusBuffer, ReceiveTransmittedMessageByEvent) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;
    using TMessage = typename TypeParam::Message;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controllerContainer{};
    FillWithRandom(controllerContainer);

    TController controller = controllerContainer.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(MakeProtocol(LATEST_VERSION, protocol));

    std::unique_ptr<BusBuffer> senderBusBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {controller}, *protocol, senderBusBuffer));

    std::unique_ptr<BusBuffer> receiverBusBuffer;
    ExpectOk(CreateBusBuffer(GetCounterPart(coSimType), connectionKind, GetCounterPart(name, connectionKind), {controller}, *protocol, receiverBusBuffer));

    std::deque<std::tuple<TController, TMessage>> expectedEvents;

    TMessageContainer sendMessageContainer{};
    FillWithRandom(sendMessageContainer, controller.id);

    TMessage sendMessage{};
    sendMessageContainer.WriteTo(sendMessage);

    ExpectOk(senderBusBuffer->Transmit(sendMessage));
    expectedEvents.push_back({controller, sendMessage});

    // Act and assert
    TestBusBuffer<TypeParam>::Transfer(connectionKind, *senderBusBuffer, *receiverBusBuffer, expectedEvents);
}

TYPED_TEST(TestBusBuffer, ReceiveMultipleTransmittedMessageContainers) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controllerContainer1{};
    FillWithRandom(controllerContainer1);

    TController controller1 = controllerContainer1.Convert();

    TControllerContainer controllerContainer2{};
    FillWithRandom(controllerContainer2);

    TController controller2 = controllerContainer2.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(MakeProtocol(LATEST_VERSION, protocol));

    std::unique_ptr<BusBuffer> senderBusBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {controller1, controller2}, *protocol, senderBusBuffer));

    std::unique_ptr<BusBuffer> receiverBusBuffer;
    ExpectOk(CreateBusBuffer(GetCounterPart(coSimType),
                             connectionKind,
                             GetCounterPart(name, connectionKind),
                             {controller1, controller2},
                             *protocol,
                             receiverBusBuffer));

    std::deque<TMessageContainer> sendMessageContainers;

    for (uint32_t i = 0; i < controller1.queueSize + controller2.queueSize; i++) {
        BusControllerId controllerId = (i % 2) == 0 ? controller1.id : controller2.id;

        TMessageContainer sendMessageContainer{};
        FillWithRandom(sendMessageContainer, controllerId);

        sendMessageContainers.push_back(sendMessageContainer);
        ExpectOk(senderBusBuffer->Transmit(sendMessageContainer));
    }

    TestBusBuffer<TypeParam>::Transfer(connectionKind, *senderBusBuffer, *receiverBusBuffer);

    TMessageContainer receivedMessageContainer{};

    // Act and Assert
    for (uint32_t i = 0; i < controller1.queueSize + controller2.queueSize; i++) {
        AssertOk(receiverBusBuffer->Receive(receivedMessageContainer));
        AssertEq(sendMessageContainers[i], receivedMessageContainer);
    }

    AssertEmpty(receiverBusBuffer->Receive(receivedMessageContainer));
}

TYPED_TEST(TestBusBuffer, ReceiveMultipleTransmittedMessages) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;
    using TMessage = typename TypeParam::Message;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controllerContainer1{};
    FillWithRandom(controllerContainer1);

    TController controller1 = controllerContainer1.Convert();

    TControllerContainer controllerContainer2{};
    FillWithRandom(controllerContainer2);

    TController controller2 = controllerContainer2.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(MakeProtocol(LATEST_VERSION, protocol));

    std::unique_ptr<BusBuffer> senderBusBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {controller1, controller2}, *protocol, senderBusBuffer));

    std::unique_ptr<BusBuffer> receiverBusBuffer;
    ExpectOk(CreateBusBuffer(GetCounterPart(coSimType),
                             connectionKind,
                             GetCounterPart(name, connectionKind),
                             {controller1, controller2},
                             *protocol,
                             receiverBusBuffer));

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
        ExpectOk(senderBusBuffer->Transmit(sendMessage));
    }

    TestBusBuffer<TypeParam>::Transfer(connectionKind, *senderBusBuffer, *receiverBusBuffer);

    TMessage receivedMessage{};

    // Act and Assert
    for (uint32_t i = 0; i < controller1.queueSize + controller2.queueSize; i++) {
        AssertOk(receiverBusBuffer->Receive(receivedMessage));
        AssertEq(sendMessages[i], receivedMessage);
    }

    AssertEmpty(receiverBusBuffer->Receive(receivedMessage));
}

TYPED_TEST(TestBusBuffer, ReceiveTransmittedMessageContainersByEventWithTransfer) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controllerContainer1{};
    FillWithRandom(controllerContainer1);

    TController controller1 = controllerContainer1.Convert();

    TControllerContainer controllerContainer2{};
    FillWithRandom(controllerContainer2);

    TController controller2 = controllerContainer2.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(MakeProtocol(LATEST_VERSION, protocol));

    std::unique_ptr<BusBuffer> senderBusBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {controller1, controller2}, *protocol, senderBusBuffer));

    std::unique_ptr<BusBuffer> receiverBusBuffer;
    ExpectOk(CreateBusBuffer(GetCounterPart(coSimType),
                             connectionKind,
                             GetCounterPart(name, connectionKind),
                             {controller1, controller2},
                             *protocol,
                             receiverBusBuffer));

    std::deque<std::tuple<TController, TMessageContainer>> expectedEvents;

    for (uint32_t i = 0; i < controller1.queueSize + controller2.queueSize; i++) {
        TController* controller = (i % 2) == 0 ? &controller1 : &controller2;

        TMessageContainer sendMessageContainer{};
        FillWithRandom(sendMessageContainer, controller->id);

        expectedEvents.push_back({*controller, sendMessageContainer});
        AssertOk(senderBusBuffer->Transmit(sendMessageContainer));
    }

    // Act and assert
    TestBusBuffer<TypeParam>::Transfer(connectionKind, *senderBusBuffer, *receiverBusBuffer, expectedEvents);
}

TYPED_TEST(TestBusBuffer, ReceiveTransmittedMessagesByEventWithTransfer) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;
    using TMessage = typename TypeParam::Message;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");

    TControllerContainer controllerContainer1{};
    FillWithRandom(controllerContainer1);

    TController controller1 = controllerContainer1.Convert();

    TControllerContainer controllerContainer2{};
    FillWithRandom(controllerContainer2);

    TController controller2 = controllerContainer2.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(MakeProtocol(LATEST_VERSION, protocol));

    std::unique_ptr<BusBuffer> senderBusBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {controller1, controller2}, *protocol, senderBusBuffer));

    std::unique_ptr<BusBuffer> receiverBusBuffer;
    ExpectOk(CreateBusBuffer(GetCounterPart(coSimType),
                             connectionKind,
                             GetCounterPart(name, connectionKind),
                             {controller1, controller2},
                             *protocol,
                             receiverBusBuffer));

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
        AssertOk(senderBusBuffer->Transmit(sendMessage));
    }

    // Act and assert
    TestBusBuffer<TypeParam>::Transfer(connectionKind, *senderBusBuffer, *receiverBusBuffer, expectedEvents);
}

TYPED_TEST(TestBusBuffer, DoNotReceiveNotFullyTransmittedMessageContainer) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");
    std::string fakeName = GenerateString("FakeBusBuffer名前");

    TControllerContainer controllerContainer{};
    FillWithRandom(controllerContainer);

    TController controller = controllerContainer.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(MakeProtocol(LATEST_VERSION, protocol));

    std::unique_ptr<BusBuffer> senderBusBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {controller}, *protocol, senderBusBuffer));

    std::unique_ptr<BusBuffer> fakeSenderBusBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, fakeName, {controller}, *protocol, fakeSenderBusBuffer));

    std::unique_ptr<BusBuffer> receiverBusBuffer;
    ExpectOk(CreateBusBuffer(GetCounterPart(coSimType), connectionKind, GetCounterPart(name, connectionKind), {controller}, *protocol, receiverBusBuffer));

    TMessageContainer sendMessageContainer{};
    FillWithRandom(sendMessageContainer, controller.id);

    ExpectOk(senderBusBuffer->Transmit(sendMessageContainer));

    // Should not transfer anything
    TestBusBuffer<TypeParam>::Transfer(connectionKind, *fakeSenderBusBuffer, *receiverBusBuffer);

    TMessageContainer receivedMessageContainer{};

    // Act and assert
    AssertEmpty(receiverBusBuffer->Receive(receivedMessageContainer));
}

TYPED_TEST(TestBusBuffer, DoNotReceiveNotFullyTransmittedMessage) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;
    using TMessage = typename TypeParam::Message;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");
    std::string fakeName = GenerateString("FakeBusBuffer名前");

    TControllerContainer controllerContainer{};
    FillWithRandom(controllerContainer);

    TController controller = controllerContainer.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(MakeProtocol(LATEST_VERSION, protocol));

    std::unique_ptr<BusBuffer> senderBusBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {controller}, *protocol, senderBusBuffer));

    std::unique_ptr<BusBuffer> fakeSenderBusBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, fakeName, {controller}, *protocol, fakeSenderBusBuffer));

    std::unique_ptr<BusBuffer> receiverBusBuffer;
    ExpectOk(CreateBusBuffer(GetCounterPart(coSimType), connectionKind, GetCounterPart(name, connectionKind), {controller}, *protocol, receiverBusBuffer));

    TMessageContainer sendMessageContainer{};
    FillWithRandom(sendMessageContainer, controller.id);

    TMessage sendMessage{};
    sendMessageContainer.WriteTo(sendMessage);

    ExpectOk(senderBusBuffer->Transmit(sendMessage));

    // Should not transfer anything
    TestBusBuffer<TypeParam>::Transfer(connectionKind, *fakeSenderBusBuffer, *receiverBusBuffer);

    TMessage receivedMessage{};

    // Act and assert
    AssertEmpty(receiverBusBuffer->Receive(receivedMessage));
}

TYPED_TEST(TestBusBuffer, DoNotReceiveNotFullyTransmittedMessageContainerByEvent) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");
    std::string fakeName = GenerateString("FakeBusBuffer名前");

    TControllerContainer controllerContainer{};
    FillWithRandom(controllerContainer);

    TController controller = controllerContainer.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(MakeProtocol(LATEST_VERSION, protocol));

    std::unique_ptr<BusBuffer> senderBusBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {controller}, *protocol, senderBusBuffer));

    std::unique_ptr<BusBuffer> fakeSenderBusBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, fakeName, {controller}, *protocol, fakeSenderBusBuffer));

    std::unique_ptr<BusBuffer> receiverBusBuffer;
    ExpectOk(CreateBusBuffer(GetCounterPart(coSimType), connectionKind, GetCounterPart(name, connectionKind), {controller}, *protocol, receiverBusBuffer));

    std::deque<std::tuple<TController, TMessageContainer>> expectedEvents;

    TMessageContainer sendMessageContainer{};
    FillWithRandom(sendMessageContainer, controller.id);

    ExpectOk(senderBusBuffer->Transmit(sendMessageContainer));

    // Act and assert
    TestBusBuffer<TypeParam>::Transfer(connectionKind, *fakeSenderBusBuffer, *receiverBusBuffer,
                                       expectedEvents);  // Should not transfer anything
}

TYPED_TEST(TestBusBuffer, DoNotReceiveNotFullyTransmittedMessageByEvent) {
    using TControllerContainer = typename TypeParam::ControllerContainer;
    using TController = typename TypeParam::Controller;
    using TMessageContainer = typename TypeParam::MessageContainer;
    using TMessage = typename TypeParam::Message;

    CoSimType coSimType = TypeParam::GetCoSimType();
    ConnectionKind connectionKind = TypeParam::GetConnectionKind();

    // Arrange
    std::string name = GenerateString("BusBuffer名前");
    std::string fakeName = GenerateString("FakeBusBuffer名前");

    TControllerContainer controllerContainer{};
    FillWithRandom(controllerContainer);

    TController controller = controllerContainer.Convert();

    std::unique_ptr<IProtocol> protocol;
    AssertOk(MakeProtocol(LATEST_VERSION, protocol));

    std::unique_ptr<BusBuffer> senderBusBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, name, {controller}, *protocol, senderBusBuffer));

    std::unique_ptr<BusBuffer> fakeSenderBusBuffer;
    ExpectOk(CreateBusBuffer(coSimType, connectionKind, fakeName, {controller}, *protocol, fakeSenderBusBuffer));

    std::unique_ptr<BusBuffer> receiverBusBuffer;
    ExpectOk(CreateBusBuffer(GetCounterPart(coSimType), connectionKind, GetCounterPart(name, connectionKind), {controller}, *protocol, receiverBusBuffer));

    std::deque<std::tuple<TController, TMessage>> expectedEvents;

    TMessageContainer sendMessageContainer{};
    FillWithRandom(sendMessageContainer, controller.id);

    TMessage sendMessage{};
    sendMessageContainer.WriteTo(sendMessage);

    ExpectOk(senderBusBuffer->Transmit(sendMessage));

    // Act and assert
    TestBusBuffer<TypeParam>::Transfer(connectionKind, *fakeSenderBusBuffer, *receiverBusBuffer,
                                       expectedEvents);  // Should not transfer anything
}

}  // namespace
