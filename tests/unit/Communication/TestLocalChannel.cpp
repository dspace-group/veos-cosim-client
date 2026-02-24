// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "Channel.hpp"
#include "Helper.hpp"
#include "TestHelper.hpp"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] std::string GenerateName() {
    return GenerateString("LocalChannel");
}

void EstablishConnection(const std::string& name, std::unique_ptr<Channel>& connectChannel, std::unique_ptr<Channel>& acceptChannel) {
    std::unique_ptr<ChannelServer> server;
    AssertOk(CreateLocalChannelServer(name, server));

    AssertOk(TryConnectToLocalChannel(name, connectChannel));

    AssertOk(server->TryAccept(acceptChannel));
}

class TestLocalChannel : public testing::Test {};

TEST_F(TestLocalChannel, StartServer) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server;

    // Act
    Result result = CreateLocalChannelServer(name, server);

    // Assert
    AssertOk(result);
}

TEST_F(TestLocalChannel, ConnectWithoutStart) {
    // Arrange
    std::string name = GenerateName();

    {
        std::unique_ptr<ChannelServer> server;
        AssertOk(CreateLocalChannelServer(name, server));
    }

    std::unique_ptr<Channel> connectChannel;

    // Act
    Result result = TryConnectToLocalChannel(name, connectChannel);

    // Assert
    AssertNotConnected(result);
}

TEST_F(TestLocalChannel, Connect) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server;
    AssertOk(CreateLocalChannelServer(name, server));

    std::unique_ptr<Channel> connectChannel;

    // Act
    Result result = TryConnectToLocalChannel(name, connectChannel);

    // Assert
    AssertOk(result);
}

TEST_F(TestLocalChannel, AcceptWithoutConnect) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server;
    AssertOk(CreateLocalChannelServer(name, server));

    std::unique_ptr<Channel> acceptChannel;

    // Act
    Result result = server->TryAccept(acceptChannel);

    // Assert
    AssertNotConnected(result);
}

TEST_F(TestLocalChannel, Accept) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server;
    AssertOk(CreateLocalChannelServer(name, server));

    std::unique_ptr<Channel> connectChannel;
    AssertOk(TryConnectToLocalChannel(name, connectChannel));

    std::unique_ptr<Channel> acceptChannel;

    // Act
    Result result = server->TryAccept(acceptChannel);

    // Assert
    AssertOk(result);
}

// After disconnect, the server should still be able to accept it
TEST_F(TestLocalChannel, AcceptAfterDisconnect) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server;
    AssertOk(CreateLocalChannelServer(name, server));

    std::unique_ptr<Channel> connectChannel;
    AssertOk(TryConnectToLocalChannel(name, connectChannel));

    connectChannel->Disconnect();

    std::unique_ptr<Channel> acceptChannel;

    // Act
    Result result = server->TryAccept(acceptChannel);

    // Assert
    AssertOk(result);
}

TEST_F(TestLocalChannel, WriteUInt16ToChannel) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(name, connectChannel, acceptChannel);

    // Act and assert
    TestWriteUInt16ToChannel(connectChannel);
}

TEST_F(TestLocalChannel, WriteUInt32ToChannel) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(name, connectChannel, acceptChannel);

    // Act and assert
    TestWriteUInt32ToChannel(connectChannel);
}

TEST_F(TestLocalChannel, WriteUInt64ToChannel) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(name, connectChannel, acceptChannel);

    // Act and assert
    TestWriteUInt64ToChannel(connectChannel);
}

TEST_F(TestLocalChannel, WriteBufferToChannel) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(name, connectChannel, acceptChannel);

    // Act and assert
    TestWriteBufferToChannel(connectChannel);
}

TEST_F(TestLocalChannel, ReadUInt16FromChannel) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(name, connectChannel, acceptChannel);

    // Act and assert
    TestReadUInt16FromChannel(connectChannel, acceptChannel);
}

TEST_F(TestLocalChannel, ReadUInt32FromChannel) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(name, connectChannel, acceptChannel);

    // Act and assert
    TestReadUInt32FromChannel(connectChannel, acceptChannel);
}

TEST_F(TestLocalChannel, ReadUInt64FromChannel) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(name, connectChannel, acceptChannel);

    // Act and assert
    TestReadUInt64FromChannel(connectChannel, acceptChannel);
}

TEST_F(TestLocalChannel, ReadBufferFromChannel) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(name, connectChannel, acceptChannel);

    // Act and assert
    TestReadBufferFromChannel(connectChannel, acceptChannel);
}

TEST_F(TestLocalChannel, PingPong) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(name, connectChannel, acceptChannel);

    // Act and assert
    TestPingPong(connectChannel, acceptChannel);
}

TEST_F(TestLocalChannel, SendTwoFramesAtOnce) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(name, connectChannel, acceptChannel);

    // Act and assert
    TestSendTwoFramesAtOnce(connectChannel, acceptChannel);
}

TEST_F(TestLocalChannel, Stream) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(name, connectChannel, acceptChannel);

    // Act and assert
    TestStream(connectChannel, acceptChannel);
}

TEST_F(TestLocalChannel, SendAndReceiveBigElement) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(name, connectChannel, acceptChannel);

    // Act and assert
    TestBigElement(connectChannel, acceptChannel);
}

}  // namespace
