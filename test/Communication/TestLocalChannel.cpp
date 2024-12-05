// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <thread>

#include "Generator.h"
#include "Helper.h"
#include "LocalChannel.h"

using namespace DsVeosCoSim;

namespace {

constexpr uint32_t BigNumber = 4 * 1024 * 1024;

[[nodiscard]] std::string GenerateName() {
    return GenerateString("Channel名前\xF0\x9F\x98\x80");
}

class TestLocalChannel : public testing::Test {};

TEST_F(TestLocalChannel, StartServer) {
    // Arrange
    const std::string name = GenerateName();

    // Act and assert
    ASSERT_NO_THROW((void)LocalChannelServer(name));
}

TEST_F(TestLocalChannel, ConnectWithoutStart) {
    // Arrange
    const std::string name = GenerateName();

    { LocalChannelServer server(name); }

    // Act
    std::optional<LocalChannel> connectedChannel = TryConnectToLocalChannel(name);

    // Assert
    ASSERT_FALSE(connectedChannel);
}

TEST_F(TestLocalChannel, Connect) {
    // Arrange
    const std::string name = GenerateName();

    LocalChannelServer server(name);

    // Act
    std::optional<LocalChannel> connectedChannel = TryConnectToLocalChannel(name);

    // Assert
    ASSERT_TRUE(connectedChannel);
}

TEST_F(TestLocalChannel, AcceptWithoutConnect) {
    // Arrange
    const std::string name = GenerateName();

    LocalChannelServer server(name);

    // Act
    std::optional<LocalChannel> acceptedChannel = server.TryAccept();

    // Assert
    ASSERT_FALSE(acceptedChannel);
}

TEST_F(TestLocalChannel, Accept) {
    // Arrange
    const std::string name = GenerateName();

    LocalChannelServer server(name);

    (void)ConnectToLocalChannel(name);

    // Act
    std::optional<LocalChannel> acceptedChannel = server.TryAccept();

    // Assert
    ASSERT_TRUE(acceptedChannel);
}

TEST_F(TestLocalChannel, AcceptAfterDisconnect) {
    // Arrange
    const std::string name = GenerateName();

    LocalChannelServer server(name);

    LocalChannel connectedChannel = ConnectToLocalChannel(name);

    // After disconnect, the server should still be able to accept it, because that is the nature of sockets
    connectedChannel.Disconnect();

    // Act
    std::optional<LocalChannel> acceptedChannel = server.TryAccept();

    // Assert
    ASSERT_TRUE(acceptedChannel);
}

TEST_F(TestLocalChannel, WriteToChannel) {
    // Arrange
    const std::string name = GenerateName();

    LocalChannelServer server(name);

    LocalChannel connectedChannel = ConnectToLocalChannel(name);
    LocalChannel acceptedChannel = Accept(server);

    const uint32_t sendValue = GenerateU32();

    // Act and assert
    ASSERT_TRUE(connectedChannel.GetWriter().Write(sendValue));
    ASSERT_TRUE(connectedChannel.GetWriter().EndWrite());
}

TEST_F(TestLocalChannel, ReadFromChannel) {
    // Arrange
    const std::string name = GenerateName();

    LocalChannelServer server(name);

    LocalChannel connectedChannel = ConnectToLocalChannel(name);
    LocalChannel acceptedChannel = Accept(server);

    const uint32_t sendValue = GenerateU32();

    EXPECT_TRUE(connectedChannel.GetWriter().Write(sendValue));
    EXPECT_TRUE(connectedChannel.GetWriter().EndWrite());

    uint32_t receiveValue{};

    // Act
    ASSERT_TRUE(acceptedChannel.GetReader().Read(receiveValue));

    // Assert
    ASSERT_EQ(sendValue, receiveValue);
}

TEST_F(TestLocalChannel, PingPong) {
    // Arrange
    const std::string name = GenerateName();

    LocalChannelServer server(name);

    LocalChannel connectedChannel = ConnectToLocalChannel(name);
    LocalChannel acceptedChannel = Accept(server);

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

TEST_F(TestLocalChannel, SendTwoFramesAtOnce) {
    // Arrange
    const std::string name = GenerateName();

    LocalChannelServer server(name);

    LocalChannel connectedChannel = ConnectToLocalChannel(name);
    LocalChannel acceptedChannel = Accept(server);

    uint32_t sendValue1 = GenerateU32();
    uint64_t sendValue2 = GenerateU64();
    uint32_t receiveValue1{};
    uint32_t receiveValue2{};

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

void StreamClient(LocalChannel& channel) {
    for (uint32_t i = 0; i < BigNumber; i++) {
        uint32_t receiveValue{};
        ASSERT_TRUE(channel.GetReader().Read(receiveValue));

        ASSERT_EQ(i, receiveValue);
    }
}

TEST_F(TestLocalChannel, Stream) {
    // Arrange
    const std::string name = GenerateName();

    LocalChannelServer server(name);

    LocalChannel connectedChannel = ConnectToLocalChannel(name);
    LocalChannel acceptedChannel = Accept(server);

    std::thread thread(StreamClient, std::ref(connectedChannel));

    // Act and assert
    for (uint32_t i = 0; i < BigNumber; i++) {
        ASSERT_TRUE(acceptedChannel.GetWriter().Write(i));
    }

    ASSERT_TRUE(acceptedChannel.GetWriter().EndWrite());

    thread.join();
}

void ReceiveBigElement(LocalChannel& channel) {
    const auto receiveArray = std::make_unique<std::array<uint32_t, BigNumber>>();
    ASSERT_TRUE(channel.GetReader().Read(receiveArray.get(), receiveArray->size() * 4));

    for (size_t i = 0; i < receiveArray->size(); i++) {
        ASSERT_EQ((*receiveArray)[i], static_cast<uint32_t>(i));
    }
}

TEST_F(TestLocalChannel, SendAndReceiveBigElement) {
    // Arrange
    const std::string name = GenerateName();

    LocalChannelServer server(name);

    LocalChannel connectedChannel = ConnectToLocalChannel(name);
    LocalChannel acceptedChannel = Accept(server);

    std::thread thread(ReceiveBigElement, std::ref(connectedChannel));

    const auto sendArray = std::make_unique<std::array<uint32_t, BigNumber>>();
    for (size_t i = 0; i < sendArray->size(); i++) {
        (*sendArray)[i] = static_cast<uint32_t>(i);
    }

    // Act and assert
    ASSERT_TRUE(acceptedChannel.GetWriter().Write(sendArray.get(), sendArray->size() * 4));
    ASSERT_TRUE(acceptedChannel.GetWriter().EndWrite());

    thread.join();
}

}  // namespace

#endif
