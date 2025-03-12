// Copyright dSPACE GmbH. All rights reserved.

#include <gtest/gtest.h>

#include <deque>
#include <memory>
#include <string>
#include <thread>

#include "Channel.h"
#include "DsVeosCoSim/CoSimTypes.h"
#include "Generator.h"
#include "Helper.h"
#include "IoBuffer.h"
#include "LogHelper.h"
#include "TestHelper.h"

using namespace DsVeosCoSim;
using namespace testing;

namespace {

auto CoSimTypes = Values(CoSimType::Client, CoSimType::Server);

auto IoBufferConnectionKinds = Values(ConnectionKind::Local, ConnectionKind::Remote);

auto DataTypes = Values(DataType::Bool,
                        DataType::Int8,
                        DataType::Int16,
                        DataType::Int32,
                        DataType::Int64,
                        DataType::UInt8,
                        DataType::UInt16,
                        DataType::UInt32,
                        DataType::UInt64,
                        DataType::Float32,
                        DataType::Float64);

struct EventData {
    IoSignalContainer signal{};
    std::vector<uint8_t> data;
};

void SwitchSignals(std::vector<IoSignal>& incomingSignals,
                   std::vector<IoSignal>& outgoingSignals,
                   const CoSimType coSimType) {
    if (coSimType == CoSimType::Server) {
        std::swap(incomingSignals, outgoingSignals);
    }
}

class TestIoBufferWithCoSimType : public TestWithParam<std::tuple<CoSimType, ConnectionKind>> {
protected:
    void SetUp() override {
        ClearLastMessage();
    }
};

INSTANTIATE_TEST_SUITE_P(Test,
                         TestIoBufferWithCoSimType,
                         testing::Combine(CoSimTypes, IoBufferConnectionKinds),
                         [](const testing::TestParamInfo<std::tuple<CoSimType, ConnectionKind>>& info) {
                             return fmt::format("{}_{}",
                                                ToString(std::get<0>(info.param)),
                                                ToString(std::get<1>(info.param)));
                         });

TEST_P(TestIoBufferWithCoSimType, CreateWithZeroIoSignalInfos) {
    // Arrange
    auto [coSimType, connectionKind] = GetParam();

    const std::string name = GenerateString("IoBuffer名前");

    // Act
    std::unique_ptr<IoBuffer> ioBuffer = CreateIoBuffer(coSimType, connectionKind, name, {}, {});

    // Assert
    ASSERT_TRUE(ioBuffer);
}

class TestIoBuffer : public TestWithParam<std::tuple<CoSimType, ConnectionKind, DataType>> {
protected:
    static std::unique_ptr<Channel> _senderChannel;
    static std::unique_ptr<Channel> _receiverChannel;

    static void SetUpTestSuite() {
        std::unique_ptr<ChannelServer> remoteServer = CreateTcpChannelServer(0, true);
        const uint16_t port = remoteServer->GetLocalPort();

        _senderChannel = ConnectToTcpChannel("127.0.0.1", port);
        _receiverChannel = Accept(*remoteServer);
    }

    static void TearDownTestSuite() {
        _senderChannel->Disconnect();
        _receiverChannel->Disconnect();

        _senderChannel.reset();
        _receiverChannel.reset();
    }

    void SetUp() override {
        ClearLastMessage();
    }

    static void Transfer(IoBuffer& writerIoBuffer, IoBuffer& readerIoBuffer) {
        ChannelReader& reader = _receiverChannel->GetReader();
        ChannelWriter& writer = _senderChannel->GetWriter();

        std::thread thread([&] {
            ASSERT_TRUE(readerIoBuffer.Deserialize(reader, GenerateSimulationTime(), {}));
        });

        ASSERT_TRUE(writerIoBuffer.Serialize(writer));
        ASSERT_TRUE(writer.EndWrite());

        thread.join();
    }

    static void TransferWithEvents(IoBuffer& writerIoBuffer,
                                   IoBuffer& readerIoBuffer,
                                   std::deque<EventData> expectedCallbacks) {
        ChannelReader& reader = _receiverChannel->GetReader();
        ChannelWriter& writer = _senderChannel->GetWriter();

        SimulationTime simulationTime = GenerateSimulationTime();

        Callbacks callbacks{};
        callbacks.incomingSignalChangedCallback = [&](const SimulationTime simTime,
                                                      const IoSignal& changedIoSignal,
                                                      const uint32_t length,
                                                      const void* value) {
            ASSERT_EQ(simTime, simulationTime);
            ASSERT_FALSE(expectedCallbacks.empty());
            const auto [signal, data] = expectedCallbacks.front();
            ASSERT_EQ(signal.id, changedIoSignal.id);
            ASSERT_EQ(signal.length, length);
            AssertByteArray(data.data(), value, data.size());
            expectedCallbacks.pop_front();
        };

        std::thread thread([&] {
            ASSERT_TRUE(readerIoBuffer.Deserialize(reader, simulationTime, callbacks));
        });

        ASSERT_TRUE(writerIoBuffer.Serialize(writer));
        ASSERT_TRUE(writer.EndWrite());

        thread.join();

        ASSERT_TRUE(expectedCallbacks.empty());
    }
};

std::unique_ptr<Channel> TestIoBuffer::_senderChannel;
std::unique_ptr<Channel> TestIoBuffer::_receiverChannel;

INSTANTIATE_TEST_SUITE_P(,
                         TestIoBuffer,
                         testing::Combine(CoSimTypes, IoBufferConnectionKinds, DataTypes),
                         [](const testing::TestParamInfo<std::tuple<CoSimType, ConnectionKind, DataType>>& info) {
                             return fmt::format("{}_{}_{}",
                                                ToString(std::get<0>(info.param)),
                                                ToString(std::get<1>(info.param)),
                                                ToString(std::get<2>(info.param)));
                         });

TEST_P(TestIoBuffer, CreateWithSingleIoSignalInfo) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    const std::string name = GenerateString("IoBuffer名前");

    const IoSignalContainer incomingSignal = CreateSignal(dataType);
    const IoSignalContainer outgoingSignal = CreateSignal(dataType);

    // Act and assert
    ASSERT_NO_THROW((void)CreateIoBuffer(coSimType,
                                         connectionKind,
                                         name,
                                         {static_cast<IoSignal>(incomingSignal)},
                                         {static_cast<IoSignal>(outgoingSignal)}));
}

TEST_P(TestIoBuffer, CreateWithMultipleIoSignalInfos) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    const std::string name = GenerateString("IoBuffer名前");

    const IoSignalContainer incomingSignal1 = CreateSignal(dataType);
    const IoSignalContainer incomingSignal2 = CreateSignal(dataType);
    const IoSignalContainer outgoingSignal1 = CreateSignal(dataType);
    const IoSignalContainer outgoingSignal2 = CreateSignal(dataType);

    // Act and assert
    ASSERT_NO_THROW(
        (void)CreateIoBuffer(coSimType,
                             connectionKind,
                             name,
                             {static_cast<IoSignal>(incomingSignal1), static_cast<IoSignal>(incomingSignal2)},
                             {static_cast<IoSignal>(outgoingSignal1), static_cast<IoSignal>(outgoingSignal2)}));
}

TEST_P(TestIoBuffer, InitialDataOfFixedSizedSignal) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    const std::string name = GenerateString("IoBuffer名前");

    const IoSignalContainer signal = CreateSignal(dataType, SizeKind::Fixed);

    std::vector incomingSignals = {static_cast<IoSignal>(signal)};
    std::vector<IoSignal> outgoingSignals;
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    const std::unique_ptr<IoBuffer> ioBuffer =
        CreateIoBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals);

    const std::vector<uint8_t> initialValue = CreateZeroedIoData(signal);

    uint32_t readLength{};
    std::vector<uint8_t> readValue = CreateZeroedIoData(signal);

    // Act
    ASSERT_NO_THROW(ioBuffer->Read(signal.id, readLength, readValue.data()));

    // Assert
    ASSERT_EQ(signal.length, readLength);
    AssertByteArray(initialValue.data(), readValue.data(), initialValue.size());
}

TEST_P(TestIoBuffer, InitialDataOfVariableSizedSignal) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    const std::string name = GenerateString("IoBuffer名前");

    const IoSignalContainer signal = CreateSignal(dataType, SizeKind::Variable);

    std::vector incomingSignals = {static_cast<IoSignal>(signal)};
    std::vector<IoSignal> outgoingSignals;
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    const std::unique_ptr<IoBuffer> ioBuffer =
        CreateIoBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals);

    uint32_t readLength{};
    std::vector<uint8_t> readValue = CreateZeroedIoData(signal);

    constexpr uint32_t expectedReadLength = 0;

    // Act
    ASSERT_NO_THROW(ioBuffer->Read(signal.id, readLength, readValue.data()));

    // Assert
    ASSERT_EQ(expectedReadLength, readLength);
}

TEST_P(TestIoBuffer, WriteFixedSizedData) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    const std::string name = GenerateString("IoBuffer名前");

    const IoSignalContainer signal = CreateSignal(dataType, SizeKind::Fixed);

    std::vector<IoSignal> incomingSignals;
    std::vector outgoingSignals = {static_cast<IoSignal>(signal)};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    const std::unique_ptr<IoBuffer> ioBuffer =
        CreateIoBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals);

    const std::vector<uint8_t> writeValue = GenerateIoData(signal);

    // Act and assert
    ASSERT_NO_THROW(ioBuffer->Write(signal.id, signal.length, writeValue.data()));
}

TEST_P(TestIoBuffer, WriteFixedSizedDataAndRead) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("IoBuffer名前");

    IoSignalContainer signal = CreateSignal(dataType, SizeKind::Fixed);
    IoSignalContainer signal1 = CreateSignal();

    std::vector<IoSignal> incomingSignals;
    std::vector outgoingSignals = {static_cast<IoSignal>(signal1), static_cast<IoSignal>(signal)};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    std::unique_ptr<IoBuffer> writerIoBuffer =
        CreateIoBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals);

    std::unique_ptr<IoBuffer> readerIoBuffer = CreateIoBuffer(GetCounterPart(coSimType),
                                                              connectionKind,
                                                              GetCounterPart(name, connectionKind),
                                                              incomingSignals,
                                                              outgoingSignals);

    std::vector<uint8_t> writeValue = GenerateIoData(signal);
    writerIoBuffer->Write(signal.id, signal.length, writeValue.data());

    uint32_t readLength{};
    std::vector<uint8_t> readValue = CreateZeroedIoData(signal);

    Transfer(*writerIoBuffer, *readerIoBuffer);

    // Act
    ASSERT_NO_THROW(readerIoBuffer->Read(signal.id, readLength, readValue.data()));

    // Assert
    ASSERT_EQ(signal.length, readLength);
    AssertByteArray(writeValue.data(), readValue.data(), writeValue.size());
}

TEST_P(TestIoBuffer, WriteFixedSizedDataTwiceAndReadLatestValue) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    const std::string name = GenerateString("IoBuffer名前");

    IoSignalContainer signal = CreateSignal(dataType, SizeKind::Fixed);
    IoSignalContainer signal1 = CreateSignal();

    std::vector<IoSignal> incomingSignals;
    std::vector outgoingSignals = {static_cast<IoSignal>(signal), static_cast<IoSignal>(signal1)};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    std::unique_ptr<IoBuffer> writerIoBuffer =
        CreateIoBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals);

    std::unique_ptr<IoBuffer> readerIoBuffer = CreateIoBuffer(GetCounterPart(coSimType),
                                                              connectionKind,
                                                              GetCounterPart(name, connectionKind),
                                                              incomingSignals,
                                                              outgoingSignals);

    std::vector<uint8_t> writeValue = GenerateIoData(signal);
    writerIoBuffer->Write(signal.id, signal.length, writeValue.data());

    // Second write with different data
    writeValue = GenerateIoData(signal);
    writerIoBuffer->Write(signal.id, signal.length, writeValue.data());

    uint32_t readLength{};
    std::vector<uint8_t> readValue = CreateZeroedIoData(signal);

    Transfer(*writerIoBuffer, *readerIoBuffer);

    // Act
    ASSERT_NO_THROW(readerIoBuffer->Read(signal.id, readLength, readValue.data()));

    // Assert
    ASSERT_EQ(signal.length, readLength);
    AssertByteArray(writeValue.data(), readValue.data(), writeValue.size());
}

TEST_P(TestIoBuffer, WriteFixedSizedDataAndReceiveEvent) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    const std::string name = GenerateString("IoBuffer名前");

    const IoSignalContainer signal = CreateSignal(dataType, SizeKind::Fixed);
    const IoSignalContainer signal1 = CreateSignal();
    const IoSignalContainer signal2 = CreateSignal();

    std::vector<IoSignal> incomingSignals;
    std::vector outgoingSignals = {static_cast<IoSignal>(signal),
                                   static_cast<IoSignal>(signal1),
                                   static_cast<IoSignal>(signal2)};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    std::unique_ptr<IoBuffer> writerIoBuffer =
        CreateIoBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals);

    std::unique_ptr<IoBuffer> readerIoBuffer = CreateIoBuffer(GetCounterPart(coSimType),
                                                              connectionKind,
                                                              GetCounterPart(name, connectionKind),
                                                              incomingSignals,
                                                              outgoingSignals);

    // Act and assert
    for (uint32_t i = 0; i < 2; i++) {
        std::vector<uint8_t> writeValue = GenerateIoData(signal);
        writerIoBuffer->Write(signal.id, signal.length, writeValue.data());
        TransferWithEvents(*writerIoBuffer, *readerIoBuffer, {{signal, writeValue}});
    }
}

TEST_P(TestIoBuffer, WriteFixedSizedDataTwiceAndReceiveOneEvent) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    const std::string name = GenerateString("IoBuffer名前");

    const IoSignalContainer signal = CreateSignal(dataType, SizeKind::Fixed);
    const IoSignalContainer signal1 = CreateSignal();
    const IoSignalContainer signal2 = CreateSignal();

    std::vector<IoSignal> incomingSignals;
    std::vector outgoingSignals = {static_cast<IoSignal>(signal1),
                                   static_cast<IoSignal>(signal2),
                                   static_cast<IoSignal>(signal)};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    std::unique_ptr<IoBuffer> writerIoBuffer =
        CreateIoBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals);

    std::unique_ptr<IoBuffer> readerIoBuffer = CreateIoBuffer(GetCounterPart(coSimType),
                                                              connectionKind,
                                                              GetCounterPart(name, connectionKind),
                                                              incomingSignals,
                                                              outgoingSignals);

    // Act and assert
    for (uint32_t i = 0; i < 2; i++) {
        std::vector<uint8_t> writeValue = GenerateIoData(signal);
        writerIoBuffer->Write(signal.id, signal.length, writeValue.data());

        // Second write with different data
        writeValue = GenerateIoData(signal);
        writerIoBuffer->Write(signal.id, signal.length, writeValue.data());

        // Act and assert
        TransferWithEvents(*writerIoBuffer, *readerIoBuffer, {{signal, writeValue}});
    }
}

TEST_P(TestIoBuffer, NoNewEventIfFixedSizedDataDoesNotChangeWithSharedMemory) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    const std::string name = GenerateString("IoBuffer名前");

    IoSignalContainer signal = CreateSignal(dataType, SizeKind::Fixed);
    signal.length = GenerateRandom(2U, 10U);

    std::vector<IoSignal> incomingSignals;
    std::vector outgoingSignals = {static_cast<IoSignal>(signal)};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    std::unique_ptr<IoBuffer> writerIoBuffer =
        CreateIoBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals);

    std::unique_ptr<IoBuffer> readerIoBuffer = CreateIoBuffer(GetCounterPart(coSimType),
                                                              connectionKind,
                                                              GetCounterPart(name, connectionKind),
                                                              incomingSignals,
                                                              outgoingSignals);

    const std::vector<uint8_t> writeValue = GenerateIoData(signal);
    writerIoBuffer->Write(signal.id, signal.length, writeValue.data());

    TransferWithEvents(*writerIoBuffer, *readerIoBuffer, {{signal, writeValue}});

    // Second write with same data
    writerIoBuffer->Write(signal.id, signal.length, writeValue.data());

    // Act and assert
    TransferWithEvents(*writerIoBuffer, *readerIoBuffer, {});
}

TEST_P(TestIoBuffer, WriteVariableSizedDataAndReceiveEvent) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    const std::string name = GenerateString("IoBuffer名前");

    const IoSignalContainer signal = CreateSignal(dataType, SizeKind::Variable);

    std::vector<IoSignal> incomingSignals;
    std::vector outgoingSignals = {static_cast<IoSignal>(signal)};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    std::unique_ptr<IoBuffer> writerIoBuffer =
        CreateIoBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals);

    std::unique_ptr<IoBuffer> readerIoBuffer = CreateIoBuffer(GetCounterPart(coSimType),
                                                              connectionKind,
                                                              GetCounterPart(name, connectionKind),
                                                              incomingSignals,
                                                              outgoingSignals);

    // Act and assert
    for (uint32_t i = 0; i < 2; i++) {
        std::vector<uint8_t> writeValue = GenerateIoData(signal);
        writerIoBuffer->Write(signal.id, signal.length, writeValue.data());

        TransferWithEvents(*writerIoBuffer, *readerIoBuffer, {{signal, writeValue}});
    }
}

TEST_P(TestIoBuffer, WriteVariableSizedDataWhereOnlyOneElementChangedAndReceiveEvent) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    const std::string name = GenerateString("IoBuffer名前");

    IoSignalContainer signal = CreateSignal(dataType, SizeKind::Variable);
    signal.length = GenerateRandom(2U, 10U);

    std::vector<IoSignal> incomingSignals;
    std::vector outgoingSignals = {static_cast<IoSignal>(signal)};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    std::unique_ptr<IoBuffer> writerIoBuffer =
        CreateIoBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals);

    std::unique_ptr<IoBuffer> readerIoBuffer = CreateIoBuffer(GetCounterPart(coSimType),
                                                              connectionKind,
                                                              GetCounterPart(name, connectionKind),
                                                              incomingSignals,
                                                              outgoingSignals);

    std::vector<uint8_t> writeValue = CreateZeroedIoData(signal);

    // Act and assert
    for (uint32_t i = 0; i < 2; i++) {
        // Only change one byte, so that only element is changed
        ++writeValue[0];
        writerIoBuffer->Write(signal.id, signal.length, writeValue.data());

        // Act and assert
        TransferWithEvents(*writerIoBuffer, *readerIoBuffer, {{signal, writeValue}});
    }
}

TEST_P(TestIoBuffer, WriteVariableSizedDataWithOnlyChangedLengthAndReceiveEventWithSharedMemory) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    const std::string name = GenerateString("IoBuffer名前");

    IoSignalContainer signal = CreateSignal(dataType, SizeKind::Variable);
    signal.length = GenerateRandom(2U, 10U);

    std::vector<IoSignal> incomingSignals;
    std::vector outgoingSignals = {static_cast<IoSignal>(signal)};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    std::unique_ptr<IoBuffer> writerIoBuffer =
        CreateIoBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals);

    std::unique_ptr<IoBuffer> readerIoBuffer = CreateIoBuffer(GetCounterPart(coSimType),
                                                              connectionKind,
                                                              GetCounterPart(name, connectionKind),
                                                              incomingSignals,
                                                              outgoingSignals);

    IoSignalContainer signalCopy = signal;
    signalCopy.length--;

    const std::vector<uint8_t> writeValue = GenerateIoData(signalCopy);
    writerIoBuffer->Write(signal.id, signalCopy.length, writeValue.data());

    // Act and assert
    TransferWithEvents(*writerIoBuffer, *readerIoBuffer, {{signalCopy, writeValue}});
}

TEST_P(TestIoBuffer, NoNewEventIfVariableSizedDataDoesNotChangeWithSharedMemory) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    const std::string name = GenerateString("IoBuffer名前");

    IoSignalContainer signal = CreateSignal(dataType, SizeKind::Variable);
    signal.length = GenerateRandom(2U, 10U);

    std::vector<IoSignal> incomingSignals;
    std::vector outgoingSignals = {static_cast<IoSignal>(signal)};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    std::unique_ptr<IoBuffer> writerIoBuffer =
        CreateIoBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals);

    std::unique_ptr<IoBuffer> readerIoBuffer = CreateIoBuffer(GetCounterPart(coSimType),
                                                              connectionKind,
                                                              GetCounterPart(name, connectionKind),
                                                              incomingSignals,
                                                              outgoingSignals);

    const std::vector<uint8_t> writeValue = GenerateIoData(signal);
    writerIoBuffer->Write(signal.id, signal.length, writeValue.data());

    TransferWithEvents(*writerIoBuffer, *readerIoBuffer, {{signal, writeValue}});

    // Second write with same data
    writerIoBuffer->Write(signal.id, signal.length, writeValue.data());

    // Act and assert
    TransferWithEvents(*writerIoBuffer, *readerIoBuffer, {});
}

}  // namespace
