// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <chrono>
#include <string>

#include <gtest/gtest.h>

#include "Helper.hpp"
#include "Socket.hpp"
#include "TestHelper.hpp"

using namespace std::chrono_literals;
using namespace DsVeosCoSim;

namespace {

[[nodiscard]] std::string GenerateName() {
    return GenerateString("LocalPath");
}

void EstablishConnection(const std::string& name, SocketClient& connectClient, SocketClient& acceptClient) {
    SocketListener listener;
    AssertOk(SocketListener::Create(name, listener));

    AssertOk(SocketClient::TryConnect(name, connectClient));

    AssertOk(listener.TryAccept(acceptClient));
}

class TestLocalSocket : public testing::Test {};

TEST_F(TestLocalSocket, CreateListenerShouldWork) {
    // Arrange
    std::string name = GenerateName();

    SocketListener listener;

    // Act
    Result result = SocketListener::Create(name, listener);

    // Assert
    AssertOk(result);
}

TEST_F(TestLocalSocket, ConnectToListeningSocketShouldWork) {
    // Arrange
    std::string name = GenerateName();

    SocketListener listener;
    AssertOk(SocketListener::Create(name, listener));

    SocketClient client;

    // Act
    Result result = SocketClient::TryConnect(name, client);

    // Assert
    AssertOk(result);
}

TEST_F(TestLocalSocket, ConnectWithoutListeningShouldNotWork) {
    // Arrange
    std::string name = GenerateName();

    {
        SocketListener listener;
        AssertOk(SocketListener::Create(name, listener));
    }

    SocketClient client;

    // Act
    Result result = SocketClient::TryConnect(name, client);

    // Assert
    AssertNotConnected(result);
}

TEST_F(TestLocalSocket, AcceptWithoutConnectShouldNotWork) {
    // Arrange
    std::string name = GenerateName();

    SocketListener listener;
    AssertOk(SocketListener::Create(name, listener));

    SocketClient client;

    // Act
    Result result = listener.TryAccept(client);

    // Assert
    AssertNotConnected(result);
}

TEST_F(TestLocalSocket, AcceptAfterStopShouldNotWork) {
    // Arrange
    std::string name = GenerateName();

    SocketListener listener;
    AssertOk(SocketListener::Create(name, listener));

    listener.Stop();

    SocketClient client;

    // Act
    Result result = listener.TryAccept(client);

    // Assert
    AssertError(result);
}

TEST_F(TestLocalSocket, AcceptWithConnectShouldWork) {
    // Arrange
    std::string name = GenerateName();

    SocketListener listener;
    AssertOk(SocketListener::Create(name, listener));

    SocketClient connectClient;
    AssertOk(SocketClient::TryConnect(name, connectClient));

    SocketClient acceptClient;

    // Act
    Result result = listener.TryAccept(acceptClient);

    // Assert
    AssertOk(result);
}

TEST_F(TestLocalSocket, WakeUpBlockingCallInConnectClientOnRemoteClient) {
    // Arrange
    std::string name = GenerateName();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(name, connectClient, acceptClient);

    std::thread t([&] {
        std::array<uint8_t, 10> buffer{};
        size_t receivedSize{};
        AssertNotConnected(connectClient.Receive(buffer.data(), buffer.size(), receivedSize));
        ASSERT_EQ(0, receivedSize);
    });

    std::this_thread::sleep_for(100ms);

    // Act and assert
    acceptClient.Disconnect();

    // Cleanup
    t.join();
}

TEST_F(TestLocalSocket, WakeUpBlockingCallInAcceptClientOnRemoteClient) {
    // Arrange
    std::string name = GenerateName();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(name, connectClient, acceptClient);

    std::thread t([&] {
        std::array<uint8_t, 10> buffer{};
        size_t receivedSize{};
        AssertNotConnected(acceptClient.Receive(buffer.data(), buffer.size(), receivedSize));
        ASSERT_EQ(0, receivedSize);
    });

    std::this_thread::sleep_for(100ms);

    // Act and assert
    connectClient.Disconnect();

    // Cleanup
    t.join();
}

TEST_F(TestLocalSocket, WakeUpBlockingCallInConnectClientOnLocalClient) {
    // Arrange
    std::string name = GenerateName();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(name, connectClient, acceptClient);

    std::thread t([&] {
        std::array<uint8_t, 10> buffer{};
        size_t receivedSize{};
        AssertNotConnected(connectClient.Receive(buffer.data(), buffer.size(), receivedSize));
        ASSERT_EQ(0, receivedSize);
    });

    std::this_thread::sleep_for(100ms);

    // Act and assert
    connectClient.Disconnect();

    // Cleanup
    t.join();
}

TEST_F(TestLocalSocket, WakeUpBlockingCallInAcceptClientOnLocalClient) {
    // Arrange
    std::string name = GenerateName();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(name, connectClient, acceptClient);

    std::thread t([&] {
        std::array<uint8_t, 10> buffer{};
        size_t receivedSize{};
        AssertNotConnected(acceptClient.Receive(buffer.data(), buffer.size(), receivedSize));
        ASSERT_EQ(0, receivedSize);
    });

    std::this_thread::sleep_for(100ms);

    // Act and assert
    acceptClient.Disconnect();

    // Cleanup
    t.join();
}

TEST_F(TestLocalSocket, SendOnConnectClientAndReceiveOnAcceptClientShouldWork) {
    // Arrange
    std::string name = GenerateName();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(name, connectClient, acceptClient);

    // Act and assert
    TestSendAndReceive(connectClient, acceptClient);
}

TEST_F(TestLocalSocket, SendOnAcceptClientAndReceiveOnConnectClientShouldWork) {
    // Arrange
    std::string name = GenerateName();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(name, connectClient, acceptClient);

    // Act and assert
    TestSendAndReceive(acceptClient, connectClient);
}

TEST_F(TestLocalSocket, PingPongBeginningWithConnectClientShouldWork) {
    // Arrange
    std::string name = GenerateName();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(name, connectClient, acceptClient);

    // Act and assert
    TestPingPong(connectClient, acceptClient);
}

TEST_F(TestLocalSocket, PingPongBeginningWithAcceptClientShouldWork) {
    // Arrange
    std::string name = GenerateName();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(name, connectClient, acceptClient);

    // Act and assert
    TestPingPong(acceptClient, connectClient);
}

TEST_F(TestLocalSocket, SendManyElementsFromConnectClientToAcceptClientShouldWork) {
    // Arrange
    std::string name = GenerateName();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(name, connectClient, acceptClient);

    // Act and assert
    TestManyElements(connectClient, acceptClient);
}

TEST_F(TestLocalSocket, SendManyElementsFromAcceptClientToConnectClientShouldWork) {
    // Arrange
    std::string name = GenerateName();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(name, connectClient, acceptClient);

    // Act and assert
    TestManyElements(acceptClient, connectClient);
}

TEST_F(TestLocalSocket, SendBigElementFromConnectClientToAcceptClientShouldWork) {
    // Arrange
    std::string name = GenerateName();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(name, connectClient, acceptClient);

    // Act and assert
    TestBigElement(connectClient, acceptClient);
}

TEST_F(TestLocalSocket, SendBigElementFromAcceptClientToConnectClientShouldWork) {
    // Arrange
    std::string name = GenerateName();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(name, connectClient, acceptClient);

    // Act and assert
    TestBigElement(acceptClient, connectClient);
}

TEST_F(TestLocalSocket, SendOnDisconnectedConnectClientShouldNotWork) {
    // Arrange
    std::string name = GenerateName();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(name, connectClient, acceptClient);

    // Act and assert
    TestSendAfterDisconnect(connectClient);
}

TEST_F(TestLocalSocket, SendOnDisconnectedAcceptClientShouldNotWork) {
    // Arrange
    std::string name = GenerateName();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(name, connectClient, acceptClient);

    // Act and assert
    TestSendAfterDisconnect(acceptClient);
}

// TEST_F(TestLocalSocket, SendOnDisconnectedRemoteConnectClientShouldNotWork) {
//     // Arrange
//     std::string name = GenerateName();

//     SocketClient connectClient;
//     SocketClient acceptClient;
//     EstablishConnection(name, connectClient, acceptClient);

//     // Act and assert
//     TestSendAfterDisconnectOnRemoteClient(connectClient, acceptClient);
// }

// TEST_F(TestLocalSocket, SendOnDisconnectedRemoteAcceptClientShouldNotWork) {
//     // Arrange
//     std::string name = GenerateName();

//     SocketClient connectClient;
//     SocketClient acceptClient;
//     EstablishConnection(name, connectClient, acceptClient);

//     // Act and assert
//     TestSendAfterDisconnectOnRemoteClient(acceptClient, connectClient);
// }

TEST_F(TestLocalSocket, ReceiveOnDisconnectedConnectClientShouldNotWork) {
    // Arrange
    std::string name = GenerateName();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(name, connectClient, acceptClient);

    // Act and assert
    TestReceiveAfterDisconnect(connectClient);
}

TEST_F(TestLocalSocket, ReceiveOnDisconnectedAcceptClientShouldNotWork) {
    // Arrange
    std::string name = GenerateName();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(name, connectClient, acceptClient);

    // Act and assert
    TestReceiveAfterDisconnect(acceptClient);
}

TEST_F(TestLocalSocket, ReceiveOnDisconnectedRemoteConnectClientShouldNotWork) {
    // Arrange
    std::string name = GenerateName();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(name, connectClient, acceptClient);

    // Act and assert
    TestReceiveAfterDisconnectOnRemoteClient(connectClient, acceptClient);
}

TEST_F(TestLocalSocket, ReceiveOnDisconnectedRemoteAcceptClientShouldNotWork) {
    // Arrange
    std::string name = GenerateName();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(name, connectClient, acceptClient);

    // Act and assert
    TestReceiveAfterDisconnectOnRemoteClient(acceptClient, connectClient);
}

}  // namespace
