// Copyright dSPACE GmbH. All rights reserved.

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>
#include <thread>
#include <vector>

#include "Channel.h"
#include "Helper.h"
#include "Socket.h"
#include "TestHelper.h"

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
    std::unique_ptr<ChannelServer> server;

    // Act
    AssertOk(CreateTcpChannelServer(0, true, server));

    // Assert
    AssertTrue(server);
}

TEST_F(TestTcpChannel, ServerStartWithZeroPort) {
    // Arrange
    std::unique_ptr<ChannelServer> server;
    ExpectOk(CreateTcpChannelServer(0, true, server));
    ExpectTrue(server);

    // Act
    uint16_t port = server->GetLocalPort();

    // Assert
    AssertNotEq(uint16_t(0), port);
}

#ifdef _WIN32
TEST_P(TestTcpChannel, ConnectWithoutStart) {
    // Arrange
    Param param = GetParam();
    std::string_view ipAddress = GetLoopBackAddress(param.addressFamily);

    uint16_t port{};

    {
        std::unique_ptr<ChannelServer> server;
        ExpectOk(CreateTcpChannelServer(0, true, server));
        ExpectTrue(server);
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
    std::string_view ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server;
    ExpectOk(CreateTcpChannelServer(0, true, server));
    ExpectTrue(server);
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectedChannel;

    // Act
    AssertOk(TryConnectToTcpChannel(ipAddress, port, 0, DefaultTimeout, connectedChannel));

    // Assert
    AssertTrue(connectedChannel);
}

TEST_P(TestTcpChannel, AcceptWithoutConnect) {
    // Arrange
    std::unique_ptr<ChannelServer> server;
    ExpectOk(CreateTcpChannelServer(0, true, server));
    ExpectTrue(server);

    std::unique_ptr<Channel> acceptedChannel;

    // Act
    AssertOk(server->TryAccept(acceptedChannel));

    // Assert
    AssertFalse(acceptedChannel);
}

TEST_P(TestTcpChannel, Accept) {
    // Arrange
    Param param = GetParam();
    std::string_view ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server;
    ExpectOk(CreateTcpChannelServer(0, true, server));
    ExpectTrue(server);
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectedChannel;
    ExpectOk(TryConnectToTcpChannel(ipAddress, port, 0, DefaultTimeout, connectedChannel));
    ExpectTrue(connectedChannel);

    std::unique_ptr<Channel> acceptedChannel;

    // Act
    AssertOk(server->TryAccept(acceptedChannel));

    // Assert
    AssertTrue(acceptedChannel);
}

TEST_P(TestTcpChannel, AcceptedClientHasCorrectAddresses) {
    // Arrange
    Param param = GetParam();
    std::string_view ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server;
    ExpectOk(CreateTcpChannelServer(0, true, server));
    ExpectTrue(server);
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectedChannel;
    ExpectOk(TryConnectToTcpChannel(ipAddress, port, 0, DefaultTimeout, connectedChannel));
    ExpectTrue(connectedChannel);
    std::unique_ptr<Channel> acceptedChannel;
    ExpectOk(server->TryAccept(acceptedChannel));
    ExpectTrue(acceptedChannel);

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
    std::unique_ptr<ChannelServer> server;
    ExpectOk(CreateTcpChannelServer(0, true, server));
    ExpectTrue(server);
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectedChannel;

    // Act
    AssertOk(TryConnectToTcpChannel("localhost", port, 0, DefaultTimeout, connectedChannel));

    // Assert
    AssertTrue(connectedChannel);
}

TEST_F(TestTcpChannel, AcceptClientWithHostName) {
    // Arrange
    std::unique_ptr<ChannelServer> server;
    ExpectOk(CreateTcpChannelServer(0, true, server));
    ExpectTrue(server);
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectedChannel;
    ExpectOk(TryConnectToTcpChannel("localhost", port, 0, DefaultTimeout, connectedChannel));
    ExpectTrue(connectedChannel);

    std::unique_ptr<Channel> acceptedChannel;

    // Act
    AssertOk(server->TryAccept(acceptedChannel));

    // Assert
    AssertTrue(acceptedChannel);
}

TEST_P(TestTcpChannel, AcceptAfterDisconnect) {
    // Arrange
    Param param = GetParam();
    std::string_view ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server;
    ExpectOk(CreateTcpChannelServer(0, true, server));
    ExpectTrue(server);
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectedChannel;
    ExpectOk(TryConnectToTcpChannel(ipAddress, port, 0, DefaultTimeout, connectedChannel));
    ExpectTrue(connectedChannel);

    // After disconnect, the server should still be able to accept it, because that is the nature of sockets
    connectedChannel->Disconnect();

    std::unique_ptr<Channel> acceptedChannel;

    // Act
    AssertOk(server->TryAccept(acceptedChannel));

    // Assert
    AssertTrue(acceptedChannel);
}

TEST_P(TestTcpChannel, WriteToChannel) {
    // Arrange
    Param param = GetParam();
    std::string_view ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server;
    ExpectOk(CreateTcpChannelServer(0, true, server));
    ExpectTrue(server);
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectedChannel;
    ExpectOk(TryConnectToTcpChannel(ipAddress, port, 0, DefaultTimeout, connectedChannel));
    ExpectTrue(connectedChannel);
    std::unique_ptr<Channel> acceptedChannel;
    ExpectOk(server->TryAccept(acceptedChannel));
    ExpectTrue(acceptedChannel);

    uint32_t sendValue = GenerateU32();

    // Act and assert
    AssertOk(connectedChannel->GetWriter().Write(sendValue));
    AssertOk(connectedChannel->GetWriter().EndWrite());
}

TEST_P(TestTcpChannel, ReadFromChannel) {
    // Arrange
    Param param = GetParam();
    std::string_view ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server;
    ExpectOk(CreateTcpChannelServer(0, true, server));
    ExpectTrue(server);
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectedChannel;
    ExpectOk(TryConnectToTcpChannel(ipAddress, port, 0, DefaultTimeout, connectedChannel));
    ExpectTrue(connectedChannel);
    std::unique_ptr<Channel> acceptedChannel;
    ExpectOk(server->TryAccept(acceptedChannel));
    ExpectTrue(acceptedChannel);

    uint32_t sendValue = GenerateU32();

    ExpectOk(connectedChannel->GetWriter().Write(sendValue));
    ExpectOk(connectedChannel->GetWriter().EndWrite());

    uint32_t receiveValue{};

    // Act
    AssertOk(acceptedChannel->GetReader().Read(receiveValue));

    // Assert
    AssertEq(sendValue, receiveValue);
}

TEST_P(TestTcpChannel, PingPong) {
    // Arrange
    Param param = GetParam();
    std::string_view ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server;
    ExpectOk(CreateTcpChannelServer(0, true, server));
    ExpectTrue(server);
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectedChannel;
    ExpectOk(TryConnectToTcpChannel(ipAddress, port, 0, DefaultTimeout, connectedChannel));
    ExpectTrue(connectedChannel);
    std::unique_ptr<Channel> acceptedChannel;
    ExpectOk(server->TryAccept(acceptedChannel));
    ExpectTrue(acceptedChannel);

    // Act and assert
    for (uint16_t i = 0; i < 100; i++) {
        Channel* sendChannel = connectedChannel.get();
        Channel* receiveChannel = acceptedChannel.get();
        if (i % 2 == 1) {
            sendChannel = acceptedChannel.get();
            receiveChannel = connectedChannel.get();
        }

        uint16_t sendValue = GenerateU16();
        AssertOk(sendChannel->GetWriter().Write(sendValue));
        AssertOk(sendChannel->GetWriter().EndWrite());

        uint16_t receiveValue{};
        AssertOk(receiveChannel->GetReader().Read(receiveValue));

        AssertEq(sendValue, receiveValue);
    }
}

TEST_P(TestTcpChannel, SendTwoFramesAtOnce) {
    // Arrange
    Param param = GetParam();
    std::string_view ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server;
    ExpectOk(CreateTcpChannelServer(0, true, server));
    ExpectTrue(server);
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectedChannel;
    ExpectOk(TryConnectToTcpChannel(ipAddress, port, 0, DefaultTimeout, connectedChannel));
    ExpectTrue(connectedChannel);
    std::unique_ptr<Channel> acceptedChannel;
    ExpectOk(server->TryAccept(acceptedChannel));
    ExpectTrue(acceptedChannel);

    uint32_t sendValue1 = GenerateU32();
    uint64_t sendValue2 = GenerateU64();
    uint32_t receiveValue1{};
    uint64_t receiveValue2{};

    // Act
    AssertOk(acceptedChannel->GetWriter().Write(sendValue1));
    AssertOk(acceptedChannel->GetWriter().EndWrite());

    AssertOk(acceptedChannel->GetWriter().Write(sendValue2));
    AssertOk(acceptedChannel->GetWriter().EndWrite());

    AssertOk(connectedChannel->GetReader().Read(receiveValue1));
    AssertOk(connectedChannel->GetReader().Read(receiveValue2));

    // Assert
    AssertEq(sendValue1, receiveValue1);
    AssertEq(sendValue2, receiveValue2);
}

void StreamClient(Channel& channel) {
    for (uint32_t i = 0; i < BigNumber; i++) {
        uint32_t receiveValue{};
        ExpectOk(channel.GetReader().Read(receiveValue));

        AssertEq(i, receiveValue);
    }
}

TEST_P(TestTcpChannel, Stream) {
    // Arrange
    Param param = GetParam();
    std::string_view ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server;
    ExpectOk(CreateTcpChannelServer(0, true, server));
    ExpectTrue(server);
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectedChannel;
    ExpectOk(TryConnectToTcpChannel(ipAddress, port, 0, DefaultTimeout, connectedChannel));
    ExpectTrue(connectedChannel);
    std::unique_ptr<Channel> acceptedChannel;
    ExpectOk(server->TryAccept(acceptedChannel));
    ExpectTrue(acceptedChannel);

    std::thread thread(StreamClient, std::ref(*connectedChannel));

    // Act and assert
    for (uint32_t i = 0; i < BigNumber; i++) {
        AssertOk(acceptedChannel->GetWriter().Write(i));
    }

    AssertOk(acceptedChannel->GetWriter().EndWrite());

    thread.join();
}

void ReceiveBigElement(Channel& channel) {
    auto receiveArray = std::make_unique<std::array<uint32_t, BigNumber>>();
    ExpectOk(channel.GetReader().Read(receiveArray.get(), receiveArray->size() * 4));

    for (size_t i = 0; i < receiveArray->size(); i++) {
        auto expected = static_cast<uint32_t>(i);
        uint32_t actual = (*receiveArray)[i];
        AssertEq(expected, actual);
    }
}

TEST_P(TestTcpChannel, SendAndReceiveBigElement) {
    // Arrange
    Param param = GetParam();
    std::string_view ipAddress = GetLoopBackAddress(param.addressFamily);

    std::unique_ptr<ChannelServer> server;
    ExpectOk(CreateTcpChannelServer(0, true, server));
    ExpectTrue(server);
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectedChannel;
    ExpectOk(TryConnectToTcpChannel(ipAddress, port, 0, DefaultTimeout, connectedChannel));
    ExpectTrue(connectedChannel);
    std::unique_ptr<Channel> acceptedChannel;
    ExpectOk(server->TryAccept(acceptedChannel));
    ExpectTrue(acceptedChannel);

    std::thread thread(ReceiveBigElement, std::ref(*connectedChannel));

    auto sendArray = std::make_unique<std::array<uint32_t, BigNumber>>();
    for (size_t i = 0; i < sendArray->size(); i++) {
        (*sendArray)[i] = static_cast<uint32_t>(i);
    }

    // Act and assert
    AssertOk(acceptedChannel->GetWriter().Write(sendArray.get(), sendArray->size() * 4));
    AssertOk(acceptedChannel->GetWriter().EndWrite());

    thread.join();
}

}  // namespace
