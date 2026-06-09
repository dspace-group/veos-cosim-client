// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <CoSimTypes.hpp>
#include <Logger.hpp>
#include <Result.hpp>

using namespace std::chrono;
using namespace DsVeosCoSim;
using namespace testing;

namespace {

class TestTypes : public Test {};

TEST_F(TestTypes, DataToString) {
    // Arrange
    std::array<uint8_t, 2> data{0xde, 0xaf};
    char separator = '/';

    // Act
    std::string string = DataToString(data.data(), data.size(), separator);

    // Assert
    ASSERT_EQ("de/af", string);
}

TEST_F(TestTypes, SimulationTimeToString) {
    // Arrange
    SimulationTime simulationTime{42};

    // Act
    std::string string = SimulationTimeToString(simulationTime);

    // Assert
    ASSERT_EQ("0.000000042", string);
}

TEST_F(TestTypes, ResultToString) {
    // Arrange
    Result result = CreateError();

    // Act
    std::string_view string = format_as(result);

    // Assert
    ASSERT_EQ("Error", string);
}

TEST_F(TestTypes, CoSimTypeToString) {
    // Arrange
    auto coSimType = CoSimType::Server;

    // Act
    std::string_view string = format_as(coSimType);

    // Assert
    ASSERT_EQ("Server", string);
}

TEST_F(TestTypes, ConnectionKindToString) {
    // Arrange
    auto connectionKind = ConnectionKind::Remote;

    // Act
    std::string_view string = format_as(connectionKind);

    // Assert
    ASSERT_EQ("Remote", string);
}

TEST_F(TestTypes, CommandToString) {
    // Arrange
    auto command = Command::Start;

    // Act
    std::string_view string = format_as(command);

    // Assert
    ASSERT_EQ("Start", string);
}

TEST_F(TestTypes, SeverityToString) {
    // Arrange
    auto severity = Severity::Warning;

    // Act
    std::string_view string = format_as(severity);

    // Assert
    ASSERT_EQ("Warning", string);
}

TEST_F(TestTypes, TerminateReasonToString) {
    // Arrange
    auto terminateReason = TerminateReason::Finished;

    // Act
    std::string_view string = format_as(terminateReason);

    // Assert
    ASSERT_EQ("Finished", string);
}

TEST_F(TestTypes, ConnectionStateToString) {
    // Arrange
    auto connectionState = ConnectionState::Disconnected;

    // Act
    std::string_view string = format_as(connectionState);

    // Assert
    ASSERT_EQ("Disconnected", string);
}

TEST_F(TestTypes, GetDataTypeSize) {
    // Arrange
    auto dataType = DataType::UInt64;

    // Act
    size_t size = GetDataTypeSize(dataType);

    // Assert
    ASSERT_EQ(8, size);
}

TEST_F(TestTypes, DataTypeToString) {
    // Arrange
    auto dataType = DataType::Float64;

    // Act
    std::string_view string = format_as(dataType);

    // Assert
    ASSERT_EQ("Float64", string);
}

TEST_F(TestTypes, SizeKindToString) {
    // Arrange
    auto sizeKind = SizeKind::Variable;

    // Act
    std::string_view string = format_as(sizeKind);

    // Assert
    ASSERT_EQ("Variable", string);
}

TEST_F(TestTypes, ValueToString) {
    // Arrange
    auto dataType = DataType::Float64;
    uint32_t length = 3;
    std::vector<double> data{4.2, 0.000789, 200};

    // Act
    std::string string = ValueToString(dataType, length, data.data());

    // Assert
    ASSERT_EQ("4.2 0.000789 200", string);
}

TEST_F(TestTypes, SimulationStateToString) {
    // Arrange
    auto simulationState = SimulationState::Stopped;

    // Act
    std::string_view string = format_as(simulationState);

    // Assert
    ASSERT_EQ("Stopped", string);
}

TEST_F(TestTypes, ModeToString) {
    // Arrange
    Mode mode{};

    // Act
    std::string_view string = format_as(mode);

    // Assert
    ASSERT_EQ("<Unused>", string);
}

TEST_F(TestTypes, IoSignalIdToString) {
    // Arrange
    auto signalId = static_cast<IoSignalId>(86);

    // Act
    std::string string = format_as(signalId);

    // Assert
    ASSERT_EQ("86", string);
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
    ASSERT_EQ("IO Signal { Id: 42, Length: 43, DataType: UInt16, SizeKind: Variable, Name: \"MySignal\" }", string);
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
    ASSERT_EQ("IO Signal { Id: 42, Length: 43, DataType: UInt16, SizeKind: Variable, Name: \"MySignal\" }", string);
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
    ASSERT_EQ("IO Data { Id: 42, Length: 1, Data: 65432 }", string);
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

    std::vector signalContainers = {signalContainer1, signalContainer2};

    // Act
    std::string string = format_as(signalContainers);

    // Assert
    ASSERT_EQ(
        "[IO Signal { Id: 42, Length: 43, DataType: UInt16, SizeKind: Variable, Name: \"MySignal1\" }, IO Signal { Id: 44, Length: 45, DataType: Bool, "
        "SizeKind: Fixed, Name: \"MySignal2\" }]",
        string);
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

    std::vector signalContainers = {signalContainer1, signalContainer2};

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

    std::vector expectedSignals = {expectedSignal1, expectedSignal2};

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
    ASSERT_EQ("123", string);
}

TEST_F(TestTypes, BusMessageIdToString) {
    // Arrange
    auto messageId = static_cast<BusMessageId>(234);

    // Act
    std::string string = format_as(messageId);

    // Assert
    ASSERT_EQ("234", string);
}

TEST_F(TestTypes, CanMessageFlagsToString) {
    // Arrange
    CanMessageFlags flags = CanMessageFlags::Error | CanMessageFlags::Loopback;

    // Act
    std::string string = format_as(flags);

    // Assert
    ASSERT_EQ("Loopback,Error", string);
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
    ASSERT_EQ(
        "CAN Controller { Id: 12, QueueSize: 14, BitsPerSecond: 16, FlexibleDataRateBitsPerSecond: 18, Name: \"name\", ChannelName: \"channelName\", "
        "ClusterName: \"clusterName\" }",
        string);
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
    ASSERT_EQ(
        "CAN Controller { Id: 12, QueueSize: 14, BitsPerSecond: 16, FlexibleDataRateBitsPerSecond: 18, Name: \"name\", ChannelName: "
        "\"channelName\", ClusterName: \"clusterName\" }",
        string);
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

TEST_F(TestTypes, EthControllerToString) {
    // Arrange
    std::string name = "name";
    std::string channelName = "channelName";
    std::string clusterName = "clusterName";

    EthController controller{};
    controller.id = static_cast<BusControllerId>(12);
    controller.queueSize = 14;
    controller.bitsPerSecond = 16;
    controller.name = name.c_str();
    controller.channelName = channelName.c_str();
    controller.clusterName = clusterName.c_str();

    // Act
    std::string string = format_as(controller);

    // Assert
    ASSERT_EQ(
        "Ethernet Controller { Id: 12, QueueSize: 14, BitsPerSecond: 16, MacAddress: [00:00:00:00:00:00], Name: \"name\", ChannelName: \"channelName\", "
        "ClusterName: \"clusterName\" }",
        string);
}

TEST_F(TestTypes, EthControllerContainerToString) {
    // Arrange
    EthControllerContainer controllerContainer{};
    controllerContainer.id = static_cast<BusControllerId>(12);
    controllerContainer.queueSize = 14;
    controllerContainer.bitsPerSecond = 16;
    controllerContainer.name = "name";
    controllerContainer.channelName = "channelName";
    controllerContainer.clusterName = "clusterName";

    // Act
    std::string string = format_as(controllerContainer);

    // Assert
    ASSERT_EQ(
        "Ethernet Controller { Id: 12, QueueSize: 14, BitsPerSecond: 16, MacAddress: [00:00:00:00:00:00], Name: \"name\", ChannelName: "
        "\"channelName\", ClusterName: \"clusterName\" }",
        string);
}

TEST_F(TestTypes, EthControllerContainerConvert) {
    // Arrange
    EthControllerContainer controllerContainer{};
    controllerContainer.id = static_cast<BusControllerId>(22);
    controllerContainer.queueSize = 24;
    controllerContainer.bitsPerSecond = 26;
    controllerContainer.name = "name1";
    controllerContainer.channelName = "channelName1";
    controllerContainer.clusterName = "clusterName1";

    EthController expectedController{};
    expectedController.id = controllerContainer.id;
    expectedController.queueSize = controllerContainer.queueSize;
    expectedController.bitsPerSecond = controllerContainer.bitsPerSecond;
    expectedController.name = controllerContainer.name.c_str();
    expectedController.channelName = controllerContainer.channelName.c_str();
    expectedController.clusterName = controllerContainer.clusterName.c_str();

    // Act
    EthController actualController = controllerContainer.Convert();

    // Assert
    ASSERT_EQ(expectedController, actualController);
}

TEST_F(TestTypes, LinControllerToString) {
    // Arrange
    std::string name = "name";
    std::string channelName = "channelName";
    std::string clusterName = "clusterName";

    LinController controller{};
    controller.id = static_cast<BusControllerId>(12);
    controller.queueSize = 14;
    controller.bitsPerSecond = 16;
    controller.type = LinControllerType::Commander;
    controller.name = name.c_str();
    controller.channelName = channelName.c_str();
    controller.clusterName = clusterName.c_str();

    // Act
    std::string string = format_as(controller);

    // Assert
    ASSERT_EQ(
        "LIN Controller { Id: 12, QueueSize: 14, BitsPerSecond: 16, Type: Commander, Name: \"name\", ChannelName: \"channelName\", "
        "ClusterName: \"clusterName\" }",
        string);
}

TEST_F(TestTypes, LinControllerContainerToString) {
    // Arrange
    LinControllerContainer controllerContainer{};
    controllerContainer.id = static_cast<BusControllerId>(12);
    controllerContainer.queueSize = 14;
    controllerContainer.bitsPerSecond = 16;
    controllerContainer.type = LinControllerType::Responder;
    controllerContainer.name = "name";
    controllerContainer.channelName = "channelName";
    controllerContainer.clusterName = "clusterName";

    // Act
    std::string string = format_as(controllerContainer);

    // Assert
    ASSERT_EQ(
        "LIN Controller { Id: 12, QueueSize: 14, BitsPerSecond: 16, Type: Responder, Name: \"name\", ChannelName: "
        "\"channelName\", ClusterName: \"clusterName\" }",
        string);
}

TEST_F(TestTypes, LinControllerContainerConvert) {
    // Arrange
    LinControllerContainer controllerContainer{};
    controllerContainer.id = static_cast<BusControllerId>(22);
    controllerContainer.queueSize = 24;
    controllerContainer.bitsPerSecond = 26;
    controllerContainer.name = "name1";
    controllerContainer.channelName = "channelName1";
    controllerContainer.clusterName = "clusterName1";

    LinController expectedController{};
    expectedController.id = controllerContainer.id;
    expectedController.queueSize = controllerContainer.queueSize;
    expectedController.bitsPerSecond = controllerContainer.bitsPerSecond;
    expectedController.name = controllerContainer.name.c_str();
    expectedController.channelName = controllerContainer.channelName.c_str();
    expectedController.clusterName = controllerContainer.clusterName.c_str();

    // Act
    LinController actualController = controllerContainer.Convert();

    // Assert
    ASSERT_EQ(expectedController, actualController);
}

TEST_F(TestTypes, FrControllerToString) {
    // Arrange
    std::string name = "name";
    std::string channelName = "channelName";
    std::string clusterName = "clusterName";

    FrController controller{};
    controller.id = static_cast<BusControllerId>(12);
    controller.queueSize = 14;
    controller.bitsPerSecond = 16;
    controller.name = name.c_str();
    controller.channelName = channelName.c_str();
    controller.clusterName = clusterName.c_str();

    // Act
    std::string string = format_as(controller);

    // Assert
    ASSERT_EQ(
        "FlexRay Controller { Id: 12, QueueSize: 14, BitsPerSecond: 16, Name: \"name\", ChannelName: \"channelName\", "
        "ClusterName: \"clusterName\" }",
        string);
}

TEST_F(TestTypes, FrControllerContainerToString) {
    // Arrange
    FrControllerContainer controllerContainer{};
    controllerContainer.id = static_cast<BusControllerId>(12);
    controllerContainer.queueSize = 14;
    controllerContainer.bitsPerSecond = 16;
    controllerContainer.name = "name";
    controllerContainer.channelName = "channelName";
    controllerContainer.clusterName = "clusterName";

    // Act
    std::string string = format_as(controllerContainer);

    // Assert
    ASSERT_EQ(
        "FlexRay Controller { Id: 12, QueueSize: 14, BitsPerSecond: 16, Name: \"name\", ChannelName: "
        "\"channelName\", ClusterName: \"clusterName\" }",
        string);
}

TEST_F(TestTypes, FrControllerContainerConvert) {
    // Arrange
    FrControllerContainer controllerContainer{};
    controllerContainer.id = static_cast<BusControllerId>(22);
    controllerContainer.queueSize = 24;
    controllerContainer.bitsPerSecond = 26;
    controllerContainer.name = "name1";
    controllerContainer.channelName = "channelName1";
    controllerContainer.clusterName = "clusterName1";

    FrController expectedController{};
    expectedController.id = controllerContainer.id;
    expectedController.queueSize = controllerContainer.queueSize;
    expectedController.bitsPerSecond = controllerContainer.bitsPerSecond;
    expectedController.name = controllerContainer.name.c_str();
    expectedController.channelName = controllerContainer.channelName.c_str();
    expectedController.clusterName = controllerContainer.clusterName.c_str();

    // Act
    FrController actualController = controllerContainer.Convert();

    // Assert
    ASSERT_EQ(expectedController, actualController);
}

TEST_F(TestTypes, CanMessageFlagsNoFlagsToString) {
    // Arrange
    CanMessageFlags flags{};

    // Act
    std::string string = format_as(flags);

    // Assert
    ASSERT_EQ("", string);
}

TEST_F(TestTypes, EthMessageFlagsToString) {
    // Arrange
    EthMessageFlags flags = EthMessageFlags::Loopback | EthMessageFlags::Error;

    // Act
    std::string string = format_as(flags);

    // Assert
    ASSERT_EQ("Loopback,Error", string);
}

TEST_F(TestTypes, LinMessageFlagsToString) {
    // Arrange
    auto flags = LinMessageFlags::Response;

    // Act
    std::string string = format_as(flags);

    // Assert
    ASSERT_EQ("Response", string);
}

TEST_F(TestTypes, FrMessageFlagsToString) {
    // Arrange
    auto flags = FrMessageFlags::SyncFrame;

    // Act
    std::string string = format_as(flags);

    // Assert
    ASSERT_EQ("SyncFrame", string);
}

TEST_F(TestTypes, CanMessageToString) {
    // Arrange
    std::array<uint8_t, 2> data{0xAB, 0xCD};

    CanMessage message{};
    message.timestamp = SimulationTime{42};
    message.controllerId = static_cast<BusControllerId>(5);
    message.id = static_cast<BusMessageId>(10);
    message.length = 2;
    message.data = data.data();

    // Act
    std::string string = format_as(message);

    // Assert
    ASSERT_EQ("CAN Message { Timestamp: 0.000000042, ControllerId: 5, Id: 10, Length: 2, Data: ab-cd, Flags:  }", string);
}

TEST_F(TestTypes, CanMessageContainerToString) {
    // Arrange
    CanMessageContainer message{};
    message.timestamp = SimulationTime{42};
    message.controllerId = static_cast<BusControllerId>(5);
    message.id = static_cast<BusMessageId>(10);
    message.length = 2;
    message.data[0] = 0xAB;
    message.data[1] = 0xCD;

    // Act
    std::string string = format_as(message);

    // Assert
    ASSERT_EQ("CAN Message { Timestamp: 0.000000042, ControllerId: 5, Id: 10, Length: 2, Data: ab-cd, Flags:  }", string);
}

TEST_F(TestTypes, EthMessageToString) {
    // Arrange
    std::array<uint8_t, 2> data{0xAB, 0xCD};

    EthMessage message{};
    message.timestamp = SimulationTime{42};
    message.controllerId = static_cast<BusControllerId>(5);
    message.length = 2;
    message.data = data.data();

    // Act
    std::string string = format_as(message);

    // Assert
    ASSERT_EQ("Ethernet Message { Timestamp: 0.000000042, ControllerId: 5, Length: 2, Data: ab-cd, Flags:  }", string);
}

TEST_F(TestTypes, EthMessageContainerToString) {
    // Arrange
    EthMessageContainer message{};
    message.timestamp = SimulationTime{42};
    message.controllerId = static_cast<BusControllerId>(5);
    message.length = 2;
    message.data[0] = 0xAB;
    message.data[1] = 0xCD;

    // Act
    std::string string = format_as(message);

    // Assert
    ASSERT_EQ("Ethernet Message { Timestamp: 0.000000042, ControllerId: 5, Length: 2, Data: ab-cd, Flags:  }", string);
}

TEST_F(TestTypes, LinMessageToString) {
    // Arrange
    std::array<uint8_t, 2> data{0xAB, 0xCD};

    LinMessage message{};
    message.timestamp = SimulationTime{42};
    message.controllerId = static_cast<BusControllerId>(5);
    message.id = static_cast<BusMessageId>(10);
    message.length = 2;
    message.data = data.data();

    // Act
    std::string string = format_as(message);

    // Assert
    ASSERT_EQ("LIN Message { Timestamp: 0.000000042, ControllerId: 5, Id: 10, Length: 2, Data: ab-cd, Flags:  }", string);
}

TEST_F(TestTypes, LinMessageContainerToString) {
    // Arrange
    LinMessageContainer message{};
    message.timestamp = SimulationTime{42};
    message.controllerId = static_cast<BusControllerId>(5);
    message.id = static_cast<BusMessageId>(10);
    message.length = 2;
    message.data[0] = 0xAB;
    message.data[1] = 0xCD;

    // Act
    std::string string = format_as(message);

    // Assert
    ASSERT_EQ("LIN Message { Timestamp: 0.000000042, ControllerId: 5, Id: 10, Length: 2, Data: ab-cd, Flags:  }", string);
}

TEST_F(TestTypes, FrMessageToString) {
    // Arrange
    std::array<uint8_t, 2> data{0xAB, 0xCD};

    FrMessage message{};
    message.timestamp = SimulationTime{42};
    message.controllerId = static_cast<BusControllerId>(5);
    message.id = static_cast<BusMessageId>(10);
    message.length = 2;
    message.data = data.data();

    // Act
    std::string string = format_as(message);

    // Assert
    ASSERT_EQ("FlexRay Message { Timestamp: 0.000000042, ControllerId: 5, Id: 10, Length: 2, Data: ab-cd, Flags:  }", string);
}

TEST_F(TestTypes, FrMessageContainerToString) {
    // Arrange
    FrMessageContainer message{};
    message.timestamp = SimulationTime{42};
    message.controllerId = static_cast<BusControllerId>(5);
    message.id = static_cast<BusMessageId>(10);
    message.length = 2;
    message.data[0] = 0xAB;
    message.data[1] = 0xCD;

    // Act
    std::string string = format_as(message);

    // Assert
    ASSERT_EQ("FlexRay Message { Timestamp: 0.000000042, ControllerId: 5, Id: 10, Length: 2, Data: ab-cd, Flags:  }", string);
}

TEST_F(TestTypes, ResultOkToString) {
    // Arrange
    Result result = CreateOk();

    // Act
    std::string_view string = format_as(result);

    // Assert
    ASSERT_EQ("Ok", string);
}

TEST_F(TestTypes, SimulationStateRunningToString) {
    // Arrange
    auto simulationState = SimulationState::Running;

    // Act
    std::string_view string = format_as(simulationState);

    // Assert
    ASSERT_EQ("Running", string);
}

TEST_F(TestTypes, ConnectionStateConnectedToString) {
    // Arrange
    auto connectionState = ConnectionState::Connected;

    // Act
    std::string_view string = format_as(connectionState);

    // Assert
    ASSERT_EQ("Connected", string);
}

TEST_F(TestTypes, DataToStringWithZeroLength) {
    // Arrange
    uint8_t data = 0;

    // Act
    std::string string = DataToString(&data, 0, '/');

    // Assert
    ASSERT_EQ("", string);
}

TEST_F(TestTypes, SimulationTimeToStringWithZero) {
    // Arrange
    SimulationTime simulationTime{0};

    // Act
    std::string string = SimulationTimeToString(simulationTime);

    // Assert
    ASSERT_EQ("0", string);
}

TEST_F(TestTypes, ValueToStringWithZeroLength) {
    // Arrange
    auto dataType = DataType::Float64;
    double data = 0.0;

    // Act
    std::string string = ValueToString(dataType, 0, &data);

    // Assert
    ASSERT_EQ("", string);
}

TEST_F(TestTypes, GetDataTypeSizeForAllTypes) {
    ASSERT_EQ(1U, GetDataTypeSize(DataType::Bool));
    ASSERT_EQ(1U, GetDataTypeSize(DataType::Int8));
    ASSERT_EQ(1U, GetDataTypeSize(DataType::UInt8));
    ASSERT_EQ(2U, GetDataTypeSize(DataType::Int16));
    ASSERT_EQ(2U, GetDataTypeSize(DataType::UInt16));
    ASSERT_EQ(4U, GetDataTypeSize(DataType::Int32));
    ASSERT_EQ(4U, GetDataTypeSize(DataType::UInt32));
    ASSERT_EQ(4U, GetDataTypeSize(DataType::Float32));
    ASSERT_EQ(8U, GetDataTypeSize(DataType::Int64));
    ASSERT_EQ(8U, GetDataTypeSize(DataType::UInt64));
    ASSERT_EQ(8U, GetDataTypeSize(DataType::Float64));
}

TEST_F(TestTypes, GetDataTypeSizeForInvalidType) {
    // Arrange
    auto invalidType = static_cast<DataType>(255);

    // Act
    size_t size = GetDataTypeSize(invalidType);

    // Assert
    ASSERT_EQ(0U, size);
}

}  // namespace
