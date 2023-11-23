// Copyright dSPACE GmbH. All rights reserved.

#include <array>

#include "Communication.h"
#include "Generator.h"
#include "IoBuffer.h"
#include "Logger.h"
#include "TestHelper.h"

using namespace DsVeosCoSim;

class TestIoBuffer : public testing::Test {
protected:
    Channel _senderChannel;
    Channel _receiverChannel;

    void SetUp() override {
        SetLogCallback(OnLogCallback);

        Server server;
        uint16_t port = 0;
        ASSERT_OK(server.Start(port, true));

        ASSERT_OK(ConnectToServer("127.0.0.1", port, 0, _senderChannel));

        ASSERT_OK(server.Accept(_receiverChannel));

        ClearLastMessage();
    }

    void TearDown() override {
        _senderChannel.Disconnect();
        _receiverChannel.Disconnect();
    }
};

TEST_F(TestIoBuffer, CreateWithZeroIoSignalInfos) {
    // Arrange
    IoBuffer ioBuffer;

    // Act
    const Result result = ioBuffer.Initialize({}, {});

    // Assert
    ASSERT_OK(result);
}

TEST_F(TestIoBuffer, CreateWithSingleIoSignalInfo) {
    // Arrange
    const std::vector<IoSignal> incomingSignals = CreateSignals(1);
    const std::vector<IoSignal> outgoingSignals = CreateSignals(1);
    IoBuffer ioBuffer;

    // Act
    const Result result = ioBuffer.Initialize(incomingSignals, outgoingSignals);

    // Assert
    ASSERT_OK(result);
}

TEST_F(TestIoBuffer, CreateWithMultipleIoSignalInfos) {
    // Arrange
    const std::vector<IoSignal> incomingSignals = CreateSignals(2);
    const std::vector<IoSignal> outgoingSignals = CreateSignals(2);
    IoBuffer ioBuffer;

    // Act
    const Result result = ioBuffer.Initialize(incomingSignals, outgoingSignals);

    // Assert
    ASSERT_OK(result);
}

TEST_F(TestIoBuffer, DuplicatedReadIds) {
    // Arrange
    IoSignal signal{};
    CreateSignal(signal);

    IoBuffer ioBuffer;

    // Act
    const Result result = ioBuffer.Initialize({signal, signal}, {});

    // Assert
    ASSERT_ERROR(result);
    AssertLastMessage("Duplicated IO signal id " + std::to_string(signal.id) + ".");
}

TEST_F(TestIoBuffer, DuplicatedWriteIds) {
    // Arrange
    IoSignal signal{};
    CreateSignal(signal);

    IoBuffer ioBuffer;

    // Act
    const Result result = ioBuffer.Initialize({}, {signal, signal});

    // Assert
    ASSERT_ERROR(result);
    AssertLastMessage("Duplicated IO signal id " + std::to_string(signal.id) + ".");
}

TEST_F(TestIoBuffer, ReadInvalidId) {
    // Arrange
    IoSignal signal{};
    CreateSignal(signal);

    IoBuffer ioBuffer;
    ASSERT_OK(ioBuffer.Initialize({signal}, {}));

    const uint32_t readId = signal.id + 1;
    uint32_t readLength{};
    std::vector<uint8_t> readValue;
    readValue.resize(GetDataTypeSize(signal.dataType));

    // Act
    const Result result = ioBuffer.Read(readId, readLength, readValue.data());

    // Assert
    ASSERT_INVALID_ARGUMENT(result);
    AssertLastMessage("IO signal id " + std::to_string(readId) + " is unknown.");
}

TEST_F(TestIoBuffer, WriteInvalidId) {
    // Arrange
    IoSignal signal{};
    CreateSignal(signal);

    IoBuffer ioBuffer;
    ASSERT_OK(ioBuffer.Initialize({}, {signal}));

    const uint32_t writeId = signal.id + 1;
    const uint32_t writeLength = signal.length;
    std::vector<uint8_t> writeValue;
    writeValue.resize(GetDataTypeSize(signal.dataType));
    FillWithRandom(writeValue.data(), writeValue.size());

    // Act
    const Result result = ioBuffer.Write(writeId, writeLength, writeValue.data());

    // Assert
    ASSERT_INVALID_ARGUMENT(result);
    AssertLastMessage("IO signal id " + std::to_string(writeId) + " is unknown.");
}

TEST_F(TestIoBuffer, ScalarInitialData) {
    // Arrange
    IoSignal signal{};
    CreateSignal(signal);
    signal.sizeKind = SizeKind::Fixed;
    signal.length = 1;

    IoBuffer ioBuffer;
    ASSERT_OK(ioBuffer.Initialize({signal}, {}));

    std::vector<uint8_t> initialValue;
    initialValue.resize(GetDataTypeSize(signal.dataType));

    uint32_t readLength{};
    std::vector<uint8_t> readValue;
    readValue.resize(initialValue.size());

    // Act
    const Result result = ioBuffer.Read(signal.id, readLength, readValue.data());

    // Assert
    ASSERT_OK(result);
    ASSERT_EQ(signal.length, readLength);
    AssertByteArray(initialValue.data(), readValue.data(), initialValue.size());
}

TEST_F(TestIoBuffer, ScalarChanged) {
    // Arrange
    IoSignal signal{};
    CreateSignal(signal);
    signal.sizeKind = SizeKind::Fixed;
    signal.length = 1;

    IoBuffer senderIoBuffer;
    ASSERT_OK(senderIoBuffer.Initialize({}, {signal}));

    IoBuffer receiverIoBuffer;
    ASSERT_OK(receiverIoBuffer.Initialize({signal}, {}));

    std::vector<uint8_t> writeValue;
    writeValue.resize(static_cast<size_t>(signal.length) * GetDataTypeSize(signal.dataType));
    FillWithRandom(writeValue.data(), writeValue.size());

    ASSERT_OK(senderIoBuffer.Write(signal.id, signal.length, writeValue.data()));

    ASSERT_OK(senderIoBuffer.Serialize(_senderChannel));
    ASSERT_OK(_senderChannel.EndWrite());

    const SimulationTime writeSimulationTime = GenerateI64();

    Callbacks callbacks{};
    callbacks.incomingSignalChangedCallback =
        [&writeSimulationTime, &signal, &writeValue](SimulationTime simulationTime, const IoSignal& changedIoSignal, uint32_t length, const void* value) {
            ASSERT_EQ(writeSimulationTime, simulationTime);
            ASSERT_EQ(signal.id, changedIoSignal.id);
            ASSERT_EQ(signal.length, length);
            AssertByteArray(writeValue.data(), static_cast<const uint8_t*>(value), writeValue.size());
        };

    uint32_t readLength{};
    std::vector<uint8_t> readValue;
    readValue.resize(writeValue.size());

    // Act
    const Result deserializeResult = receiverIoBuffer.Deserialize(_receiverChannel, writeSimulationTime, callbacks);
    const Result readResult = receiverIoBuffer.Read(signal.id, readLength, readValue.data());

    // Assert
    ASSERT_OK(deserializeResult);
    ASSERT_OK(readResult);
    ASSERT_EQ(signal.length, readLength);
    AssertByteArray(writeValue.data(), readValue.data(), writeValue.size());
}

TEST_F(TestIoBuffer, ScalarWrongLength) {
    // Arrange
    IoSignal signal{};
    CreateSignal(signal);
    signal.sizeKind = SizeKind::Fixed;
    signal.length = 1;

    IoBuffer ioBuffer;
    ASSERT_OK(ioBuffer.Initialize({}, {signal}));

    std::vector<uint8_t> writeValue;
    writeValue.resize(static_cast<size_t>(signal.length) * GetDataTypeSize(signal.dataType));
    FillWithRandom(writeValue.data(), writeValue.size());
    const uint32_t writeLength = signal.length + 1;

    // Act
    const Result result = ioBuffer.Write(signal.id, writeLength, writeValue.data());

    // Assert
    ASSERT_ERROR(result);
    AssertLastMessage("Length of fixed sized IO signal '" + signal.name + "' must be " + std::to_string(signal.length) + " but was " +
                      std::to_string(writeLength) + ".");
}

TEST_F(TestIoBuffer, FixedSizedVectorInitialData) {
    // Arrange
    IoSignal signal{};
    CreateSignal(signal);
    signal.sizeKind = SizeKind::Fixed;
    signal.length = GenerateRandom(2, 10);

    IoBuffer ioBuffer;
    ASSERT_OK(ioBuffer.Initialize({signal}, {}));

    std::vector<uint8_t> initialValue;
    initialValue.resize(static_cast<size_t>(signal.length) * GetDataTypeSize(signal.dataType));

    uint32_t readLength{};
    std::vector<uint8_t> readValue;
    readValue.resize(initialValue.size());

    // Act
    const Result result = ioBuffer.Read(signal.id, readLength, readValue.data());

    // Assert
    ASSERT_OK(result);
    ASSERT_EQ(signal.length, readLength);
    AssertByteArray(initialValue.data(), readValue.data(), initialValue.size());
}

TEST_F(TestIoBuffer, FixedSizedVectorChanged) {
    // Arrange
    IoSignal signal{};
    CreateSignal(signal);
    signal.sizeKind = SizeKind::Fixed;
    signal.length = GenerateRandom(2, 10);

    IoBuffer senderIoBuffer;
    ASSERT_OK(senderIoBuffer.Initialize({}, {signal}));

    IoBuffer receiverIoBuffer;
    ASSERT_OK(receiverIoBuffer.Initialize({signal}, {}));

    std::vector<uint8_t> writeValue{};
    writeValue.resize(static_cast<size_t>(signal.length) * GetDataTypeSize(signal.dataType));
    FillWithRandom(writeValue.data(), writeValue.size());
    ASSERT_OK(senderIoBuffer.Write(signal.id, signal.length, writeValue.data()));
    ASSERT_OK(senderIoBuffer.Serialize(_senderChannel));
    ASSERT_OK(_senderChannel.EndWrite());

    const SimulationTime writeSimulationTime = GenerateI64();

    Callbacks callbacks{};
    callbacks.incomingSignalChangedCallback =
        [&writeSimulationTime, &signal, &writeValue](SimulationTime simulationTime, const IoSignal& changedIoSignal, uint32_t length, const void* value) {
            ASSERT_EQ(writeSimulationTime, simulationTime);
            ASSERT_EQ(signal.id, changedIoSignal.id);
            ASSERT_EQ(signal.length, length);
            AssertByteArray(writeValue.data(), static_cast<const uint8_t*>(value), writeValue.size());
        };

    uint32_t readLength{};
    std::vector<uint8_t> readValue{};
    readValue.resize(writeValue.size());

    // Act
    const Result deserializeResult = receiverIoBuffer.Deserialize(_receiverChannel, writeSimulationTime, callbacks);
    const Result readResult = receiverIoBuffer.Read(signal.id, readLength, readValue.data());

    // Assert
    ASSERT_OK(deserializeResult);
    ASSERT_OK(readResult);
    ASSERT_EQ(signal.length, readLength);
    AssertByteArray(writeValue.data(), readValue.data(), writeValue.size());
}

TEST_F(TestIoBuffer, FixedSizedVectorWrongLength) {
    // Arrange
    IoSignal signal{};
    CreateSignal(signal);
    signal.sizeKind = SizeKind::Fixed;
    signal.length = GenerateRandom(2, 10);

    IoBuffer ioBuffer;
    ASSERT_OK(ioBuffer.Initialize({}, {signal}));

    const uint32_t writeLength = signal.length + 1;
    std::vector<uint8_t> writeValue{};
    writeValue.resize(static_cast<size_t>(signal.length) * GetDataTypeSize(signal.dataType));
    FillWithRandom(writeValue.data(), writeValue.size());

    // Act
    const Result result = ioBuffer.Write(signal.id, writeLength, writeValue.data());

    // Assert
    ASSERT_ERROR(result);
    AssertLastMessage("Length of fixed sized IO signal '" + signal.name + "' must be " + std::to_string(signal.length) + " but was " +
                      std::to_string(writeLength) + ".");
}

TEST_F(TestIoBuffer, VariableSizedVectorInitialData) {
    // Arrange
    IoSignal signal{};
    CreateSignal(signal);
    signal.sizeKind = SizeKind::Variable;

    IoBuffer ioBuffer;
    ASSERT_OK(ioBuffer.Initialize({signal}, {}));

    std::vector<uint8_t> initialValue;
    initialValue.resize(static_cast<size_t>(signal.length) * GetDataTypeSize(signal.dataType));

    uint32_t readLength{};
    std::vector<uint8_t> readValue;
    readValue.resize(initialValue.size());

    // Act
    const Result result = ioBuffer.Read(signal.id, readLength, readValue.data());

    // Assert
    ASSERT_OK(result);
    ASSERT_EQ(0U, readLength);
}

TEST_F(TestIoBuffer, VariableSizedVectorAllElementsChanged) {
    // Arrange
    IoSignal signal{};
    CreateSignal(signal);
    signal.sizeKind = SizeKind::Variable;

    IoBuffer senderIoBuffer;
    ASSERT_OK(senderIoBuffer.Initialize({}, {signal}));

    IoBuffer receiverIoBuffer;
    ASSERT_OK(receiverIoBuffer.Initialize({signal}, {}));

    std::vector<uint8_t> writeValue{};
    writeValue.resize(static_cast<size_t>(signal.length) * GetDataTypeSize(signal.dataType));
    FillWithRandom(writeValue.data(), writeValue.size());
    ASSERT_OK(senderIoBuffer.Write(signal.id, signal.length, writeValue.data()));
    ASSERT_OK(senderIoBuffer.Serialize(_senderChannel));
    ASSERT_OK(_senderChannel.EndWrite());

    const SimulationTime writeSimulationTime = GenerateI64();

    Callbacks callbacks{};
    callbacks.incomingSignalChangedCallback =
        [&writeSimulationTime, &signal, &writeValue](SimulationTime simulationTime, const IoSignal& changedIoSignal, uint32_t length, const void* value) {
            ASSERT_EQ(writeSimulationTime, simulationTime);
            ASSERT_EQ(signal.id, changedIoSignal.id);
            ASSERT_EQ(signal.length, length);
            AssertByteArray(writeValue.data(), static_cast<const uint8_t*>(value), writeValue.size());
        };

    uint32_t readLength{};
    std::vector<uint8_t> readValue{};
    readValue.resize(writeValue.size());

    // Act
    const Result deserializeResult = receiverIoBuffer.Deserialize(_receiverChannel, writeSimulationTime, callbacks);
    const Result readResult = receiverIoBuffer.Read(signal.id, readLength, readValue.data());

    // Assert
    ASSERT_OK(deserializeResult);
    ASSERT_OK(readResult);
    ASSERT_EQ(signal.length, readLength);
    AssertByteArray(writeValue.data(), readValue.data(), writeValue.size());
}

TEST_F(TestIoBuffer, VariableSizedVectorSomeElementsChanged) {
    // Arrange
    IoSignal signal{};
    CreateSignal(signal);
    signal.sizeKind = SizeKind::Variable;
    signal.length = GenerateRandom(2, 10);

    IoBuffer senderIoBuffer;
    ASSERT_OK(senderIoBuffer.Initialize({}, {signal}));

    IoBuffer receiverIoBuffer;
    ASSERT_OK(receiverIoBuffer.Initialize({signal}, {}));

    std::vector<uint8_t> writeValue{};
    writeValue.resize(static_cast<size_t>(signal.length - 1) * GetDataTypeSize(signal.dataType));
    FillWithRandom(writeValue.data(), writeValue.size());
    ASSERT_OK(senderIoBuffer.Write(signal.id, signal.length - 1, writeValue.data()));
    ASSERT_OK(senderIoBuffer.Serialize(_senderChannel));
    ASSERT_OK(_senderChannel.EndWrite());

    const SimulationTime writeSimulationTime = GenerateI64();

    Callbacks callbacks{};
    callbacks.incomingSignalChangedCallback =
        [&writeSimulationTime, &signal, &writeValue](SimulationTime simulationTime, const IoSignal& changedIoSignal, uint32_t length, const void* value) {
            ASSERT_EQ(writeSimulationTime, simulationTime);
            ASSERT_EQ(signal.id, changedIoSignal.id);
            ASSERT_EQ(signal.length - 1, length);
            AssertByteArray(writeValue.data(), static_cast<const uint8_t*>(value), writeValue.size());
        };

    uint32_t readLength{};
    std::vector<uint8_t> readValue{};
    readValue.resize(writeValue.size());

    // Act
    const Result deserializeResult = receiverIoBuffer.Deserialize(_receiverChannel, writeSimulationTime, callbacks);
    const Result readResult = receiverIoBuffer.Read(signal.id, readLength, readValue.data());

    // Assert
    ASSERT_OK(deserializeResult);
    ASSERT_OK(readResult);
    ASSERT_EQ(signal.length - 1, readLength);
    AssertByteArray(writeValue.data(), readValue.data(), writeValue.size());
}

TEST_F(TestIoBuffer, VariableSizedVectorInitialLengthIsZero) {
    // Arrange
    IoSignal signal{};
    CreateSignal(signal);
    signal.sizeKind = SizeKind::Variable;

    IoBuffer senderIoBuffer;
    ASSERT_OK(senderIoBuffer.Initialize({}, {signal}));

    IoBuffer receiverIoBuffer;
    ASSERT_OK(receiverIoBuffer.Initialize({signal}, {}));

    constexpr uint32_t writeLength = 0;  // No element should be written
    std::vector<uint8_t> writeValue{};
    writeValue.resize(static_cast<size_t>(signal.length) * GetDataTypeSize(signal.dataType));
    FillWithRandom(writeValue.data(), writeValue.size());

    const SimulationTime writeSimulationTime = GenerateI64();

    int callbacksCounter{};

    Callbacks callbacks{};
    callbacks.incomingSignalChangedCallback = [&callbacksCounter](SimulationTime, const IoSignal&, uint32_t, const void*) {
        callbacksCounter++;
    };

    ASSERT_OK(senderIoBuffer.Serialize(_senderChannel));
    ASSERT_OK(_senderChannel.EndWrite());

    uint32_t readLength{};
    std::vector<uint8_t> readValue{};
    readValue.resize(writeValue.size());

    // Act
    const Result deserializeResult = receiverIoBuffer.Deserialize(_receiverChannel, writeSimulationTime, callbacks);
    const Result readResult = receiverIoBuffer.Read(signal.id, readLength, readValue.data());

    // Assert
    ASSERT_OK(deserializeResult);
    ASSERT_OK(readResult);
    ASSERT_EQ(0, callbacksCounter);
    ASSERT_EQ(writeLength, readLength);
}

TEST_F(TestIoBuffer, VariableSizedVectorInvalidLength) {
    // Arrange
    IoSignal signal{};
    CreateSignal(signal);
    signal.sizeKind = SizeKind::Variable;
    signal.length = GenerateRandom(2, 10);

    IoBuffer ioBuffer;
    ASSERT_OK(ioBuffer.Initialize({}, {signal}));

    const uint32_t writeLength = signal.length + 1;
    std::vector<uint8_t> writeValue{};
    writeValue.resize(static_cast<size_t>(signal.length) * GetDataTypeSize(signal.dataType));
    FillWithRandom(writeValue.data(), writeValue.size());

    // Act
    const Result result = ioBuffer.Write(signal.id, writeLength, writeValue.data());

    // Assert
    ASSERT_ERROR(result);
    AssertLastMessage("Length of variable sized IO signal '" + signal.name + "' exceeds max size.");
}

// Add more tests
