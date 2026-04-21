// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <fmt/format.h>

#include <gtest/gtest.h>

#include "Channel.hpp"
#include "Socket.hpp"
#include "TestHelper.hpp"

using namespace DsVeosCoSim;

namespace {

struct TcpChannelParam {
    AddressFamily addressFamily{};
    bool enableRemoteAccess{};
};

void PrintTo(const TcpChannelParam& param, std::ostream* os) {  // NOLINT
    std::string name = fmt::format("{}_{}", param.addressFamily, param.enableRemoteAccess ? "Remote" : "Local");
    *os << name;
}

[[nodiscard]] std::vector<TcpChannelParam> GetTcpChannelTestParameters() {
    std::vector<TcpChannelParam> values;

    if (IsIpv4SocketSupported()) {
        values.push_back(TcpChannelParam{AddressFamily::Ipv4, true});
        values.push_back(TcpChannelParam{AddressFamily::Ipv4, false});
    }

    if (IsIpv6SocketSupported()) {
        values.push_back(TcpChannelParam{AddressFamily::Ipv6, true});
        values.push_back(TcpChannelParam{AddressFamily::Ipv6, false});
    }

    return values;
}

void EstablishConnection(const TcpChannelParam& param, std::unique_ptr<Channel>& connectChannel, std::unique_ptr<Channel>& acceptChannel) {
    const char* ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server;
    AssertOk(CreateTcpChannelServer(0, param.enableRemoteAccess, server));
    uint16_t port = server->GetLocalPort();

    AssertOk(TryConnectToTcpChannel(ipAddress, port, 0, DefaultTimeoutInMilliseconds, connectChannel));

    AssertOk(server->TryAccept(acceptChannel));
}

class TestTcpChannel : public testing::TestWithParam<TcpChannelParam> {};

INSTANTIATE_TEST_SUITE_P(, TestTcpChannel, testing::ValuesIn(GetTcpChannelTestParameters()), [](const testing::TestParamInfo<TestTcpChannel::ParamType>& info) {
    std::string access = info.param.enableRemoteAccess ? "Remote" : "Local";
    return fmt::format("{}_{}", info.param.addressFamily, access);
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
    TcpChannelParam param = GetParam();
    const char* ipAddress = GetLoopBackAddress(param.addressFamily);

    uint16_t port{};

    {
        std::unique_ptr<ChannelServer> server;
        AssertOk(CreateTcpChannelServer(0, param.enableRemoteAccess, server));
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
    TcpChannelParam param = GetParam();
    const char* ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server;
    AssertOk(CreateTcpChannelServer(0, param.enableRemoteAccess, server));
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectChannel;

    // Act
    Result result = TryConnectToTcpChannel(ipAddress, port, 0, DefaultTimeoutInMilliseconds, connectChannel);

    // Assert
    AssertOk(result);
    ASSERT_TRUE(connectChannel);
}

TEST_P(TestTcpChannel, AcceptWithoutConnect) {
    // Arrange
    TcpChannelParam param = GetParam();

    std::unique_ptr<ChannelServer> server;
    AssertOk(CreateTcpChannelServer(0, param.enableRemoteAccess, server));

    std::unique_ptr<Channel> acceptChannel;

    // Act
    Result result = server->TryAccept(acceptChannel);

    // Assert
    AssertNotConnected(result);
}

TEST_P(TestTcpChannel, Accept) {
    // Arrange
    TcpChannelParam param = GetParam();
    const char* ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server;
    AssertOk(CreateTcpChannelServer(0, param.enableRemoteAccess, server));
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectChannel;
    AssertOk(TryConnectToTcpChannel(ipAddress, port, 0, DefaultTimeoutInMilliseconds, connectChannel));

    std::unique_ptr<Channel> acceptChannel;

    // Act
    Result result = server->TryAccept(acceptChannel);

    // Assert
    AssertOk(result);
    ASSERT_TRUE(acceptChannel);
}

TEST_P(TestTcpChannel, AcceptedClientHasCorrectAddresses) {
    // Arrange
    TcpChannelParam param = GetParam();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(param, connectChannel, acceptChannel);

    std::string acceptChannelRemoteAddress;

    // Act
    Result result = acceptChannel->GetRemoteAddress(acceptChannelRemoteAddress);

    // Assert
    AssertOk(result);
    ASSERT_NE(acceptChannelRemoteAddress, "");
}

TEST_P(TestTcpChannel, ConnectedClientHasCorrectAddresses) {
    // Arrange
    TcpChannelParam param = GetParam();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(param, connectChannel, acceptChannel);

    std::string connectChannelRemoteAddress;

    // Act
    Result result = connectChannel->GetRemoteAddress(connectChannelRemoteAddress);

    // Assert
    AssertOk(result);
    ASSERT_NE(connectChannelRemoteAddress, "");
}

TEST_F(TestTcpChannel, ConnectClientUsingHostName) {
    // Arrange
    std::unique_ptr<ChannelServer> server;
    AssertOk(CreateTcpChannelServer(0, true, server));
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectChannel;

    // Act
    Result result = TryConnectToTcpChannel("localhost", port, 0, DefaultTimeoutInMilliseconds, connectChannel);

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
    AssertOk(TryConnectToTcpChannel("localhost", port, 0, DefaultTimeoutInMilliseconds, connectChannel));

    std::unique_ptr<Channel> acceptChannel;

    // Act
    Result result = server->TryAccept(acceptChannel);

    // Assert
    AssertOk(result);
    ASSERT_TRUE(acceptChannel);
}

TEST_P(TestTcpChannel, AcceptAfterDisconnect) {
    // Arrange
    TcpChannelParam param = GetParam();
    const char* ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server;
    AssertOk(CreateTcpChannelServer(0, param.enableRemoteAccess, server));
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectChannel;
    AssertOk(TryConnectToTcpChannel(ipAddress, port, 0, DefaultTimeoutInMilliseconds, connectChannel));

    // After disconnect, the server should still be able to accept it, because that is the nature of sockets
    connectChannel->Disconnect();

    std::unique_ptr<Channel> acceptChannel;

    // Act
    Result result = server->TryAccept(acceptChannel);

    // Assert
    AssertOk(result);
    ASSERT_TRUE(acceptChannel);
}

TEST_P(TestTcpChannel, ReadUInt16FromChannel) {
    // Arrange
    TcpChannelParam param = GetParam();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(param, connectChannel, acceptChannel);

    // Act and assert
    TestReadUInt16FromChannel(connectChannel, acceptChannel);
}

TEST_P(TestTcpChannel, ReadUInt32FromChannel) {
    // Arrange
    TcpChannelParam param = GetParam();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(param, connectChannel, acceptChannel);

    // Act and assert
    TestReadUInt32FromChannel(connectChannel, acceptChannel);
}

TEST_P(TestTcpChannel, ReadUInt64FromChannel) {
    // Arrange
    TcpChannelParam param = GetParam();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(param, connectChannel, acceptChannel);

    // Act and assert
    TestReadUInt64FromChannel(connectChannel, acceptChannel);
}

TEST_P(TestTcpChannel, ReadBufferFromChannel) {
    // Arrange
    TcpChannelParam param = GetParam();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(param, connectChannel, acceptChannel);

    // Act and assert
    TestReadBufferFromChannel(connectChannel, acceptChannel);
}

TEST_P(TestTcpChannel, PingPong) {
    // Arrange
    TcpChannelParam param = GetParam();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(param, connectChannel, acceptChannel);

    // Act and assert
    TestPingPong(connectChannel, acceptChannel);
}

TEST_P(TestTcpChannel, SendTwoFramesAtOnce) {
    // Arrange
    TcpChannelParam param = GetParam();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(param, connectChannel, acceptChannel);

    // Act and assert
    TestSendTwoFramesAtOnce(connectChannel, acceptChannel);
}

TEST_P(TestTcpChannel, Stream) {
    // Arrange
    TcpChannelParam param = GetParam();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(param, connectChannel, acceptChannel);

    // Act and assert
    TestStream(connectChannel, acceptChannel);
}

TEST_P(TestTcpChannel, SendAndReceiveBigElement) {
    // Arrange
    TcpChannelParam param = GetParam();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(param, connectChannel, acceptChannel);

    // Act and assert
    TestBigElement(connectChannel, acceptChannel);
}

}  // namespace
