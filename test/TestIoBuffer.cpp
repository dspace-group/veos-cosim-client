// Copyright dSPACE GmbH. All rights reserved.

#include <array>

#include "Communication.h"
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
    const std::vector<IoSignalContainer> incomingSignals = CreateSignals(1);
    const std::vector<IoSignalContainer> outgoingSignals = CreateSignals(1);
    IoBuffer ioBuffer;

    // Act
    const Result result = ioBuffer.Initialize(incomingSignals, outgoingSignals);

    // Assert
    ASSERT_OK(result);
}

TEST_F(TestIoBuffer, CreateWithMultipleIoSignalInfos) {
    // Arrange
    const std::vector<IoSignalContainer> incomingSignals = CreateSignals(2);
    const std::vector<IoSignalContainer> outgoingSignals = CreateSignals(2);
    IoBuffer ioBuffer;

    // Act
    const Result result = ioBuffer.Initialize(incomingSignals, outgoingSignals);

    // Assert
    ASSERT_OK(result);
}

TEST_F(TestIoBuffer, DuplicatedReadIds) {
    // Arrange
    IoSignalContainer container{};
    CreateSignal(container);

    IoBuffer ioBuffer;

    // Act
    const Result result = ioBuffer.Initialize({container, container}, {});

    // Assert
    ASSERT_ERROR(result);
    AssertLastMessage("Duplicated IO signal id " + std::to_string(container.signal.id) + ".");
}

TEST_F(TestIoBuffer, DuplicatedWriteIds) {
    // Arrange
    IoSignalContainer container{};
    CreateSignal(container);

    IoBuffer ioBuffer;

    // Act
    const Result result = ioBuffer.Initialize({}, {container, container});

    // Assert
    ASSERT_ERROR(result);
    AssertLastMessage("Duplicated IO signal id " + std::to_string(container.signal.id) + ".");
}

TEST_F(TestIoBuffer, ReadInvalidId) {
    // Arrange
    IoSignalContainer container{};
    CreateSignal(container);

    IoBuffer ioBuffer;
    ASSERT_OK(ioBuffer.Initialize({container}, {}));

    const uint32_t readId = container.signal.id + 1;
    uint32_t readLength{};
    std::vector<uint8_t> readValue;
    readValue.resize(GetDataTypeSize(container.signal.dataType));

    // Act
    const Result result = ioBuffer.Read(readId, readLength, readValue.data());

    // Assert
    ASSERT_INVALID_ARGUMENT(result);
    AssertLastMessage("IO signal id " + std::to_string(readId) + " is unknown.");
}

TEST_F(TestIoBuffer, WriteInvalidId) {
    // Arrange
    IoSignalContainer container{};
    CreateSignal(container);

    IoBuffer ioBuffer;
    ASSERT_OK(ioBuffer.Initialize({}, {container}));

    const uint32_t writeId = container.signal.id + 1;
    const uint32_t writeLength = container.signal.length;
    std::vector<uint8_t> writeValue;
    writeValue.resize(GetDataTypeSize(container.signal.dataType));
    FillWithRandom(writeValue.data(), writeValue.size());

    // Act
    const Result result = ioBuffer.Write(writeId, writeLength, writeValue.data());

    // Assert
    ASSERT_INVALID_ARGUMENT(result);
    AssertLastMessage("IO signal id " + std::to_string(writeId) + " is unknown.");
}

TEST_F(TestIoBuffer, ScalarInitialData) {
    // Arrange
    IoSignalContainer container{};
    CreateSignal(container);
    container.signal.sizeKind = SizeKind::Fixed;
    container.signal.length = 1;

    IoBuffer ioBuffer;
    ASSERT_OK(ioBuffer.Initialize({container}, {}));

    std::vector<uint8_t> initialValue;
    initialValue.resize(GetDataTypeSize(container.signal.dataType));

    uint32_t readLength{};
    std::vector<uint8_t> readValue;
    readValue.resize(initialValue.size());

    // Act
    const Result result = ioBuffer.Read(container.signal.id, readLength, readValue.data());

    // Assert
    ASSERT_OK(result);
    ASSERT_EQ(container.signal.length, readLength);
    AssertByteArray(initialValue.data(), readValue.data(), initialValue.size());
}

TEST_F(TestIoBuffer, ScalarChanged) {
    // Arrange
    IoSignalContainer container{};
    CreateSignal(container);
    container.signal.sizeKind = SizeKind::Fixed;
    container.signal.length = 1;

    IoBuffer senderIoBuffer;
    ASSERT_OK(senderIoBuffer.Initialize({}, {container}));

    IoBuffer receiverIoBuffer;
    ASSERT_OK(receiverIoBuffer.Initialize({container}, {}));

    std::vector<uint8_t> writeValue;
    writeValue.resize((size_t)container.signal.length * GetDataTypeSize(container.signal.dataType));
    FillWithRandom(writeValue.data(), writeValue.size());

    ASSERT_OK(senderIoBuffer.Write(container.signal.id, container.signal.length, writeValue.data()));

    ASSERT_OK(senderIoBuffer.Serialize(_senderChannel));
    ASSERT_OK(_senderChannel.EndWrite());

    const SimulationTime writeSimulationTime = GenerateI64();

    Callbacks callbacks{};
    callbacks.incomingSignalChangedCallback =
        [&writeSimulationTime, &container, &writeValue](SimulationTime simulationTime, const IoSignal& changedIoSignal, uint32_t length, const void* value) {
            ASSERT_EQ(writeSimulationTime, simulationTime);
            ASSERT_EQ(container.signal.id, changedIoSignal.id);
            ASSERT_EQ(container.signal.length, length);
            AssertByteArray(writeValue.data(), (const uint8_t*)value, writeValue.size());
        };

    uint32_t readLength{};
    std::vector<uint8_t> readValue;
    readValue.resize(writeValue.size());

    // Act
    const Result deserializeResult = receiverIoBuffer.Deserialize(_receiverChannel, writeSimulationTime, callbacks);
    const Result readResult = receiverIoBuffer.Read(container.signal.id, readLength, readValue.data());

    // Assert
    ASSERT_OK(deserializeResult);
    ASSERT_OK(readResult);
    ASSERT_EQ(container.signal.length, readLength);
    AssertByteArray(writeValue.data(), readValue.data(), writeValue.size());
}

TEST_F(TestIoBuffer, ScalarWrongLength) {
    // Arrange
    IoSignalContainer container{};
    CreateSignal(container);
    container.signal.sizeKind = SizeKind::Fixed;
    container.signal.length = 1;

    IoBuffer ioBuffer;
    ASSERT_OK(ioBuffer.Initialize({}, {container}));

    std::vector<uint8_t> writeValue;
    writeValue.resize((size_t)container.signal.length * GetDataTypeSize(container.signal.dataType));
    FillWithRandom(writeValue.data(), writeValue.size());
    const uint32_t writeLength = container.signal.length + 1;

    // Act
    const Result result = ioBuffer.Write(container.signal.id, writeLength, writeValue.data());

    // Assert
    ASSERT_ERROR(result);
    AssertLastMessage("Length of fixed sized IO signal '" + container.name + "' must be " + std::to_string(container.signal.length) + " but was " +
                      std::to_string(writeLength) + ".");
}

TEST_F(TestIoBuffer, FixedSizedVectorInitialData) {
    // Arrange
    IoSignalContainer container{};
    CreateSignal(container);
    container.signal.sizeKind = SizeKind::Fixed;
    container.signal.length = GenerateRandom(2, 10);

    IoBuffer ioBuffer;
    ASSERT_OK(ioBuffer.Initialize({container}, {}));

    std::vector<uint8_t> initialValue;
    initialValue.resize((size_t)container.signal.length * GetDataTypeSize(container.signal.dataType));

    uint32_t readLength{};
    std::vector<uint8_t> readValue;
    readValue.resize(initialValue.size());

    // Act
    const Result result = ioBuffer.Read(container.signal.id, readLength, readValue.data());

    // Assert
    ASSERT_OK(result);
    ASSERT_EQ(container.signal.length, readLength);
    AssertByteArray(initialValue.data(), readValue.data(), initialValue.size());
}

TEST_F(TestIoBuffer, FixedSizedVectorChanged) {
    // Arrange
    IoSignalContainer container{};
    CreateSignal(container);
    container.signal.sizeKind = SizeKind::Fixed;
    container.signal.length = GenerateRandom(2, 10);

    IoBuffer senderIoBuffer;
    ASSERT_OK(senderIoBuffer.Initialize({}, {container}));

    IoBuffer receiverIoBuffer;
    ASSERT_OK(receiverIoBuffer.Initialize({container}, {}));

    std::vector<uint8_t> writeValue{};
    writeValue.resize((size_t)container.signal.length * GetDataTypeSize(container.signal.dataType));
    FillWithRandom(writeValue.data(), writeValue.size());
    ASSERT_OK(senderIoBuffer.Write(container.signal.id, container.signal.length, writeValue.data()));
    ASSERT_OK(senderIoBuffer.Serialize(_senderChannel));
    ASSERT_OK(_senderChannel.EndWrite());

    const SimulationTime writeSimulationTime = GenerateI64();

    Callbacks callbacks{};
    callbacks.incomingSignalChangedCallback =
        [&writeSimulationTime, &container, &writeValue](SimulationTime simulationTime, const IoSignal& changedIoSignal, uint32_t length, const void* value) {
            ASSERT_EQ(writeSimulationTime, simulationTime);
            ASSERT_EQ(container.signal.id, changedIoSignal.id);
            ASSERT_EQ(container.signal.length, length);
            AssertByteArray(writeValue.data(), (const uint8_t*)value, writeValue.size());
        };

    uint32_t readLength{};
    std::vector<uint8_t> readValue{};
    readValue.resize(writeValue.size());

    // Act
    const Result deserializeResult = receiverIoBuffer.Deserialize(_receiverChannel, writeSimulationTime, callbacks);
    const Result readResult = receiverIoBuffer.Read(container.signal.id, readLength, readValue.data());

    // Assert
    ASSERT_OK(deserializeResult);
    ASSERT_OK(readResult);
    ASSERT_EQ(container.signal.length, readLength);
    AssertByteArray(writeValue.data(), readValue.data(), writeValue.size());
}

TEST_F(TestIoBuffer, FixedSizedVectorWrongLength) {
    // Arrange
    IoSignalContainer container{};
    CreateSignal(container);
    container.signal.sizeKind = SizeKind::Fixed;
    container.signal.length = GenerateRandom(2, 10);

    IoBuffer ioBuffer;
    ASSERT_OK(ioBuffer.Initialize({}, {container}));

    const uint32_t writeLength = container.signal.length + 1;
    std::vector<uint8_t> writeValue{};
    writeValue.resize((size_t)container.signal.length * GetDataTypeSize(container.signal.dataType));
    FillWithRandom(writeValue.data(), writeValue.size());

    // Act
    const Result result = ioBuffer.Write(container.signal.id, writeLength, writeValue.data());

    // Assert
    ASSERT_ERROR(result);
    AssertLastMessage("Length of fixed sized IO signal '" + container.name + "' must be " + std::to_string(container.signal.length) + " but was " +
                      std::to_string(writeLength) + ".");
}

TEST_F(TestIoBuffer, VariableSizedVectorInitialData) {
    // Arrange
    IoSignalContainer container{};
    CreateSignal(container);
    container.signal.sizeKind = SizeKind::Variable;

    IoBuffer ioBuffer;
    ASSERT_OK(ioBuffer.Initialize({container}, {}));

    std::vector<uint8_t> initialValue;
    initialValue.resize((size_t)container.signal.length * GetDataTypeSize(container.signal.dataType));

    uint32_t readLength{};
    std::vector<uint8_t> readValue;
    readValue.resize(initialValue.size());

    // Act
    const Result result = ioBuffer.Read(container.signal.id, readLength, readValue.data());

    // Assert
    ASSERT_OK(result);
    ASSERT_EQ(0, readLength);
}

TEST_F(TestIoBuffer, VariableSizedVectorAllElementsChanged) {
    // Arrange
    IoSignalContainer container{};
    CreateSignal(container);
    container.signal.sizeKind = SizeKind::Variable;

    IoBuffer senderIoBuffer;
    ASSERT_OK(senderIoBuffer.Initialize({}, {container}));

    IoBuffer receiverIoBuffer;
    ASSERT_OK(receiverIoBuffer.Initialize({container}, {}));

    std::vector<uint8_t> writeValue{};
    writeValue.resize((size_t)container.signal.length * GetDataTypeSize(container.signal.dataType));
    FillWithRandom(writeValue.data(), writeValue.size());
    ASSERT_OK(senderIoBuffer.Write(container.signal.id, container.signal.length, writeValue.data()));
    ASSERT_OK(senderIoBuffer.Serialize(_senderChannel));
    ASSERT_OK(_senderChannel.EndWrite());

    const SimulationTime writeSimulationTime = GenerateI64();

    Callbacks callbacks{};
    callbacks.incomingSignalChangedCallback =
        [&writeSimulationTime, &container, &writeValue](SimulationTime simulationTime, const IoSignal& changedIoSignal, uint32_t length, const void* value) {
            ASSERT_EQ(writeSimulationTime, simulationTime);
            ASSERT_EQ(container.signal.id, changedIoSignal.id);
            ASSERT_EQ(container.signal.length, length);
            AssertByteArray(writeValue.data(), (const uint8_t*)value, writeValue.size());
        };

    uint32_t readLength{};
    std::vector<uint8_t> readValue{};
    readValue.resize(writeValue.size());

    // Act
    const Result deserializeResult = receiverIoBuffer.Deserialize(_receiverChannel, writeSimulationTime, callbacks);
    const Result readResult = receiverIoBuffer.Read(container.signal.id, readLength, readValue.data());

    // Assert
    ASSERT_OK(deserializeResult);
    ASSERT_OK(readResult);
    ASSERT_EQ(container.signal.length, readLength);
    AssertByteArray(writeValue.data(), readValue.data(), writeValue.size());
}

TEST_F(TestIoBuffer, VariableSizedVectorSomeElementsChanged) {
    // Arrange
    IoSignalContainer container{};
    CreateSignal(container);
    container.signal.sizeKind = SizeKind::Variable;
    container.signal.length = GenerateRandom(2, 10);

    IoBuffer senderIoBuffer;
    ASSERT_OK(senderIoBuffer.Initialize({}, {container}));

    IoBuffer receiverIoBuffer;
    ASSERT_OK(receiverIoBuffer.Initialize({container}, {}));

    std::vector<uint8_t> writeValue{};
    writeValue.resize((size_t)(container.signal.length - 1) * GetDataTypeSize(container.signal.dataType));
    FillWithRandom(writeValue.data(), writeValue.size());
    ASSERT_OK(senderIoBuffer.Write(container.signal.id, container.signal.length - 1, writeValue.data()));
    ASSERT_OK(senderIoBuffer.Serialize(_senderChannel));
    ASSERT_OK(_senderChannel.EndWrite());

    const SimulationTime writeSimulationTime = GenerateI64();

    Callbacks callbacks{};
    callbacks.incomingSignalChangedCallback =
        [&writeSimulationTime, &container, &writeValue](SimulationTime simulationTime, const IoSignal& changedIoSignal, uint32_t length, const void* value) {
            ASSERT_EQ(writeSimulationTime, simulationTime);
            ASSERT_EQ(container.signal.id, changedIoSignal.id);
            ASSERT_EQ(container.signal.length - 1, length);
            AssertByteArray(writeValue.data(), (const uint8_t*)value, writeValue.size());
        };

    uint32_t readLength{};
    std::vector<uint8_t> readValue{};
    readValue.resize(writeValue.size());

    // Act
    const Result deserializeResult = receiverIoBuffer.Deserialize(_receiverChannel, writeSimulationTime, callbacks);
    const Result readResult = receiverIoBuffer.Read(container.signal.id, readLength, readValue.data());

    // Assert
    ASSERT_OK(deserializeResult);
    ASSERT_OK(readResult);
    ASSERT_EQ(container.signal.length - 1, readLength);
    AssertByteArray(writeValue.data(), readValue.data(), writeValue.size());
}

TEST_F(TestIoBuffer, VariableSizedVectorInitialLengthIsZero) {
    // Arrange
    IoSignalContainer container{};
    CreateSignal(container);
    container.signal.sizeKind = SizeKind::Variable;

    IoBuffer senderIoBuffer;
    ASSERT_OK(senderIoBuffer.Initialize({}, {container}));

    IoBuffer receiverIoBuffer;
    ASSERT_OK(receiverIoBuffer.Initialize({container}, {}));

    constexpr uint32_t writeLength = 0;  // No element should be written
    std::vector<uint8_t> writeValue{};
    writeValue.resize((size_t)container.signal.length * GetDataTypeSize(container.signal.dataType));
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
    const Result readResult = receiverIoBuffer.Read(container.signal.id, readLength, readValue.data());

    // Assert
    ASSERT_OK(deserializeResult);
    ASSERT_OK(readResult);
    ASSERT_EQ(0, callbacksCounter);
    ASSERT_EQ(writeLength, readLength);
}

TEST_F(TestIoBuffer, VariableSizedVectorInvalidLength) {
    // Arrange
    IoSignalContainer container{};
    CreateSignal(container);
    container.signal.sizeKind = SizeKind::Variable;
    container.signal.length = GenerateRandom(2, 10);

    IoBuffer ioBuffer;
    ASSERT_OK(ioBuffer.Initialize({}, {container}));

    const uint32_t writeLength = container.signal.length + 1;
    std::vector<uint8_t> writeValue{};
    writeValue.resize((size_t)container.signal.length * GetDataTypeSize(container.signal.dataType));
    FillWithRandom(writeValue.data(), writeValue.size());

    // Act
    const Result result = ioBuffer.Write(container.signal.id, writeLength, writeValue.data());

    // Assert
    ASSERT_ERROR(result);
    AssertLastMessage("Length of variable sized IO signal '" + container.name + "' exceeds max size.");
}

// Add more tests
