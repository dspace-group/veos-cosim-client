// Copyright dSPACE GmbH. All rights reserved.

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <thread>

#include "Channel.h"
#include "Generator.h"
#include "Helper.h"

using namespace DsVeosCoSim;

namespace {

constexpr uint32_t BigNumber = 4 * 1024 * 1024;

[[nodiscard]] std::string GenerateName() {
    return GenerateString("UdsChannel");
}

class TestUdsChannel : public testing::Test {};

TEST_F(TestUdsChannel, StartServer) {
    // Arrange
    const std::string name = GenerateName();

    // Act and assert
    ASSERT_NO_THROW((void)CreateUdsChannelServer(name));
}

TEST_F(TestUdsChannel, ConnectWithoutStart) {
    // Arrange
    const std::string name = GenerateName();

    {
        const std::unique_ptr<ChannelServer> server = CreateUdsChannelServer(name);
    }

    // Act
    const std::unique_ptr<Channel> connectedChannel = TryConnectToUdsChannel(name);

    // Assert
    ASSERT_FALSE(connectedChannel);
}

TEST_F(TestUdsChannel, Connect) {
    // Arrange
    const std::string name = GenerateName();

    const std::unique_ptr<ChannelServer> server = CreateUdsChannelServer(name);

    // Act
    const std::unique_ptr<Channel> connectedChannel = TryConnectToUdsChannel(name);

    // Assert
    ASSERT_TRUE(connectedChannel);
}

TEST_F(TestUdsChannel, AcceptWithoutConnect) {
    // Arrange
    const std::string name = GenerateName();

    const std::unique_ptr<ChannelServer> server = CreateUdsChannelServer(name);

    // Act
    const std::unique_ptr<Channel> acceptedChannel = server->TryAccept();

    // Assert
    ASSERT_FALSE(acceptedChannel);
}

TEST_F(TestUdsChannel, Accept) {
    // Arrange
    const std::string name = GenerateName();

    const std::unique_ptr<ChannelServer> server = CreateUdsChannelServer(name);

    const std::unique_ptr<Channel> connectedChannel = TryConnectToUdsChannel(name);
    EXPECT_TRUE(connectedChannel);

    // Act
    const std::unique_ptr<Channel> acceptedChannel = server->TryAccept(DefaultTimeout);

    // Assert
    ASSERT_TRUE(acceptedChannel);
}

TEST_F(TestUdsChannel, AcceptAfterDisconnect) {
    // Arrange
    std::string name = GenerateName();

    const std::unique_ptr<ChannelServer> server = CreateUdsChannelServer(name);

    const std::unique_ptr<Channel> connectedChannel = TryConnectToUdsChannel(name);
    EXPECT_TRUE(connectedChannel);

    // After disconnect, the server should still be able to accept it, because that is the nature of sockets
    connectedChannel->Disconnect();

    // Act
    const std::unique_ptr<Channel> acceptedChannel = server->TryAccept(DefaultTimeout);

    // Assert
    ASSERT_TRUE(acceptedChannel);
}

TEST_F(TestUdsChannel, WriteToChannel) {
    // Arrange
    std::string name = GenerateName();

    const std::unique_ptr<ChannelServer> server = CreateUdsChannelServer(name);

    const std::unique_ptr<Channel> connectedChannel = TryConnectToUdsChannel(name);
    EXPECT_TRUE(connectedChannel);
    const std::unique_ptr<Channel> acceptedChannel = server->TryAccept(DefaultTimeout);
    EXPECT_TRUE(acceptedChannel);

    const uint32_t sendValue = GenerateU32();

    // Act and assert
    ASSERT_TRUE(connectedChannel->GetWriter().Write(sendValue));
    ASSERT_TRUE(connectedChannel->GetWriter().EndWrite());
}

TEST_F(TestUdsChannel, ReadFromChannel) {
    // Arrange
    std::string name = GenerateName();

    const std::unique_ptr<ChannelServer> server = CreateUdsChannelServer(name);

    const std::unique_ptr<Channel> connectedChannel = TryConnectToUdsChannel(name);
    EXPECT_TRUE(connectedChannel);
    const std::unique_ptr<Channel> acceptedChannel = server->TryAccept(DefaultTimeout);
    EXPECT_TRUE(acceptedChannel);

    const uint32_t sendValue = GenerateU32();

    ASSERT_TRUE(connectedChannel->GetWriter().Write(sendValue));
    ASSERT_TRUE(connectedChannel->GetWriter().EndWrite());

    uint32_t receiveValue{};

    // Act
    ASSERT_TRUE(acceptedChannel->GetReader().Read(receiveValue));

    // Assert
    ASSERT_EQ(sendValue, receiveValue);
}

TEST_F(TestUdsChannel, PingPong) {
    // Arrange
    std::string name = GenerateName();

    const std::unique_ptr<ChannelServer> server = CreateUdsChannelServer(name);

    const std::unique_ptr<Channel> connectedChannel = TryConnectToUdsChannel(name);
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

TEST_F(TestUdsChannel, SendTwoFramesAtOnce) {
    // Arrange
    std::string name = GenerateName();

    const std::unique_ptr<ChannelServer> server = CreateUdsChannelServer(name);

    const std::unique_ptr<Channel> connectedChannel = TryConnectToUdsChannel(name);
    EXPECT_TRUE(connectedChannel);
    const std::unique_ptr<Channel> acceptedChannel = server->TryAccept(DefaultTimeout);
    EXPECT_TRUE(acceptedChannel);

    uint32_t sendValue1 = GenerateU32();
    uint64_t sendValue2 = GenerateU64();
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

TEST_F(TestUdsChannel, Stream) {
    // Arrange
    std::string name = GenerateName();

    const std::unique_ptr<ChannelServer> server = CreateUdsChannelServer(name);

    const std::unique_ptr<Channel> connectedChannel = TryConnectToUdsChannel(name);
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

TEST_F(TestUdsChannel, SendAndReceiveBigElement) {
    // Arrange
    std::string name = GenerateName();

    const std::unique_ptr<ChannelServer> server = CreateUdsChannelServer(name);

    const std::unique_ptr<Channel> connectedChannel = TryConnectToUdsChannel(name);
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
