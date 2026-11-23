// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <fmt/format.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "Channel.hpp"
#include "CheckedAssertions.hpp"
#include "Result.hpp"
#include "Socket.hpp"
#include "TestHelper.hpp"

using namespace DsVeosCoSim;

namespace {

struct TcpChannelParam {
    AddressFamily addressFamily{};
    std::vector<std::string> listenAddresses;
    std::string access;
};

[[maybe_unused]] void PrintTo(const TcpChannelParam& param, std::ostream* os)
{
    std::string name = fmt::format("{}_{}", param.addressFamily, param.access);
    *os << name;
}

[[nodiscard]] std::vector<TcpChannelParam> GetTcpChannelTestParameters()
{
    std::vector<TcpChannelParam> values;

    if (IsIpv4SocketSupported()) {
        values.push_back(TcpChannelParam{AddressFamily::Ipv4, {"0.0.0.0"}, "Any"});
        values.push_back(TcpChannelParam{AddressFamily::Ipv4, {"127.0.0.1"}, "Loopback"});
    }

    if (IsIpv6SocketSupported()) {
        values.push_back(TcpChannelParam{AddressFamily::Ipv6, {"::"}, "Any"});
        values.push_back(TcpChannelParam{AddressFamily::Ipv6, {"::1"}, "Loopback"});
    }

    return values;
}

void EstablishConnection(const TcpChannelParam& param, std::unique_ptr<Channel>& connectChannel, std::unique_ptr<Channel>& acceptChannel)
{
    const char* ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server;
    AssertOk(CreateTcpChannelServer(0, param.listenAddresses, server));
    uint16_t port = server->GetLocalPort();

    AssertOk(TryConnectToTcpChannel(ipAddress, port, 0, DefaultTimeoutInMilliseconds, connectChannel));

    AssertOk(server->TryAccept(acceptChannel));
}

class TestTcpChannel : public testing::TestWithParam<TcpChannelParam> {};

INSTANTIATE_TEST_SUITE_P(, TestTcpChannel, testing::ValuesIn(GetTcpChannelTestParameters()), [](const testing::TestParamInfo<TestTcpChannel::ParamType>& info) {
    return fmt::format("{}_{}", info.param.addressFamily, info.param.access);
});

TEST_F(TestTcpChannel, StartServerWithoutListenAddressesShouldNotListen)
{
    // Arrange
    std::unique_ptr<ChannelServer> server;
    AssertOk(CreateTcpChannelServer(0, {}, server));

    std::unique_ptr<Channel> acceptChannel;

    // Act
    uint16_t port = server->GetLocalPort();
    Result result = server->TryAccept(acceptChannel);

    // Assert
    ASSERT_CHECKED_EQ(static_cast<uint16_t>(0), port);
    AssertNotConnected(result);
}

TEST_F(TestTcpChannel, StartServerWithInvalidListenAddressShouldFail)
{
    // Arrange
    std::unique_ptr<ChannelServer> server;

    // Act
    Result result = CreateTcpChannelServer(0, {"no-ip-address"}, server);

    // Assert
    AssertError(result);
}

TEST_F(TestTcpChannel, StartServerWithOverlappingListenAddressesShouldFail)
{
    if (!IsIpv4SocketSupported()) {
        GTEST_SKIP() << "IPv4 is required.";
    }

    for (const std::vector<std::string>& listenAddresses : std::vector<std::vector<std::string>>{{"127.0.0.1", "127.0.0.1"}, {"0.0.0.0", "127.0.0.1"}}) {
        // Arrange
        std::unique_ptr<ChannelServer> server;

        // Act
        Result result = CreateTcpChannelServer(0, listenAddresses, server);

        // Assert
        AssertError(result);
    }
}

TEST_F(TestTcpChannel, StartServer)
{
    // Arrange
    std::unique_ptr<ChannelServer> server;

    // Act
    Result result = CreateTcpChannelServer(0, {"127.0.0.1"}, server);

    // Assert
    AssertOk(result);
    ASSERT_TRUE(server);
}

TEST_F(TestTcpChannel, ServerStartWithZeroPort)
{
    // Arrange
    std::unique_ptr<ChannelServer> server;
    AssertOk(CreateTcpChannelServer(0, {"127.0.0.1"}, server));

    // Act
    uint16_t port = server->GetLocalPort();

    // Assert
    ASSERT_CHECKED_NE(static_cast<uint16_t>(0), port);
}

TEST_P(TestTcpChannel, ConnectWithoutStart)
{
    // Arrange
    const TcpChannelParam& param = GetParam();
    const char* ipAddress = GetLoopBackAddress(param.addressFamily);

    uint16_t port{};

    {
        std::unique_ptr<ChannelServer> server;
        AssertOk(CreateTcpChannelServer(0, param.listenAddresses, server));
        port = server->GetLocalPort();
    }

    std::unique_ptr<Channel> connectChannel;

    // Act
    Result result = TryConnectToTcpChannel(ipAddress, port, 0, 0, connectChannel);

    // Assert
    AssertNotConnected(result);
}

TEST_P(TestTcpChannel, Connect)
{
    // Arrange
    const TcpChannelParam& param = GetParam();
    const char* ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server;
    AssertOk(CreateTcpChannelServer(0, param.listenAddresses, server));
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectChannel;

    // Act
    Result result = TryConnectToTcpChannel(ipAddress, port, 0, DefaultTimeoutInMilliseconds, connectChannel);

    // Assert
    AssertOk(result);
    ASSERT_TRUE(connectChannel);
}

TEST_P(TestTcpChannel, AcceptWithoutConnect)
{
    // Arrange
    const TcpChannelParam& param = GetParam();

    std::unique_ptr<ChannelServer> server;
    AssertOk(CreateTcpChannelServer(0, param.listenAddresses, server));

    std::unique_ptr<Channel> acceptChannel;

    // Act
    Result result = server->TryAccept(acceptChannel);

    // Assert
    AssertNotConnected(result);
}

TEST_P(TestTcpChannel, Accept)
{
    // Arrange
    const TcpChannelParam& param = GetParam();
    const char* ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server;
    AssertOk(CreateTcpChannelServer(0, param.listenAddresses, server));
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

TEST_P(TestTcpChannel, AcceptedClientHasCorrectAddresses)
{
    // Arrange
    const TcpChannelParam& param = GetParam();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(param, connectChannel, acceptChannel);

    std::string acceptChannelRemoteAddress;

    // Act
    Result result = acceptChannel->GetRemoteAddress(acceptChannelRemoteAddress);

    // Assert
    AssertOk(result);
    ASSERT_CHECKED_NE(acceptChannelRemoteAddress, "");
}

TEST_P(TestTcpChannel, ConnectedClientHasCorrectAddresses)
{
    // Arrange
    const TcpChannelParam& param = GetParam();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(param, connectChannel, acceptChannel);

    std::string connectChannelRemoteAddress;

    // Act
    Result result = connectChannel->GetRemoteAddress(connectChannelRemoteAddress);

    // Assert
    AssertOk(result);
    ASSERT_CHECKED_NE(connectChannelRemoteAddress, "");
}

TEST_F(TestTcpChannel, ConnectClientUsingHostName)
{
    // Arrange
    std::unique_ptr<ChannelServer> server;
    AssertOk(CreateTcpChannelServer(0, {"127.0.0.1"}, server));
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectChannel;

    // Act
    Result result = TryConnectToTcpChannel("localhost", port, 0, DefaultTimeoutInMilliseconds, connectChannel);

    // Assert
    AssertOk(result);
    ASSERT_TRUE(connectChannel);
}

TEST_F(TestTcpChannel, AcceptClientWithHostName)
{
    // Arrange
    std::unique_ptr<ChannelServer> server;
    AssertOk(CreateTcpChannelServer(0, {"127.0.0.1"}, server));
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

TEST_P(TestTcpChannel, AcceptAfterDisconnect)
{
    // Arrange
    const TcpChannelParam& param = GetParam();
    const char* ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server;
    AssertOk(CreateTcpChannelServer(0, param.listenAddresses, server));
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

TEST_P(TestTcpChannel, ReadUInt16FromChannel)
{
    // Arrange
    const TcpChannelParam& param = GetParam();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(param, connectChannel, acceptChannel);

    // Act and assert
    TestReadUInt16FromChannel(connectChannel, acceptChannel);
}

TEST_P(TestTcpChannel, ReadUInt32FromChannel)
{
    // Arrange
    const TcpChannelParam& param = GetParam();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(param, connectChannel, acceptChannel);

    // Act and assert
    TestReadUInt32FromChannel(connectChannel, acceptChannel);
}

TEST_P(TestTcpChannel, ReadUInt64FromChannel)
{
    // Arrange
    const TcpChannelParam& param = GetParam();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(param, connectChannel, acceptChannel);

    // Act and assert
    TestReadUInt64FromChannel(connectChannel, acceptChannel);
}

TEST_P(TestTcpChannel, ReadBufferFromChannel)
{
    // Arrange
    const TcpChannelParam& param = GetParam();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(param, connectChannel, acceptChannel);

    // Act and assert
    TestReadBufferFromChannel(connectChannel, acceptChannel);
}

TEST_P(TestTcpChannel, PingPong)
{
    // Arrange
    const TcpChannelParam& param = GetParam();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(param, connectChannel, acceptChannel);

    // Act and assert
    TestPingPong(connectChannel, acceptChannel);
}

TEST_P(TestTcpChannel, SendTwoFramesAtOnce)
{
    // Arrange
    const TcpChannelParam& param = GetParam();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(param, connectChannel, acceptChannel);

    // Act and assert
    TestSendTwoFramesAtOnce(connectChannel, acceptChannel);
}

TEST_P(TestTcpChannel, Stream)
{
    // Arrange
    const TcpChannelParam& param = GetParam();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(param, connectChannel, acceptChannel);

    // Act and assert
    TestStream(connectChannel, acceptChannel);
}

TEST_P(TestTcpChannel, SendAndReceiveBigElement)
{
    // Arrange
    const TcpChannelParam& param = GetParam();

    std::unique_ptr<Channel> connectChannel;
    std::unique_ptr<Channel> acceptChannel;
    EstablishConnection(param, connectChannel, acceptChannel);

    // Act and assert
    TestBigElement(connectChannel, acceptChannel);
}

}  // namespace
