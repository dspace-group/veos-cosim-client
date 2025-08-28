// Copyright dSPACE GmbH. All rights reserved.

#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "Channel.h"
#include "Helper.h"
#include "Socket.h"
#include "TestHelper.h"

using namespace DsVeosCoSim;

namespace {

struct Param {
    AddressFamily addressFamily{};
};

[[nodiscard]] std::vector<Param> GetValues() {
    std::vector<Param> values;

    if (Socket::IsIpv4Supported()) {
        values.push_back(Param{AddressFamily::Ipv4});
    }

    if (Socket::IsIpv6Supported()) {
        values.push_back(Param{AddressFamily::Ipv6});
    }

    return values;
}

[[nodiscard]] std::unique_ptr<ChannelServer> CreateServer() {
    std::unique_ptr<ChannelServer> server;
    ExpectOk(CreateTcpChannelServer(0, true, server));
    ExpectTrue(server);

    return server;
}

[[nodiscard]] std::unique_ptr<Channel> ConnectToServer(const char* ipAddress, uint16_t port) {
    std::unique_ptr<Channel> connectedChannel;
    ExpectOk(TryConnectToTcpChannel(ipAddress, port, 0, DefaultTimeout, connectedChannel));
    ExpectTrue(connectedChannel);

    return connectedChannel;
}

class TestTcpChannel : public testing::TestWithParam<Param> {};

INSTANTIATE_TEST_SUITE_P(,
                         TestTcpChannel,
                         testing::ValuesIn(GetValues()),
                         [](const testing::TestParamInfo<TestTcpChannel::ParamType>& info) {
                             return std::string(ToString(info.param.addressFamily));
                         });

TEST_F(TestTcpChannel, StartServer) {
    // Arrange
    std::unique_ptr<ChannelServer> server;

    // Act
    AssertOk(CreateTcpChannelServer(0, true, server));

    // Assert
    AssertTrue(server);
}

TEST_F(TestTcpChannel, ServerStartWithZeroPort) {
    // Arrange
    std::unique_ptr<ChannelServer> server = CreateServer();

    // Act
    uint16_t port = server->GetLocalPort();

    // Assert
    AssertNotEq(static_cast<uint16_t>(0), port);
}

#ifdef _WIN32
TEST_P(TestTcpChannel, ConnectWithoutStart) {
    // Arrange
    Param param = GetParam();
    const char* ipAddress = GetLoopBackAddress(param.addressFamily);

    uint16_t port{};

    {
        std::unique_ptr<ChannelServer> server = CreateServer();
        port = server->GetLocalPort();
    }

    std::unique_ptr<Channel> connectedChannel;

    // Act
    AssertOk(TryConnectToTcpChannel(ipAddress, port, 0, 0, connectedChannel));

    // Assert
    AssertFalse(connectedChannel);
}
#endif

TEST_P(TestTcpChannel, Connect) {
    // Arrange
    Param param = GetParam();
    const char* ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server = CreateServer();
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectedChannel;

    // Act
    AssertOk(TryConnectToTcpChannel(ipAddress, port, 0, DefaultTimeout, connectedChannel));

    // Assert
    AssertTrue(connectedChannel);
}

TEST_P(TestTcpChannel, AcceptWithoutConnect) {
    // Arrange
    std::unique_ptr<ChannelServer> server = CreateServer();

    std::unique_ptr<Channel> acceptedChannel;

    // Act
    AssertOk(server->TryAccept(acceptedChannel));

    // Assert
    AssertFalse(acceptedChannel);
}

TEST_P(TestTcpChannel, Accept) {
    // Arrange
    Param param = GetParam();
    const char* ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server = CreateServer();
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(ipAddress, port);

    std::unique_ptr<Channel> acceptedChannel;

    // Act
    AssertOk(server->TryAccept(acceptedChannel));

    // Assert
    AssertTrue(acceptedChannel);
}

TEST_P(TestTcpChannel, AcceptedClientHasCorrectAddresses) {
    // Arrange
    Param param = GetParam();
    const char* ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server = CreateServer();
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(ipAddress, port);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    std::string connectedChannelRemoteAddress;
    std::string acceptedChannelRemoteAddress;

    // Act
    AssertOk(connectedChannel->GetRemoteAddress(connectedChannelRemoteAddress));
    AssertOk(acceptedChannel->GetRemoteAddress(acceptedChannelRemoteAddress));

    // Assert
    AssertNotEqHelper(connectedChannelRemoteAddress, acceptedChannelRemoteAddress);
}

TEST_F(TestTcpChannel, ConnectClientUsingHostName) {
    // Arrange
    std::unique_ptr<ChannelServer> server = CreateServer();
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectedChannel;

    // Act
    AssertOk(TryConnectToTcpChannel("localhost", port, 0, DefaultTimeout, connectedChannel));

    // Assert
    AssertTrue(connectedChannel);
}

TEST_F(TestTcpChannel, AcceptClientWithHostName) {
    // Arrange
    std::unique_ptr<ChannelServer> server = CreateServer();
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectedChannel = ConnectToServer("localhost", port);

    std::unique_ptr<Channel> acceptedChannel;

    // Act
    AssertOk(server->TryAccept(acceptedChannel));

    // Assert
    AssertTrue(acceptedChannel);
}

TEST_P(TestTcpChannel, AcceptAfterDisconnect) {
    // Arrange
    Param param = GetParam();
    const char* ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server = CreateServer();
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(ipAddress, port);

    // After disconnect, the server should still be able to accept it, because that is the nature of sockets
    connectedChannel->Disconnect();

    std::unique_ptr<Channel> acceptedChannel;

    // Act
    AssertOk(server->TryAccept(acceptedChannel));

    // Assert
    AssertTrue(acceptedChannel);
}

TEST_P(TestTcpChannel, WriteUInt16ToChannel) {
    // Arrange
    Param param = GetParam();
    const char* ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server = CreateServer();
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(ipAddress, port);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestWriteUInt16ToChannel(connectedChannel);
}

TEST_P(TestTcpChannel, WriteUInt32ToChannel) {
    // Arrange
    Param param = GetParam();
    const char* ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server = CreateServer();
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(ipAddress, port);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestWriteUInt32ToChannel(connectedChannel);
    AssertOk(connectedChannel->GetWriter().EndWrite());
}

TEST_P(TestTcpChannel, WriteUInt64ToChannel) {
    // Arrange
    Param param = GetParam();
    const char* ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server = CreateServer();
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(ipAddress, port);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestWriteUInt64ToChannel(connectedChannel);
    AssertOk(connectedChannel->GetWriter().EndWrite());
}

TEST_P(TestTcpChannel, WriteBufferToChannel) {
    // Arrange
    Param param = GetParam();
    const char* ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server = CreateServer();
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(ipAddress, port);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestWriteBufferToChannel(connectedChannel);
    AssertOk(connectedChannel->GetWriter().EndWrite());
}

TEST_P(TestTcpChannel, ReadUInt16FromChannel) {
    // Arrange
    Param param = GetParam();
    const char* ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server = CreateServer();
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(ipAddress, port);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestReadUInt16FromChannel(connectedChannel, acceptedChannel);
}

TEST_P(TestTcpChannel, ReadUInt32FromChannel) {
    // Arrange
    Param param = GetParam();
    const char* ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server = CreateServer();
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(ipAddress, port);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestReadUInt32FromChannel(connectedChannel, acceptedChannel);
}

TEST_P(TestTcpChannel, ReadUInt64FromChannel) {
    // Arrange
    Param param = GetParam();
    const char* ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server = CreateServer();
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(ipAddress, port);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestReadUInt64FromChannel(connectedChannel, acceptedChannel);
}

TEST_P(TestTcpChannel, ReadBufferFromChannel) {
    // Arrange
    Param param = GetParam();
    const char* ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server = CreateServer();
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(ipAddress, port);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestReadBufferFromChannel(connectedChannel, acceptedChannel);
}

TEST_P(TestTcpChannel, PingPong) {
    // Arrange
    Param param = GetParam();
    const char* ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server = CreateServer();
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(ipAddress, port);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestPingPong(connectedChannel, acceptedChannel);
}

TEST_P(TestTcpChannel, SendTwoFramesAtOnce) {
    // Arrange
    Param param = GetParam();
    const char* ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server = CreateServer();
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(ipAddress, port);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestSendTwoFramesAtOnce(connectedChannel, acceptedChannel);
}

TEST_P(TestTcpChannel, Stream) {
    // Arrange
    Param param = GetParam();
    const char* ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server = CreateServer();
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(ipAddress, port);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestStream(connectedChannel, acceptedChannel);
}

TEST_P(TestTcpChannel, SendAndReceiveBigElement) {
    // Arrange
    Param param = GetParam();
    const char* ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server = CreateServer();
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectedChannel = ConnectToServer(ipAddress, port);
    std::unique_ptr<Channel> acceptedChannel = AcceptFromServer(server);

    // Act and assert
    TestBigElement(connectedChannel, acceptedChannel);
}

}  // namespace
