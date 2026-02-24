// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "Channel.hpp"
#include "Helper.hpp"
#include "Socket.hpp"
#include "TestHelper.hpp"

using namespace DsVeosCoSim;

namespace {

struct Param {
    AddressFamily addressFamily{};
};

[[nodiscard]] std::vector<Param> GetValues() {
    std::vector<Param> values;

    if (IsIpv4SocketSupported()) {
        values.push_back(Param{AddressFamily::Ipv4});
    }

    if (IsIpv6SocketSupported()) {
        values.push_back(Param{AddressFamily::Ipv6});
    }

    return values;
}

void EstablishConnection(const Param& param, std::unique_ptr<Channel>& connectChannel, std::unique_ptr<Channel>& acceptChannel) {
    const char* ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server;
    AssertOk(CreateTcpChannelServer(0, true, server));
    uint16_t port = server->GetLocalPort();

    AssertOk(TryConnectToTcpChannel(ipAddress, port, 0, DefaultTimeout, connectChannel));

    AssertOk(server->TryAccept(acceptChannel));
}

class TestTcpChannel : public testing::TestWithParam<Param> {};

INSTANTIATE_TEST_SUITE_P(, TestTcpChannel, testing::ValuesIn(GetValues()), [](const testing::TestParamInfo<TestTcpChannel::ParamType>& info) {
    return fmt::format("{}", info.param.addressFamily);
});

TEST_F(TestTcpChannel, StartServer) {
    // Arrange
    std::unique_ptr<ChannelServer> server;

    // Act
    Result result = CreateTcpChannelServer(0, true, server);

    // Assert
    AssertOk(result);
    ASSERT_TRUE(server);
}

TEST_F(TestTcpChannel, ServerStartWithZeroPort) {
    // Arrange
    std::unique_ptr<ChannelServer> server;
    AssertOk(CreateTcpChannelServer(0, true, server));

    // Act
    uint16_t port = server->GetLocalPort();

    // Assert
    ASSERT_NE(static_cast<uint16_t>(0), port);
}

TEST_P(TestTcpChannel, ConnectWithoutStart) {
    // Arrange
    Param param = GetParam();
    const char* ipAddress = GetLoopBackAddress(param.addressFamily);

    uint16_t port{};

    {
        std::unique_ptr<ChannelServer> server;
        AssertOk(CreateTcpChannelServer(0, true, server));
        port = server->GetLocalPort();
    }

    std::unique_ptr<Channel> connectChannel;

    // Act
    Result result = TryConnectToTcpChannel(ipAddress, port, 0, 0, connectChannel);

    // Assert
    AssertNotConnected(result);
}

TEST_P(TestTcpChannel, Connect) {
    // Arrange
    Param param = GetParam();
    const char* ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server;
    AssertOk(CreateTcpChannelServer(0, true, server));
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectChannel;

    // Act
    Result result = TryConnectToTcpChannel(ipAddress, port, 0, DefaultTimeout, connectChannel);

    // Assert
    AssertOk(result);
    ASSERT_TRUE(connectChannel);
}

TEST_P(TestTcpChannel, AcceptWithoutConnect) {
    // Arrange
    std::unique_ptr<ChannelServer> server;
    AssertOk(CreateTcpChannelServer(0, true, server));

    std::unique_ptr<Channel> acceptChannel;

    // Act
    Result result = server->TryAccept(acceptChannel);

    // Assert
    AssertNotConnected(result);
}

TEST_P(TestTcpChannel, Accept) {
    // Arrange
    Param param = GetParam();
    const char* ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server;
    AssertOk(CreateTcpChannelServer(0, true, server));
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectChannel;
    AssertOk(TryConnectToTcpChannel(ipAddress, port, 0, DefaultTimeout, connectChannel));

    std::unique_ptr<Channel> acceptChannel;

    // Act
    Result result = server->TryAccept(acceptChannel);

    // Assert
    AssertOk(result);
    ASSERT_TRUE(acceptChannel);
}

TEST_P(TestTcpChannel, AcceptedClientHasCorrectAddresses) {
    // Arrange
    Param param = GetParam();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(param, connectChannel, acceptChannel);

    std::string acceptChannelRemoteAddress;

    // Act
    Result result = acceptChannel->GetRemoteAddress(acceptChannelRemoteAddress);

    // Assert
    AssertOk(result);
    ASSERT_STRNE(acceptChannelRemoteAddress.c_str(), "");
}

TEST_P(TestTcpChannel, ConnectedClientHasCorrectAddresses) {
    // Arrange
    Param param = GetParam();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(param, connectChannel, acceptChannel);

    std::string connectChannelRemoteAddress;

    // Act
    Result result = connectChannel->GetRemoteAddress(connectChannelRemoteAddress);

    // Assert
    AssertOk(result);
    ASSERT_STRNE(connectChannelRemoteAddress.c_str(), "");
}

TEST_F(TestTcpChannel, ConnectClientUsingHostName) {
    // Arrange
    std::unique_ptr<ChannelServer> server;
    AssertOk(CreateTcpChannelServer(0, true, server));
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectChannel;

    // Act
    Result result = TryConnectToTcpChannel("localhost", port, 0, DefaultTimeout, connectChannel);

    // Assert
    AssertOk(result);
    ASSERT_TRUE(connectChannel);
}

TEST_F(TestTcpChannel, AcceptClientWithHostName) {
    // Arrange
    std::unique_ptr<ChannelServer> server;
    AssertOk(CreateTcpChannelServer(0, true, server));
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectChannel;
    AssertOk(TryConnectToTcpChannel("localhost", port, 0, DefaultTimeout, connectChannel));

    std::unique_ptr<Channel> acceptChannel;

    // Act
    Result result = server->TryAccept(acceptChannel);

    // Assert
    AssertOk(result);
    ASSERT_TRUE(acceptChannel);
}

TEST_P(TestTcpChannel, AcceptAfterDisconnect) {
    // Arrange
    Param param = GetParam();
    const char* ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server;
    AssertOk(CreateTcpChannelServer(0, true, server));
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectChannel;
    AssertOk(TryConnectToTcpChannel(ipAddress, port, 0, DefaultTimeout, connectChannel));

    // After disconnect, the server should still be able to accept it, because that is the nature of sockets
    connectChannel->Disconnect();

    std::unique_ptr<Channel> acceptChannel;

    // Act
    Result result = server->TryAccept(acceptChannel);

    // Assert
    AssertOk(result);
    ASSERT_TRUE(acceptChannel);
}

TEST_P(TestTcpChannel, WriteUInt16ToChannel) {
    // Arrange
    Param param = GetParam();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(param, connectChannel, acceptChannel);

    // Act and assert
    TestWriteUInt16ToChannel(connectChannel);
}

TEST_P(TestTcpChannel, WriteUInt32ToChannel) {
    // Arrange
    Param param = GetParam();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(param, connectChannel, acceptChannel);

    // Act and assert
    TestWriteUInt32ToChannel(connectChannel);
}

TEST_P(TestTcpChannel, WriteUInt64ToChannel) {
    // Arrange
    Param param = GetParam();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(param, connectChannel, acceptChannel);

    // Act and assert
    TestWriteUInt64ToChannel(connectChannel);
}

TEST_P(TestTcpChannel, WriteBufferToChannel) {
    // Arrange
    Param param = GetParam();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(param, connectChannel, acceptChannel);

    // Act and assert
    TestWriteBufferToChannel(connectChannel);
}

TEST_P(TestTcpChannel, ReadUInt16FromChannel) {
    // Arrange
    Param param = GetParam();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(param, connectChannel, acceptChannel);

    // Act and assert
    TestReadUInt16FromChannel(connectChannel, acceptChannel);
}

TEST_P(TestTcpChannel, ReadUInt32FromChannel) {
    // Arrange
    Param param = GetParam();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(param, connectChannel, acceptChannel);

    // Act and assert
    TestReadUInt32FromChannel(connectChannel, acceptChannel);
}

TEST_P(TestTcpChannel, ReadUInt64FromChannel) {
    // Arrange
    Param param = GetParam();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(param, connectChannel, acceptChannel);

    // Act and assert
    TestReadUInt64FromChannel(connectChannel, acceptChannel);
}

TEST_P(TestTcpChannel, ReadBufferFromChannel) {
    // Arrange
    Param param = GetParam();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(param, connectChannel, acceptChannel);

    // Act and assert
    TestReadBufferFromChannel(connectChannel, acceptChannel);
}

TEST_P(TestTcpChannel, PingPong) {
    // Arrange
    Param param = GetParam();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(param, connectChannel, acceptChannel);

    // Act and assert
    TestPingPong(connectChannel, acceptChannel);
}

TEST_P(TestTcpChannel, SendTwoFramesAtOnce) {
    // Arrange
    Param param = GetParam();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(param, connectChannel, acceptChannel);

    // Act and assert
    TestSendTwoFramesAtOnce(connectChannel, acceptChannel);
}

TEST_P(TestTcpChannel, Stream) {
    // Arrange
    Param param = GetParam();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(param, connectChannel, acceptChannel);

    // Act and assert
    TestStream(connectChannel, acceptChannel);
}

TEST_P(TestTcpChannel, SendAndReceiveBigElement) {
    // Arrange
    Param param = GetParam();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(param, connectChannel, acceptChannel);

    // Act and assert
    TestBigElement(connectChannel, acceptChannel);
}

}  // namespace
