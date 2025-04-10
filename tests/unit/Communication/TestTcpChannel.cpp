// Copyright dSPACE GmbH. All rights reserved.

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>  // IWYU pragma: keep
#include <thread>
#include <vector>

#include "Channel.h"
#include "Generator.h"
#include "Helper.h"
#include "Socket.h"

using namespace DsVeosCoSim;

namespace {

constexpr uint32_t BigNumber = 4 * 1024 * 1024;

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

class TestTcpChannel : public testing::TestWithParam<Param> {};

INSTANTIATE_TEST_SUITE_P(,
                         TestTcpChannel,
                         testing::ValuesIn(GetValues()),
                         [](const testing::TestParamInfo<TestTcpChannel::ParamType>& info) {
                             return std::string(ToString(info.param.addressFamily));
                         });

TEST_F(TestTcpChannel, StartServer) {
    // Arrange

    // Act
    const std::unique_ptr<ChannelServer> server = CreateTcpChannelServer(0, true);

    // Assert
    ASSERT_TRUE(server);
}

TEST_F(TestTcpChannel, ServerStartWithZeroPort) {
    // Arrange
    const std::unique_ptr<ChannelServer> server = CreateTcpChannelServer(0, true);
    EXPECT_TRUE(server);

    // Act
    const uint16_t port = server->GetLocalPort();

    // Assert
    ASSERT_NE(0, port);
}

#ifdef _WIN32
TEST_P(TestTcpChannel, ConnectWithoutStart) {
    // Arrange
    const Param param = GetParam();
    const std::string_view ipAddress = GetLoopBackAddress(param.addressFamily);

    uint16_t port{};

    {
        const std::unique_ptr<ChannelServer> server = CreateTcpChannelServer(0, true);
        EXPECT_TRUE(server);
        port = server->GetLocalPort();
    }

    // Act
    const std::unique_ptr<Channel> connectedChannel = TryConnectToTcpChannel(ipAddress, port, 0, 0);

    // Assert
    ASSERT_FALSE(connectedChannel);
}
#endif

TEST_P(TestTcpChannel, Connect) {
    // Arrange
    const Param param = GetParam();
    const std::string_view ipAddress = GetLoopBackAddress(param.addressFamily);

    const std::unique_ptr<ChannelServer> server = CreateTcpChannelServer(0, true);
    EXPECT_TRUE(server);
    const uint16_t port = server->GetLocalPort();

    // Act
    const std::unique_ptr<Channel> connectedChannel = TryConnectToTcpChannel(ipAddress, port, 0, DefaultTimeout);

    // Assert
    ASSERT_TRUE(connectedChannel);
}

TEST_P(TestTcpChannel, AcceptWithoutConnect) {
    // Arrange
    const std::unique_ptr<ChannelServer> server = CreateTcpChannelServer(0, true);

    // Act
    const std::unique_ptr<Channel> acceptedChannel = server->TryAccept();

    // Assert
    ASSERT_FALSE(acceptedChannel);
}

TEST_P(TestTcpChannel, Accept) {
    // Arrange
    const Param param = GetParam();
    const std::string_view ipAddress = GetLoopBackAddress(param.addressFamily);

    const std::unique_ptr<ChannelServer> server = CreateTcpChannelServer(0, true);
    EXPECT_TRUE(server);
    const uint16_t port = server->GetLocalPort();

    const std::unique_ptr<Channel> connectedChannel = TryConnectToTcpChannel(ipAddress, port, 0, DefaultTimeout);
    EXPECT_TRUE(connectedChannel);

    // Act
    const std::unique_ptr<Channel> acceptedChannel = server->TryAccept(DefaultTimeout);

    // Assert
    ASSERT_TRUE(acceptedChannel);
}

TEST_P(TestTcpChannel, AcceptedClientHasCorrectAddresses) {
    // Arrange
    const Param param = GetParam();
    const std::string_view ipAddress = GetLoopBackAddress(param.addressFamily);

    const std::unique_ptr<ChannelServer> server = CreateTcpChannelServer(0, true);
    EXPECT_TRUE(server);
    const uint16_t port = server->GetLocalPort();

    const std::unique_ptr<Channel> connectedChannel = TryConnectToTcpChannel(ipAddress, port, 0, DefaultTimeout);
    EXPECT_TRUE(connectedChannel);
    const std::unique_ptr<Channel> acceptedChannel = server->TryAccept(DefaultTimeout);
    EXPECT_TRUE(acceptedChannel);

    // Act
    const std::string connectedChannelRemoteAddress = connectedChannel->GetRemoteAddress();
    const std::string acceptedChannelRemoteAddress = acceptedChannel->GetRemoteAddress();

    // Assert
    ASSERT_STRNE(connectedChannelRemoteAddress.c_str(), acceptedChannelRemoteAddress.c_str());
}

TEST_F(TestTcpChannel, ConnectClientUsingHostName) {
    // Arrange
    const std::unique_ptr<ChannelServer> server = CreateTcpChannelServer(0, true);
    EXPECT_TRUE(server);
    const uint16_t port = server->GetLocalPort();

    // Act
    const std::unique_ptr<Channel> connectedChannel = TryConnectToTcpChannel("localhost", port, 0, DefaultTimeout);

    // Assert
    ASSERT_TRUE(connectedChannel);
}

TEST_F(TestTcpChannel, AcceptClientWithHostName) {
    // Arrange
    const std::unique_ptr<ChannelServer> server = CreateTcpChannelServer(0, true);
    EXPECT_TRUE(server);
    const uint16_t port = server->GetLocalPort();

    const std::unique_ptr<Channel> connectedChannel = TryConnectToTcpChannel("localhost", port, 0, DefaultTimeout);
    EXPECT_TRUE(connectedChannel);

    // Act
    const std::unique_ptr<Channel> acceptedChannel = server->TryAccept(DefaultTimeout);

    // Assert
    ASSERT_TRUE(acceptedChannel);
}

TEST_P(TestTcpChannel, AcceptAfterDisconnect) {
    // Arrange
    const Param param = GetParam();
    const std::string_view ipAddress = GetLoopBackAddress(param.addressFamily);

    const std::unique_ptr<ChannelServer> server = CreateTcpChannelServer(0, true);
    EXPECT_TRUE(server);
    uint16_t port = server->GetLocalPort();

    const std::unique_ptr<Channel> connectedChannel = TryConnectToTcpChannel(ipAddress, port, 0, DefaultTimeout);
    EXPECT_TRUE(connectedChannel);

    // After disconnect, the server should still be able to accept it, because that is the nature of sockets
    connectedChannel->Disconnect();

    // Act
    const std::unique_ptr<Channel> acceptedChannel = server->TryAccept(DefaultTimeout);

    // Assert
    ASSERT_TRUE(acceptedChannel);
}

TEST_P(TestTcpChannel, WriteToChannel) {
    // Arrange
    const Param param = GetParam();
    const std::string_view ipAddress = GetLoopBackAddress(param.addressFamily);

    const std::unique_ptr<ChannelServer> server = CreateTcpChannelServer(0, true);
    EXPECT_TRUE(server);
    const uint16_t port = server->GetLocalPort();

    const std::unique_ptr<Channel> connectedChannel = TryConnectToTcpChannel(ipAddress, port, 0, DefaultTimeout);
    EXPECT_TRUE(connectedChannel);
    const std::unique_ptr<Channel> acceptedChannel = server->TryAccept(DefaultTimeout);
    EXPECT_TRUE(acceptedChannel);

    const uint32_t sendValue = GenerateU32();

    // Act and assert
    ASSERT_TRUE(connectedChannel->GetWriter().Write(sendValue));
    ASSERT_TRUE(connectedChannel->GetWriter().EndWrite());
}

TEST_P(TestTcpChannel, ReadFromChannel) {
    // Arrange
    const Param param = GetParam();
    const std::string_view ipAddress = GetLoopBackAddress(param.addressFamily);

    const std::unique_ptr<ChannelServer> server = CreateTcpChannelServer(0, true);
    EXPECT_TRUE(server);
    const uint16_t port = server->GetLocalPort();

    const std::unique_ptr<Channel> connectedChannel = TryConnectToTcpChannel(ipAddress, port, 0, DefaultTimeout);
    EXPECT_TRUE(connectedChannel);
    const std::unique_ptr<Channel> acceptedChannel = server->TryAccept(DefaultTimeout);
    EXPECT_TRUE(acceptedChannel);

    const uint32_t sendValue = GenerateU32();

    EXPECT_TRUE(connectedChannel->GetWriter().Write(sendValue));
    EXPECT_TRUE(connectedChannel->GetWriter().EndWrite());

    uint32_t receiveValue{};

    // Act
    ASSERT_TRUE(acceptedChannel->GetReader().Read(receiveValue));

    // Assert
    ASSERT_EQ(sendValue, receiveValue);
}

TEST_P(TestTcpChannel, PingPong) {
    // Arrange
    const Param param = GetParam();
    const std::string_view ipAddress = GetLoopBackAddress(param.addressFamily);

    const std::unique_ptr<ChannelServer> server = CreateTcpChannelServer(0, true);
    EXPECT_TRUE(server);
    const uint16_t port = server->GetLocalPort();

    const std::unique_ptr<Channel> connectedChannel = TryConnectToTcpChannel(ipAddress, port, 0, DefaultTimeout);
    EXPECT_TRUE(connectedChannel);
    const std::unique_ptr<Channel> acceptedChannel = server->TryAccept(DefaultTimeout);
    EXPECT_TRUE(acceptedChannel);

    // Act and assert
    for (uint16_t i = 0; i < 100; i++) {
        Channel* sendChannel = connectedChannel.get();
        Channel* receiveChannel = acceptedChannel.get();
        if (i % 2 == 1) {
            sendChannel = acceptedChannel.get();
            receiveChannel = connectedChannel.get();
        }

        const uint16_t sendValue = GenerateU16();
        ASSERT_TRUE(sendChannel->GetWriter().Write(sendValue));
        ASSERT_TRUE(sendChannel->GetWriter().EndWrite());

        uint16_t receiveValue{};
        ASSERT_TRUE(receiveChannel->GetReader().Read(receiveValue));

        ASSERT_EQ(sendValue, receiveValue);
    }
}

TEST_P(TestTcpChannel, SendTwoFramesAtOnce) {
    // Arrange
    const Param param = GetParam();
    const std::string_view ipAddress = GetLoopBackAddress(param.addressFamily);

    const std::unique_ptr<ChannelServer> server = CreateTcpChannelServer(0, true);
    EXPECT_TRUE(server);
    uint16_t port = server->GetLocalPort();

    const std::unique_ptr<Channel> connectedChannel = TryConnectToTcpChannel(ipAddress, port, 0, DefaultTimeout);
    EXPECT_TRUE(connectedChannel);
    const std::unique_ptr<Channel> acceptedChannel = server->TryAccept(DefaultTimeout);
    EXPECT_TRUE(acceptedChannel);

    const uint32_t sendValue1 = GenerateU32();
    const uint64_t sendValue2 = GenerateU64();
    uint32_t receiveValue1{};
    uint64_t receiveValue2{};

    // Act
    ASSERT_TRUE(acceptedChannel->GetWriter().Write(sendValue1));
    ASSERT_TRUE(acceptedChannel->GetWriter().EndWrite());

    ASSERT_TRUE(acceptedChannel->GetWriter().Write(sendValue2));
    ASSERT_TRUE(acceptedChannel->GetWriter().EndWrite());

    ASSERT_TRUE(connectedChannel->GetReader().Read(receiveValue1));
    ASSERT_TRUE(connectedChannel->GetReader().Read(receiveValue2));

    // Assert
    ASSERT_EQ(sendValue1, receiveValue1);
    ASSERT_EQ(sendValue2, receiveValue2);
}

void StreamClient(Channel& channel) {
    for (uint32_t i = 0; i < BigNumber; i++) {
        uint32_t receiveValue{};
        ASSERT_TRUE(channel.GetReader().Read(receiveValue));

        ASSERT_EQ(i, receiveValue);
    }
}

TEST_P(TestTcpChannel, Stream) {
    // Arrange
    const Param param = GetParam();
    const std::string_view ipAddress = GetLoopBackAddress(param.addressFamily);

    const std::unique_ptr<ChannelServer> server = CreateTcpChannelServer(0, true);
    EXPECT_TRUE(server);
    const uint16_t port = server->GetLocalPort();

    const std::unique_ptr<Channel> connectedChannel = TryConnectToTcpChannel(ipAddress, port, 0, DefaultTimeout);
    EXPECT_TRUE(connectedChannel);
    const std::unique_ptr<Channel> acceptedChannel = server->TryAccept(DefaultTimeout);
    EXPECT_TRUE(acceptedChannel);

    std::thread thread(StreamClient, std::ref(*connectedChannel));

    // Act and assert
    for (uint32_t i = 0; i < BigNumber; i++) {
        ASSERT_TRUE(acceptedChannel->GetWriter().Write(i));
    }

    ASSERT_TRUE(acceptedChannel->GetWriter().EndWrite());

    thread.join();
}

void ReceiveBigElement(Channel& channel) {
    const auto receiveArray = std::make_unique<std::array<uint32_t, BigNumber>>();
    ASSERT_TRUE(channel.GetReader().Read(receiveArray.get(), receiveArray->size() * 4));

    for (size_t i = 0; i < receiveArray->size(); i++) {
        ASSERT_EQ((*receiveArray)[i], static_cast<uint32_t>(i));
    }
}

TEST_P(TestTcpChannel, SendAndReceiveBigElement) {
    // Arrange
    const Param param = GetParam();
    const std::string_view ipAddress = GetLoopBackAddress(param.addressFamily);

    const std::unique_ptr<ChannelServer> server = CreateTcpChannelServer(0, true);
    EXPECT_TRUE(server);
    const uint16_t port = server->GetLocalPort();

    const std::unique_ptr<Channel> connectedChannel = TryConnectToTcpChannel(ipAddress, port, 0, DefaultTimeout);
    EXPECT_TRUE(connectedChannel);
    const std::unique_ptr<Channel> acceptedChannel = server->TryAccept(DefaultTimeout);
    EXPECT_TRUE(acceptedChannel);

    std::thread thread(ReceiveBigElement, std::ref(*connectedChannel));

    const auto sendArray = std::make_unique<std::array<uint32_t, BigNumber>>();
    for (size_t i = 0; i < sendArray->size(); i++) {
        (*sendArray)[i] = static_cast<uint32_t>(i);
    }

    // Act and assert
    ASSERT_TRUE(acceptedChannel->GetWriter().Write(sendArray.get(), sendArray->size() * 4));
    ASSERT_TRUE(acceptedChannel->GetWriter().EndWrite());

    thread.join();
}

}  // namespace
