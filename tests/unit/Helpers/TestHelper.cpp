// Copyright dSPACE GmbH. All rights reserved.

#include "TestHelper.h"

#include <fmt/format.h>

#include <gtest/gtest.h>

#include <string>

#include "Helper.h"

using namespace DsVeosCoSim;

namespace {

constexpr uint64_t BigNumber = 0x800000;

}  // namespace

[[nodiscard]] CoSimType GetCounterPart(CoSimType coSimType) {
    return coSimType == CoSimType::Client ? CoSimType::Server : CoSimType::Client;
}

[[nodiscard]] std::string GetCounterPart(const std::string& name, ConnectionKind connectionKind) {
    return connectionKind == ConnectionKind::Local ? name : fmt::format("Other{}", name);
}

void AssertByteArray(const void* expected, const void* actual, size_t size) {
    const auto* expectedBytes = static_cast<const uint8_t*>(expected);
    const auto* actualBytes = static_cast<const uint8_t*>(actual);
    for (size_t i = 0; i < size; i++) {
        AssertEq(expectedBytes[i], actualBytes[i]);
    }
}

void AssertLastMessage(const std::string& message) {
    AssertEqHelper(message, GetLastMessage());
}

void AssertEqHelper(const std::string& expected, const std::string& actual) {
    ASSERT_STREQ(expected.c_str(), actual.c_str());
}

void AssertNotEqHelper(const std::string& expected, const std::string& actual) {
    ASSERT_STRNE(expected.c_str(), actual.c_str());
}

[[nodiscard]] std::unique_ptr<Channel> AcceptFromServer(std::unique_ptr<ChannelServer>& server) {
    std::unique_ptr<Channel> acceptedChannel;
    ExpectOk(server->TryAccept(acceptedChannel));
    ExpectTrue(acceptedChannel);

    return std::move(acceptedChannel);
}

void TestWriteUInt16ToChannel(std::unique_ptr<Channel>& writeChannel) {
    uint16_t sendValue = GenerateU16();

    // Act and assert
    AssertOk(writeChannel->GetWriter().Write(sendValue));
    AssertOk(writeChannel->GetWriter().EndWrite());
}

void TestWriteUInt32ToChannel(std::unique_ptr<Channel>& writeChannel) {
    uint32_t sendValue = GenerateU32();

    // Act and assert
    AssertOk(writeChannel->GetWriter().Write(sendValue));
    AssertOk(writeChannel->GetWriter().EndWrite());
}

void TestWriteUInt64ToChannel(std::unique_ptr<Channel>& writeChannel) {
    uint64_t sendValue = GenerateU64();

    // Act and assert
    AssertOk(writeChannel->GetWriter().Write(sendValue));
    AssertOk(writeChannel->GetWriter().EndWrite());
}

void TestWriteBufferToChannel(std::unique_ptr<Channel>& writeChannel) {
    std::vector<uint8_t> buffer = GenerateBytes(10);

    // Act and assert
    AssertOk(writeChannel->GetWriter().Write(buffer.data(), buffer.size()));
    AssertOk(writeChannel->GetWriter().EndWrite());
}

void TestReadUInt16FromChannel(std::unique_ptr<Channel>& writeChannel,
                               std::unique_ptr<DsVeosCoSim::Channel>& readChannel) {
    uint16_t sendValue = GenerateU16();

    ExpectOk(writeChannel->GetWriter().Write(sendValue));
    ExpectOk(writeChannel->GetWriter().EndWrite());

    decltype(sendValue) receiveValue{};

    // Act
    AssertOk(readChannel->GetReader().Read(receiveValue));

    // Assert
    AssertEq(sendValue, receiveValue);
}

void TestReadUInt32FromChannel(std::unique_ptr<Channel>& writeChannel,
                               std::unique_ptr<DsVeosCoSim::Channel>& readChannel) {
    uint32_t sendValue = GenerateU32();

    ExpectOk(writeChannel->GetWriter().Write(sendValue));
    ExpectOk(writeChannel->GetWriter().EndWrite());

    decltype(sendValue) receiveValue{};

    // Act
    AssertOk(readChannel->GetReader().Read(receiveValue));

    // Assert
    AssertEq(sendValue, receiveValue);
}

void TestReadUInt64FromChannel(std::unique_ptr<Channel>& writeChannel,
                               std::unique_ptr<DsVeosCoSim::Channel>& readChannel) {
    uint64_t sendValue = GenerateU64();

    ExpectOk(writeChannel->GetWriter().Write(sendValue));
    ExpectOk(writeChannel->GetWriter().EndWrite());

    decltype(sendValue) receiveValue{};

    // Act
    AssertOk(readChannel->GetReader().Read(receiveValue));

    // Assert
    AssertEq(sendValue, receiveValue);
}

void TestReadBufferFromChannel(std::unique_ptr<Channel>& writeChannel,
                               std::unique_ptr<DsVeosCoSim::Channel>& readChannel) {
    std::vector<uint8_t> sendBuffer = GenerateBytes(10);

    ExpectOk(writeChannel->GetWriter().Write(sendBuffer.data(), sendBuffer.size()));
    ExpectOk(writeChannel->GetWriter().EndWrite());

    std::vector<uint8_t> receiveBuffer;
    receiveBuffer.resize(sendBuffer.size());

    // Act
    AssertOk(readChannel->GetReader().Read(receiveBuffer.data(), receiveBuffer.size()));

    // Assert
    AssertEqHelper(sendBuffer, receiveBuffer);
}

void TestPingPong(std::unique_ptr<DsVeosCoSim::Channel>& firstChannel,
                  std::unique_ptr<DsVeosCoSim::Channel>& secondChannel) {
    // Act and assert
    for (uint16_t i = 0; i < 100; i++) {
        Channel* sendChannel = firstChannel.get();
        Channel* receiveChannel = secondChannel.get();
        if (i % 2 == 1) {
            sendChannel = secondChannel.get();
            receiveChannel = firstChannel.get();
        }

        uint16_t sendValue = GenerateU16();
        AssertOk(sendChannel->GetWriter().Write(sendValue));
        AssertOk(sendChannel->GetWriter().EndWrite());

        uint16_t receiveValue{};
        AssertOk(receiveChannel->GetReader().Read(receiveValue));

        AssertEq(sendValue, receiveValue);
    }
}

void TestSendTwoFramesAtOnce(std::unique_ptr<DsVeosCoSim::Channel>& writeChannel,
                             std::unique_ptr<DsVeosCoSim::Channel>& readChannel) {
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
    AssertEq(sendValue1, receiveValue1);
    AssertEq(sendValue2, receiveValue2);
}

void TestStream(std::unique_ptr<DsVeosCoSim::Channel>& writeChannel,
                std::unique_ptr<DsVeosCoSim::Channel>& readChannel) {
    std::thread thread([&] {
        uint16_t firstValue{};
        ExpectOk(readChannel->GetReader().Read(firstValue));
        EXPECT_EQ(static_cast<uint16_t>(42), firstValue);

        for (uint64_t i = 0; i < BigNumber; i++) {
            uint64_t receiveValue{};
            ExpectOk(readChannel->GetReader().Read(receiveValue));

            AssertEq(i, receiveValue);
        }
    });

    // Act and assert
    AssertOk(
        writeChannel->GetWriter().Write(static_cast<uint16_t>(42)));  // Forcing the following elements to be unaligned
    for (uint64_t i = 0; i < BigNumber; i++) {
        AssertOk(writeChannel->GetWriter().Write(i));
    }

    AssertOk(writeChannel->GetWriter().EndWrite());

    thread.join();
}

void TestBigElement(std::unique_ptr<DsVeosCoSim::Channel>& writeChannel,
                    std::unique_ptr<DsVeosCoSim::Channel>& readChannel) {
    std::thread thread([&] {
        auto receiveArray = std::make_unique<std::array<uint32_t, BigNumber>>();
        ExpectOk(readChannel->GetReader().Read(receiveArray.get(), receiveArray->size() * 4));

        for (size_t i = 0; i < receiveArray->size(); i++) {
            auto expected = static_cast<uint32_t>(i);
            uint32_t actual = (*receiveArray)[i];
            AssertEq(expected, actual);
        }
    });

    auto sendArray = std::make_unique<std::array<uint32_t, BigNumber>>();
    for (size_t i = 0; i < sendArray->size(); i++) {
        (*sendArray)[i] = static_cast<uint32_t>(i);
    }

    // Act and assert
    AssertOk(writeChannel->GetWriter().Write(sendArray.get(), sendArray->size() * 4));
    AssertOk(writeChannel->GetWriter().EndWrite());

    thread.join();
}
