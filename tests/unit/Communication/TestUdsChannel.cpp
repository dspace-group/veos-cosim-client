// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "Channel.h"
#include "Helper.h"
#include "TestHelper.h"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] std::string GenerateName() {
    return GenerateString("UdsChannel");
}

[[nodiscard]] std::unique_ptr<ChannelServer> CreateServer(const std::string& name) {
    std::unique_ptr<ChannelServer> server;
    ExpectOk(CreateUdsChannelServer(name, server));
    ExpectTrue(server);

    return server;
}

[[nodiscard]] std::unique_ptr<Channel> ConnectToServer(const std::string& name) {
    std::unique_ptr<Channel> connectedChannel;
    ExpectOk(TryConnectToUdsChannel(name, connectedChannel));
    ExpectTrue(connectedChannel);

    return connectedChannel;
}

class TestUdsChannel : public testing::Test {};

TEST_F(TestUdsChannel, StartServer) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server;

    // Act
    AssertOk(CreateUdsChannelServer(name, server));

    // Assert
    AssertTrue(server);
}

TEST_F(TestUdsChannel, ConnectWithoutStart) {
    // Arrange
    std::string name = GenerateName();

    {
        std::unique_ptr<ChannelServer> server = CreateServer(name);
    }

    std::unique_ptr<Channel> connectedChannel;

    // Act
    AssertOk(TryConnectToUdsChannel(name, connectedChannel));

    // Assert
    AssertFalse(connectedChannel);
}

TEST_F(TestUdsChannel, Connect) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateServer(name);

    std::unique_ptr<Channel> connectedChannel;

    // Act
    AssertOk(TryConnectToUdsChannel(name, connectedChannel));

    // Assert
    AssertTrue(connectedChannel);
}

TEST_F(TestUdsChannel, AcceptWithoutConnect) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateServer(name);

    std::unique_ptr<Channel> acceptedChannel;

    // Act
    AssertOk(server->TryAccept(acceptedChannel));

    // Assert
    AssertFalse(acceptedChannel);
}

TEST_F(TestUdsChannel, Accept) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateServer(name);

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(name);

    std::unique_ptr<Channel> acceptedChannel;

    // Act
    AssertOk(server->TryAccept(acceptedChannel));

    // Assert
    AssertTrue(acceptedChannel);
}

TEST_F(TestUdsChannel, AcceptAfterDisconnect) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateServer(name);

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(name);

    // After disconnect, the server should still be able to accept it, because that is the nature of sockets
    connectedChannel->Disconnect();

    std::unique_ptr<Channel> acceptedChannel;

    // Act
    AssertOk(server->TryAccept(acceptedChannel));

    // Assert
    AssertTrue(acceptedChannel);
}

TEST_F(TestUdsChannel, WriteUInt16ToChannel) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateServer(name);

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(name);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestWriteUInt16ToChannel(connectedChannel);
}

TEST_F(TestUdsChannel, WriteUInt32ToChannel) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateServer(name);

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(name);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestWriteUInt32ToChannel(connectedChannel);
}

TEST_F(TestUdsChannel, WriteUInt64ToChannel) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateServer(name);

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(name);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestWriteUInt64ToChannel(connectedChannel);
}

TEST_F(TestUdsChannel, WriteBufferToChannel) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateServer(name);

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(name);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestWriteBufferToChannel(connectedChannel);
}

TEST_F(TestUdsChannel, ReadUInt16FromChannel) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateServer(name);

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(name);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestReadUInt16FromChannel(connectedChannel, acceptedChannel);
}

TEST_F(TestUdsChannel, ReadUInt32FromChannel) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateServer(name);

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(name);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestReadUInt32FromChannel(connectedChannel, acceptedChannel);
}

TEST_F(TestUdsChannel, ReadUInt64FromChannel) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateServer(name);

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(name);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestReadUInt64FromChannel(connectedChannel, acceptedChannel);
}

TEST_F(TestUdsChannel, ReadBufferFromChannel) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateServer(name);

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(name);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestReadBufferFromChannel(connectedChannel, acceptedChannel);
}

TEST_F(TestUdsChannel, PingPong) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateServer(name);

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(name);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestPingPong(connectedChannel, acceptedChannel);
}

TEST_F(TestUdsChannel, SendTwoFramesAtOnce) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateServer(name);

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(name);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestSendTwoFramesAtOnce(connectedChannel, acceptedChannel);
}

TEST_F(TestUdsChannel, Stream) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateServer(name);

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(name);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestStream(connectedChannel, acceptedChannel);
}

TEST_F(TestUdsChannel, SendAndReceiveBigElement) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateServer(name);

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(name);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestBigElement(connectedChannel, acceptedChannel);
}

}  // namespace
