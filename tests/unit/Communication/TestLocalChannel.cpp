// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "Channel.h"
#include "Helper.h"
#include "TestHelper.h"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] std::string GenerateName() {
    return GenerateString("Channel名前\xF0\x9F\x98\x80");
}

[[nodiscard]] std::unique_ptr<ChannelServer> CreateServer(const std::string& name) {
    std::unique_ptr<ChannelServer> server;
    ExpectOk(CreateLocalChannelServer(name, server));
    ExpectTrue(server);

    return std::move(server);
}

[[nodiscard]] std::unique_ptr<Channel> ConnectToServer(const std::string& name) {
    std::unique_ptr<Channel> connectedChannel;
    ExpectOk(TryConnectToLocalChannel(name, connectedChannel));
    ExpectTrue(connectedChannel);

    return std::move(connectedChannel);
}

class TestLocalChannel : public testing::Test {};

TEST_F(TestLocalChannel, StartServer) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server;

    // Act
    AssertOk(CreateLocalChannelServer(name, server));

    // Assert
    AssertTrue(server);
}

TEST_F(TestLocalChannel, ConnectWithoutStart) {
    // Arrange
    std::string name = GenerateName();

    {
        std::unique_ptr<ChannelServer> server = CreateServer(name);
    }

    std::unique_ptr<Channel> connectedChannel;

    // Act
    AssertOk(TryConnectToLocalChannel(name, connectedChannel));

    // Assert
    AssertFalse(connectedChannel);
}

TEST_F(TestLocalChannel, Connect) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateServer(name);

    std::unique_ptr<Channel> connectedChannel;

    // Act
    AssertOk(TryConnectToLocalChannel(name, connectedChannel));

    // Assert
    AssertTrue(connectedChannel);
}

TEST_F(TestLocalChannel, AcceptWithoutConnect) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateServer(name);

    std::unique_ptr<Channel> acceptedChannel;

    // Act
    AssertOk(server->TryAccept(acceptedChannel));

    // Assert
    AssertFalse(acceptedChannel);
}

TEST_F(TestLocalChannel, Accept) {
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

TEST_F(TestLocalChannel, AcceptAfterDisconnect) {
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

TEST_F(TestLocalChannel, WriteUInt16ToChannel) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateServer(name);

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(name);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestWriteUInt16ToChannel(connectedChannel);
}

TEST_F(TestLocalChannel, WriteUInt32ToChannel) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateServer(name);

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(name);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestWriteUInt32ToChannel(connectedChannel);
}

TEST_F(TestLocalChannel, WriteUInt64ToChannel) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateServer(name);

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(name);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestWriteUInt64ToChannel(connectedChannel);
}

TEST_F(TestLocalChannel, WriteBufferToChannel) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateServer(name);

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(name);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestWriteBufferToChannel(connectedChannel);
}

TEST_F(TestLocalChannel, ReadUInt16FromChannel) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateServer(name);

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(name);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestReadUInt16FromChannel(connectedChannel, acceptedChannel);
}

TEST_F(TestLocalChannel, ReadUInt32FromChannel) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateServer(name);

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(name);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestReadUInt32FromChannel(connectedChannel, acceptedChannel);
}

TEST_F(TestLocalChannel, ReadUInt64FromChannel) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateServer(name);

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(name);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestReadUInt64FromChannel(connectedChannel, acceptedChannel);
}

TEST_F(TestLocalChannel, ReadBufferFromChannel) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateServer(name);

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(name);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestReadBufferFromChannel(connectedChannel, acceptedChannel);
}

TEST_F(TestLocalChannel, PingPong) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateServer(name);

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(name);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestPingPong(connectedChannel, acceptedChannel);
}

TEST_F(TestLocalChannel, SendTwoFramesAtOnce) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateServer(name);

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(name);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestSendTwoFramesAtOnce(connectedChannel, acceptedChannel);
}

TEST_F(TestLocalChannel, Stream) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateServer(name);

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(name);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestStream(connectedChannel, acceptedChannel);
}

TEST_F(TestLocalChannel, SendAndReceiveBigElement) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateServer(name);

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(name);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestBigElement(connectedChannel, acceptedChannel);
}

}  // namespace

#endif
