// Copyright dSPACE GmbH. All rights reserved.

#include "BusBuffer.h"
#include "Communication.h"
#include "Generator.h"
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
    CanController controller;
    CreateController(controller, GenerateU32());

    BusBuffer busBuffer;

    // Act
    const Result result = busBuffer.Initialize({controller, controller}, {}, {});

    // Assert
    ASSERT_ERROR(result);
    AssertLastMessage("Duplicated controller id " + ToString(controller.id) + ".");
}

TEST_F(TestBusBuffer, ReceiveCanMessageOnEmptyBuffer) {
    // Arrange
    CanController controller;
    CreateController(controller, GenerateU32());

    BusBuffer receiverBusBuffer;
    ASSERT_OK(receiverBusBuffer.Initialize({controller}, {}, {}));

    DsVeosCoSim_CanMessage receivedMessage{};

    // Act
    const Result result = receiverBusBuffer.Receive(receivedMessage);

    // Assert
    ASSERT_EMPTY(result);
}

TEST_F(TestBusBuffer, TransmitAndReceiveCanMessages) {  // NOLINT(readability-function-cognitive-complexity)
    // Arrange
    CanController controller;
    CreateController(controller, GenerateU32());

    BusBuffer senderBusBuffer;
    BusBuffer receiverBusBuffer;

    ASSERT_OK(senderBusBuffer.Initialize({controller}, {}, {}));
    ASSERT_OK(receiverBusBuffer.Initialize({controller}, {}, {}));

    std::vector<CanMessageContainer> sendMessages;
    sendMessages.reserve(controller.queueSize);

    // Act
    for (uint32_t i = 0; i < controller.queueSize; i++) {
        CanMessageContainer& sendMessage = sendMessages.emplace_back();
        CreateMessage(controller.id, sendMessage);
        ASSERT_OK(senderBusBuffer.Transmit(sendMessage.message));
    }

    CanMessageContainer rejectedMessage{};
    CreateMessage(controller.id, rejectedMessage);
    ASSERT_FULL(senderBusBuffer.Transmit(rejectedMessage.message));

    Transfer(senderBusBuffer, receiverBusBuffer);

    // Assert
    DsVeosCoSim_CanMessage receivedMessage{};
    for (uint32_t i = 0; i < controller.queueSize; i++) {
        ASSERT_OK(receiverBusBuffer.Receive(receivedMessage));
        AssertEq(sendMessages[i].message, receivedMessage);
    }

    ASSERT_EMPTY(receiverBusBuffer.Receive(receivedMessage));
}

TEST_F(TestBusBuffer, DuplicateEthIds) {
    // Arrange
    EthController controller;
    CreateController(controller, GenerateU32());

    BusBuffer busBuffer;

    // Act
    const Result result = busBuffer.Initialize({}, {controller, controller}, {});

    // Assert
    ASSERT_ERROR(result);
    AssertLastMessage("Duplicated controller id " + ToString(controller.id) + ".");
}

TEST_F(TestBusBuffer, ReceiveEthMessageOnEmptyBuffer) {
    // Arrange
    EthController controller;
    CreateController(controller, GenerateU32());

    BusBuffer receiverBusBuffer;
    ASSERT_OK(receiverBusBuffer.Initialize({}, {controller}, {}));

    DsVeosCoSim_EthMessage receivedMessage{};

    // Act
    const Result result = receiverBusBuffer.Receive(receivedMessage);

    // Assert
    ASSERT_EMPTY(result);
}

TEST_F(TestBusBuffer, TransmitAndReceiveEthMessages) {  // NOLINT(readability-function-cognitive-complexity)
    // Arrange
    EthController controller;
    CreateController(controller, GenerateU32());

    BusBuffer senderBusBuffer;
    BusBuffer receiverBusBuffer;

    ASSERT_OK(senderBusBuffer.Initialize({}, {controller}, {}));
    ASSERT_OK(receiverBusBuffer.Initialize({}, {controller}, {}));

    std::vector<EthMessageContainer> sendMessages;
    sendMessages.reserve(controller.queueSize);

    // Act
    for (uint32_t i = 0; i < controller.queueSize; i++) {
        EthMessageContainer& sendMessage = sendMessages.emplace_back();
        CreateMessage(controller.id, sendMessage);
        ASSERT_OK(senderBusBuffer.Transmit(sendMessage.message));
    }

    EthMessageContainer rejectedMessage{};
    CreateMessage(controller.id, rejectedMessage);
    ASSERT_FULL(senderBusBuffer.Transmit(rejectedMessage.message));

    Transfer(senderBusBuffer, receiverBusBuffer);

    // Assert
    DsVeosCoSim_EthMessage receivedMessage{};
    for (uint32_t i = 0; i < controller.queueSize; i++) {
        ASSERT_OK(receiverBusBuffer.Receive(receivedMessage));
        AssertEq(sendMessages[i].message, receivedMessage);
    }

    ASSERT_EMPTY(receiverBusBuffer.Receive(receivedMessage));
}

TEST_F(TestBusBuffer, DuplicateLinIds) {
    // Arrange
    LinController controller;
    CreateController(controller, GenerateU32());

    BusBuffer busBuffer;

    // Act
    const Result result = busBuffer.Initialize({}, {}, {controller, controller});

    // Assert
    ASSERT_ERROR(result);
    AssertLastMessage("Duplicated controller id " + ToString(controller.id) + ".");
}

TEST_F(TestBusBuffer, ReceiveLinMessageOnEmptyBuffer) {
    // Arrange
    LinController controller;
    CreateController(controller, GenerateU32());

    BusBuffer receiverBusBuffer;
    ASSERT_OK(receiverBusBuffer.Initialize({}, {}, {controller}));

    DsVeosCoSim_LinMessage receivedMessage{};

    // Act
    const Result result = receiverBusBuffer.Receive(receivedMessage);

    // Assert
    ASSERT_EMPTY(result);
}

TEST_F(TestBusBuffer, TransmitAndReceiveLinMessages) {  // NOLINT(readability-function-cognitive-complexity)
    // Arrange
    LinController controller;
    CreateController(controller, GenerateU32());

    BusBuffer senderBusBuffer;
    BusBuffer receiverBusBuffer;

    ASSERT_OK(senderBusBuffer.Initialize({}, {}, {controller}));
    ASSERT_OK(receiverBusBuffer.Initialize({}, {}, {controller}));

    std::vector<LinMessageContainer> sendMessages;
    sendMessages.reserve(controller.queueSize);

    // Act
    for (uint32_t i = 0; i < controller.queueSize; i++) {
        LinMessageContainer& sendMessage = sendMessages.emplace_back();
        CreateMessage(controller.id, sendMessage);
        ASSERT_OK(senderBusBuffer.Transmit(sendMessage.message));
    }

    LinMessageContainer rejectedMessage{};
    CreateMessage(controller.id, rejectedMessage);
    ASSERT_FULL(senderBusBuffer.Transmit(rejectedMessage.message));

    Transfer(senderBusBuffer, receiverBusBuffer);

    // Assert
    DsVeosCoSim_LinMessage receivedMessage{};
    for (uint32_t i = 0; i < controller.queueSize; i++) {
        ASSERT_OK(receiverBusBuffer.Receive(receivedMessage));
        AssertEq(sendMessages[i].message, receivedMessage);
    }

    ASSERT_EMPTY(receiverBusBuffer.Receive(receivedMessage));
}

// Add more tests
