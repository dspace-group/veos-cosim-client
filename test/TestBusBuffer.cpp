// Copyright dSPACE GmbH. All rights reserved.

#include "BusBuffer.h"
#include "Communication.h"
#include "Logger.h"
#include "TestHelper.h"

using namespace DsVeosCoSim;

class TestBusBuffer : public testing::Test {
protected:
    Channel _senderChannel;
    Channel _receiverChannel;

    void SetUp() override {
        SetLogCallback(OnLogCallback);

        Server server;
        uint16_t port{};
        ASSERT_OK(server.Start(port, true));

        ASSERT_OK(ConnectToServer("127.0.0.1", port, 0, _senderChannel));

        ASSERT_OK(server.Accept(_receiverChannel));

        ClearLastMessage();
    }

    void TearDown() override {
        _senderChannel.Disconnect();
        _receiverChannel.Disconnect();
    }

    void Transfer(BusBuffer& sender, BusBuffer& receiver) {
        ASSERT_OK(sender.Serialize(_senderChannel));
        ASSERT_OK(_senderChannel.EndWrite());
        ASSERT_OK(receiver.Deserialize(_receiverChannel, 0, {}));
    }
};

TEST_F(TestBusBuffer, DuplicateCanIds) {
    // Arrange
    CanControllerContainer container;
    CreateController(container);

    BusBuffer busBuffer;

    // Act
    const Result result = busBuffer.Initialize({container, container}, {}, {});

    // Assert
    ASSERT_ERROR(result);
    AssertLastMessage("Duplicated CAN controller id " + std::to_string(container.controller.id) + ".");
}

TEST_F(TestBusBuffer, ReceiveCanMessageOnEmptyBuffer) {
    // Arrange
    CanControllerContainer container;
    CreateController(container);

    BusBuffer receiverBusBuffer;
    ASSERT_OK(receiverBusBuffer.Initialize({container}, {}, {}));

    CanMessage receivedMessage{};

    // Act
    const Result result = receiverBusBuffer.Receive(receivedMessage);

    // Assert
    ASSERT_EMPTY(result);
}

TEST_F(TestBusBuffer, TransmitAndReceiveCanMessages) {
    // Arrange
    CanControllerContainer container;
    CreateController(container);

    BusBuffer senderBusBuffer;
    BusBuffer receiverBusBuffer;

    ASSERT_OK(senderBusBuffer.Initialize({container}, {}, {}));
    ASSERT_OK(receiverBusBuffer.Initialize({container}, {}, {}));

    std::vector<CanMessageContainer> sendMessages;

    // Act
    for (uint32_t i = 0; i < container.controller.queueSize; i++) {
        CanMessageContainer& sendMessage = sendMessages.emplace_back();
        CreateMessage(container.controller.id, sendMessage);
        ASSERT_OK(senderBusBuffer.Transmit(sendMessage.message));
    }

    CanMessageContainer rejectedMessage{};
    CreateMessage(container.controller.id, rejectedMessage);
    ASSERT_FULL(senderBusBuffer.Transmit(rejectedMessage.message));

    Transfer(senderBusBuffer, receiverBusBuffer);

    // Assert
    CanMessage receivedMessage{};
    for (uint32_t i = 0; i < container.controller.queueSize; i++) {
        ASSERT_OK(receiverBusBuffer.Receive(receivedMessage));
        AssertEq(sendMessages[i].message, receivedMessage);
    }

    ASSERT_EMPTY(receiverBusBuffer.Receive(receivedMessage));
}

TEST_F(TestBusBuffer, DuplicateEthIds) {
    // Arrange
    EthControllerContainer container;
    CreateController(container);

    BusBuffer busBuffer;

    // Act
    const Result result = busBuffer.Initialize({}, {container, container}, {});

    // Assert
    ASSERT_ERROR(result);
    AssertLastMessage("Duplicated ethernet controller id " + std::to_string(container.controller.id) + ".");
}

TEST_F(TestBusBuffer, ReceiveEthMessageOnEmptyBuffer) {
    // Arrange
    EthControllerContainer container;
    CreateController(container);

    BusBuffer receiverBusBuffer;
    ASSERT_OK(receiverBusBuffer.Initialize({}, {container}, {}));

    EthMessage receivedMessage{};

    // Act
    const Result result = receiverBusBuffer.Receive(receivedMessage);

    // Assert
    ASSERT_EMPTY(result);
}

TEST_F(TestBusBuffer, TransmitAndReceiveEthMessages) {
    // Arrange
    EthControllerContainer container;
    CreateController(container);

    BusBuffer senderBusBuffer;
    BusBuffer receiverBusBuffer;

    ASSERT_OK(senderBusBuffer.Initialize({}, {container}, {}));
    ASSERT_OK(receiverBusBuffer.Initialize({}, {container}, {}));

    std::vector<EthMessageContainer> sendMessages;

    // Act
    for (uint32_t i = 0; i < container.controller.queueSize; i++) {
        EthMessageContainer& sendMessage = sendMessages.emplace_back();
        CreateMessage(container.controller.id, sendMessage);
        ASSERT_OK(senderBusBuffer.Transmit(sendMessage.message));
    }

    EthMessageContainer rejectedMessage{};
    CreateMessage(container.controller.id, rejectedMessage);
    ASSERT_FULL(senderBusBuffer.Transmit(rejectedMessage.message));

    Transfer(senderBusBuffer, receiverBusBuffer);

    // Assert
    EthMessage receivedMessage{};
    for (uint32_t i = 0; i < container.controller.queueSize; i++) {
        ASSERT_OK(receiverBusBuffer.Receive(receivedMessage));
        AssertEq(sendMessages[i].message, receivedMessage);
    }

    ASSERT_EMPTY(receiverBusBuffer.Receive(receivedMessage));
}

TEST_F(TestBusBuffer, DuplicateLinIds) {
    // Arrange
    LinControllerContainer container;
    CreateController(container);

    BusBuffer busBuffer;

    // Act
    const Result result = busBuffer.Initialize({}, {}, {container, container});

    // Assert
    ASSERT_ERROR(result);
    AssertLastMessage("Duplicated LIN controller id " + std::to_string(container.controller.id) + ".");
}

TEST_F(TestBusBuffer, ReceiveLinMessageOnEmptyBuffer) {
    // Arrange
    LinControllerContainer container;
    CreateController(container);

    BusBuffer receiverBusBuffer;
    ASSERT_OK(receiverBusBuffer.Initialize({}, {}, {container}));

    LinMessage receivedMessage{};

    // Act
    const Result result = receiverBusBuffer.Receive(receivedMessage);

    // Assert
    ASSERT_EMPTY(result);
}

TEST_F(TestBusBuffer, TransmitAndReceiveLinMessages) {
    // Arrange
    LinControllerContainer container;
    CreateController(container);

    BusBuffer senderBusBuffer;
    BusBuffer receiverBusBuffer;

    ASSERT_OK(senderBusBuffer.Initialize({}, {}, {container}));
    ASSERT_OK(receiverBusBuffer.Initialize({}, {}, {container}));

    std::vector<LinMessageContainer> sendMessages;

    // Act
    for (uint32_t i = 0; i < container.controller.queueSize; i++) {
        LinMessageContainer& sendMessage = sendMessages.emplace_back();
        CreateMessage(container.controller.id, sendMessage);
        ASSERT_OK(senderBusBuffer.Transmit(sendMessage.message));
    }

    LinMessageContainer rejectedMessage{};
    CreateMessage(container.controller.id, rejectedMessage);
    ASSERT_FULL(senderBusBuffer.Transmit(rejectedMessage.message));

    Transfer(senderBusBuffer, receiverBusBuffer);

    // Assert
    LinMessage receivedMessage{};
    for (uint32_t i = 0; i < container.controller.queueSize; i++) {
        ASSERT_OK(receiverBusBuffer.Receive(receivedMessage));
        AssertEq(sendMessages[i].message, receivedMessage);
    }

    ASSERT_EMPTY(receiverBusBuffer.Receive(receivedMessage));
}

// Add more tests
