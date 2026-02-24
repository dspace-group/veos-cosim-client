// Copyright dSPACE SE & Co. KG. All rights reserved.

#ifdef _WIN32

#include <gtest/gtest.h>

#include <string>

#include "Helper.hpp"
#include "OsUtilities.hpp"
#include "TestHelper.hpp"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] std::string GenerateName() {
    return GenerateString("ShmPipe");
}

void EstablishConnection(ShmPipeClient& connectClient, ShmPipeClient& acceptClient) {
    std::string name = GenerateName();

    ShmPipeListener listener;
    AssertOk(ShmPipeListener::Create(name, listener));

    AssertOk(ShmPipeClient::TryConnect(name, connectClient));

    AssertOk(listener.TryAccept(acceptClient));
}

class TestShmPipe : public testing::Test {};

TEST_F(TestShmPipe, CreateListenerShouldWork) {
    // Arrange
    std::string name = GenerateName();

    ShmPipeListener listener;

    // Act
    Result result = ShmPipeListener::Create(name, listener);

    // Assert
    AssertOk(result);
}

TEST_F(TestShmPipe, ConnectToListeningSocketShouldWork) {
    // Arrange
    std::string name = GenerateName();

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
    std::string name = GenerateName();

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
    std::string name = GenerateName();

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
    std::string name = GenerateName();

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
    std::string name = GenerateName();

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

TEST_F(TestShmPipe, PingPongBeginningWithAcceptClientShouldWork) {
    // Arrange
    ShmPipeClient connectClient;
    ShmPipeClient acceptClient;
    EstablishConnection(connectClient, acceptClient);

    // Act and assert
    TestPingPong(acceptClient, connectClient);
}

TEST_F(TestShmPipe, SendManyElementsFromConnectClientToAcceptClientShouldWork) {
    // Arrange
    ShmPipeClient connectClient;
    ShmPipeClient acceptClient;
    EstablishConnection(connectClient, acceptClient);

    // Act and assert
    TestManyElements(connectClient, acceptClient);
}

TEST_F(TestShmPipe, SendManyElementsFromAcceptClientToConnectClientShouldWork) {
    // Arrange
    ShmPipeClient connectClient;
    ShmPipeClient acceptClient;
    EstablishConnection(connectClient, acceptClient);

    // Act and assert
    TestManyElements(acceptClient, connectClient);
}

TEST_F(TestShmPipe, SendBigElementFromConnectClientToAcceptClientShouldWork) {
    // Arrange
    ShmPipeClient connectClient;
    ShmPipeClient acceptClient;
    EstablishConnection(connectClient, acceptClient);

    // Act and assert
    TestBigElement(connectClient, acceptClient);
}

TEST_F(TestShmPipe, SendBigElementFromAcceptClientToConnectClientShouldWork) {
    // Arrange
    ShmPipeClient connectClient;
    ShmPipeClient acceptClient;
    EstablishConnection(connectClient, acceptClient);

    // Act and assert
    TestBigElement(acceptClient, connectClient);
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

}  // namespace

#endif
