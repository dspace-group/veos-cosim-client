// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <deque>
#include <memory>
#include <string>

#include <fmt/format.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Channel.hpp"
#include "CoSimTypes.hpp"
#include "Helper.hpp"
#include "SignalExchange.hpp"
#include "Protocol.hpp"
#include "TestHelper.hpp"

using namespace DsVeosCoSim;
using namespace testing;

namespace {

// #define SINGLE_TEST

#ifdef SINGLE_TEST
auto CoSimTypes = Values(CoSimType::Client);

auto SignalExchangeConnectionKinds = Values(ConnectionKind::Remote);

auto DataTypes = Values(DataType::Float64);
#else
auto CoSimTypes = Values(CoSimType::Client, CoSimType::Server);

auto SignalExchangeConnectionKinds = Values(ConnectionKind::Local, ConnectionKind::Remote);

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

class TestSignalExchangeWithCoSimType : public TestWithParam<std::tuple<CoSimType, ConnectionKind>> {
protected:
    std::unique_ptr<IProtocol> _protocol;

    void SetUp() override {
        AssertOk(CreateProtocol(ProtocolVersionLatest, _protocol));
    }
};

INSTANTIATE_TEST_SUITE_P(Test,
                         TestSignalExchangeWithCoSimType,
                         testing::Combine(CoSimTypes, SignalExchangeConnectionKinds),
                         [](const testing::TestParamInfo<std::tuple<CoSimType, ConnectionKind>>& info) {
                             return fmt::format("{}_{}", std::get<0>(info.param), std::get<1>(info.param));
                         });

TEST_P(TestSignalExchangeWithCoSimType, CreateWithZeroIoSignalInfos) {
    // Arrange
    auto [coSimType, connectionKind] = GetParam();

    std::string name = GenerateString("SignalExchange名前");

    std::unique_ptr<SignalExchange> signalExchange;

    // Act
    Result result = CreateSignalExchange(coSimType, connectionKind, name, {}, {}, *_protocol, signalExchange);

    // Assert
    AssertOk(result);
}

class TestSignalExchange : public TestWithParam<std::tuple<CoSimType, ConnectionKind, DataType>> {
protected:
    static std::unique_ptr<Channel> _senderChannel;
    static std::unique_ptr<Channel> _receiverChannel;
    std::unique_ptr<IProtocol> _protocol;

    static void SetUpTestSuite() {
        std::unique_ptr<ChannelServer> remoteServer;
        AssertOk(CreateTcpChannelServer(0, true, remoteServer));
        uint16_t port = remoteServer->GetLocalPort();

        AssertOk(TryConnectToTcpChannel("127.0.0.1", port, 0, DefaultTimeout, _senderChannel));
        AssertOk(remoteServer->TryAccept(_receiverChannel));
    }

    static void TearDownTestSuite() {
        _senderChannel->Disconnect();
        _receiverChannel->Disconnect();

        _senderChannel.reset();
        _receiverChannel.reset();
    }

    void SetUp() override {
        AssertOk(CreateProtocol(ProtocolVersionLatest, _protocol));
    }

    static void Transfer(SignalExchange& writerSignalExchange, SignalExchange& readerSignalExchange) {
        ChannelReader& reader = _receiverChannel->GetReader();
        ChannelWriter& writer = _senderChannel->GetWriter();

        AssertOk(writerSignalExchange.Serialize(writer));
        AssertOk(writer.EndWrite());

        AssertOk(readerSignalExchange.Deserialize(reader, GenerateSimulationTime(), {}));
    }

    static void TransferWithEvents(SignalExchange& writerSignalExchange, SignalExchange& readerSignalExchange, std::deque<EventData> expectedCallbacks) {
        ChannelReader& reader = _receiverChannel->GetReader();
        ChannelWriter& writer = _senderChannel->GetWriter();

        SimulationTime simulationTime = GenerateSimulationTime();

        Callbacks callbacks{};
        callbacks.incomingSignalChangedCallback = [&](SimulationTime simTime, const IoSignal& changedIoSignal, uint32_t length, const void* value) {
            ASSERT_EQ(simTime, simulationTime);
            ASSERT_FALSE(expectedCallbacks.empty());
            auto [signal, data] = expectedCallbacks.front();
            ASSERT_EQ(signal.id, changedIoSignal.id);
            ASSERT_EQ(signal.length, length);
            std::vector<uint8_t> receivedData;
            receivedData.resize(length * GetDataTypeSize(changedIoSignal.dataType));
            memcpy(receivedData.data(), value, receivedData.size());
            ASSERT_THAT(receivedData, ContainerEq(data));
            expectedCallbacks.pop_front();
        };

        AssertOk(writerSignalExchange.Serialize(writer));
        AssertOk(writer.EndWrite());

        AssertOk(readerSignalExchange.Deserialize(reader, simulationTime, callbacks));

        ASSERT_TRUE(expectedCallbacks.empty()) << "Not all expected callbacks were called.";
    }
};

std::unique_ptr<Channel> TestSignalExchange::_senderChannel;
std::unique_ptr<Channel> TestSignalExchange::_receiverChannel;

INSTANTIATE_TEST_SUITE_P(,
                         TestSignalExchange,
                         testing::Combine(CoSimTypes, SignalExchangeConnectionKinds, DataTypes),
                         [](const testing::TestParamInfo<std::tuple<CoSimType, ConnectionKind, DataType>>& info) {
                             return fmt::format("{}_{}_{}", std::get<0>(info.param), std::get<1>(info.param), std::get<2>(info.param));
                         });

TEST_P(TestSignalExchange, CreateWithSingleIoSignalInfo) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("SignalExchange名前");

    IoSignalContainer incomingSignal = CreateSignal(dataType);
    IoSignalContainer outgoingSignal = CreateSignal(dataType);

    std::unique_ptr<SignalExchange> signalExchange;

    // Act
    Result result = CreateSignalExchange(coSimType, connectionKind, name, {incomingSignal.Convert()}, {outgoingSignal.Convert()}, *_protocol, signalExchange);

    // Assert
    AssertOk(result);
}

TEST_P(TestSignalExchange, CreateWithMultipleIoSignalInfos) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("SignalExchange名前");

    IoSignalContainer incomingSignal1 = CreateSignal(dataType);
    IoSignalContainer incomingSignal2 = CreateSignal(dataType);
    IoSignalContainer outgoingSignal1 = CreateSignal(dataType);
    IoSignalContainer outgoingSignal2 = CreateSignal(dataType);

    std::unique_ptr<SignalExchange> signalExchange;

    // Act
    Result result = (CreateSignalExchange(coSimType,
                                    connectionKind,
                                    name,
                                    {incomingSignal1.Convert(), incomingSignal2.Convert()},
                                    {outgoingSignal1.Convert(), outgoingSignal2.Convert()},
                                    *_protocol,
                                    signalExchange));

    // Assert
    AssertOk(result);
}

TEST_P(TestSignalExchange, InitialDataOfFixedSizedSignal) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("SignalExchange名前");

    IoSignalContainer signal = CreateSignal(dataType, SizeKind::Fixed);

    std::vector incomingSignals = {signal.Convert()};
    std::vector<IoSignal> outgoingSignals;
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    std::unique_ptr<SignalExchange> signalExchange;
    AssertOk(CreateSignalExchange(coSimType, connectionKind, name, incomingSignals, outgoingSignals, *_protocol, signalExchange));

    std::vector<uint8_t> initialValue = CreateZeroedIoData(signal);

    uint32_t readLength{};
    std::vector<uint8_t> readValue = CreateZeroedIoData(signal);

    // Act
    Result result = signalExchange->Read(signal.id, readLength, readValue.data());

    // Assert
    AssertOk(result);
    ASSERT_EQ(signal.length, readLength);
    ASSERT_THAT(readValue, ContainerEq(initialValue));
}

TEST_P(TestSignalExchange, InitialDataOfVariableSizedSignal) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("SignalExchange名前");

    IoSignalContainer signal = CreateSignal(dataType, SizeKind::Variable);

    std::vector incomingSignals = {signal.Convert()};
    std::vector<IoSignal> outgoingSignals;
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    std::unique_ptr<SignalExchange> signalExchange;
    AssertOk(CreateSignalExchange(coSimType, connectionKind, name, incomingSignals, outgoingSignals, *_protocol, signalExchange));

    uint32_t readLength{};
    std::vector<uint8_t> readValue = CreateZeroedIoData(signal);

    // Act
    Result result = signalExchange->Read(signal.id, readLength, readValue.data());

    // Assert
    AssertOk(result);
    ASSERT_EQ(0U, readLength);
}

TEST_P(TestSignalExchange, WriteFixedSizedData) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("SignalExchange名前");

    IoSignalContainer signal = CreateSignal(dataType, SizeKind::Fixed);

    std::vector<IoSignal> incomingSignals;
    std::vector outgoingSignals = {signal.Convert()};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    std::unique_ptr<SignalExchange> signalExchange;
    AssertOk(CreateSignalExchange(coSimType, connectionKind, name, incomingSignals, outgoingSignals, *_protocol, signalExchange));

    std::vector<uint8_t> writeValue = GenerateIoData(signal);

    // Act
    Result result = signalExchange->Write(signal.id, signal.length, writeValue.data());

    // Assert
    AssertOk(result);
}

TEST_P(TestSignalExchange, WriteFixedSizedDataAndRead) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("SignalExchange名前");

    IoSignalContainer signal = CreateSignal(dataType, SizeKind::Fixed);
    IoSignalContainer signal1 = CreateSignal();

    std::vector<IoSignal> incomingSignals;
    std::vector outgoingSignals = {signal1.Convert(), signal.Convert()};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    std::unique_ptr<SignalExchange> writerSignalExchange;
    AssertOk(CreateSignalExchange(coSimType, connectionKind, name, incomingSignals, outgoingSignals, *_protocol, writerSignalExchange));

    std::unique_ptr<SignalExchange> readerSignalExchange;
    AssertOk(CreateSignalExchange(GetCounterPart(coSimType),
                            connectionKind,
                            GetCounterPart(name, connectionKind),
                            incomingSignals,
                            outgoingSignals,
                            *_protocol,
                            readerSignalExchange));

    std::vector<uint8_t> writeValue = GenerateIoData(signal);
    AssertOk(writerSignalExchange->Write(signal.id, signal.length, writeValue.data()));

    uint32_t readLength{};
    std::vector<uint8_t> readValue = CreateZeroedIoData(signal);

    Transfer(*writerSignalExchange, *readerSignalExchange);

    // Act
    AssertOk(readerSignalExchange->Read(signal.id, readLength, readValue.data()));

    // Assert
    ASSERT_EQ(signal.length, readLength);
    ASSERT_THAT(readValue, ContainerEq(writeValue));
}

TEST_P(TestSignalExchange, WriteFixedSizedDataTwiceAndReadLatestValue) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("SignalExchange名前");

    IoSignalContainer signal = CreateSignal(dataType, SizeKind::Fixed);
    IoSignalContainer signal1 = CreateSignal();

    std::vector<IoSignal> incomingSignals;
    std::vector outgoingSignals = {signal.Convert(), signal1.Convert()};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    std::unique_ptr<SignalExchange> writerSignalExchange;
    AssertOk(CreateSignalExchange(coSimType, connectionKind, name, incomingSignals, outgoingSignals, *_protocol, writerSignalExchange));

    std::unique_ptr<SignalExchange> readerSignalExchange;
    AssertOk(CreateSignalExchange(GetCounterPart(coSimType),
                            connectionKind,
                            GetCounterPart(name, connectionKind),
                            incomingSignals,
                            outgoingSignals,
                            *_protocol,
                            readerSignalExchange));

    std::vector<uint8_t> writeValue = GenerateIoData(signal);
    AssertOk(writerSignalExchange->Write(signal.id, signal.length, writeValue.data()));

    // Second write with different data
    writeValue = GenerateIoData(signal);
    AssertOk(writerSignalExchange->Write(signal.id, signal.length, writeValue.data()));

    uint32_t readLength{};
    std::vector<uint8_t> readValue = CreateZeroedIoData(signal);

    Transfer(*writerSignalExchange, *readerSignalExchange);

    // Act
    AssertOk(readerSignalExchange->Read(signal.id, readLength, readValue.data()));

    // Assert
    ASSERT_EQ(signal.length, readLength);
    ASSERT_THAT(readValue, ContainerEq(writeValue));
}

TEST_P(TestSignalExchange, WriteFixedSizedDataAndReceiveEvent) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("SignalExchange名前");

    IoSignalContainer signal = CreateSignal(dataType, SizeKind::Fixed);
    IoSignalContainer signal1 = CreateSignal();
    IoSignalContainer signal2 = CreateSignal();

    std::vector<IoSignal> incomingSignals;
    std::vector outgoingSignals = {signal.Convert(), signal1.Convert(), signal2.Convert()};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    std::unique_ptr<SignalExchange> writerSignalExchange;
    AssertOk(CreateSignalExchange(coSimType, connectionKind, name, incomingSignals, outgoingSignals, *_protocol, writerSignalExchange));

    std::unique_ptr<SignalExchange> readerSignalExchange;
    AssertOk(CreateSignalExchange(GetCounterPart(coSimType),
                            connectionKind,
                            GetCounterPart(name, connectionKind),
                            incomingSignals,
                            outgoingSignals,
                            *_protocol,
                            readerSignalExchange));

    std::vector<uint8_t> writeValue = GenerateIoData(signal);

    // Act and assert
    AssertOk(writerSignalExchange->Write(signal.id, signal.length, writeValue.data()));
    TransferWithEvents(*writerSignalExchange, *readerSignalExchange, {{signal, writeValue}});
}

TEST_P(TestSignalExchange, WriteFixedSizedDataTwiceAndReceiveOneEvent) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("SignalExchange名前");

    IoSignalContainer signal = CreateSignal(dataType, SizeKind::Fixed);
    IoSignalContainer signal1 = CreateSignal();
    IoSignalContainer signal2 = CreateSignal();

    std::vector<IoSignal> incomingSignals;
    std::vector outgoingSignals = {signal1.Convert(), signal2.Convert(), signal.Convert()};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    std::unique_ptr<SignalExchange> writerSignalExchange;
    AssertOk(CreateSignalExchange(coSimType, connectionKind, name, incomingSignals, outgoingSignals, *_protocol, writerSignalExchange));

    std::unique_ptr<SignalExchange> readerSignalExchange;
    AssertOk(CreateSignalExchange(GetCounterPart(coSimType),
                            connectionKind,
                            GetCounterPart(name, connectionKind),
                            incomingSignals,
                            outgoingSignals,
                            *_protocol,
                            readerSignalExchange));

    std::vector<uint8_t> writeValue = GenerateIoData(signal);

    // Act and assert
    AssertOk(writerSignalExchange->Write(signal.id, signal.length, writeValue.data()));

    // Second write with different data
    writeValue[0]++;
    AssertOk(writerSignalExchange->Write(signal.id, signal.length, writeValue.data()));

    // Act and assert
    TransferWithEvents(*writerSignalExchange, *readerSignalExchange, {{signal, writeValue}});
}

TEST_P(TestSignalExchange, NoNewEventIfFixedSizedDataDoesNotChangeWithSharedMemory) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("SignalExchange名前");

    IoSignalContainer signal = CreateSignal(dataType, SizeKind::Fixed);
    signal.length = GenerateRandom(2U, 10U);

    std::vector<IoSignal> incomingSignals;
    std::vector outgoingSignals = {signal.Convert()};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    std::unique_ptr<SignalExchange> writerSignalExchange;
    AssertOk(CreateSignalExchange(coSimType, connectionKind, name, incomingSignals, outgoingSignals, *_protocol, writerSignalExchange));

    std::unique_ptr<SignalExchange> readerSignalExchange;
    AssertOk(CreateSignalExchange(GetCounterPart(coSimType),
                            connectionKind,
                            GetCounterPart(name, connectionKind),
                            incomingSignals,
                            outgoingSignals,
                            *_protocol,
                            readerSignalExchange));

    std::vector<uint8_t> writeValue = GenerateIoData(signal);
    AssertOk(writerSignalExchange->Write(signal.id, signal.length, writeValue.data()));

    TransferWithEvents(*writerSignalExchange, *readerSignalExchange, {{signal, writeValue}});

    // Second write with same data
    AssertOk(writerSignalExchange->Write(signal.id, signal.length, writeValue.data()));

    // Act and assert
    TransferWithEvents(*writerSignalExchange, *readerSignalExchange, {});
}

TEST_P(TestSignalExchange, WriteVariableSizedDataAndReceiveEvent) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("SignalExchange名前");

    IoSignalContainer signal = CreateSignal(dataType, SizeKind::Variable);

    std::vector<IoSignal> incomingSignals;
    std::vector outgoingSignals = {signal.Convert()};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    std::unique_ptr<SignalExchange> writerSignalExchange;
    AssertOk(CreateSignalExchange(coSimType, connectionKind, name, incomingSignals, outgoingSignals, *_protocol, writerSignalExchange));

    std::unique_ptr<SignalExchange> readerSignalExchange;
    AssertOk(CreateSignalExchange(GetCounterPart(coSimType),
                            connectionKind,
                            GetCounterPart(name, connectionKind),
                            incomingSignals,
                            outgoingSignals,
                            *_protocol,
                            readerSignalExchange));

    std::vector<uint8_t> writeValue = GenerateIoData(signal);

    // Act and assert
    AssertOk(writerSignalExchange->Write(signal.id, signal.length, writeValue.data()));

    TransferWithEvents(*writerSignalExchange, *readerSignalExchange, {{signal, writeValue}});
}

TEST_P(TestSignalExchange, WriteVariableSizedDataWhereOnlyOneElementChangedAndReceiveEvent) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("SignalExchange名前");

    IoSignalContainer signal = CreateSignal(dataType, SizeKind::Variable);
    signal.length = GenerateRandom(2U, 10U);

    std::vector<IoSignal> incomingSignals;
    std::vector outgoingSignals = {signal.Convert()};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    std::unique_ptr<SignalExchange> writerSignalExchange;
    AssertOk(CreateSignalExchange(coSimType, connectionKind, name, incomingSignals, outgoingSignals, *_protocol, writerSignalExchange));

    std::unique_ptr<SignalExchange> readerSignalExchange;
    AssertOk(CreateSignalExchange(GetCounterPart(coSimType),
                            connectionKind,
                            GetCounterPart(name, connectionKind),
                            incomingSignals,
                            outgoingSignals,
                            *_protocol,
                            readerSignalExchange));

    std::vector<uint8_t> writeValue = GenerateIoData(signal);
    AssertOk(writerSignalExchange->Write(signal.id, signal.length, writeValue.data()));

    TransferWithEvents(*writerSignalExchange, *readerSignalExchange, {{signal, writeValue}});

    // Act and assert

    ++writeValue[0];  // Only change one byte, so that only element is changed
    AssertOk(writerSignalExchange->Write(signal.id, signal.length, writeValue.data()));

    TransferWithEvents(*writerSignalExchange, *readerSignalExchange, {{signal, writeValue}});
}

TEST_P(TestSignalExchange, WriteVariableSizedDataWithOnlyChangedLengthAndReceiveEventWithSharedMemory) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("SignalExchange名前");

    IoSignalContainer signal = CreateSignal(dataType, SizeKind::Variable);
    signal.length = GenerateRandom(2U, 10U);

    std::vector<IoSignal> incomingSignals;
    std::vector outgoingSignals = {signal.Convert()};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    std::unique_ptr<SignalExchange> writerSignalExchange;
    AssertOk(CreateSignalExchange(coSimType, connectionKind, name, incomingSignals, outgoingSignals, *_protocol, writerSignalExchange));

    std::unique_ptr<SignalExchange> readerSignalExchange;
    AssertOk(CreateSignalExchange(GetCounterPart(coSimType),
                            connectionKind,
                            GetCounterPart(name, connectionKind),
                            incomingSignals,
                            outgoingSignals,
                            *_protocol,
                            readerSignalExchange));

    IoSignalContainer signalCopy = signal;
    signalCopy.length--;

    std::vector<uint8_t> writeValue = GenerateIoData(signalCopy);
    AssertOk(writerSignalExchange->Write(signal.id, signalCopy.length, writeValue.data()));

    // Act and assert
    TransferWithEvents(*writerSignalExchange, *readerSignalExchange, {{signalCopy, writeValue}});
}

TEST_P(TestSignalExchange, NoNewEventIfVariableSizedDataDoesNotChangeWithSharedMemory) {
    // Arrange
    auto [coSimType, connectionKind, dataType] = GetParam();

    std::string name = GenerateString("SignalExchange名前");

    IoSignalContainer signal = CreateSignal(dataType, SizeKind::Variable);
    signal.length = GenerateRandom(2U, 10U);

    std::vector<IoSignal> incomingSignals;
    std::vector outgoingSignals = {signal.Convert()};
    SwitchSignals(incomingSignals, outgoingSignals, coSimType);

    std::unique_ptr<SignalExchange> writerSignalExchange;
    AssertOk(CreateSignalExchange(coSimType, connectionKind, name, incomingSignals, outgoingSignals, *_protocol, writerSignalExchange));

    std::unique_ptr<SignalExchange> readerSignalExchange;
    AssertOk(CreateSignalExchange(GetCounterPart(coSimType),
                            connectionKind,
                            GetCounterPart(name, connectionKind),
                            incomingSignals,
                            outgoingSignals,
                            *_protocol,
                            readerSignalExchange));

    std::vector<uint8_t> writeValue = GenerateIoData(signal);
    AssertOk(writerSignalExchange->Write(signal.id, signal.length, writeValue.data()));

    TransferWithEvents(*writerSignalExchange, *readerSignalExchange, {{signal, writeValue}});

    // Second write with same data
    AssertOk(writerSignalExchange->Write(signal.id, signal.length, writeValue.data()));

    // Act and assert
    TransferWithEvents(*writerSignalExchange, *readerSignalExchange, {});
}

}  // namespace
