// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <thread>

#include "Channel.h"
#include "Generator.h"

using namespace DsVeosCoSim;

namespace {

constexpr uint32_t BigNumber = 4 * 1024 * 1024;

[[nodiscard]] std::string GenerateName() {
    return GenerateString("Channel名前\xF0\x9F\x98\x80");
}

class TestLocalChannel : public testing::Test {};

TEST_F(TestLocalChannel, StartServer) {
    // Arrange
    std::string name = GenerateName();

    // Act
    std::unique_ptr<ChannelServer> server = CreateLocalChannelServer(name);

    // Assert
    ASSERT_TRUE(server);
}

TEST_F(TestLocalChannel, ConnectWithoutStart) {
    // Arrange
    std::string name = GenerateName();

    {
        std::unique_ptr<ChannelServer> server = CreateLocalChannelServer(name);
        EXPECT_TRUE(server);
    }

    // Act
    std::unique_ptr<Channel> connectedChannel = TryConnectToLocalChannel(name);

    // Assert
    ASSERT_FALSE(connectedChannel);
}

TEST_F(TestLocalChannel, Connect) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateLocalChannelServer(name);
    EXPECT_TRUE(server);

    // Act
    std::unique_ptr<Channel> connectedChannel = TryConnectToLocalChannel(name);

    // Assert
    ASSERT_TRUE(connectedChannel);
}

TEST_F(TestLocalChannel, AcceptWithoutConnect) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateLocalChannelServer(name);
    EXPECT_TRUE(server);

    // Act
    std::unique_ptr<Channel> acceptedChannel = server->TryAccept();

    // Assert
    ASSERT_FALSE(acceptedChannel);
}

TEST_F(TestLocalChannel, Accept) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateLocalChannelServer(name);
    EXPECT_TRUE(server);

    std::unique_ptr<Channel> connectedChannel = TryConnectToLocalChannel(name);
    EXPECT_TRUE(connectedChannel);

    // Act
    std::unique_ptr<Channel> acceptedChannel = server->TryAccept();

    // Assert
    ASSERT_TRUE(acceptedChannel);
}

TEST_F(TestLocalChannel, AcceptAfterDisconnect) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateLocalChannelServer(name);
    EXPECT_TRUE(server);

    std::unique_ptr<Channel> connectedChannel = TryConnectToLocalChannel(name);
    EXPECT_TRUE(connectedChannel);

    // After disconnect, the server should still be able to accept it, because that is the nature of sockets
    connectedChannel->Disconnect();

    // Act
    std::unique_ptr<Channel> acceptedChannel = server->TryAccept();

    // Assert
    ASSERT_TRUE(acceptedChannel);
}

TEST_F(TestLocalChannel, WriteToChannel) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateLocalChannelServer(name);
    EXPECT_TRUE(server);

    std::unique_ptr<Channel> connectedChannel = TryConnectToLocalChannel(name);
    EXPECT_TRUE(connectedChannel);
    std::unique_ptr<Channel> acceptedChannel = server->TryAccept();
    EXPECT_TRUE(acceptedChannel);

    uint32_t sendValue = GenerateU32();

    // Act and assert
    ASSERT_TRUE(connectedChannel->GetWriter().Write(sendValue));
    ASSERT_TRUE(connectedChannel->GetWriter().EndWrite());
}

TEST_F(TestLocalChannel, ReadFromChannel) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateLocalChannelServer(name);
    EXPECT_TRUE(server);

    std::unique_ptr<Channel> connectedChannel = TryConnectToLocalChannel(name);
    EXPECT_TRUE(connectedChannel);
    std::unique_ptr<Channel> acceptedChannel = server->TryAccept();
    EXPECT_TRUE(acceptedChannel);

    uint32_t sendValue = GenerateU32();

    EXPECT_TRUE(connectedChannel->GetWriter().Write(sendValue));
    EXPECT_TRUE(connectedChannel->GetWriter().EndWrite());

    uint32_t receiveValue{};

    // Act
    ASSERT_TRUE(acceptedChannel->GetReader().Read(receiveValue));

    // Assert
    ASSERT_EQ(sendValue, receiveValue);
}

TEST_F(TestLocalChannel, PingPong) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateLocalChannelServer(name);
    EXPECT_TRUE(server);

    std::unique_ptr<Channel> connectedChannel = TryConnectToLocalChannel(name);
    EXPECT_TRUE(connectedChannel);
    std::unique_ptr<Channel> acceptedChannel = server->TryAccept();
    EXPECT_TRUE(acceptedChannel);

    // Act and assert
    for (uint16_t i = 0; i < 100; i++) {
        Channel* sendChannel = connectedChannel.get();
        Channel* receiveChannel = acceptedChannel.get();
        if (i % 2 == 1) {
            sendChannel = acceptedChannel.get();
            receiveChannel = connectedChannel.get();
        }

        uint16_t sendValue = GenerateU16();
        ASSERT_TRUE(sendChannel->GetWriter().Write(sendValue));
        ASSERT_TRUE(sendChannel->GetWriter().EndWrite());

        uint16_t receiveValue{};
        ASSERT_TRUE(receiveChannel->GetReader().Read(receiveValue));

        ASSERT_EQ(sendValue, receiveValue);
    }
}

TEST_F(TestLocalChannel, SendTwoFramesAtOnce) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateLocalChannelServer(name);
    EXPECT_TRUE(server);

    std::unique_ptr<Channel> connectedChannel = TryConnectToLocalChannel(name);
    EXPECT_TRUE(connectedChannel);
    std::unique_ptr<Channel> acceptedChannel = server->TryAccept();
    EXPECT_TRUE(acceptedChannel);

    uint32_t sendValue1 = GenerateU32();
    uint64_t sendValue2 = GenerateU64();
    uint32_t receiveValue1{};
    uint32_t receiveValue2{};

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

TEST_F(TestLocalChannel, Stream) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateLocalChannelServer(name);
    EXPECT_TRUE(server);

    std::unique_ptr<Channel> connectedChannel = TryConnectToLocalChannel(name);
    EXPECT_TRUE(connectedChannel);
    std::unique_ptr<Channel> acceptedChannel = server->TryAccept();
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
    auto receiveArray = std::make_unique<std::array<uint32_t, BigNumber>>();
    ASSERT_TRUE(channel.GetReader().Read(receiveArray.get(), receiveArray->size() * 4));

    for (size_t i = 0; i < receiveArray->size(); i++) {
        ASSERT_EQ((*receiveArray)[i], static_cast<uint32_t>(i));
    }
}

TEST_F(TestLocalChannel, SendAndReceiveBigElement) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server = CreateLocalChannelServer(name);
    EXPECT_TRUE(server);

    std::unique_ptr<Channel> connectedChannel = TryConnectToLocalChannel(name);
    EXPECT_TRUE(connectedChannel);
    std::unique_ptr<Channel> acceptedChannel = server->TryAccept();
    EXPECT_TRUE(acceptedChannel);

    std::thread thread(ReceiveBigElement, std::ref(*connectedChannel));

    auto sendArray = std::make_unique<std::array<uint32_t, BigNumber>>();
    for (size_t i = 0; i < sendArray->size(); i++) {
        (*sendArray)[i] = static_cast<uint32_t>(i);
    }

    // Act and assert
    ASSERT_TRUE(acceptedChannel->GetWriter().Write(sendArray.get(), sendArray->size() * 4));
    ASSERT_TRUE(acceptedChannel->GetWriter().EndWrite());

    thread.join();
}

}  // namespace

#endif
