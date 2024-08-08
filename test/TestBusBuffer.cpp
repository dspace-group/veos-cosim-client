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
    FillWithRandom(controller);

    BusBuffer busBuffer;

    // Act
    const Result result = busBuffer.Initialize({controller.Convert(), controller.Convert()}, {}, {});

    // Assert
    ASSERT_ERROR(result);
    AssertLastMessage("Duplicated controller id " + std::to_string(controller.id) + ".");
}

TEST_F(TestBusBuffer, ReceiveCanMessageOnEmptyBuffer) {
    // Arrange
    CanController controller;
    FillWithRandom(controller);

    BusBuffer receiverBusBuffer;
    ASSERT_OK(receiverBusBuffer.Initialize({controller.Convert()}, {}, {}));

    DsVeosCoSim_CanMessage receivedMessage{};

    // Act
    const Result result = receiverBusBuffer.Receive(receivedMessage);

    // Assert
    ASSERT_EMPTY(result);
}

TEST_F(TestBusBuffer, TransmitAndReceiveCanMessages) {  // NOLINT(readability-function-cognitive-complexity)
    // Arrange
    CanController controller;
    FillWithRandom(controller);

    BusBuffer senderBusBuffer;
    BusBuffer receiverBusBuffer;

    ASSERT_OK(senderBusBuffer.Initialize({controller.Convert()}, {}, {}));
    ASSERT_OK(receiverBusBuffer.Initialize({controller.Convert()}, {}, {}));

    std::vector<CanMessage> sendMessages;
    sendMessages.reserve(controller.queueSize);

    // Act
    for (uint32_t i = 0; i < controller.queueSize; i++) {
        CanMessage& sendMessage = sendMessages.emplace_back();
        FillWithRandom(sendMessage, controller.id);
        ASSERT_OK(senderBusBuffer.Transmit(sendMessage.Convert()));
    }

    CanMessage rejectedMessage{};
    FillWithRandom(rejectedMessage, controller.id);
    ASSERT_FULL(senderBusBuffer.Transmit(rejectedMessage.Convert()));

    Transfer(senderBusBuffer, receiverBusBuffer);

    // Assert
    DsVeosCoSim_CanMessage receivedMessage{};
    for (uint32_t i = 0; i < controller.queueSize; i++) {
        ASSERT_OK(receiverBusBuffer.Receive(receivedMessage));
        AssertEq(sendMessages[i].Convert(), receivedMessage);
    }

    ASSERT_EMPTY(receiverBusBuffer.Receive(receivedMessage));
}

TEST_F(TestBusBuffer, DuplicateEthIds) {
    // Arrange
    EthController controller;
    FillWithRandom(controller);

    BusBuffer busBuffer;

    // Act
    const Result result = busBuffer.Initialize({}, {controller.Convert(), controller.Convert()}, {});

    // Assert
    ASSERT_ERROR(result);
    AssertLastMessage("Duplicated controller id " + std::to_string(controller.id) + ".");
}

TEST_F(TestBusBuffer, ReceiveEthMessageOnEmptyBuffer) {
    // Arrange
    EthController controller;
    FillWithRandom(controller);

    BusBuffer receiverBusBuffer;
    ASSERT_OK(receiverBusBuffer.Initialize({}, {controller.Convert()}, {}));

    DsVeosCoSim_EthMessage receivedMessage{};

    // Act
    const Result result = receiverBusBuffer.Receive(receivedMessage);

    // Assert
    ASSERT_EMPTY(result);
}

TEST_F(TestBusBuffer, TransmitAndReceiveEthMessages) {  // NOLINT(readability-function-cognitive-complexity)
    // Arrange
    EthController controller;
    FillWithRandom(controller);

    BusBuffer senderBusBuffer;
    BusBuffer receiverBusBuffer;

    ASSERT_OK(senderBusBuffer.Initialize({}, {controller.Convert()}, {}));
    ASSERT_OK(receiverBusBuffer.Initialize({}, {controller.Convert()}, {}));

    std::vector<EthMessage> sendMessages;
    sendMessages.reserve(controller.queueSize);

    // Act
    for (uint32_t i = 0; i < controller.queueSize; i++) {
        EthMessage& sendMessage = sendMessages.emplace_back();
        FillWithRandom(sendMessage, controller.id);
        ASSERT_OK(senderBusBuffer.Transmit(sendMessage.Convert()));
    }

    EthMessage rejectedMessage{};
    FillWithRandom(rejectedMessage, controller.id);
    ASSERT_FULL(senderBusBuffer.Transmit(rejectedMessage.Convert()));

    Transfer(senderBusBuffer, receiverBusBuffer);

    // Assert
    DsVeosCoSim_EthMessage receivedMessage{};
    for (uint32_t i = 0; i < controller.queueSize; i++) {
        ASSERT_OK(receiverBusBuffer.Receive(receivedMessage));
        AssertEq(sendMessages[i].Convert(), receivedMessage);
    }

    ASSERT_EMPTY(receiverBusBuffer.Receive(receivedMessage));
}

TEST_F(TestBusBuffer, DuplicateLinIds) {
    // Arrange
    LinController controller;
    FillWithRandom(controller);

    BusBuffer busBuffer;

    // Act
    const Result result = busBuffer.Initialize({}, {}, {controller.Convert(), controller.Convert()});

    // Assert
    ASSERT_ERROR(result);
    AssertLastMessage("Duplicated controller id " + std::to_string(controller.id) + ".");
}

TEST_F(TestBusBuffer, ReceiveLinMessageOnEmptyBuffer) {
    // Arrange
    LinController controller;
    FillWithRandom(controller);

    BusBuffer receiverBusBuffer;
    ASSERT_OK(receiverBusBuffer.Initialize({}, {}, {controller.Convert()}));

    DsVeosCoSim_LinMessage receivedMessage{};

    // Act
    const Result result = receiverBusBuffer.Receive(receivedMessage);

    // Assert
    ASSERT_EMPTY(result);
}

TEST_F(TestBusBuffer, TransmitAndReceiveLinMessages) {  // NOLINT(readability-function-cognitive-complexity)
    // Arrange
    LinController controller;
    FillWithRandom(controller);

    BusBuffer senderBusBuffer;
    BusBuffer receiverBusBuffer;

    ASSERT_OK(senderBusBuffer.Initialize({}, {}, {controller.Convert()}));
    ASSERT_OK(receiverBusBuffer.Initialize({}, {}, {controller.Convert()}));

    std::vector<LinMessage> sendMessages;
    sendMessages.reserve(controller.queueSize);

    // Act
    for (uint32_t i = 0; i < controller.queueSize; i++) {
        LinMessage& sendMessage = sendMessages.emplace_back();
        FillWithRandom(sendMessage, controller.id);
        ASSERT_OK(senderBusBuffer.Transmit(sendMessage.Convert()));
    }

    LinMessage rejectedMessage{};
    FillWithRandom(rejectedMessage, controller.id);
    ASSERT_FULL(senderBusBuffer.Transmit(rejectedMessage.Convert()));

    Transfer(senderBusBuffer, receiverBusBuffer);

    // Assert
    DsVeosCoSim_LinMessage receivedMessage{};
    for (uint32_t i = 0; i < controller.queueSize; i++) {
        ASSERT_OK(receiverBusBuffer.Receive(receivedMessage));
        AssertEq(sendMessages[i].Convert(), receivedMessage);
    }

    ASSERT_EMPTY(receiverBusBuffer.Receive(receivedMessage));
}

// Add more tests
