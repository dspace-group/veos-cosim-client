// Copyright dSPACE GmbH. All rights reserved.

#include <gtest/gtest.h>

#include <deque>
#include <string>
#include <thread>

#include "CoSimTypes.h"
#include "DsVeosCoSim/DsVeosCoSim.h"
#include "Generator.h"
#include "Helper.h"
#include "IoBuffer.h"
#include "LogHelper.h"
#include "SocketChannel.h"
#include "TestHelper.h"

using namespace DsVeosCoSim;
using namespace testing;

auto coSimTypes = testing::Values(CoSimType::Client, CoSimType::Server);

auto ioBufferConnectionKinds = testing::Values(ConnectionKind::Local, ConnectionKind::Remote);

auto dataTypes = testing::Values(DsVeosCoSim_DataType_Bool,
                                 DsVeosCoSim_DataType_Int8,
                                 DsVeosCoSim_DataType_Int16,
                                 DsVeosCoSim_DataType_Int32,
                                 DsVeosCoSim_DataType_Int64,
                                 DsVeosCoSim_DataType_UInt8,
                                 DsVeosCoSim_DataType_UInt16,
                                 DsVeosCoSim_DataType_UInt32,
                                 DsVeosCoSim_DataType_UInt64,
                                 DsVeosCoSim_DataType_Float32,
                                 DsVeosCoSim_DataType_Float64);

void Transfer(IoBuffer& writerIoBuffer, IoBuffer& readerIoBuffer) {
    TcpChannelServer server(0, true);
    uint16_t port = server.GetLocalPort();

    SocketChannel senderChannel = ConnectToTcpChannel("127.0.0.1", port);
    SocketChannel receiverChannel = Accept(server);

    std::thread thread(
        [&] { ASSERT_TRUE(readerIoBuffer.Deserialize(receiverChannel.GetReader(), GenerateI64(), {})); });

    ASSERT_TRUE(writerIoBuffer.Serialize(senderChannel.GetWriter()));
    ASSERT_TRUE(senderChannel.GetWriter().EndWrite());

    thread.join();
}

struct EventData {
    IoSignal signal{};
    std::vector<uint8_t> data;
};

void TransferWithEvents(IoBuffer& writerIoBuffer, IoBuffer& readerIoBuffer, std::deque<EventData> expectedCallbacks) {
    TcpChannelServer server(0, true);
    uint16_t port = server.GetLocalPort();

    SocketChannel senderChannel = ConnectToTcpChannel("127.0.0.1", port);
    SocketChannel receiverChannel = Accept(server);

    DsVeosCoSim_SimulationTime simulationTime = GenerateI64();

    Callbacks callbacks{};
    callbacks.incomingSignalChangedCallback = [&](DsVeosCoSim_SimulationTime simTime,
                                                  const DsVeosCoSim_IoSignal& changedIoSignal,
                                                  uint32_t length,
                                                  const void* value) {
        ASSERT_EQ(simTime, simulationTime);
        ASSERT_FALSE(expectedCallbacks.empty());
        const auto [signal, data] = expectedCallbacks.front();
        ASSERT_EQ(signal.id, changedIoSignal.id);
        ASSERT_EQ(signal.length, length);
        AssertByteArray(data.data(), value, data.size());
        expectedCallbacks.pop_front();
    };

    std::thread thread(
        [&] { ASSERT_TRUE(readerIoBuffer.Deserialize(receiverChannel.GetReader(), simulationTime, callbacks)); });

    ASSERT_TRUE(writerIoBuffer.Serialize(senderChannel.GetWriter()));
    ASSERT_TRUE(senderChannel.GetWriter().EndWrite());

    thread.join();

    ASSERT_TRUE(expectedCallbacks.empty());
}

void SwitchSignals(std::vector<DsVeosCoSim_IoSignal>& incomingSignals,
                   std::vector<DsVeosCoSim_IoSignal>& outgoingSignals,
                   CoSimType coSimType) {
    if (coSimType == CoSimType::Server) {
        std::swap(incomingSignals, outgoingSignals);
    }
}

class TestIoBufferWithCoSimType : public testing::TestWithParam<std::tuple<CoSimType, ConnectionKind>> {
protected:
    void SetUp() override {
        ClearLastMessage();
    }
};

INSTANTIATE_TEST_SUITE_P(Test,
                         TestIoBufferWithCoSimType,
                         testing::Combine(coSimTypes, ioBufferConnectionKinds),
                         [](const testing::TestParamInfo<std::tuple<CoSimType, ConnectionKind>>& info) {
                             return fmt::format("{}_{}",
                                                ToString(std::get<0>(info.param)),
                                                ToString(std::get<1>(info.param)));
                         });

TEST_P(TestIoBufferWithCoSimType, CreateWithZeroIoSignalInfos) {
    // Arrange
    auto [coSimType, connectionKind] = GetParam();

    std::string name = GenerateString("IoBuffer名前");

    // Act and assert
    ASSERT_NO_THROW(IoBuffer(coSimType, connectionKind, name, {}, {}));
}

class TestIoBuffer : public testing::TestWithParam<std::tuple<CoSimType, ConnectionKind, DsVeosCoSim_DataType>> {
protected:
    void SetUp() override {
        ClearLastMessage();
    }
};

INSTANTIATE_TEST_SUITE_P(
    ,
    TestIoBuffer,
    testing::Combine(coSimTypes, ioBufferConnectionKinds, dataTypes),
    [](const testing::TestParamInfo<std::tuple<CoSimType, ConnectionKind, DsVeosCoSim_DataType>>& info) {
        return fmt::format("{}_{}_{}",
                           ToString(std::get<0>(info.param)),
                           ToString(std::get<1>(info.param)),
                           ToString(std::get<2>(info.param)));
    });

TEST_P(TestIoBuffer, CreateWithSingleIoSignalInfo) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("IoBuffer名前");

    IoSignal incomingSignal = CreateSignal(dataType);
    IoSignal outgoingSignal = CreateSignal(dataType);

    // Act and assert
    ASSERT_NO_THROW(IoBuffer(coSimType, connectionKind, name, {incomingSignal}, {outgoingSignal}));
}

TEST_P(TestIoBuffer, CreateWithMultipleIoSignalInfos) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("IoBuffer名前");

    IoSignal incomingSignal1 = CreateSignal(dataType);
    IoSignal incomingSignal2 = CreateSignal(dataType);
    IoSignal outgoingSignal1 = CreateSignal(dataType);
    IoSignal outgoingSignal2 = CreateSignal(dataType);

    // Act and assert
    ASSERT_NO_THROW(IoBuffer(coSimType,
                             connectionKind,
                             name,
                             {incomingSignal1, incomingSignal2},
                             {outgoingSignal1, outgoingSignal2}));
}

TEST_P(TestIoBuffer, InitialDataOfFixedSizedSignal) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("IoBuffer名前");

    IoSignal signal = CreateSignal(dataType, DsVeosCoSim_SizeKind_Fixed);

    std::vector<DsVeosCoSim_IoSignal> incomingSignals = {signal};
    std::vector<DsVeosCoSim_IoSignal> outgoingSignals;
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    IoBuffer ioBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals);

    std::vector<uint8_t> initialValue = CreateZeroedIoData(signal);

    uint32_t readLength{};
    std::vector<uint8_t> readValue = CreateZeroedIoData(signal);

    // Act
    ASSERT_NO_THROW(ioBuffer.Read(signal.id, readLength, readValue.data()));

    // Assert
    ASSERT_EQ(signal.length, readLength);
    AssertByteArray(initialValue.data(), readValue.data(), initialValue.size());
}

TEST_P(TestIoBuffer, InitialDataOfVariableSizedSignal) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("IoBuffer名前");

    IoSignal signal = CreateSignal(dataType, DsVeosCoSim_SizeKind_Variable);

    std::vector<DsVeosCoSim_IoSignal> incomingSignals = {signal};
    std::vector<DsVeosCoSim_IoSignal> outgoingSignals;
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    IoBuffer ioBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals);

    std::vector<uint8_t> initialValue = CreateZeroedIoData(signal);

    uint32_t readLength{};
    std::vector<uint8_t> readValue = CreateZeroedIoData(signal);

    uint32_t expectedReadLength = 0;

    // Act
    ASSERT_NO_THROW(ioBuffer.Read(signal.id, readLength, readValue.data()));

    // Assert
    ASSERT_EQ(expectedReadLength, readLength);
}

TEST_P(TestIoBuffer, WriteFixedSizedData) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("IoBuffer名前");

    IoSignal signal = CreateSignal(dataType, DsVeosCoSim_SizeKind_Fixed);

    std::vector<DsVeosCoSim_IoSignal> incomingSignals;
    std::vector<DsVeosCoSim_IoSignal> outgoingSignals = {signal};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    IoBuffer ioBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals);

    std::vector<uint8_t> writeValue = GenerateIoData(signal);

    // Act and assert
    ASSERT_NO_THROW(ioBuffer.Write(signal.id, signal.length, writeValue.data()));
}

TEST_P(TestIoBuffer, WriteFixedSizedDataAndRead) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("IoBuffer名前");

    IoSignal signal = CreateSignal(dataType, DsVeosCoSim_SizeKind_Fixed);
    IoSignal signal1 = CreateSignal();

    std::vector<DsVeosCoSim_IoSignal> incomingSignals;
    std::vector<DsVeosCoSim_IoSignal> outgoingSignals = {signal1, signal};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    IoBuffer writerIoBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals);

    IoBuffer readerIoBuffer(GetCounterPart(coSimType),
                            connectionKind,
                            GetCounterPart(name, connectionKind),
                            incomingSignals,
                            outgoingSignals);

    std::vector<uint8_t> writeValue = GenerateIoData(signal);
    writerIoBuffer.Write(signal.id, signal.length, writeValue.data());

    uint32_t readLength{};
    std::vector<uint8_t> readValue = CreateZeroedIoData(signal);

    Transfer(writerIoBuffer, readerIoBuffer);

    // Act
    ASSERT_NO_THROW(readerIoBuffer.Read(signal.id, readLength, readValue.data()));

    // Assert
    ASSERT_EQ(signal.length, readLength);
    AssertByteArray(writeValue.data(), readValue.data(), writeValue.size());
}

TEST_P(TestIoBuffer, WriteFixedSizedDataTwiceAndReadLatestValue) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("IoBuffer名前");

    IoSignal signal = CreateSignal(dataType, DsVeosCoSim_SizeKind_Fixed);
    IoSignal signal1 = CreateSignal();

    std::vector<DsVeosCoSim_IoSignal> incomingSignals;
    std::vector<DsVeosCoSim_IoSignal> outgoingSignals = {signal, signal1};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    IoBuffer writerIoBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals);

    IoBuffer readerIoBuffer(GetCounterPart(coSimType),
                            connectionKind,
                            GetCounterPart(name, connectionKind),
                            incomingSignals,
                            outgoingSignals);

    std::vector<uint8_t> writeValue = GenerateIoData(signal);
    writerIoBuffer.Write(signal.id, signal.length, writeValue.data());

    // Second write with different data
    writeValue = GenerateIoData(signal);
    writerIoBuffer.Write(signal.id, signal.length, writeValue.data());

    uint32_t readLength{};
    std::vector<uint8_t> readValue = CreateZeroedIoData(signal);

    Transfer(writerIoBuffer, readerIoBuffer);

    // Act
    ASSERT_NO_THROW(readerIoBuffer.Read(signal.id, readLength, readValue.data()));

    // Assert
    ASSERT_EQ(signal.length, readLength);
    AssertByteArray(writeValue.data(), readValue.data(), writeValue.size());
}

TEST_P(TestIoBuffer, WriteFixedSizedDataAndReceiveEvent) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("IoBuffer名前");

    IoSignal signal = CreateSignal(dataType, DsVeosCoSim_SizeKind_Fixed);
    IoSignal signal1 = CreateSignal();
    IoSignal signal2 = CreateSignal();

    std::vector<DsVeosCoSim_IoSignal> incomingSignals;
    std::vector<DsVeosCoSim_IoSignal> outgoingSignals = {signal, signal1, signal2};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    IoBuffer writerIoBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals);

    IoBuffer readerIoBuffer(GetCounterPart(coSimType),
                            connectionKind,
                            GetCounterPart(name, connectionKind),
                            incomingSignals,
                            outgoingSignals);

    // Act and assert
    for (uint32_t i = 0; i < 2; i++) {
        std::vector<uint8_t> writeValue = GenerateIoData(signal);
        writerIoBuffer.Write(signal.id, signal.length, writeValue.data());
        TransferWithEvents(writerIoBuffer, readerIoBuffer, {{signal, writeValue}});
    }
}

TEST_P(TestIoBuffer, WriteFixedSizedDataTwiceAndReceiveOneEvent) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("IoBuffer名前");

    IoSignal signal = CreateSignal(dataType, DsVeosCoSim_SizeKind_Fixed);
    IoSignal signal1 = CreateSignal();
    IoSignal signal2 = CreateSignal();

    std::vector<DsVeosCoSim_IoSignal> incomingSignals;
    std::vector<DsVeosCoSim_IoSignal> outgoingSignals = {signal1, signal2, signal};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    IoBuffer writerIoBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals);

    IoBuffer readerIoBuffer(GetCounterPart(coSimType),
                            connectionKind,
                            GetCounterPart(name, connectionKind),
                            incomingSignals,
                            outgoingSignals);

    // Act and assert
    for (uint32_t i = 0; i < 2; i++) {
        std::vector<uint8_t> writeValue = GenerateIoData(signal);
        writerIoBuffer.Write(signal.id, signal.length, writeValue.data());

        // Second write with different data
        writeValue = GenerateIoData(signal);
        writerIoBuffer.Write(signal.id, signal.length, writeValue.data());

        // Act and assert
        TransferWithEvents(writerIoBuffer, readerIoBuffer, {{signal, writeValue}});
    }
}

TEST_P(TestIoBuffer, NoNewEventIfFixedSizedDataDoesNotChangeWithSharedMemory) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("IoBuffer名前");

    IoSignal signal = CreateSignal(dataType, DsVeosCoSim_SizeKind_Fixed);
    signal.length = GenerateRandom(2U, 10U);

    std::vector<DsVeosCoSim_IoSignal> incomingSignals;
    std::vector<DsVeosCoSim_IoSignal> outgoingSignals = {signal};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    IoBuffer writerIoBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals);

    IoBuffer readerIoBuffer(GetCounterPart(coSimType),
                            connectionKind,
                            GetCounterPart(name, connectionKind),
                            incomingSignals,
                            outgoingSignals);

    std::vector<uint8_t> writeValue = GenerateIoData(signal);
    writerIoBuffer.Write(signal.id, signal.length, writeValue.data());

    TransferWithEvents(writerIoBuffer, readerIoBuffer, {{signal, writeValue}});

    // Second write with same data
    writerIoBuffer.Write(signal.id, signal.length, writeValue.data());

    // Act and assert
    TransferWithEvents(writerIoBuffer, readerIoBuffer, {});
}

TEST_P(TestIoBuffer, WriteVariableSizedDataAndReceiveEvent) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("IoBuffer名前");

    IoSignal signal = CreateSignal(dataType, DsVeosCoSim_SizeKind_Variable);

    std::vector<DsVeosCoSim_IoSignal> incomingSignals;
    std::vector<DsVeosCoSim_IoSignal> outgoingSignals = {signal};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    IoBuffer writerIoBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals);

    IoBuffer readerIoBuffer(GetCounterPart(coSimType),
                            connectionKind,
                            GetCounterPart(name, connectionKind),
                            incomingSignals,
                            outgoingSignals);

    // Act and assert
    for (uint32_t i = 0; i < 2; i++) {
        std::vector<uint8_t> writeValue = GenerateIoData(signal);
        writerIoBuffer.Write(signal.id, signal.length, writeValue.data());

        TransferWithEvents(writerIoBuffer, readerIoBuffer, {{signal, writeValue}});
    }
}

TEST_P(TestIoBuffer, WriteVariableSizedDataWhereOnlyOneElementChangedAndReceiveEvent) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("IoBuffer名前");

    IoSignal signal = CreateSignal(dataType, DsVeosCoSim_SizeKind_Variable);
    signal.length = GenerateRandom(2U, 10U);

    std::vector<DsVeosCoSim_IoSignal> incomingSignals;
    std::vector<DsVeosCoSim_IoSignal> outgoingSignals = {signal};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    IoBuffer writerIoBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals);

    IoBuffer readerIoBuffer(GetCounterPart(coSimType),
                            connectionKind,
                            GetCounterPart(name, connectionKind),
                            incomingSignals,
                            outgoingSignals);

    std::vector<uint8_t> writeValue = CreateZeroedIoData(signal);

    // Act and assert
    for (uint32_t i = 0; i < 2; i++) {
        // Only change one byte, so that only element is changed
        ++writeValue[0];
        writerIoBuffer.Write(signal.id, signal.length, writeValue.data());

        // Act and assert
        TransferWithEvents(writerIoBuffer, readerIoBuffer, {{signal, writeValue}});
    }
}

TEST_P(TestIoBuffer, WriteVariableSizedDataWithOnlyChangedLengthAndReceiveEventWithSharedMemory) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("IoBuffer名前");

    IoSignal signal = CreateSignal(dataType, DsVeosCoSim_SizeKind_Variable);
    signal.length = GenerateRandom(2U, 10U);

    std::vector<DsVeosCoSim_IoSignal> incomingSignals;
    std::vector<DsVeosCoSim_IoSignal> outgoingSignals = {signal};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    IoBuffer writerIoBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals);

    IoBuffer readerIoBuffer(GetCounterPart(coSimType),
                            connectionKind,
                            GetCounterPart(name, connectionKind),
                            incomingSignals,
                            outgoingSignals);

    IoSignal signalCopy = signal;
    signalCopy.length--;

    std::vector<uint8_t> writeValue = GenerateIoData(signalCopy);
    writerIoBuffer.Write(signal.id, signalCopy.length, writeValue.data());

    // Act and assert
    TransferWithEvents(writerIoBuffer, readerIoBuffer, {{signalCopy, writeValue}});
}

TEST_P(TestIoBuffer, NoNewEventIfVariableSizedDataDoesNotChangeWithSharedMemory) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("IoBuffer名前");

    IoSignal signal = CreateSignal(dataType, DsVeosCoSim_SizeKind_Variable);
    signal.length = GenerateRandom(2U, 10U);

    std::vector<DsVeosCoSim_IoSignal> incomingSignals;
    std::vector<DsVeosCoSim_IoSignal> outgoingSignals = {signal};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    IoBuffer writerIoBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals);

    IoBuffer readerIoBuffer(GetCounterPart(coSimType),
                            connectionKind,
                            GetCounterPart(name, connectionKind),
                            incomingSignals,
                            outgoingSignals);

    std::vector<uint8_t> writeValue = GenerateIoData(signal);
    writerIoBuffer.Write(signal.id, signal.length, writeValue.data());

    TransferWithEvents(writerIoBuffer, readerIoBuffer, {{signal, writeValue}});

    // Second write with same data
    writerIoBuffer.Write(signal.id, signal.length, writeValue.data());

    // Act and assert
    TransferWithEvents(writerIoBuffer, readerIoBuffer, {});
}
