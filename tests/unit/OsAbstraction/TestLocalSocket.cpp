// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <future>
#include <string>
#include <string_view>
#include <thread>

#include <gtest/gtest.h>

#include <Result.hpp>
#include <Socket.hpp>

#include "Helper.hpp"
#include "TestHelper.hpp"

using namespace std::chrono_literals;
using namespace DsVeosCoSim;

namespace {

[[nodiscard]] std::string GenerateLocalSocketName() {
    return GenerateString("LocalPath");
}

void EstablishConnection(std::string_view name, SocketClient& connectClient, SocketClient& acceptClient) {
    SocketListener listener;
    AssertOk(SocketListener::Create(name, listener));

    AssertOk(SocketClient::TryConnect(name, connectClient));

    AssertOk(listener.TryAccept(acceptClient));
}

class TestLocalSocket : public testing::Test {};

TEST_F(TestLocalSocket, IsRunningAfterCreateShouldBeTrue) {
    // Arrange
    std::string name = GenerateLocalSocketName();

    SocketListener listener;
    AssertOk(SocketListener::Create(name, listener));

    // Act
    bool isRunning = listener.IsRunning();

    // Assert
    ASSERT_TRUE(isRunning);
}

TEST_F(TestLocalSocket, IsNotRunningAfterStopShouldBeFalse) {
    // Arrange
    std::string name = GenerateLocalSocketName();

    SocketListener listener;
    AssertOk(SocketListener::Create(name, listener));

    listener.Stop();

    // Act
    bool isRunning = listener.IsRunning();

    // Assert
    ASSERT_FALSE(isRunning);
}

TEST_F(TestLocalSocket, CreateListenerShouldWork) {
    // Arrange
    std::string name = GenerateLocalSocketName();

    SocketListener listener;

    // Act
    Result result = SocketListener::Create(name, listener);

    // Assert
    AssertOk(result);
}

TEST_F(TestLocalSocket, ConnectToListeningSocketShouldWork) {
    // Arrange
    std::string name = GenerateLocalSocketName();

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
    std::string name = GenerateLocalSocketName();

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
    std::string name = GenerateLocalSocketName();

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
    std::string name = GenerateLocalSocketName();

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
    std::string name = GenerateLocalSocketName();

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

TEST_F(TestLocalSocket, IsConnectedAfterConnectAndAcceptShouldBeTrue) {
    // Arrange
    std::string name = GenerateLocalSocketName();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(name, connectClient, acceptClient);

    // Act
    bool isConnected = connectClient.IsConnected();

    // Assert
    ASSERT_TRUE(isConnected);
}

TEST_F(TestLocalSocket, IsNotConnectedAfterDisconnectShouldBeFalse) {
    // Arrange
    std::string name = GenerateLocalSocketName();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(name, connectClient, acceptClient);

    connectClient.Disconnect();

    // Act
    bool isConnected = connectClient.IsConnected();

    // Assert
    ASSERT_FALSE(isConnected);
}

TEST_F(TestLocalSocket, WakeUpBlockingCallInConnectClientOnRemoteClient) {
    // Arrange
    std::string name = GenerateLocalSocketName();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(name, connectClient, acceptClient);

    std::promise<void> aboutToReceive;
    auto aboutToReceiveFuture = aboutToReceive.get_future();

    std::thread t([&] {
        std::array<uint8_t, 10> buffer{};
        size_t receivedSize{};
        aboutToReceive.set_value();
        AssertNotConnected(connectClient.Receive(buffer.data(), buffer.size(), receivedSize));
        ASSERT_EQ(0, receivedSize);
    });

    ASSERT_EQ(aboutToReceiveFuture.wait_for(1s), std::future_status::ready);

    // Act and assert
    acceptClient.Disconnect();

    // Cleanup
    t.join();
}

TEST_F(TestLocalSocket, WakeUpBlockingCallInConnectClientOnLocalClient) {
    // Arrange
    std::string name = GenerateLocalSocketName();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(name, connectClient, acceptClient);

    std::promise<void> aboutToReceive;
    auto aboutToReceiveFuture = aboutToReceive.get_future();

    std::thread t([&] {
        std::array<uint8_t, 10> buffer{};
        size_t receivedSize{};
        aboutToReceive.set_value();
        AssertNotConnected(connectClient.Receive(buffer.data(), buffer.size(), receivedSize));
        ASSERT_EQ(0, receivedSize);
    });

    ASSERT_EQ(aboutToReceiveFuture.wait_for(1s), std::future_status::ready);

    // Act and assert
    connectClient.Disconnect();

    // Cleanup
    t.join();
}

TEST_F(TestLocalSocket, SendOnConnectClientAndReceiveOnAcceptClientShouldWork) {
    // Arrange
    std::string name = GenerateLocalSocketName();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(name, connectClient, acceptClient);

    // Act and assert
    TestSendAndReceive(connectClient, acceptClient);
}

TEST_F(TestLocalSocket, PingPongBeginningWithConnectClientShouldWork) {
    // Arrange
    std::string name = GenerateLocalSocketName();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(name, connectClient, acceptClient);

    // Act and assert
    TestPingPong(connectClient, acceptClient);
}

TEST_F(TestLocalSocket, SendManyElementsFromConnectClientToAcceptClientShouldWork) {
    // Arrange
    std::string name = GenerateLocalSocketName();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(name, connectClient, acceptClient);

    // Act and assert
    TestManyElements(connectClient, acceptClient);
}

TEST_F(TestLocalSocket, SendBigElementFromConnectClientToAcceptClientShouldWork) {
    // Arrange
    std::string name = GenerateLocalSocketName();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(name, connectClient, acceptClient);

    // Act and assert
    TestBigElement(connectClient, acceptClient);
}

TEST_F(TestLocalSocket, SendOnDisconnectedConnectClientShouldNotWork) {
    // Arrange
    std::string name = GenerateLocalSocketName();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(name, connectClient, acceptClient);

    // Act and assert
    TestSendAfterDisconnect(connectClient);
}

TEST_F(TestLocalSocket, ReceiveOnDisconnectedConnectClientShouldNotWork) {
    // Arrange
    std::string name = GenerateLocalSocketName();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(name, connectClient, acceptClient);

    // Act and assert
    TestReceiveAfterDisconnect(connectClient);
}

TEST_F(TestLocalSocket, ReceiveOnDisconnectedRemoteConnectClientShouldNotWork) {
    // Arrange
    std::string name = GenerateLocalSocketName();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(name, connectClient, acceptClient);

    // Act and assert
    TestReceiveAfterDisconnectOnRemoteClient(connectClient, acceptClient);
}

}  // namespace
