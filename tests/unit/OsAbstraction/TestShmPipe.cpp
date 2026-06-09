// Copyright dSPACE SE & Co. KG. All rights reserved.

#ifdef _WIN32

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <future>
#include <string>
#include <thread>

#include <gtest/gtest.h>

#include <OsUtilities.hpp>
#include <Result.hpp>

#include "Helper.hpp"
#include "TestHelper.hpp"

using namespace std::chrono_literals;
using namespace DsVeosCoSim;

namespace {

[[nodiscard]] std::string GenerateShmPipeName() {
    return GenerateString("ShmPipe");
}

void EstablishConnection(ShmPipeClient& connectClient, ShmPipeClient& acceptClient) {
    std::string name = GenerateShmPipeName();

    ShmPipeListener listener;
    AssertOk(ShmPipeListener::Create(name, listener));

    AssertOk(ShmPipeClient::TryConnect(name, connectClient));

    AssertOk(listener.TryAccept(acceptClient));
}

class TestShmPipe : public testing::Test {};

TEST_F(TestShmPipe, IsRunningAfterCreateShouldBeTrue) {
    // Arrange
    std::string name = GenerateShmPipeName();

    ShmPipeListener listener;
    AssertOk(ShmPipeListener::Create(name, listener));

    // Act
    bool isRunning = listener.IsRunning();

    // Assert
    ASSERT_TRUE(isRunning);
}

TEST_F(TestShmPipe, IsNotRunningAfterStopShouldBeFalse) {
    // Arrange
    std::string name = GenerateShmPipeName();

    ShmPipeListener listener;
    AssertOk(ShmPipeListener::Create(name, listener));

    listener.Stop();

    // Act
    bool isRunning = listener.IsRunning();

    // Assert
    ASSERT_FALSE(isRunning);
}

TEST_F(TestShmPipe, CreateListenerShouldWork) {
    // Arrange
    std::string name = GenerateShmPipeName();

    ShmPipeListener listener;

    // Act
    Result result = ShmPipeListener::Create(name, listener);

    // Assert
    AssertOk(result);
}

TEST_F(TestShmPipe, ConnectToListeningSocketShouldWork) {
    // Arrange
    std::string name = GenerateShmPipeName();

    ShmPipeListener listener;
    AssertOk(ShmPipeListener::Create(name, listener));

    ShmPipeClient client;

    // Act
    Result result = ShmPipeClient::TryConnect(name, client);

    // Assert
    AssertOk(result);
}

TEST_F(TestShmPipe, ConnectWithoutListeningShouldNotWork) {
    // Arrange
    std::string name = GenerateShmPipeName();

    {
        ShmPipeListener listener;
        AssertOk(ShmPipeListener::Create(name, listener));
    }

    ShmPipeClient client;

    // Act
    Result result = ShmPipeClient::TryConnect(name, client);

    // Assert
    AssertNotConnected(result);
}

TEST_F(TestShmPipe, AcceptWithoutConnectShouldNotWork) {
    // Arrange
    std::string name = GenerateShmPipeName();

    ShmPipeListener listener;
    AssertOk(ShmPipeListener::Create(name, listener));

    ShmPipeClient client;

    // Act
    Result result = listener.TryAccept(client);

    // Assert
    AssertNotConnected(result);
}

TEST_F(TestShmPipe, AcceptAfterStopShouldNotWork) {
    // Arrange
    std::string name = GenerateShmPipeName();

    ShmPipeListener listener;
    AssertOk(ShmPipeListener::Create(name, listener));

    listener.Stop();

    ShmPipeClient client;

    // Act
    Result result = listener.TryAccept(client);

    // Assert
    AssertError(result);
}

TEST_F(TestShmPipe, AcceptWithConnectShouldWork) {
    // Arrange
    std::string name = GenerateShmPipeName();

    ShmPipeListener listener;
    AssertOk(ShmPipeListener::Create(name, listener));

    ShmPipeClient connectClient;
    AssertOk(ShmPipeClient::TryConnect(name, connectClient));

    ShmPipeClient acceptClient;

    // Act
    Result result = listener.TryAccept(acceptClient);

    // Assert
    AssertOk(result);
}

// After disconnect, the listener should still be able to accept
TEST_F(TestShmPipe, AcceptAfterDisconnectShouldWork) {
    // Arrange
    std::string name = GenerateShmPipeName();

    ShmPipeListener listener;
    AssertOk(ShmPipeListener::Create(name, listener));

    ShmPipeClient connectClient;
    AssertOk(ShmPipeClient::TryConnect(name, connectClient));

    connectClient.Disconnect();

    ShmPipeClient acceptClient;

    // Act
    Result result = listener.TryAccept(acceptClient);

    // Assert
    AssertOk(result);
}

TEST_F(TestShmPipe, IsConnectedAfterConnectAndAcceptShouldBeTrue) {
    // Arrange
    ShmPipeClient connectClient;
    ShmPipeClient acceptClient;
    EstablishConnection(connectClient, acceptClient);

    // Act and assert
    ASSERT_TRUE(connectClient.IsConnected());
    ASSERT_TRUE(acceptClient.IsConnected());
}

TEST_F(TestShmPipe, IsNotConnectedAfterDisconnectShouldBeFalse) {
    // Arrange
    ShmPipeClient connectClient;
    ShmPipeClient acceptClient;
    EstablishConnection(connectClient, acceptClient);

    connectClient.Disconnect();

    // Act
    bool isConnected = connectClient.IsConnected();

    // Assert
    ASSERT_FALSE(isConnected);
}

TEST_F(TestShmPipe, IsNotConnectedAfterDisconnectShouldBeFalseForAcceptClient) {
    // Arrange
    ShmPipeClient connectClient;
    ShmPipeClient acceptClient;
    EstablishConnection(connectClient, acceptClient);

    acceptClient.Disconnect();

    // Act
    bool isConnected = acceptClient.IsConnected();

    // Assert
    ASSERT_FALSE(isConnected);
}

TEST_F(TestShmPipe, WakeUpBlockingCallInConnectClientOnRemoteClient) {
    // Arrange
    ShmPipeClient connectClient;
    ShmPipeClient acceptClient;
    EstablishConnection(connectClient, acceptClient);

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

TEST_F(TestShmPipe, WakeUpBlockingCallInConnectClientOnLocalClient) {
    // Arrange
    ShmPipeClient connectClient;
    ShmPipeClient acceptClient;
    EstablishConnection(connectClient, acceptClient);

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

TEST_F(TestShmPipe, WakeUpBlockingCallInAcceptClientOnRemoteClient) {
    // Arrange
    ShmPipeClient connectClient;
    ShmPipeClient acceptClient;
    EstablishConnection(connectClient, acceptClient);

    std::promise<void> aboutToReceive;
    auto aboutToReceiveFuture = aboutToReceive.get_future();

    std::thread t([&] {
        std::array<uint8_t, 10> buffer{};
        size_t receivedSize{};
        aboutToReceive.set_value();
        AssertNotConnected(acceptClient.Receive(buffer.data(), buffer.size(), receivedSize));
        ASSERT_EQ(0, receivedSize);
    });

    ASSERT_EQ(aboutToReceiveFuture.wait_for(1s), std::future_status::ready);

    // Act and assert
    connectClient.Disconnect();

    // Cleanup
    t.join();
}

TEST_F(TestShmPipe, WakeUpBlockingCallInAcceptClientOnLocalClient) {
    // Arrange
    ShmPipeClient connectClient;
    ShmPipeClient acceptClient;
    EstablishConnection(connectClient, acceptClient);

    std::promise<void> aboutToReceive;
    auto aboutToReceiveFuture = aboutToReceive.get_future();

    std::thread t([&] {
        std::array<uint8_t, 10> buffer{};
        size_t receivedSize{};
        aboutToReceive.set_value();
        AssertNotConnected(acceptClient.Receive(buffer.data(), buffer.size(), receivedSize));
        ASSERT_EQ(0, receivedSize);
    });

    ASSERT_EQ(aboutToReceiveFuture.wait_for(1s), std::future_status::ready);

    // Act and assert
    acceptClient.Disconnect();

    // Cleanup
    t.join();
}

TEST_F(TestShmPipe, SendOnConnectClientAndReceiveOnAcceptClientShouldWork) {
    // Arrange
    ShmPipeClient connectClient;
    ShmPipeClient acceptClient;
    EstablishConnection(connectClient, acceptClient);

    // Act and assert
    TestSendAndReceive(connectClient, acceptClient);
}

TEST_F(TestShmPipe, SendOnAcceptClientAndReceiveOnConnectClientShouldWork) {
    // Arrange
    ShmPipeClient connectClient;
    ShmPipeClient acceptClient;
    EstablishConnection(connectClient, acceptClient);

    // Act and assert
    TestSendAndReceive(acceptClient, connectClient);
}

TEST_F(TestShmPipe, PingPongBeginningWithConnectClientShouldWork) {
    // Arrange
    ShmPipeClient connectClient;
    ShmPipeClient acceptClient;
    EstablishConnection(connectClient, acceptClient);

    // Act and assert
    TestPingPong(connectClient, acceptClient);
}

TEST_F(TestShmPipe, SendManyElementsFromConnectClientToAcceptClientShouldWork) {
    // Arrange
    ShmPipeClient connectClient;
    ShmPipeClient acceptClient;
    EstablishConnection(connectClient, acceptClient);

    // Act and assert
    TestManyElements(connectClient, acceptClient);
}

TEST_F(TestShmPipe, SendBigElementFromConnectClientToAcceptClientShouldWork) {
    // Arrange
    ShmPipeClient connectClient;
    ShmPipeClient acceptClient;
    EstablishConnection(connectClient, acceptClient);

    // Act and assert
    TestBigElement(connectClient, acceptClient);
}

TEST_F(TestShmPipe, SendOnDisconnectedConnectClientShouldNotWork) {
    // Arrange
    ShmPipeClient connectClient;
    ShmPipeClient acceptClient;
    EstablishConnection(connectClient, acceptClient);

    // Act and assert
    TestSendAfterDisconnect(connectClient);
}

TEST_F(TestShmPipe, SendOnDisconnectedAcceptClientShouldNotWork) {
    // Arrange
    ShmPipeClient connectClient;
    ShmPipeClient acceptClient;
    EstablishConnection(connectClient, acceptClient);

    // Act and assert
    TestSendAfterDisconnect(acceptClient);
}

TEST_F(TestShmPipe, SendOnDisconnectedRemoteConnectClientShouldNotWork) {
    // Arrange
    ShmPipeClient connectClient;
    ShmPipeClient acceptClient;
    EstablishConnection(connectClient, acceptClient);

    // Since the connection is established lazy, we have to send and receive at least one element per reader and writer
    TestSendAndReceive(acceptClient, connectClient);
    TestSendAndReceive(connectClient, acceptClient);

    // Act and assert
    TestSendAfterDisconnectOnRemoteClient(connectClient, acceptClient);
}

TEST_F(TestShmPipe, SendOnDisconnectedRemoteAcceptClientShouldNotWork) {
    // Arrange
    ShmPipeClient connectClient;
    ShmPipeClient acceptClient;
    EstablishConnection(connectClient, acceptClient);

    // Since the connection is established lazy, we have to send and receive at least one element per reader and writer
    TestSendAndReceive(acceptClient, connectClient);
    TestSendAndReceive(connectClient, acceptClient);

    // Act and assert
    TestSendAfterDisconnectOnRemoteClient(acceptClient, connectClient);
}

TEST_F(TestShmPipe, ReceiveOnDisconnectedConnectClientShouldNotWork) {
    // Arrange
    ShmPipeClient connectClient;
    ShmPipeClient acceptClient;
    EstablishConnection(connectClient, acceptClient);

    // Act and assert
    TestReceiveAfterDisconnect(connectClient);
}

TEST_F(TestShmPipe, ReceiveOnDisconnectedAcceptClientShouldNotWork) {
    // Arrange
    ShmPipeClient connectClient;
    ShmPipeClient acceptClient;
    EstablishConnection(connectClient, acceptClient);

    // Act and assert
    TestReceiveAfterDisconnect(acceptClient);
}

TEST_F(TestShmPipe, ReceiveOnDisconnectedRemoteConnectClientShouldNotWork) {
    // Arrange
    ShmPipeClient connectClient;
    ShmPipeClient acceptClient;
    EstablishConnection(connectClient, acceptClient);

    // Since the connection is established lazy, we have to send and receive at least one element per reader and writer
    TestSendAndReceive(acceptClient, connectClient);
    TestSendAndReceive(connectClient, acceptClient);

    // Act and assert
    TestReceiveAfterDisconnectOnRemoteClient(connectClient, acceptClient);
}

TEST_F(TestShmPipe, ReceiveOnDisconnectedRemoteAcceptClientShouldNotWork) {
    // Arrange
    ShmPipeClient connectClient;
    ShmPipeClient acceptClient;
    EstablishConnection(connectClient, acceptClient);

    // Since the connection is established lazy, we have to send and receive at least one element per reader and writer
    TestSendAndReceive(acceptClient, connectClient);
    TestSendAndReceive(connectClient, acceptClient);

    // Act and assert
    TestReceiveAfterDisconnectOnRemoteClient(acceptClient, connectClient);
}

TEST_F(TestShmPipe, WaitForDataTimesOutWhenNoDataArrives) {
    // Arrange
    ShmPipeClient connectClient;
    ShmPipeClient acceptClient;
    EstablishConnection(connectClient, acceptClient);

    // Act
    Result result = connectClient.WaitForData(50);

    // Assert
    AssertTimeout(result);
}

}  // namespace

#endif
