// Copyright dSPACE GmbH. All rights reserved.

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <thread>

#include "Channel.h"
#include "Helper.h"
#include "TestHelper.h"

using namespace DsVeosCoSim;

namespace {

constexpr uint32_t BigNumber = 4 * 1024 * 1024;

[[nodiscard]] std::string GenerateName() {
    return GenerateString("UdsChannel");
}

class TestUdsChannel : public testing::Test {};

TEST_F(TestUdsChannel, StartServer) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server;

    // Act
    AssertOk(CreateUdsChannelServer(name, server));

    // Assert
    AssertTrue(server);
}

TEST_F(TestUdsChannel, ConnectWithoutStart) {
    // Arrange
    std::string name = GenerateName();

    {
        std::unique_ptr<ChannelServer> server;
        ExpectOk(CreateUdsChannelServer(name, server));
        ExpectTrue(server);
    }

    std::unique_ptr<Channel> connectedChannel;

    // Act
    AssertOk(TryConnectToUdsChannel(name, connectedChannel));

    // Assert
    AssertFalse(connectedChannel);
}

TEST_F(TestUdsChannel, Connect) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server;
    ExpectOk(CreateUdsChannelServer(name, server));
    ExpectTrue(server);

    std::unique_ptr<Channel> connectedChannel;

    // Act
    AssertOk(TryConnectToUdsChannel(name, connectedChannel));

    // Assert
    AssertTrue(connectedChannel);
}

TEST_F(TestUdsChannel, AcceptWithoutConnect) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server;
    ExpectOk(CreateUdsChannelServer(name, server));
    ExpectTrue(server);

    std::unique_ptr<Channel> acceptedChannel;

    // Act
    AssertOk(server->TryAccept(acceptedChannel));

    // Assert
    AssertFalse(acceptedChannel);
}

TEST_F(TestUdsChannel, Accept) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server;
    ExpectOk(CreateUdsChannelServer(name, server));
    ExpectTrue(server);

    std::unique_ptr<Channel> connectedChannel;
    ExpectOk(TryConnectToUdsChannel(name, connectedChannel));
    ExpectTrue(connectedChannel);

    std::unique_ptr<Channel> acceptedChannel;

    // Act
    AssertOk(server->TryAccept(acceptedChannel));

    // Assert
    AssertTrue(acceptedChannel);
}

TEST_F(TestUdsChannel, AcceptAfterDisconnect) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server;
    ExpectOk(CreateUdsChannelServer(name, server));
    ExpectTrue(server);

    std::unique_ptr<Channel> connectedChannel;
    ExpectOk(TryConnectToUdsChannel(name, connectedChannel));
    ExpectTrue(connectedChannel);

    // After disconnect, the server should still be able to accept it, because that is the nature of sockets
    connectedChannel->Disconnect();

    std::unique_ptr<Channel> acceptedChannel;

    // Act
    AssertOk(server->TryAccept(acceptedChannel));

    // Assert
    AssertTrue(acceptedChannel);
}

TEST_F(TestUdsChannel, WriteToChannel) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server;
    ExpectOk(CreateUdsChannelServer(name, server));
    ExpectTrue(server);

    std::unique_ptr<Channel> connectedChannel;
    ExpectOk(TryConnectToUdsChannel(name, connectedChannel));
    ExpectTrue(connectedChannel);
    std::unique_ptr<Channel> acceptedChannel;
    ExpectOk(server->TryAccept(acceptedChannel));
    ExpectTrue(acceptedChannel);

    uint32_t sendValue = GenerateU32();

    // Act and assert
    AssertOk(connectedChannel->GetWriter().Write(sendValue));
    AssertOk(connectedChannel->GetWriter().EndWrite());
}

TEST_F(TestUdsChannel, ReadFromChannel) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server;
    ExpectOk(CreateUdsChannelServer(name, server));
    ExpectTrue(server);

    std::unique_ptr<Channel> connectedChannel;
    ExpectOk(TryConnectToUdsChannel(name, connectedChannel));
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

TEST_F(TestUdsChannel, PingPong) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server;
    ExpectOk(CreateUdsChannelServer(name, server));
    ExpectTrue(server);

    std::unique_ptr<Channel> connectedChannel;
    ExpectOk(TryConnectToUdsChannel(name, connectedChannel));
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

TEST_F(TestUdsChannel, SendTwoFramesAtOnce) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server;
    ExpectOk(CreateUdsChannelServer(name, server));
    ExpectTrue(server);

    std::unique_ptr<Channel> connectedChannel;
    ExpectOk(TryConnectToUdsChannel(name, connectedChannel));
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

TEST_F(TestUdsChannel, Stream) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server;
    ExpectOk(CreateUdsChannelServer(name, server));
    ExpectTrue(server);

    std::unique_ptr<Channel> connectedChannel;
    ExpectOk(TryConnectToUdsChannel(name, connectedChannel));
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

    // Cleanup
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

TEST_F(TestUdsChannel, SendAndReceiveBigElement) {
    // Arrange
    std::string name = GenerateName();

    std::unique_ptr<ChannelServer> server;
    ExpectOk(CreateUdsChannelServer(name, server));
    ExpectTrue(server);

    std::unique_ptr<Channel> connectedChannel;
    ExpectOk(TryConnectToUdsChannel(name, connectedChannel));
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

    // Cleanup
    thread.join();
}

}  // namespace
