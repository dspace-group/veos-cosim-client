// Copyright dSPACE GmbH. All rights reserved.

#include <array>
#include <chrono>
#include <thread>

#include "Communication.h"
#include "Generator.h"
#include "Logger.h"
#include "Socket.h"
#include "TestHelper.h"

using namespace std::chrono_literals;
using namespace DsVeosCoSim;

class TestCommunication : public testing::Test {
protected:
    void SetUp() override {
        SetLogCallback(OnLogCallback);

        ClearLastMessage();
    }
};

TEST_F(TestCommunication, ServerStartPortArgumentZero) {
    // Arrange
    Server server;
    uint16_t port{};

    // Act
    ASSERT_OK(server.Start(port, true));

    // Assert
    ASSERT_NE(static_cast<uint16_t>(0), port);
}

TEST_F(TestCommunication, ConnectToServerIpv4) {
    // Arrange
    Server server;
    uint16_t port{};
    ASSERT_OK(server.Start(port, true));

    Channel connectedChannel;

    // Act
    const Result result = ConnectToServer("127.0.0.1", port, 0, connectedChannel);

    // Assert
    ASSERT_OK(result);
}

// TEST_F(TestCommunication, ConnectToServerIpv6) {
//     // Arrange
//     Server server;
//     uint16_t port{};
//     ASSERT_OK(server.Start(port, true));

//     Channel connectedChannel;

//     // Act
//     const Result result = ConnectToServer("::1", port, 0, connectedChannel);

//     // Assert
//     ASSERT_OK(result);
// }

TEST_F(TestCommunication, AcceptClient) {
    // Arrange
    Server server;
    uint16_t port{};
    ASSERT_OK(server.Start(port, true));

    Channel connectedChannel;
    ASSERT_OK(ConnectToServer("127.0.0.1", port, 0, connectedChannel));

    Channel acceptedChannel;

    std::string clientIpAddress;
    uint16_t clientPort{};

    std::string acceptedIpAddress;
    uint16_t acceptedPort{};

    // Act
    ASSERT_OK(server.Accept(acceptedChannel));

    // Assert
    ASSERT_OK(connectedChannel.GetRemoteAddress(clientIpAddress, clientPort));
    AssertEq(clientIpAddress, "127.0.0.1");
    ASSERT_EQ(port, clientPort);

    ASSERT_OK(acceptedChannel.GetRemoteAddress(acceptedIpAddress, acceptedPort));
    AssertEq(acceptedIpAddress, "127.0.0.1");
    ASSERT_NE(static_cast<uint16_t>(0), acceptedPort);

    ASSERT_NE(port, acceptedPort);
}

TEST_F(TestCommunication, AcceptAfterDisconnect) {
    // Arrange
    Server server;
    uint16_t port{};
    ASSERT_OK(server.Start(port, true));

    Channel connectedChannel;
    ASSERT_OK(ConnectToServer("127.0.0.1", port, 0, connectedChannel));

    // After disconnect, the server should still be able to accept it.
    connectedChannel.Disconnect();

    Channel acceptedChannel;

    std::string acceptedIpAddress;
    uint16_t acceptedPort{};

    // Act
    ASSERT_OK(server.Accept(acceptedChannel));

    // Assert
    ASSERT_OK(acceptedChannel.GetRemoteAddress(acceptedIpAddress, acceptedPort));
    AssertEq(acceptedIpAddress, "127.0.0.1");

    ASSERT_NE(static_cast<uint16_t>(0), acceptedPort);
    ASSERT_NE(port, acceptedPort);
}

TEST_F(TestCommunication, ConnectAfterServerStopped) {
    // Arrange
    Server server;
    uint16_t port{};
    ASSERT_OK(server.Start(port, true));
    server.Stop();

    Channel connectedChannel;

    // Act
    const Result result = ConnectToServer("127.0.0.1", port, 0, connectedChannel);

    // Assert
    ASSERT_ERROR(result);
}

TEST_F(TestCommunication, SendByClientAndReceiveByServer) {
    // Arrange
    Server server;
    uint16_t port{};
    ASSERT_OK(server.Start(port, true));

    Channel connectedChannel;
    ASSERT_OK(ConnectToServer("127.0.0.1", port, 0, connectedChannel));

    Channel acceptedChannel;
    ASSERT_OK(server.Accept(acceptedChannel));

    const uint32_t sendValue = GenerateU32();
    uint32_t receiveValue{};

    // Act
    ASSERT_OK(connectedChannel.Write(sendValue));
    ASSERT_OK(connectedChannel.EndWrite());

    ASSERT_OK(acceptedChannel.Read(receiveValue));

    // Assert
    ASSERT_EQ(sendValue, receiveValue);
}

TEST_F(TestCommunication, SendByServerAndReceiveByClient) {
    // Arrange
    Server server;
    uint16_t port{};
    ASSERT_OK(server.Start(port, true));

    Channel connectedChannel;
    ASSERT_OK(ConnectToServer("127.0.0.1", port, 0, connectedChannel));

    Channel acceptedChannel;
    ASSERT_OK(server.Accept(acceptedChannel));

    const uint64_t sendValue = GenerateU32();
    uint64_t receiveValue{};

    // Act
    ASSERT_OK(acceptedChannel.Write(sendValue));
    ASSERT_OK(acceptedChannel.EndWrite());

    ASSERT_OK(connectedChannel.Read(receiveValue));

    // Assert
    ASSERT_EQ(sendValue, receiveValue);
}

TEST_F(TestCommunication, PingPong) {
    Server server;
    uint16_t port = 0;
    ASSERT_OK(server.Start(port, true));

    Channel connectedChannel;
    ASSERT_OK(ConnectToServer("127.0.0.1", port, 0, connectedChannel));

    Channel acceptedChannel;
    ASSERT_OK(server.Accept(acceptedChannel));

    // Send and receive
    for (uint16_t i = 0; i < 100; i++) {
        Channel* sendChannel = &connectedChannel;
        Channel* receiveChannel = &acceptedChannel;
        if (i % 2 == 1) {
            sendChannel = &acceptedChannel;
            receiveChannel = &connectedChannel;
        }

        uint16_t sendValue = i;
        ASSERT_OK(sendChannel->Write(sendValue));
        ASSERT_OK(sendChannel->EndWrite());

        uint16_t receiveValue = 0;
        ASSERT_OK(receiveChannel->Read(receiveValue));

        ASSERT_EQ(sendValue, receiveValue);
    }
}

TEST_F(TestCommunication, SendTwoFramesAtOnce) {
    Server server;
    uint16_t port = 0;
    ASSERT_OK(server.Start(port, true));

    Channel connectedChannel;
    ASSERT_OK(ConnectToServer("127.0.0.1", port, 0, connectedChannel));

    Channel acceptedChannel;
    ASSERT_OK(server.Accept(acceptedChannel));

    constexpr uint8_t sendValue = 121;
    ASSERT_OK(acceptedChannel.Write(sendValue));
    ASSERT_OK(acceptedChannel.EndWrite());

    constexpr uint8_t sendBuffer[2]{12, 24};
    ASSERT_OK(acceptedChannel.Write(sendBuffer, 2));
    ASSERT_OK(acceptedChannel.EndWrite());

    uint8_t receiveValue = 0;
    ASSERT_OK(connectedChannel.Read(receiveValue));

    uint8_t receiveBuffer[2]{};
    ASSERT_OK(connectedChannel.Read(receiveBuffer, 2));

    ASSERT_EQ(sendValue, receiveValue);
    ASSERT_EQ(sendBuffer[0], receiveBuffer[0]);
    ASSERT_EQ(sendBuffer[1], receiveBuffer[1]);
}

void WriteExactly(const Socket& socket, const uint8_t* buffer, int length) {
    while (length > 0) {
        int sentSize = 0;
        ASSERT_OK(socket.Send(buffer, length, sentSize));

        length -= sentSize;
        buffer += sentSize;
    }
}

void SendTwoMessagesDelayed(Channel& acceptedChannel) {
    uint32_t readValue1 = 0;
    ASSERT_OK(acceptedChannel.Read(readValue1));

    uint32_t readValue2 = 0;
    ASSERT_OK(acceptedChannel.Read(readValue2));

    ASSERT_EQ(readValue1, 0x11223344U);
    ASSERT_EQ(readValue2, 0x55667788U);
}

TEST_F(TestCommunication, SendDelayed) {
    Server server;
    uint16_t port = 0;
    ASSERT_OK(server.Start(port, true));

    Socket socket;
    ASSERT_OK(socket.Connect("127.0.0.1", port, 0));

    Channel acceptedChannel;
    ASSERT_OK(server.Accept(acceptedChannel));

    std::thread thread(SendTwoMessagesDelayed, std::ref(acceptedChannel));

    constexpr uint8_t buffer[]{8, 0, 0, 0, 0x44, 0x33, 0x22, 0x11, 8, 0, 0, 0, 0x88, 0x77, 0x66, 0x55};
    WriteExactly(socket, buffer, 1);
    std::this_thread::sleep_for(50ms);
    for (int i = 0; i < 7; i++) {
        const int offset = 1 + (i * 2);
        WriteExactly(socket, buffer + offset, 2);
        std::this_thread::sleep_for(50ms);
    }

    WriteExactly(socket, buffer + 15, 1);

    thread.join();
}

void StreamClient(uint16_t port) {
    Channel connectedChannel;
    ASSERT_OK(ConnectToServer("127.0.0.1", port, 0, connectedChannel));

    for (uint32_t i = 0; i < 4 * 1024 * 1024; i++) {
        uint32_t receiveValue = 0;
        ASSERT_OK(connectedChannel.Read(receiveValue));

        ASSERT_EQ(i, receiveValue);
    }
}

TEST_F(TestCommunication, Stream) {
    Server server;
    uint16_t port = 0;
    ASSERT_OK(server.Start(port, true));

    std::thread thread(StreamClient, port);

    Channel acceptedChannel;
    ASSERT_OK(server.Accept(acceptedChannel));

    for (uint32_t i = 0U; i < 4U * 1024U * 1024U; i++) {
        ASSERT_OK(acceptedChannel.Write(i));
    }

    ASSERT_OK(acceptedChannel.EndWrite());

    thread.join();
}

void ReceiveBigArray(uint16_t port) {
    Channel connectedChannel;
    ASSERT_OK(ConnectToServer("127.0.0.1", port, 0, connectedChannel));

    const auto receiveArray = std::make_unique<std::array<uint8_t, 5000000>>();
    ASSERT_OK(connectedChannel.Read(receiveArray->data(), static_cast<uint32_t>(receiveArray->size())));
}

TEST_F(TestCommunication, ArrayExceedsTheSizeOfChannelBuffer) {
    Server server;
    uint16_t port = 0;
    ASSERT_OK(server.Start(port, true));

    std::thread thread(ReceiveBigArray, port);

    Channel acceptedChannel;
    ASSERT_OK(server.Accept(acceptedChannel));

    const auto sendArray = std::make_unique<std::array<uint8_t, 5000000>>();
    ASSERT_OK(acceptedChannel.Write(sendArray->data(), static_cast<uint32_t>(sendArray->size())));
    ASSERT_OK(acceptedChannel.EndWrite());

    thread.join();
}

void ReceiveBigElement(uint16_t port) {
    Channel connectedChannel;
    ASSERT_OK(ConnectToServer("127.0.0.1", port, 0, connectedChannel));

    const auto receiveArray = std::make_unique<std::array<uint8_t, 500000>>();
    ASSERT_OK(connectedChannel.Read(receiveArray.get(), 500000));
}

TEST_F(TestCommunication, ElementExceedsTheSizeOfReadOperation) {
    Server server;
    uint16_t port = 0;
    ASSERT_OK(server.Start(port, true));

    std::thread thread(ReceiveBigElement, port);

    Channel acceptedChannel;
    ASSERT_OK(server.Accept(acceptedChannel));

    const auto sendArray = std::make_unique<std::array<uint8_t, 500000>>();
    ASSERT_OK(acceptedChannel.Write(sendArray.get(), 500000));
    ASSERT_OK(acceptedChannel.EndWrite());

    thread.join();
}
