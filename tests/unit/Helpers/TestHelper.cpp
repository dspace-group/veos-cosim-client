// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "TestHelper.hpp"

#include <memory>
#include <string>
#include <thread>

#include <fmt/format.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Helper.hpp"

using namespace testing;

using namespace DsVeosCoSim;

[[nodiscard]] CoSimType GetCounterPart(CoSimType coSimType) {
    return coSimType == CoSimType::Client ? CoSimType::Server : CoSimType::Client;
}

[[nodiscard]] std::string GetCounterPart(const std::string& name, ConnectionKind connectionKind) {
    return connectionKind == ConnectionKind::Local ? name : fmt::format("Other{}", name);
}

[[nodiscard]] Result CreateBusExchange(CoSimType coSimType,
                                       ConnectionKind connectionKind,
                                       std::string_view name,
                                       const std::vector<CanController>& controllers,
                                       IProtocol& protocol,
                                       std::unique_ptr<BusExchange>& busExchange) {
    return CreateBusExchange(coSimType, connectionKind, name, controllers, {}, {}, {}, protocol, busExchange);
}

[[nodiscard]] Result CreateBusExchange(CoSimType coSimType,
                                       ConnectionKind connectionKind,
                                       std::string_view name,
                                       const std::vector<EthController>& controllers,
                                       IProtocol& protocol,
                                       std::unique_ptr<BusExchange>& busExchange) {
    return CreateBusExchange(coSimType, connectionKind, name, {}, controllers, {}, {}, protocol, busExchange);
}

[[nodiscard]] Result CreateBusExchange(CoSimType coSimType,
                                       ConnectionKind connectionKind,
                                       std::string_view name,
                                       const std::vector<LinController>& controllers,
                                       IProtocol& protocol,
                                       std::unique_ptr<BusExchange>& busExchange) {
    return CreateBusExchange(coSimType, connectionKind, name, {}, {}, controllers, {}, protocol, busExchange);
}

[[nodiscard]] Result CreateBusExchange(CoSimType coSimType,
                                       ConnectionKind connectionKind,
                                       std::string_view name,
                                       const std::vector<FrController>& controllers,
                                       IProtocol& protocol,
                                       std::unique_ptr<BusExchange>& busExchange) {
    return CreateBusExchange(coSimType, connectionKind, name, {}, {}, {}, controllers, protocol, busExchange);
}

#ifdef _WIN32

void TestSendAfterDisconnectOnRemoteClient(ShmPipeClient& client1, ShmPipeClient& client2) {
    // Arrange
    client1.Disconnect();

    size_t sendValue = GenerateSizeT();

    // Act
    Result result = client2.Send(&sendValue, sizeof(sendValue));

    // Assert
    AssertNotConnected(result);
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
    constexpr size_t Count = 0x1000;

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

namespace DsVeosCoSim {

std::ostream& operator<<(std::ostream& stream, SimulationTime simulationTime) {
    return stream << SimulationTimeToString(simulationTime);
}

std::ostream& operator<<(std::ostream& stream, Result result) {
    return stream << format_as(result);
}

std::ostream& operator<<(std::ostream& stream, CoSimType coSimType) {
    return stream << format_as(coSimType);
}

std::ostream& operator<<(std::ostream& stream, ConnectionKind connectionKind) {
    return stream << format_as(connectionKind);
}

std::ostream& operator<<(std::ostream& stream, Command command) {
    return stream << format_as(command);
}

std::ostream& operator<<(std::ostream& stream, Severity severity) {
    return stream << format_as(severity);
}

std::ostream& operator<<(std::ostream& stream, TerminateReason terminateReason) {
    return stream << format_as(terminateReason);
}

std::ostream& operator<<(std::ostream& stream, ConnectionState connectionState) {
    return stream << format_as(connectionState);
}

std::ostream& operator<<(std::ostream& stream, SimulationState simulationState) {
    return stream << format_as(simulationState);
}

std::ostream& operator<<(std::ostream& stream, Mode mode) {
    return stream << format_as(mode);
}

std::ostream& operator<<(std::ostream& stream, IoSignalId ioSignalId) {
    return stream << format_as(ioSignalId);
}

std::ostream& operator<<(std::ostream& stream, DataType dataType) {
    return stream << format_as(dataType);
}

std::ostream& operator<<(std::ostream& stream, SizeKind sizeKind) {
    return stream << format_as(sizeKind);
}

std::ostream& operator<<(std::ostream& stream, BusControllerId busControllerId) {
    return stream << format_as(busControllerId);
}

std::ostream& operator<<(std::ostream& stream, BusMessageId busMessageId) {
    return stream << format_as(busMessageId);
}

std::ostream& operator<<(std::ostream& stream, LinControllerType linControllerType) {
    return stream << format_as(linControllerType);
}

std::ostream& operator<<(std::ostream& stream, CanMessageFlags canMessageFlags) {
    return stream << format_as(canMessageFlags);
}

std::ostream& operator<<(std::ostream& stream, EthMessageFlags ethMessageFlags) {
    return stream << format_as(ethMessageFlags);
}

std::ostream& operator<<(std::ostream& stream, LinMessageFlags linMessageFlags) {
    return stream << format_as(linMessageFlags);
}

std::ostream& operator<<(std::ostream& stream, FrMessageFlags frMessageFlags) {
    return stream << format_as(frMessageFlags);
}

std::ostream& operator<<(std::ostream& stream, FrameKind frameKind) {
    return stream << format_as(frameKind);
}

std::ostream& operator<<(std::ostream& stream, const IoSignal& ioSignal) {
    return stream << format_as(ioSignal);
}

std::ostream& operator<<(std::ostream& stream, const IoSignalContainer& ioSignalContainer) {
    return stream << format_as(ioSignalContainer);
}

std::ostream& operator<<(std::ostream& stream, const CanController& canController) {
    return stream << format_as(canController);
}

std::ostream& operator<<(std::ostream& stream, const CanControllerContainer& canControllerContainer) {
    return stream << format_as(canControllerContainer);
}

std::ostream& operator<<(std::ostream& stream, const CanMessage& canMessage) {
    return stream << format_as(canMessage);
}

std::ostream& operator<<(std::ostream& stream, const CanMessageContainer& canMessageContainer) {
    return stream << format_as(canMessageContainer);
}

std::ostream& operator<<(std::ostream& stream, const EthController& ethController) {
    return stream << format_as(ethController);
}

std::ostream& operator<<(std::ostream& stream, const EthControllerContainer& ethControllerContainer) {
    return stream << format_as(ethControllerContainer);
}

std::ostream& operator<<(std::ostream& stream, const EthMessage& ethMessage) {
    return stream << format_as(ethMessage);
}

std::ostream& operator<<(std::ostream& stream, const EthMessageContainer& ethMessageContainer) {
    return stream << format_as(ethMessageContainer);
}

std::ostream& operator<<(std::ostream& stream, const LinController& linController) {
    return stream << format_as(linController);
}

std::ostream& operator<<(std::ostream& stream, const LinControllerContainer& frControllerContainer) {
    return stream << format_as(frControllerContainer);
}

std::ostream& operator<<(std::ostream& stream, const LinMessage& linMessage) {
    return stream << format_as(linMessage);
}

std::ostream& operator<<(std::ostream& stream, const LinMessageContainer& linMessageContainer) {
    return stream << format_as(linMessageContainer);
}

std::ostream& operator<<(std::ostream& stream, const FrController& frController) {
    return stream << format_as(frController);
}

std::ostream& operator<<(std::ostream& stream, const FrControllerContainer& frControllerContainer) {
    return stream << format_as(frControllerContainer);
}

std::ostream& operator<<(std::ostream& stream, const FrMessage& frMessage) {
    return stream << format_as(frMessage);
}

std::ostream& operator<<(std::ostream& stream, const FrMessageContainer& frMessageContainer) {
    return stream << format_as(frMessageContainer);
}

std::ostream& operator<<(std::ostream& stream, const std::vector<IoSignalContainer>& ioSignalContainers) {
    return stream << format_as(ioSignalContainers);
}

std::ostream& operator<<(std::ostream& stream, const std::vector<CanControllerContainer>& canControllerContainers) {
    return stream << format_as(canControllerContainers);
}

std::ostream& operator<<(std::ostream& stream, const std::vector<EthControllerContainer>& ethControllerContainers) {
    return stream << format_as(ethControllerContainers);
}

std::ostream& operator<<(std::ostream& stream, const std::vector<LinControllerContainer>& linControllerContainers) {
    return stream << format_as(linControllerContainers);
}

std::ostream& operator<<(std::ostream& stream, const std::vector<FrControllerContainer>& frControllerContainers) {
    return stream << format_as(frControllerContainers);
}

}  // namespace DsVeosCoSim
