// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <gtest/gtest.h>

#include <deque>
#include <memory>
#include <optional>
#include <string>
#include <thread>

#include "Channel.h"
#include "DsVeosCoSim/CoSimTypes.h"
#include "Helper.h"
#include "IoBuffer.h"
#include "TestHelper.h"

using namespace DsVeosCoSim;
using namespace testing;

namespace {

// #define SINGLE_TEST

#ifdef SINGLE_TEST
auto CoSimTypes = Values(CoSimType::Client);

auto IoBufferConnectionKinds = Values(ConnectionKind::Remote);

auto DataTypes = Values(DataType::Float64);
#else
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
#endif

struct EventData {
    IoSignalContainer signal{};
    std::vector<uint8_t> data;
};

void SwitchSignals(std::vector<IoSignal>& incomingSignals, std::vector<IoSignal>& outgoingSignals, CoSimType coSimType) {
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
                             return fmt::format("{}_{}", ToString(std::get<0>(info.param)), ToString(std::get<1>(info.param)));
                         });

TEST_P(TestIoBufferWithCoSimType, CreateWithZeroIoSignalInfos) {
    // Arrange
    auto [coSimType, connectionKind] = GetParam();

    std::string name = GenerateString("IoBuffer名前");

    std::unique_ptr<IoBuffer> ioBuffer;

    // Act
    AssertOk(CreateIoBuffer(coSimType, connectionKind, name, {}, {}, GetLatestProtocol(), ioBuffer));

    // Assert
    AssertTrue(ioBuffer);
}

class TestIoBuffer : public TestWithParam<std::tuple<CoSimType, ConnectionKind, DataType>> {
protected:
    static std::unique_ptr<Channel> _senderChannel;
    static std::unique_ptr<Channel> _receiverChannel;

    static void SetUpTestSuite() {
        std::unique_ptr<ChannelServer> remoteServer;
        ExpectOk(CreateTcpChannelServer(0, true, remoteServer));
        ExpectTrue(remoteServer);
        std::optional<uint16_t> port = remoteServer->GetLocalPort();
        ExpectTrue(port);

        ExpectOk(TryConnectToTcpChannel("127.0.0.1", *port, 0, DefaultTimeout, _senderChannel));
        ExpectTrue(_senderChannel);
        ExpectOk(remoteServer->TryAccept(_receiverChannel));
        ExpectTrue(_receiverChannel);
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
            AssertOk(readerIoBuffer.Deserialize(reader, GenerateSimulationTime(), {}));
        });

        AssertOk(writerIoBuffer.Serialize(writer));
        AssertOk(writer.EndWrite());

        thread.join();
    }

    static void TransferWithEvents(IoBuffer& writerIoBuffer, IoBuffer& readerIoBuffer, std::deque<EventData> expectedCallbacks) {
        ChannelReader& reader = _receiverChannel->GetReader();
        ChannelWriter& writer = _senderChannel->GetWriter();

        SimulationTime simulationTime = GenerateSimulationTime();

        Callbacks callbacks{};
        callbacks.incomingSignalChangedCallback = [&](SimulationTime simTime, const IoSignal& changedIoSignal, uint32_t length, const void* value) {
            AssertEq(simTime, static_cast<SimulationTime>(simulationTime));
            AssertFalse(expectedCallbacks.empty());
            auto [signal, data] = expectedCallbacks.front();
            AssertEq(signal.id, changedIoSignal.id);
            AssertEq(signal.length, length);
            AssertByteArray(data.data(), value, data.size());
            expectedCallbacks.pop_front();
        };

        std::thread thread([&] {
            AssertOk(readerIoBuffer.Deserialize(reader, simulationTime, callbacks));
        });

        AssertOk(writerIoBuffer.Serialize(writer));
        AssertOk(writer.EndWrite());

        thread.join();

        if (!expectedCallbacks.empty()) {
            throw std::runtime_error("Not all expected callbacks were called.");
        }
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

    std::string name = GenerateString("IoBuffer名前");

    IoSignalContainer incomingSignal = CreateSignal(dataType);
    IoSignalContainer outgoingSignal = CreateSignal(dataType);

    std::unique_ptr<IoBuffer> ioBuffer;

    // Act
    AssertOk(CreateIoBuffer(coSimType, connectionKind, name, {incomingSignal.Convert()}, {outgoingSignal.Convert()}, GetLatestProtocol(), ioBuffer));

    // Assert
    AssertTrue(ioBuffer);
}

TEST_P(TestIoBuffer, CreateWithMultipleIoSignalInfos) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("IoBuffer名前");

    IoSignalContainer incomingSignal1 = CreateSignal(dataType);
    IoSignalContainer incomingSignal2 = CreateSignal(dataType);
    IoSignalContainer outgoingSignal1 = CreateSignal(dataType);
    IoSignalContainer outgoingSignal2 = CreateSignal(dataType);

    std::unique_ptr<IoBuffer> ioBuffer;

    // Act
    AssertOk(CreateIoBuffer(coSimType,
                            connectionKind,
                            name,
                            {incomingSignal1.Convert(), incomingSignal2.Convert()},
                            {outgoingSignal1.Convert(), outgoingSignal2.Convert()},
                            GetLatestProtocol(),
                            ioBuffer));

    // Assert
    AssertTrue(ioBuffer);
}

TEST_P(TestIoBuffer, InitialDataOfFixedSizedSignal) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("IoBuffer名前");

    IoSignalContainer signal = CreateSignal(dataType, SizeKind::Fixed);

    std::vector incomingSignals = {signal.Convert()};
    std::vector<IoSignal> outgoingSignals;
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    std::unique_ptr<IoBuffer> ioBuffer;
    ExpectOk(CreateIoBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals, GetLatestProtocol(), ioBuffer));

    std::vector<uint8_t> initialValue = CreateZeroedIoData(signal);

    uint32_t readLength{};
    std::vector<uint8_t> readValue = CreateZeroedIoData(signal);

    // Act
    AssertOk(ioBuffer->Read(signal.id, readLength, readValue.data()));

    // Assert
    AssertEq(signal.length, readLength);
    AssertByteArray(initialValue.data(), readValue.data(), initialValue.size());
}

TEST_P(TestIoBuffer, InitialDataOfVariableSizedSignal) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("IoBuffer名前");

    IoSignalContainer signal = CreateSignal(dataType, SizeKind::Variable);

    std::vector incomingSignals = {signal.Convert()};
    std::vector<IoSignal> outgoingSignals;
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    std::unique_ptr<IoBuffer> ioBuffer;
    ExpectOk(CreateIoBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals, GetLatestProtocol(), ioBuffer));

    uint32_t readLength{};
    std::vector<uint8_t> readValue = CreateZeroedIoData(signal);

    // Act
    AssertOk(ioBuffer->Read(signal.id, readLength, readValue.data()));

    // Assert
    AssertEq(0U, readLength);
}

TEST_P(TestIoBuffer, WriteFixedSizedData) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("IoBuffer名前");

    IoSignalContainer signal = CreateSignal(dataType, SizeKind::Fixed);

    std::vector<IoSignal> incomingSignals;
    std::vector outgoingSignals = {signal.Convert()};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    std::unique_ptr<IoBuffer> ioBuffer;
    ExpectOk(CreateIoBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals, GetLatestProtocol(), ioBuffer));

    std::vector<uint8_t> writeValue = GenerateIoData(signal);

    // Act and assert
    AssertOk(ioBuffer->Write(signal.id, signal.length, writeValue.data()));
}

TEST_P(TestIoBuffer, WriteFixedSizedDataAndRead) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("IoBuffer名前");

    IoSignalContainer signal = CreateSignal(dataType, SizeKind::Fixed);
    IoSignalContainer signal1 = CreateSignal();

    std::vector<IoSignal> incomingSignals;
    std::vector outgoingSignals = {signal1.Convert(), signal.Convert()};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    std::unique_ptr<IoBuffer> writerIoBuffer;
    ExpectOk(CreateIoBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals, GetLatestProtocol(), writerIoBuffer));

    std::unique_ptr<IoBuffer> readerIoBuffer;
    ExpectOk(CreateIoBuffer(GetCounterPart(coSimType),
                            connectionKind,
                            GetCounterPart(name, connectionKind),
                            incomingSignals,
                            outgoingSignals,
                            GetLatestProtocol(),
                            readerIoBuffer));

    std::vector<uint8_t> writeValue = GenerateIoData(signal);
    ExpectOk(writerIoBuffer->Write(signal.id, signal.length, writeValue.data()));

    uint32_t readLength{};
    std::vector<uint8_t> readValue = CreateZeroedIoData(signal);

    Transfer(*writerIoBuffer, *readerIoBuffer);

    // Act
    AssertOk(readerIoBuffer->Read(signal.id, readLength, readValue.data()));

    // Assert
    AssertEq(signal.length, readLength);
    AssertByteArray(writeValue.data(), readValue.data(), writeValue.size());
}

TEST_P(TestIoBuffer, WriteFixedSizedDataTwiceAndReadLatestValue) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("IoBuffer名前");

    IoSignalContainer signal = CreateSignal(dataType, SizeKind::Fixed);
    IoSignalContainer signal1 = CreateSignal();

    std::vector<IoSignal> incomingSignals;
    std::vector outgoingSignals = {signal.Convert(), signal1.Convert()};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    std::unique_ptr<IoBuffer> writerIoBuffer;
    ExpectOk(CreateIoBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals, GetLatestProtocol(), writerIoBuffer));

    std::unique_ptr<IoBuffer> readerIoBuffer;
    ExpectOk(CreateIoBuffer(GetCounterPart(coSimType),
                            connectionKind,
                            GetCounterPart(name, connectionKind),
                            incomingSignals,
                            outgoingSignals,
                            GetLatestProtocol(),
                            readerIoBuffer));

    std::vector<uint8_t> writeValue = GenerateIoData(signal);
    ExpectOk(writerIoBuffer->Write(signal.id, signal.length, writeValue.data()));

    // Second write with different data
    writeValue = GenerateIoData(signal);
    ExpectOk(writerIoBuffer->Write(signal.id, signal.length, writeValue.data()));

    uint32_t readLength{};
    std::vector<uint8_t> readValue = CreateZeroedIoData(signal);

    Transfer(*writerIoBuffer, *readerIoBuffer);

    // Act
    AssertOk(readerIoBuffer->Read(signal.id, readLength, readValue.data()));

    // Assert
    AssertEq(signal.length, readLength);
    AssertByteArray(writeValue.data(), readValue.data(), writeValue.size());
}

TEST_P(TestIoBuffer, WriteFixedSizedDataAndReceiveEvent) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("IoBuffer名前");

    IoSignalContainer signal = CreateSignal(dataType, SizeKind::Fixed);
    IoSignalContainer signal1 = CreateSignal();
    IoSignalContainer signal2 = CreateSignal();

    std::vector<IoSignal> incomingSignals;
    std::vector outgoingSignals = {signal.Convert(), signal1.Convert(), signal2.Convert()};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    std::unique_ptr<IoBuffer> writerIoBuffer;
    ExpectOk(CreateIoBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals, GetLatestProtocol(), writerIoBuffer));

    std::unique_ptr<IoBuffer> readerIoBuffer;
    ExpectOk(CreateIoBuffer(GetCounterPart(coSimType),
                            connectionKind,
                            GetCounterPart(name, connectionKind),
                            incomingSignals,
                            outgoingSignals,
                            GetLatestProtocol(),
                            readerIoBuffer));

    // Act and assert
    for (uint32_t i = 0; i < 2; i++) {
        std::vector<uint8_t> writeValue = GenerateIoData(signal);
        AssertOk(writerIoBuffer->Write(signal.id, signal.length, writeValue.data()));
        TransferWithEvents(*writerIoBuffer, *readerIoBuffer, {{signal, writeValue}});
    }
}

TEST_P(TestIoBuffer, WriteFixedSizedDataTwiceAndReceiveOneEvent) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("IoBuffer名前");

    IoSignalContainer signal = CreateSignal(dataType, SizeKind::Fixed);
    IoSignalContainer signal1 = CreateSignal();
    IoSignalContainer signal2 = CreateSignal();

    std::vector<IoSignal> incomingSignals;
    std::vector outgoingSignals = {signal1.Convert(), signal2.Convert(), signal.Convert()};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    std::unique_ptr<IoBuffer> writerIoBuffer;
    ExpectOk(CreateIoBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals, GetLatestProtocol(), writerIoBuffer));

    std::unique_ptr<IoBuffer> readerIoBuffer;
    ExpectOk(CreateIoBuffer(GetCounterPart(coSimType),
                            connectionKind,
                            GetCounterPart(name, connectionKind),
                            incomingSignals,
                            outgoingSignals,
                            GetLatestProtocol(),
                            readerIoBuffer));

    // Act and assert
    for (uint32_t i = 0; i < 2; i++) {
        std::vector<uint8_t> writeValue = GenerateIoData(signal);
        AssertOk(writerIoBuffer->Write(signal.id, signal.length, writeValue.data()));

        // Second write with different data
        writeValue = GenerateIoData(signal);
        AssertOk(writerIoBuffer->Write(signal.id, signal.length, writeValue.data()));

        // Act and assert
        TransferWithEvents(*writerIoBuffer, *readerIoBuffer, {{signal, writeValue}});
    }
}

TEST_P(TestIoBuffer, NoNewEventIfFixedSizedDataDoesNotChangeWithSharedMemory) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("IoBuffer名前");

    IoSignalContainer signal = CreateSignal(dataType, SizeKind::Fixed);
    signal.length = GenerateRandom(2U, 10U);

    std::vector<IoSignal> incomingSignals;
    std::vector outgoingSignals = {signal.Convert()};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    std::unique_ptr<IoBuffer> writerIoBuffer;
    ExpectOk(CreateIoBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals, GetLatestProtocol(), writerIoBuffer));

    std::unique_ptr<IoBuffer> readerIoBuffer;
    ExpectOk(CreateIoBuffer(GetCounterPart(coSimType),
                            connectionKind,
                            GetCounterPart(name, connectionKind),
                            incomingSignals,
                            outgoingSignals,
                            GetLatestProtocol(),
                            readerIoBuffer));

    std::vector<uint8_t> writeValue = GenerateIoData(signal);
    ExpectOk(writerIoBuffer->Write(signal.id, signal.length, writeValue.data()));

    TransferWithEvents(*writerIoBuffer, *readerIoBuffer, {{signal, writeValue}});

    // Second write with same data
    ExpectOk(writerIoBuffer->Write(signal.id, signal.length, writeValue.data()));

    // Act and assert
    TransferWithEvents(*writerIoBuffer, *readerIoBuffer, {});
}

TEST_P(TestIoBuffer, WriteVariableSizedDataAndReceiveEvent) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("IoBuffer名前");

    IoSignalContainer signal = CreateSignal(dataType, SizeKind::Variable);

    std::vector<IoSignal> incomingSignals;
    std::vector outgoingSignals = {signal.Convert()};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    std::unique_ptr<IoBuffer> writerIoBuffer;
    ExpectOk(CreateIoBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals, GetLatestProtocol(), writerIoBuffer));

    std::unique_ptr<IoBuffer> readerIoBuffer;
    ExpectOk(CreateIoBuffer(GetCounterPart(coSimType),
                            connectionKind,
                            GetCounterPart(name, connectionKind),
                            incomingSignals,
                            outgoingSignals,
                            GetLatestProtocol(),
                            readerIoBuffer));

    // Act and assert
    for (uint32_t i = 0; i < 2; i++) {
        std::vector<uint8_t> writeValue = GenerateIoData(signal);
        AssertOk(writerIoBuffer->Write(signal.id, signal.length, writeValue.data()));

        TransferWithEvents(*writerIoBuffer, *readerIoBuffer, {{signal, writeValue}});
    }
}

TEST_P(TestIoBuffer, WriteVariableSizedDataWhereOnlyOneElementChangedAndReceiveEvent) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("IoBuffer名前");

    IoSignalContainer signal = CreateSignal(dataType, SizeKind::Variable);
    signal.length = GenerateRandom(2U, 10U);

    std::vector<IoSignal> incomingSignals;
    std::vector outgoingSignals = {signal.Convert()};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    std::unique_ptr<IoBuffer> writerIoBuffer;
    ExpectOk(CreateIoBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals, GetLatestProtocol(), writerIoBuffer));

    std::unique_ptr<IoBuffer> readerIoBuffer;
    ExpectOk(CreateIoBuffer(GetCounterPart(coSimType),
                            connectionKind,
                            GetCounterPart(name, connectionKind),
                            incomingSignals,
                            outgoingSignals,
                            GetLatestProtocol(),
                            readerIoBuffer));

    std::vector<uint8_t> writeValue = CreateZeroedIoData(signal);

    // Act and assert
    for (uint32_t i = 0; i < 2; i++) {
        // Only change one byte, so that only element is changed
        ++writeValue[0];
        AssertOk(writerIoBuffer->Write(signal.id, signal.length, writeValue.data()));

        // Act and assert
        TransferWithEvents(*writerIoBuffer, *readerIoBuffer, {{signal, writeValue}});
    }
}

TEST_P(TestIoBuffer, WriteVariableSizedDataWithOnlyChangedLengthAndReceiveEventWithSharedMemory) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("IoBuffer名前");

    IoSignalContainer signal = CreateSignal(dataType, SizeKind::Variable);
    signal.length = GenerateRandom(2U, 10U);

    std::vector<IoSignal> incomingSignals;
    std::vector outgoingSignals = {signal.Convert()};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    std::unique_ptr<IoBuffer> writerIoBuffer;
    ExpectOk(CreateIoBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals, GetLatestProtocol(), writerIoBuffer));

    std::unique_ptr<IoBuffer> readerIoBuffer;
    ExpectOk(CreateIoBuffer(GetCounterPart(coSimType),
                            connectionKind,
                            GetCounterPart(name, connectionKind),
                            incomingSignals,
                            outgoingSignals,
                            GetLatestProtocol(),
                            readerIoBuffer));

    IoSignalContainer signalCopy = signal;
    signalCopy.length--;

    std::vector<uint8_t> writeValue = GenerateIoData(signalCopy);
    ExpectOk(writerIoBuffer->Write(signal.id, signalCopy.length, writeValue.data()));

    // Act and assert
    TransferWithEvents(*writerIoBuffer, *readerIoBuffer, {{signalCopy, writeValue}});
}

TEST_P(TestIoBuffer, NoNewEventIfVariableSizedDataDoesNotChangeWithSharedMemory) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("IoBuffer名前");

    IoSignalContainer signal = CreateSignal(dataType, SizeKind::Variable);
    signal.length = GenerateRandom(2U, 10U);

    std::vector<IoSignal> incomingSignals;
    std::vector outgoingSignals = {signal.Convert()};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    std::unique_ptr<IoBuffer> writerIoBuffer;
    ExpectOk(CreateIoBuffer(coSimType, connectionKind, name, incomingSignals, outgoingSignals, GetLatestProtocol(), writerIoBuffer));

    std::unique_ptr<IoBuffer> readerIoBuffer;
    ExpectOk(CreateIoBuffer(GetCounterPart(coSimType),
                            connectionKind,
                            GetCounterPart(name, connectionKind),
                            incomingSignals,
                            outgoingSignals,
                            GetLatestProtocol(),
                            readerIoBuffer));

    std::vector<uint8_t> writeValue = GenerateIoData(signal);
    ExpectOk(writerIoBuffer->Write(signal.id, signal.length, writeValue.data()));

    TransferWithEvents(*writerIoBuffer, *readerIoBuffer, {{signal, writeValue}});

    // Second write with same data
    ExpectOk(writerIoBuffer->Write(signal.id, signal.length, writeValue.data()));

    // Act and assert
    TransferWithEvents(*writerIoBuffer, *readerIoBuffer, {});
}

}  // namespace
