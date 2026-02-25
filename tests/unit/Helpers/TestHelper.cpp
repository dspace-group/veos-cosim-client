// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "TestHelper.hpp"

#include <fmt/format.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <thread>

#include "Helper.hpp"

using namespace testing;

namespace DsVeosCoSim {

[[nodiscard]] CoSimType GetCounterPart(CoSimType coSimType) {
    return coSimType == CoSimType::Client ? CoSimType::Server : CoSimType::Client;
}

[[nodiscard]] std::string GetCounterPart(const std::string& name, ConnectionKind connectionKind) {
    return connectionKind == ConnectionKind::Local ? name : fmt::format("Other{}", name);
}

void TestSendAfterDisconnect(SocketClient& client) {
    // Arrange
    client.Disconnect();

    size_t sendValue = GenerateSizeT();

    // Act
    Result result = client.Send(&sendValue, sizeof(sendValue));

    // Assert
    AssertNotConnected(result);
}

// TODO: It takes some time to detect a remote disconnect. How to force it?
// void TestSendAfterDisconnectOnRemoteClient(SocketClient& client1, SocketClient& client2) {
//     // Arrange
//     client1.Disconnect();

//     size_t sendValue = GenerateSizeT();

//     // Act
//     Result result = client2.Send(&sendValue, sizeof(sendValue));  // NOLINT

//     // Assert
//     AssertNotConnected(result);
// }

void TestReceiveAfterDisconnect(SocketClient& client) {
    // Arrange
    client.Disconnect();

    size_t receiveValue{};
    size_t receivedSize{};

    // Act
    Result result = client.Receive(&receiveValue, sizeof(receiveValue), receivedSize);

    // Assert
    AssertNotConnected(result);
}

void TestReceiveAfterDisconnectOnRemoteClient(SocketClient& client1, SocketClient& client2) {
    // Arrange
    client1.Disconnect();

    size_t receiveValue{};
    size_t receivedSize{};

    // Act
    Result result = client2.Receive(&receiveValue, sizeof(receiveValue), receivedSize);

    // Assert
    AssertNotConnected(result);
}

void TestSendAndReceive(SocketClient& client1, SocketClient& client2) {
    // Arrange
    size_t sendValue = GenerateSizeT();
    size_t receiveValue = 0;

    // Act
    Result sendResult = client1.Send(&sendValue, sizeof(sendValue));
    Result receiveResult = ReceiveComplete(client2, &receiveValue, sizeof(receiveValue));

    // Assert
    AssertOk(sendResult);
    AssertOk(receiveResult);
    ASSERT_EQ(sendValue, receiveValue);
}

void TestManyElements(SocketClient& client1, SocketClient& client2) {
    // Arrange
    constexpr size_t Count = 0x1000;

    std::thread thread([&] {
        for (size_t i = 0; i < Count; i++) {
            size_t receiveValue{};
            AssertOk(ReceiveComplete(client2, &receiveValue, sizeof(receiveValue)));
            ASSERT_EQ(i, receiveValue);
        }
    });

    // Act and assert
    for (size_t i = 0; i < Count; i++) {
        AssertOk(client1.Send(&i, sizeof(i)));
    }

    thread.join();
}

void TestBigElement(SocketClient& client1, SocketClient& client2) {
    // Arrange
    constexpr size_t Count = 0x100000;

    std::thread thread([&] {
        auto receiveArray = std::make_unique<std::array<size_t, Count>>();
        AssertOk(ReceiveComplete(client2, receiveArray->data(), receiveArray->size() * sizeof(size_t)));
        for (size_t i = 0; i < receiveArray->size(); i++) {
            ASSERT_EQ(i, (*receiveArray)[i]);
        }
    });

    // Act and assert
    auto sendArray = std::make_unique<std::array<size_t, Count>>();
    for (size_t i = 0; i < sendArray->size(); i++) {
        (*sendArray)[i] = i;
    }

    AssertOk(client1.Send(sendArray->data(), sendArray->size() * sizeof(size_t)));
    thread.join();
}

void TestPingPong(SocketClient& client1, SocketClient& client2) {
    // Arrange
    constexpr size_t Count = 100;

    // Act and assert
    for (size_t i = 0; i < Count; i++) {
        SocketClient* sendClient = &client1;
        SocketClient* receiveClient = &client2;
        if (i % 2 == 1) {
            std::swap(sendClient, receiveClient);
        }

        size_t sendValue = GenerateSizeT();
        AssertOk(sendClient->Send(&sendValue, sizeof(sendValue)));

        size_t receiveValue{};
        AssertOk(ReceiveComplete(*receiveClient, &receiveValue, sizeof(receiveValue)));

        ASSERT_EQ(sendValue, receiveValue);
    }
}

#ifdef _WIN32

void TestSendAfterDisconnect(ShmPipeClient& client) {
    // Arrange
    client.Disconnect();

    size_t sendValue = GenerateSizeT();

    // Act
    Result result = client.Send(&sendValue, sizeof(sendValue));

    // Assert
    AssertNotConnected(result);
}

void TestSendAfterDisconnectOnRemoteClient(ShmPipeClient& client1, ShmPipeClient& client2) {
    // Arrange
    client1.Disconnect();

    size_t sendValue = GenerateSizeT();

    // Act
    Result result = client2.Send(&sendValue, sizeof(sendValue));

    // Assert
    AssertNotConnected(result);
}

void TestReceiveAfterDisconnect(ShmPipeClient& client) {
    // Arrange
    client.Disconnect();

    size_t receiveValue{};
    size_t receivedSize{};

    // Act
    Result result = client.Receive(&receiveValue, sizeof(receiveValue), receivedSize);

    // Assert
    AssertNotConnected(result);
}

void TestReceiveAfterDisconnectOnRemoteClient(ShmPipeClient& client1, ShmPipeClient& client2) {
    // Arrange
    client1.Disconnect();

    size_t receiveValue{};
    size_t receivedSize{};

    // Act
    Result result = client2.Receive(&receiveValue, sizeof(receiveValue), receivedSize);

    // Assert
    AssertNotConnected(result);
}

void TestSendAndReceive(ShmPipeClient& client1, ShmPipeClient& client2) {
    // Arrange
    size_t sendValue = GenerateSizeT();
    size_t receiveValue = 0;

    // Act
    Result sendResult = client1.Send(&sendValue, sizeof(sendValue));
    Result receiveResult = ReceiveComplete(client2, &receiveValue, sizeof(receiveValue));

    // Assert
    AssertOk(sendResult);
    AssertOk(receiveResult);
    ASSERT_EQ(sendValue, receiveValue);
}

void TestManyElements(ShmPipeClient& client1, ShmPipeClient& client2) {
    // Arrange
    constexpr size_t Count = 0x1000;

    std::thread thread([&] {
        for (size_t i = 0; i < Count; i++) {
            size_t receiveValue{};
            AssertOk(ReceiveComplete(client2, &receiveValue, sizeof(receiveValue)));
            ASSERT_EQ(i, receiveValue);
        }
    });

    // Act and assert
    for (size_t i = 0; i < Count; i++) {
        AssertOk(client1.Send(&i, sizeof(i)));
    }

    thread.join();
}

void TestBigElement(ShmPipeClient& client1, ShmPipeClient& client2) {
    // Arrange
    constexpr size_t Count = 0x100000;

    std::thread thread([&] {
        auto receiveArray = std::make_unique<std::array<size_t, Count>>();
        AssertOk(ReceiveComplete(client2, receiveArray->data(), receiveArray->size() * sizeof(size_t)));
        for (size_t i = 0; i < receiveArray->size(); i++) {
            ASSERT_EQ(i, (*receiveArray)[i]);
        }
    });

    // Act and assert
    auto sendArray = std::make_unique<std::array<size_t, Count>>();
    for (size_t i = 0; i < sendArray->size(); i++) {
        (*sendArray)[i] = i;
    }

    AssertOk(client1.Send(sendArray->data(), sendArray->size() * sizeof(size_t)));
    thread.join();
}

void TestPingPong(ShmPipeClient& client1, ShmPipeClient& client2) {
    // Arrange
    constexpr size_t Count = 100;

    // Act and assert
    for (size_t i = 0; i < Count; i++) {
        ShmPipeClient* sendClient = &client1;
        ShmPipeClient* receiveClient = &client2;
        if (i % 2 == 1) {
            std::swap(sendClient, receiveClient);
        }

        size_t sendValue = GenerateSizeT();
        AssertOk(sendClient->Send(&sendValue, sizeof(sendValue)));

        size_t receiveValue{};
        AssertOk(ReceiveComplete(*receiveClient, &receiveValue, sizeof(receiveValue)));

        ASSERT_EQ(sendValue, receiveValue);
    }
}

#endif

void TestWriteUInt16ToChannel(std::unique_ptr<Channel>& writeChannel) {
    uint16_t sendValue = GenerateU16();

    // Act
    Result writeResult = writeChannel->GetWriter().Write(sendValue);
    Result endWriteResult = writeChannel->GetWriter().EndWrite();

    // Assert
    AssertOk(writeResult);
    AssertOk(endWriteResult);
}

void TestWriteUInt32ToChannel(std::unique_ptr<Channel>& writeChannel) {
    uint32_t sendValue = GenerateU32();

    // Act
    Result writeResult = writeChannel->GetWriter().Write(sendValue);
    Result endWriteResult = writeChannel->GetWriter().EndWrite();

    // Assert
    AssertOk(writeResult);
    AssertOk(endWriteResult);
}

void TestWriteUInt64ToChannel(std::unique_ptr<Channel>& writeChannel) {
    uint64_t sendValue = GenerateU64();

    // Act
    Result writeResult = writeChannel->GetWriter().Write(sendValue);
    Result endWriteResult = writeChannel->GetWriter().EndWrite();

    // Assert
    AssertOk(writeResult);
    AssertOk(endWriteResult);
}

void TestWriteBufferToChannel(std::unique_ptr<Channel>& writeChannel) {
    std::vector<uint8_t> buffer = GenerateBytes(10);

    // Act
    Result writeResult = writeChannel->GetWriter().Write(buffer.data(), buffer.size());
    Result endWriteResult = writeChannel->GetWriter().EndWrite();

    // Assert
    AssertOk(writeResult);
    AssertOk(endWriteResult);
}

void TestReadUInt16FromChannel(std::unique_ptr<Channel>& writeChannel, std::unique_ptr<Channel>& readChannel) {
    uint16_t sendValue = GenerateU16();

    AssertOk(writeChannel->GetWriter().Write(sendValue));
    AssertOk(writeChannel->GetWriter().EndWrite());

    decltype(sendValue) receiveValue{};

    // Act
    Result result = readChannel->GetReader().Read(receiveValue);

    // Assert
    AssertOk(result);
    ASSERT_EQ(sendValue, receiveValue);
}

void TestReadUInt32FromChannel(std::unique_ptr<Channel>& writeChannel, std::unique_ptr<Channel>& readChannel) {
    uint32_t sendValue = GenerateU32();

    AssertOk(writeChannel->GetWriter().Write(sendValue));
    AssertOk(writeChannel->GetWriter().EndWrite());

    decltype(sendValue) receiveValue{};

    // Act
    Result result = readChannel->GetReader().Read(receiveValue);

    // Assert
    AssertOk(result);
    ASSERT_EQ(sendValue, receiveValue);
}

void TestReadUInt64FromChannel(std::unique_ptr<Channel>& writeChannel, std::unique_ptr<Channel>& readChannel) {
    uint64_t sendValue = GenerateU64();

    AssertOk(writeChannel->GetWriter().Write(sendValue));
    AssertOk(writeChannel->GetWriter().EndWrite());

    decltype(sendValue) receiveValue{};

    // Act
    Result result = readChannel->GetReader().Read(receiveValue);

    // Assert
    AssertOk(result);
    ASSERT_EQ(sendValue, receiveValue);
}

void TestReadBufferFromChannel(std::unique_ptr<Channel>& writeChannel, std::unique_ptr<Channel>& readChannel) {
    std::vector<uint8_t> sendBuffer = GenerateBytes(10);

    AssertOk(writeChannel->GetWriter().Write(sendBuffer.data(), sendBuffer.size()));
    AssertOk(writeChannel->GetWriter().EndWrite());

    std::vector<uint8_t> receiveBuffer;
    receiveBuffer.resize(sendBuffer.size());

    // Act
    Result result = readChannel->GetReader().Read(receiveBuffer.data(), receiveBuffer.size());

    // Assert
    AssertOk(result);
    ASSERT_THAT(receiveBuffer, ContainerEq(sendBuffer));
}

void TestPingPong(std::unique_ptr<Channel>& firstChannel, std::unique_ptr<Channel>& secondChannel) {
    constexpr size_t Count = 100;

    // Act and assert
    for (size_t i = 0; i < Count; i++) {
        Channel* sendChannel = firstChannel.get();
        Channel* receiveChannel = secondChannel.get();
        if (i % 2 == 1) {
            sendChannel = secondChannel.get();
            receiveChannel = firstChannel.get();
        }

        size_t sendValue = GenerateSizeT();
        AssertOk(sendChannel->GetWriter().Write(sendValue));
        AssertOk(sendChannel->GetWriter().EndWrite());

        size_t receiveValue{};
        AssertOk(receiveChannel->GetReader().Read(receiveValue));

        ASSERT_EQ(sendValue, receiveValue);
    }
}

void TestSendTwoFramesAtOnce(std::unique_ptr<Channel>& writeChannel, std::unique_ptr<Channel>& readChannel) {
    uint32_t sendValue1 = GenerateU32();
    uint64_t sendValue2 = GenerateU64();
    uint32_t receiveValue1{};
    uint64_t receiveValue2{};

    // Act
    AssertOk(writeChannel->GetWriter().Write(sendValue1));
    AssertOk(writeChannel->GetWriter().EndWrite());

    AssertOk(writeChannel->GetWriter().Write(sendValue2));
    AssertOk(writeChannel->GetWriter().EndWrite());

    AssertOk(readChannel->GetReader().Read(receiveValue1));
    AssertOk(readChannel->GetReader().Read(receiveValue2));

    // Assert
    ASSERT_EQ(sendValue1, receiveValue1);
    ASSERT_EQ(sendValue2, receiveValue2);
}

void TestStream(std::unique_ptr<Channel>& writeChannel, std::unique_ptr<Channel>& readChannel) {
    constexpr size_t Count = 0x1000;

    std::thread thread([&] {
        uint16_t firstValue{};
        AssertOk(readChannel->GetReader().Read(firstValue));
        ASSERT_EQ(static_cast<uint16_t>(42), firstValue);

        for (size_t i = 0; i < Count; i++) {
            size_t receiveValue{};
            AssertOk(readChannel->GetReader().Read(receiveValue));

            ASSERT_EQ(i, receiveValue);
        }
    });

    // Act and assert
    AssertOk(writeChannel->GetWriter().Write(static_cast<uint16_t>(42)));  // Forcing the following elements to be unaligned
    for (size_t i = 0; i < Count; i++) {
        AssertOk(writeChannel->GetWriter().Write(i));
    }

    AssertOk(writeChannel->GetWriter().EndWrite());

    thread.join();
}

void TestBigElement(std::unique_ptr<Channel>& writeChannel, std::unique_ptr<Channel>& readChannel) {
    constexpr size_t Count = 0x100000;

    std::thread thread([&] {
        auto receiveArray = std::make_unique<std::array<size_t, Count>>();
        AssertOk(readChannel->GetReader().Read(receiveArray.get(), receiveArray->size() * sizeof(size_t)));

        for (size_t i = 0; i < receiveArray->size(); i++) {
            ASSERT_EQ(i, (*receiveArray)[i]);
        }
    });

    auto sendArray = std::make_unique<std::array<size_t, Count>>();
    for (size_t i = 0; i < sendArray->size(); i++) {
        (*sendArray)[i] = i;
    }

    // Act and assert
    AssertOk(writeChannel->GetWriter().Write(sendArray.get(), sendArray->size() * sizeof(size_t)));
    AssertOk(writeChannel->GetWriter().EndWrite());

    thread.join();
}

}  // namespace DsVeosCoSim
