// Copyright dSPACE GmbH. All rights reserved.

#include <gtest/gtest.h>
#include <array>
#include <thread>

#include "Generator.h"
#include "Helper.h"
#include "Socket.h"
#include "SocketChannel.h"

using namespace DsVeosCoSim;

namespace {

constexpr uint32_t BigNumber = 4 * 1024 * 1024;

struct Param {
    AddressFamily addressFamily{};
};

}  // namespace

class TestTcpChannel : public testing::TestWithParam<Param> {};

INSTANTIATE_TEST_SUITE_P(,
                         TestTcpChannel,
                         testing::Values(Param{AddressFamily::Ipv4}, Param{AddressFamily::Ipv6}),
                         [](const testing::TestParamInfo<TestTcpChannel::ParamType>& info) {
                             return format_as(info.param.addressFamily);
                         });

TEST_F(TestTcpChannel, StartServer) {
    // Arrange

    // Act and assert
    ASSERT_NO_THROW(TcpChannelServer(0, true));
}

TEST_F(TestTcpChannel, ServerStartWithZeroPort) {
    // Arrange
    TcpChannelServer server(0, true);

    // Act
    uint16_t port = server.GetLocalPort();

    // Assert
    ASSERT_NE(0, port);
}

TEST_P(TestTcpChannel, ConnectWithoutStart) {
    // Arrange
    Param param = GetParam();
    std::string ipAddress = GetLoopBackAddress(param.addressFamily);

    uint16_t port{};

    {
        TcpChannelServer server(0, true);
        port = server.GetLocalPort();
    }

    // Act
    std::optional<SocketChannel> connectedChannel = TryConnectToTcpChannel(ipAddress, port, 0, 0);

    // Assert
    ASSERT_FALSE(connectedChannel);
}

TEST_P(TestTcpChannel, Connect) {
    // Arrange
    Param param = GetParam();
    std::string ipAddress = GetLoopBackAddress(param.addressFamily);

    TcpChannelServer server(0, true);
    uint16_t port = server.GetLocalPort();

    // Act
    std::optional<SocketChannel> connectedChannel = TryConnectToTcpChannel(ipAddress, port, 0, DefaultTimeout);

    // Assert
    ASSERT_TRUE(connectedChannel);
}

TEST_P(TestTcpChannel, AcceptWithoutConnect) {
    // Arrange
    Param param = GetParam();
    std::string ipAddress = GetLoopBackAddress(param.addressFamily);

    TcpChannelServer server(0, true);

    // Act
    std::optional<SocketChannel> acceptedChannel = server.TryAccept();

    // Assert
    ASSERT_FALSE(acceptedChannel);
}

TEST_P(TestTcpChannel, Accept) {
    // Arrange
    Param param = GetParam();
    std::string ipAddress = GetLoopBackAddress(param.addressFamily);

    TcpChannelServer server(0, true);
    uint16_t port = server.GetLocalPort();

    (void)ConnectToTcpChannel(ipAddress, port);

    // Act
    std::optional<SocketChannel> acceptedChannel = server.TryAccept(DefaultTimeout);

    // Assert
    ASSERT_TRUE(acceptedChannel);
}

TEST_P(TestTcpChannel, AcceptedClientHasCorrectAddresses) {
    // Arrange
    Param param = GetParam();
    std::string ipAddress = GetLoopBackAddress(param.addressFamily);

    TcpChannelServer server(0, true);
    uint16_t port = server.GetLocalPort();

    SocketChannel connectedChannel = ConnectToTcpChannel(ipAddress, port);
    SocketChannel acceptedChannel = Accept(server);

    // Act
    SocketAddress connectedChannelRemoteAddress = connectedChannel.GetRemoteAddress();
    SocketAddress acceptedChannelRemoteAddress = acceptedChannel.GetRemoteAddress();

// Assert
#ifdef _WIN32
    ASSERT_STREQ(connectedChannelRemoteAddress.ipAddress.c_str(), ipAddress.c_str());
    ASSERT_EQ(connectedChannelRemoteAddress.port, port);
#else
    ASSERT_STREQ(connectedChannelRemoteAddress.ipAddress.c_str(), "127.0.0.1");
#endif

    ASSERT_STREQ(acceptedChannelRemoteAddress.ipAddress.c_str(), ipAddress.c_str());
    ASSERT_NE(acceptedChannelRemoteAddress.port, port);
    ASSERT_NE(acceptedChannelRemoteAddress.port, 0);
}

TEST_F(TestTcpChannel, ConnectClientUsingHostName) {
    // Arrange
    TcpChannelServer server(0, true);
    uint16_t port = server.GetLocalPort();

    // Act
    std::optional<SocketChannel> connectedChannel = TryConnectToTcpChannel("localhost", port, 0, DefaultTimeout);

    // Assert
    ASSERT_TRUE(connectedChannel);
}

TEST_F(TestTcpChannel, AcceptClientWithHostName) {
    // Arrange
    TcpChannelServer server(0, true);
    uint16_t port = server.GetLocalPort();

    (void)ConnectToTcpChannel("localhost", port);

    // Act
    std::optional<SocketChannel> acceptedChannel = server.TryAccept(DefaultTimeout);

    // Assert
    ASSERT_TRUE(acceptedChannel);
}

TEST_P(TestTcpChannel, AcceptAfterDisconnect) {
    // Arrange
    Param param = GetParam();
    std::string ipAddress = GetLoopBackAddress(param.addressFamily);

    TcpChannelServer server(0, true);
    uint16_t port = server.GetLocalPort();

    SocketChannel connectedChannel = ConnectToTcpChannel(ipAddress, port);

    // After disconnect, the server should still be able to accept it, because that is the nature of sockets
    connectedChannel.Disconnect();

    // Act
    std::optional<SocketChannel> acceptedChannel = server.TryAccept(DefaultTimeout);

    // Assert
    ASSERT_TRUE(acceptedChannel);
}

TEST_P(TestTcpChannel, WriteToChannel) {
    // Arrange
    Param param = GetParam();
    std::string ipAddress = GetLoopBackAddress(param.addressFamily);

    TcpChannelServer server(0, true);
    uint16_t port = server.GetLocalPort();

    SocketChannel connectedChannel = ConnectToTcpChannel(ipAddress, port);
    SocketChannel acceptedChannel = Accept(server);

    const uint32_t sendValue = GenerateU32();

    // Act and assert
    ASSERT_TRUE(connectedChannel.GetWriter().Write(sendValue));
    ASSERT_TRUE(connectedChannel.GetWriter().EndWrite());
}

TEST_P(TestTcpChannel, ReadFromChannel) {
    // Arrange
    Param param = GetParam();
    std::string ipAddress = GetLoopBackAddress(param.addressFamily);

    TcpChannelServer server(0, true);
    uint16_t port = server.GetLocalPort();

    SocketChannel connectedChannel = ConnectToTcpChannel(ipAddress, port);
    SocketChannel acceptedChannel = Accept(server);

    const uint32_t sendValue = GenerateU32();

    EXPECT_TRUE(connectedChannel.GetWriter().Write(sendValue));
    EXPECT_TRUE(connectedChannel.GetWriter().EndWrite());

    uint32_t receiveValue{};

    // Act
    ASSERT_TRUE(acceptedChannel.GetReader().Read(receiveValue));

    // Assert
    ASSERT_EQ(sendValue, receiveValue);
}

TEST_P(TestTcpChannel, PingPong) {
    // Arrange
    Param param = GetParam();
    std::string ipAddress = GetLoopBackAddress(param.addressFamily);

    TcpChannelServer server(0, true);
    uint16_t port = server.GetLocalPort();

    SocketChannel connectedChannel = ConnectToTcpChannel(ipAddress, port);
    SocketChannel acceptedChannel = Accept(server);

    // Act and assert
    for (uint16_t i = 0; i < 100; i++) {
        Channel* sendChannel = &connectedChannel;
        Channel* receiveChannel = &acceptedChannel;
        if (i % 2 == 1) {
            sendChannel = &acceptedChannel;
            receiveChannel = &connectedChannel;
        }

        uint16_t sendValue = GenerateU16();
        ASSERT_TRUE(sendChannel->GetWriter().Write(sendValue));
        ASSERT_TRUE(sendChannel->GetWriter().EndWrite());

        uint16_t receiveValue{};
        ASSERT_TRUE(receiveChannel->GetReader().Read(receiveValue));

        ASSERT_EQ(sendValue, receiveValue);
    }
}

TEST_P(TestTcpChannel, SendTwoFramesAtOnce) {
    // Arrange
    Param param = GetParam();
    std::string ipAddress = GetLoopBackAddress(param.addressFamily);

    TcpChannelServer server(0, true);
    uint16_t port = server.GetLocalPort();

    SocketChannel connectedChannel = ConnectToTcpChannel(ipAddress, port);
    SocketChannel acceptedChannel = Accept(server);

    uint32_t sendValue1 = GenerateU32();
    uint64_t sendValue2 = GenerateU64();
    uint32_t receiveValue1{};
    uint64_t receiveValue2{};

    // Act
    ASSERT_TRUE(acceptedChannel.GetWriter().Write(sendValue1));
    ASSERT_TRUE(acceptedChannel.GetWriter().EndWrite());

    ASSERT_TRUE(acceptedChannel.GetWriter().Write(sendValue2));
    ASSERT_TRUE(acceptedChannel.GetWriter().EndWrite());

    ASSERT_TRUE(connectedChannel.GetReader().Read(receiveValue1));
    ASSERT_TRUE(connectedChannel.GetReader().Read(receiveValue2));

    // Assert
    ASSERT_EQ(sendValue1, receiveValue1);
    ASSERT_EQ(sendValue2, receiveValue2);
}

static void StreamClient(SocketChannel& channel) {
    for (uint32_t i = 0; i < BigNumber; i++) {
        uint32_t receiveValue{};
        ASSERT_TRUE(channel.GetReader().Read(receiveValue));

        ASSERT_EQ(i, receiveValue);
    }
}

TEST_P(TestTcpChannel, Stream) {
    // Arrange
    Param param = GetParam();
    std::string ipAddress = GetLoopBackAddress(param.addressFamily);

    TcpChannelServer server(0, true);
    uint16_t port = server.GetLocalPort();

    SocketChannel connectedChannel = ConnectToTcpChannel(ipAddress, port);
    SocketChannel acceptedChannel = Accept(server);

    std::jthread thread(StreamClient, std::ref(connectedChannel));

    // Act and assert
    for (uint32_t i = 0; i < BigNumber; i++) {
        ASSERT_TRUE(acceptedChannel.GetWriter().Write(i));
    }

    ASSERT_TRUE(acceptedChannel.GetWriter().EndWrite());
}

static void ReceiveBigElement(SocketChannel& channel) {
    const auto receiveArray = std::make_unique<std::array<uint32_t, BigNumber>>();
    ASSERT_TRUE(channel.GetReader().Read(receiveArray.get(), receiveArray->size() * 4));

    for (size_t i = 0; i < receiveArray->size(); i++) {
        ASSERT_EQ((*receiveArray)[i], static_cast<uint32_t>(i));
    }
}

TEST_P(TestTcpChannel, SendAndReceiveBigElement) {
    // Arrange
    Param param = GetParam();
    std::string ipAddress = GetLoopBackAddress(param.addressFamily);

    TcpChannelServer server(0, true);
    uint16_t port = server.GetLocalPort();

    SocketChannel connectedChannel = ConnectToTcpChannel(ipAddress, port);
    SocketChannel acceptedChannel = Accept(server);

    std::jthread thread(ReceiveBigElement, std::ref(connectedChannel));

    const auto sendArray = std::make_unique<std::array<uint32_t, BigNumber>>();
    for (size_t i = 0; i < sendArray->size(); i++) {
        (*sendArray)[i] = static_cast<uint32_t>(i);
    }

    // Act and assert
    ASSERT_TRUE(acceptedChannel.GetWriter().Write(sendArray.get(), sendArray->size() * 4));
    ASSERT_TRUE(acceptedChannel.GetWriter().EndWrite());
}
