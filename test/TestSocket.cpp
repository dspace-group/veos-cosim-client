// Copyright dSPACE GmbH. All rights reserved.

#include "Generator.h"
#include "Logger.h"
#include "Socket.h"
#include "TestHelper.h"

#include <thread>

using namespace std::chrono_literals;
using namespace DsVeosCoSim;

namespace {

void SendExactly(const Socket& socket, const void* buffer, size_t length) {
    const auto* bufferPointer = (const uint8_t*)buffer;
    while (length > 0) {
        int sentSize = 0;
        ASSERT_OK(socket.Send(bufferPointer, (int)length, sentSize));

        length -= sentSize;
        bufferPointer += sentSize;
    }
}

void ReceiveExactly(const Socket& socket, void* buffer, size_t length) {
    auto* bufferPointer = (uint8_t*)buffer;
    while (length > 0) {
        int receivedSize = 0;
        ASSERT_OK(socket.Receive(bufferPointer, (int)length, receivedSize));

        length -= receivedSize;
        bufferPointer += receivedSize;
    }
}

bool stopThread;

void ReceiveAndSendUds(const std::string& path, size_t size) {
    Socket client;
    ASSERT_OK(client.Create(AddressFamily::Uds));

    ASSERT_OK(client.Connect(path));

    std::vector<uint8_t> buffer;
    buffer.resize(size);

    ReceiveExactly(client, buffer.data(), buffer.size());

    while (!stopThread) {
        SendExactly(client, buffer.data(), buffer.size());
        ReceiveExactly(client, buffer.data(), buffer.size());
    }
}

}  // namespace

class TestSocket : public testing::Test {
protected:
    void SetUp() override {
        SetLogCallback(OnLogCallback);

        ClearLastMessage();
    }
};

TEST_F(TestSocket, UdsSocketCreate) {
    // Arrange
    Socket server;

    // Act
    Result result = server.Create(AddressFamily::Uds);

    // Assert
    ASSERT_OK(result);
}

TEST_F(TestSocket, UdsServerBind) {
    // Arrange
    Socket server;
    std::string path = GenerateString(std::string("Path"));
    ASSERT_OK(server.Create(AddressFamily::Uds));

    // Act
    Result result = server.Bind(path);

    // Assert
    ASSERT_OK(result);
}

TEST_F(TestSocket, UdsServerListen) {
    // Arrange
    Socket server;
    std::string path = GenerateString(std::string("Path"));
    ASSERT_OK(server.Create(AddressFamily::Uds));
    ASSERT_OK(server.Bind(path));

    // Act
    Result result = server.Listen();

    // Assert
    ASSERT_OK(result);
}

TEST_F(TestSocket, UdsClientConnect) {
    // Arrange
    Socket server;
    Socket client;
    std::string path = GenerateString(std::string("Path"));
    ASSERT_OK(server.Create(AddressFamily::Uds));
    ASSERT_OK(server.Bind(path));
    ASSERT_OK(server.Listen());
    ASSERT_OK(client.Create(AddressFamily::Uds));

    // Act
    Result result = client.Connect(path);

    // Assert
    ASSERT_OK(result);
}

TEST_F(TestSocket, UdsServerAcceptAfterConnect) {
    // Arrange
    Socket server;
    Socket client;
    std::string path = GenerateString(std::string("Path"));
    ASSERT_OK(server.Create(AddressFamily::Uds));
    ASSERT_OK(server.Bind(path));
    ASSERT_OK(server.Listen());
    ASSERT_OK(client.Create(AddressFamily::Uds));
    ASSERT_OK(client.Connect(path));
    Socket acceptedClient;

    // Act
    Result result = server.Accept(acceptedClient);

    // Assert
    ASSERT_OK(result);
}

TEST_F(TestSocket, UdsSendAndReceive) {  // NOLINT(readability-function-cognitive-complexity)
    // Arrange
    Socket server;
    Socket client;
    std::string path = GenerateString(std::string("Path"));
    ASSERT_OK(server.Create(AddressFamily::Uds));
    ASSERT_OK(server.Bind(path));
    ASSERT_OK(server.Listen());
    ASSERT_OK(client.Create(AddressFamily::Uds));
    ASSERT_OK(client.Connect(path));
    Socket acceptedClient;
    ASSERT_OK(server.Accept(acceptedClient));

    constexpr int sendValue = 42;
    int receiveValue = 0;

    // Act
    SendExactly(client, &sendValue, sizeof(sendValue));
    ReceiveExactly(acceptedClient, &receiveValue, sizeof(receiveValue));

    // Assert
    ASSERT_EQ(sendValue, receiveValue);
}

TEST_F(TestSocket, UdsRoundtrip) {  // NOLINT(readability-function-cognitive-complexity)
    ASSERT_OK(StartupNetwork());

    std::string path = GenerateString("Path");
    Socket server;
    ASSERT_OK(server.Create(AddressFamily::Uds));
    ASSERT_OK(server.Bind(path));
    ASSERT_OK(server.Listen());

    size_t size = 10000;

    stopThread = false;
    std::jthread thread(ReceiveAndSendUds, path, size);

    Socket client;
    while (true) {
        Result result = server.Accept(client);
        if (result == Result::TryAgain) {
            continue;
        }

        ASSERT_OK(result);
        break;
    }

    std::vector<uint8_t> buffer;
    buffer.resize(size);

    for (int i = 0; i < 100; i++) {
        SendExactly(client, buffer.data(), buffer.size());
        ReceiveExactly(client, buffer.data(), buffer.size());
    }

    stopThread = true;
    SendExactly(client, buffer.data(), buffer.size());
}
