// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <array>
#include <string>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "CoSimTypes.hpp"
#include "Helper.hpp"

using namespace std::chrono;
using namespace DsVeosCoSim;
using namespace testing;

namespace {

class TestTypes : public testing::Test {
protected:
    void SetUp() override {
        ClearLastMessage();
    }
};

TEST_F(TestTypes, DataToString) {
    // Arrange
    std::array<uint8_t, 2> data{0xde, 0xaf};
    char separator = '/';

    // Act
    std::string string = DataToString(data.data(), data.size(), separator);

    // Assert
    ASSERT_STREQ("de/af", string.c_str());
}

TEST_F(TestTypes, SimulationTimeToString) {
    // Arrange
    SimulationTime simulationTime{42};

    // Act
    std::string string = format_as(simulationTime);

    // Assert
    ASSERT_STREQ("0.000000042", string.c_str());
}

TEST_F(TestTypes, ResultToString) {
    // Arrange
    Result result = CreateError();

    // Act
    const char* string = format_as(result);

    // Assert
    ASSERT_STREQ("Error", string);
}

TEST_F(TestTypes, CoSimTypeToString) {
    // Arrange
    CoSimType coSimType = CoSimType::Server;

    // Act
    const char* string = format_as(coSimType);

    // Assert
    ASSERT_STREQ("Server", string);
}

TEST_F(TestTypes, ConnectionKindToString) {
    // Arrange
    ConnectionKind connectionKind = ConnectionKind::Remote;

    // Act
    const char* string = format_as(connectionKind);

    // Assert
    ASSERT_STREQ("Remote", string);
}

TEST_F(TestTypes, CommandToString) {
    // Arrange
    Command command = Command::Start;

    // Act
    const char* string = format_as(command);

    // Assert
    ASSERT_STREQ("Start", string);
}

TEST_F(TestTypes, SeverityToString) {
    // Arrange
    Severity severity = Severity::Warning;

    // Act
    const char* string = format_as(severity);

    // Assert
    ASSERT_STREQ("Warning", string);
}

TEST_F(TestTypes, TerminateReasonToString) {
    // Arrange
    TerminateReason terminateReason = TerminateReason::Finished;

    // Act
    const char* string = format_as(terminateReason);

    // Assert
    ASSERT_STREQ("Finished", string);
}

TEST_F(TestTypes, ConnectionStateToString) {
    // Arrange
    ConnectionState connectionState = ConnectionState::Disconnected;

    // Act
    const char* string = format_as(connectionState);

    // Assert
    ASSERT_STREQ("Disconnected", string);
}

TEST_F(TestTypes, GetDataTypeSize) {
    // Arrange
    DataType dataType = DataType::UInt64;

    // Act
    size_t size = GetDataTypeSize(dataType);

    // Assert
    ASSERT_EQ(8, size);
}

TEST_F(TestTypes, DataTypeToString) {
    // Arrange
    DataType dataType = DataType::Float64;

    // Act
    const char* string = format_as(dataType);

    // Assert
    ASSERT_STREQ("Float64", string);
}

TEST_F(TestTypes, SizeKindToString) {
    // Arrange
    SizeKind sizeKind = SizeKind::Variable;

    // Act
    const char* string = format_as(sizeKind);

    // Assert
    ASSERT_STREQ("Variable", string);
}

TEST_F(TestTypes, ValueToString) {
    // Arrange
    DataType dataType = DataType::Float64;
    uint32_t length = 3;
    std::vector<double> data{4.2, 0.000789, 200};

    // Act
    std::string string = ValueToString(dataType, length, data.data());

    // Assert
    ASSERT_STREQ("4.2 0.000789 200", string.c_str());
}

TEST_F(TestTypes, SimulationStateToString) {
    // Arrange
    SimulationState simulationState = SimulationState::Stopped;

    // Act
    const char* string = format_as(simulationState);

    // Assert
    ASSERT_STREQ("Stopped", string);
}

TEST_F(TestTypes, ModeToString) {
    // Arrange
    Mode mode{};

    // Act
    const char* string = format_as(mode);

    // Assert
    ASSERT_STREQ("<Unused>", string);
}

TEST_F(TestTypes, IoSignalIdToString) {
    // Arrange
    auto signalId = static_cast<IoSignalId>(86);

    // Act
    std::string string = format_as(signalId);

    // Assert
    ASSERT_STREQ("86", string.c_str());
}

TEST_F(TestTypes, IoSignalToString) {
    // Arrange
    std::string name = "MySignal";

    IoSignal signal{};
    signal.id = static_cast<IoSignalId>(42);
    signal.length = 43;
    signal.dataType = DataType::UInt16;
    signal.sizeKind = SizeKind::Variable;
    signal.name = name.c_str();

    // Act
    std::string string = format_as(signal);

    // Assert
    ASSERT_STREQ("IO Signal { Id: 42, Length: 43, DataType: UInt16, SizeKind: Variable, Name: \"MySignal\" }", string.c_str());
}

TEST_F(TestTypes, IoSignalContainerToString) {
    // Arrange
    IoSignalContainer signalContainer{};
    signalContainer.id = static_cast<IoSignalId>(42);
    signalContainer.length = 43;
    signalContainer.dataType = DataType::UInt16;
    signalContainer.sizeKind = SizeKind::Variable;
    signalContainer.name = "MySignal";

    // Act
    std::string string = format_as(signalContainer);

    // Assert
    ASSERT_STREQ("IO Signal { Id: 42, Length: 43, DataType: UInt16, SizeKind: Variable, Name: \"MySignal\" }", string.c_str());
}

TEST_F(TestTypes, IoSignalContainerConvert) {
    // Arrange
    IoSignalContainer signalContainer{};
    signalContainer.id = static_cast<IoSignalId>(42);
    signalContainer.length = 43;
    signalContainer.dataType = DataType::UInt16;
    signalContainer.sizeKind = SizeKind::Variable;
    signalContainer.name = "MySignal";

    IoSignal expectedSignal{};
    expectedSignal.id = signalContainer.id;
    expectedSignal.length = signalContainer.length;
    expectedSignal.dataType = signalContainer.dataType;
    expectedSignal.sizeKind = signalContainer.sizeKind;
    expectedSignal.name = signalContainer.name.c_str();

    // Act
    IoSignal actualSignal = signalContainer.Convert();

    // Assert
    ASSERT_EQ(expectedSignal, actualSignal);
}

TEST_F(TestTypes, IoDataToString) {
    // Arrange
    std::string name = "MySignal";

    IoSignal signal{};
    signal.id = static_cast<IoSignalId>(42);
    signal.length = 43;
    signal.dataType = DataType::UInt16;
    signal.sizeKind = SizeKind::Variable;
    signal.name = name.c_str();

    uint32_t length = 1;
    uint16_t data = 65432;

    // Act
    std::string string = IoDataToString(signal, length, &data);

    // Assert
    ASSERT_STREQ("IO Data { Id: 42, Length: 1, Data: 65432 }", string.c_str());
}

TEST_F(TestTypes, SignalContainersToString) {
    // Arrange
    IoSignalContainer signalContainer1{};
    signalContainer1.id = static_cast<IoSignalId>(42);
    signalContainer1.length = 43;
    signalContainer1.dataType = DataType::UInt16;
    signalContainer1.sizeKind = SizeKind::Variable;
    signalContainer1.name = "MySignal1";

    IoSignalContainer signalContainer2{};
    signalContainer2.id = static_cast<IoSignalId>(44);
    signalContainer2.length = 45;
    signalContainer2.dataType = DataType::Bool;
    signalContainer2.sizeKind = SizeKind::Fixed;
    signalContainer2.name = "MySignal2";

    std::vector<IoSignalContainer> signalContainers = {signalContainer1, signalContainer2};

    // Act
    std::string string = format_as(signalContainers);

    // Assert
    ASSERT_STREQ(
        "[IO Signal { Id: 42, Length: 43, DataType: UInt16, SizeKind: Variable, Name: \"MySignal1\" }, IO Signal { Id: 44, Length: 45, DataType: Bool, "
        "SizeKind: Fixed, Name: \"MySignal2\" }]",
        string.c_str());
}

TEST_F(TestTypes, SignalContainersConvert) {
    // Arrange
    IoSignalContainer signalContainer1{};
    signalContainer1.id = static_cast<IoSignalId>(42);
    signalContainer1.length = 43;
    signalContainer1.dataType = DataType::UInt16;
    signalContainer1.sizeKind = SizeKind::Variable;
    signalContainer1.name = "MySignal1";

    IoSignalContainer signalContainer2{};
    signalContainer2.id = static_cast<IoSignalId>(44);
    signalContainer2.length = 45;
    signalContainer2.dataType = DataType::Bool;
    signalContainer2.sizeKind = SizeKind::Fixed;
    signalContainer2.name = "MySignal2";

    std::vector<IoSignalContainer> signalContainers = {signalContainer1, signalContainer2};

    IoSignal expectedSignal1{};
    expectedSignal1.id = signalContainer1.id;
    expectedSignal1.length = signalContainer1.length;
    expectedSignal1.dataType = signalContainer1.dataType;
    expectedSignal1.sizeKind = signalContainer1.sizeKind;
    expectedSignal1.name = signalContainer1.name.c_str();

    IoSignal expectedSignal2{};
    expectedSignal2.id = signalContainer2.id;
    expectedSignal2.length = signalContainer2.length;
    expectedSignal2.dataType = signalContainer2.dataType;
    expectedSignal2.sizeKind = signalContainer2.sizeKind;
    expectedSignal2.name = signalContainer2.name.c_str();

    std::vector<IoSignal> expectedSignals = {expectedSignal1, expectedSignal2};

    // Act
    std::vector<IoSignal> actualContainers = Convert(signalContainers);

    // Assert
    ASSERT_THAT(actualContainers, ContainerEq(expectedSignals));
}

TEST_F(TestTypes, BusControllerIdToString) {
    // Arrange
    auto controllerId = static_cast<BusControllerId>(123);

    // Act
    std::string string = format_as(controllerId);

    // Assert
    ASSERT_STREQ("123", string.c_str());
}

TEST_F(TestTypes, BusMessageIdToString) {
    // Arrange
    auto messageId = static_cast<BusMessageId>(234);

    // Act
    std::string string = format_as(messageId);

    // Assert
    ASSERT_STREQ("234", string.c_str());
}

TEST_F(TestTypes, CanMessageFlagsToString) {
    // Arrange
    CanMessageFlags flags = CanMessageFlags::Error | CanMessageFlags::Loopback;

    // Act
    std::string string = format_as(flags);

    // Assert
    ASSERT_STREQ("Loopback,Error", string.c_str());
}

TEST_F(TestTypes, CanControllerToString) {
    // Arrange
    std::string name = "name";
    std::string channelName = "channelName";
    std::string clusterName = "clusterName";

    CanController controller{};
    controller.id = static_cast<BusControllerId>(12);
    controller.queueSize = 14;
    controller.bitsPerSecond = 16;
    controller.flexibleDataRateBitsPerSecond = 18;
    controller.name = name.c_str();
    controller.channelName = channelName.c_str();
    controller.clusterName = clusterName.c_str();

    // Act
    std::string string = format_as(controller);

    // Assert
    ASSERT_STREQ(
        "CAN Controller { Id: 12, QueueSize: 14, BitsPerSecond: 16, FlexibleDataRateBitsPerSecond: 18, Name: \"name\", ChannelName: \"channelName\", "
        "ClusterName: \"clusterName\" }",
        string.c_str());
}

TEST_F(TestTypes, CanControllerContainerToString) {
    // Arrange
    CanControllerContainer controllerContainer{};
    controllerContainer.id = static_cast<BusControllerId>(12);
    controllerContainer.queueSize = 14;
    controllerContainer.bitsPerSecond = 16;
    controllerContainer.flexibleDataRateBitsPerSecond = 18;
    controllerContainer.name = "name";
    controllerContainer.channelName = "channelName";
    controllerContainer.clusterName = "clusterName";

    // Act
    std::string string = format_as(controllerContainer);

    // Assert
    ASSERT_STREQ(
        "CAN Controller { Id: 12, QueueSize: 14, BitsPerSecond: 16, FlexibleDataRateBitsPerSecond: 18, Name: \"name\", ChannelName: \"channelName\", "
        "ClusterName: \"clusterName\" }",
        string.c_str());
}

TEST_F(TestTypes, CanControllerContainerConvert) {
    // Arrange
    CanControllerContainer controllerContainer{};
    controllerContainer.id = static_cast<BusControllerId>(22);
    controllerContainer.queueSize = 24;
    controllerContainer.bitsPerSecond = 26;
    controllerContainer.flexibleDataRateBitsPerSecond = 28;
    controllerContainer.name = "name1";
    controllerContainer.channelName = "channelName1";
    controllerContainer.clusterName = "clusterName1";

    CanController expectedController{};
    expectedController.id = controllerContainer.id;
    expectedController.queueSize = controllerContainer.queueSize;
    expectedController.bitsPerSecond = controllerContainer.bitsPerSecond;
    expectedController.flexibleDataRateBitsPerSecond = controllerContainer.flexibleDataRateBitsPerSecond;
    expectedController.name = controllerContainer.name.c_str();
    expectedController.channelName = controllerContainer.channelName.c_str();
    expectedController.clusterName = controllerContainer.clusterName.c_str();

    // Act
    CanController actualController = controllerContainer.Convert();

    // Assert
    ASSERT_EQ(expectedController, actualController);
}

// Add more tests

}  // namespace
