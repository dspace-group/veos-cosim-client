// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <cstring>
#include <string>

#include <gtest/gtest.h>

#include "DsVeosCoSim/DsVeosCoSim.h"
#include "Helper.hpp"

using namespace DsVeosCoSim;

namespace {

class TestDsVeosCoSim : public testing::Test {
protected:
    void SetUp() override {
        _handle = DsVeosCoSim_Create();
        ASSERT_NE(nullptr, _handle);
    }

    void TearDown() override {
        DsVeosCoSim_Destroy(_handle);
    }

    DsVeosCoSim_Handle _handle{};
};

// --- Lifecycle ---

TEST(TestDsVeosCoSimLifecycle, CreateReturnsNonNull) {
    DsVeosCoSim_Handle handle = DsVeosCoSim_Create();
    EXPECT_NE(nullptr, handle);
    DsVeosCoSim_Destroy(handle);
}

TEST(TestDsVeosCoSimLifecycle, DestroyWithNullHandleShouldNotCrash) {
    DsVeosCoSim_Destroy(nullptr);
}

// --- Null Handle Validation ---

TEST(TestDsVeosCoSimLifecycle, AllHandleFunctionsRejectNullHandle) {
    DsVeosCoSim_ConnectConfig config{};
    DsVeosCoSim_Callbacks callbacks{};
    DsVeosCoSim_ConnectionState connectionState{};
    DsVeosCoSim_SimulationTime simulationTime{};
    DsVeosCoSim_SimulationState simulationState{};
    DsVeosCoSim_Command command{};
    int64_t roundTripTime{};
    uint32_t count{};
    const DsVeosCoSim_IoSignal* signals{};
    const DsVeosCoSim_CanController* canControllers{};
    const DsVeosCoSim_EthController* ethControllers{};
    const DsVeosCoSim_LinController* linControllers{};
    const DsVeosCoSim_FrController* frControllers{};
    DsVeosCoSim_CanMessage canMsg{};
    DsVeosCoSim_CanMessageContainer canMsgContainer{};
    DsVeosCoSim_EthMessage ethMsg{};
    DsVeosCoSim_EthMessageContainer ethMsgContainer{};
    DsVeosCoSim_LinMessage linMsg{};
    DsVeosCoSim_LinMessageContainer linMsgContainer{};
    DsVeosCoSim_FrMessage frMsg{};
    DsVeosCoSim_FrMessageContainer frMsgContainer{};
    uint8_t value{};
    uint32_t length{};

    // Connection
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_Connect(nullptr, config));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_Disconnect(nullptr));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_GetConnectionState(nullptr, &connectionState));

    // Co-simulation control
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_RunCallbackBasedCoSimulation(nullptr, callbacks));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_StartPollingBasedCoSimulation(nullptr, callbacks));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_PollCommand(nullptr, &simulationTime, &command));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_PollCommand2(nullptr, &simulationTime, &command, 0));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_FinishCommand(nullptr));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_SetNextSimulationTime(nullptr, 0));

    // Simulation state/info
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_GetStepSize(nullptr, &simulationTime));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_GetCurrentSimulationTime(nullptr, &simulationTime));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_GetSimulationState(nullptr, &simulationState));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_GetRoundTripTime(nullptr, &roundTripTime));

    // Simulation commands
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_StartSimulation(nullptr));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_StopSimulation(nullptr));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_PauseSimulation(nullptr));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_ContinueSimulation(nullptr));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_TerminateSimulation(nullptr, DsVeosCoSim_TerminateReason_Finished));

    // IO signals
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_GetIncomingSignals(nullptr, &count, &signals));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_ReadIncomingSignal(nullptr, {}, &length, &value));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_GetOutgoingSignals(nullptr, &count, &signals));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_WriteOutgoingSignal(nullptr, {}, 0, nullptr));

    // CAN
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_GetCanControllers(nullptr, &count, &canControllers));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_ReceiveCanMessage(nullptr, &canMsg));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_ReceiveCanMessageContainer(nullptr, &canMsgContainer));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_TransmitCanMessage(nullptr, &canMsg));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_TransmitCanMessageContainer(nullptr, &canMsgContainer));

    // Ethernet
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_GetEthControllers(nullptr, &count, &ethControllers));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_ReceiveEthMessage(nullptr, &ethMsg));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_ReceiveEthMessageContainer(nullptr, &ethMsgContainer));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_TransmitEthMessage(nullptr, &ethMsg));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_TransmitEthMessageContainer(nullptr, &ethMsgContainer));

    // LIN
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_GetLinControllers(nullptr, &count, &linControllers));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_ReceiveLinMessage(nullptr, &linMsg));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_ReceiveLinMessageContainer(nullptr, &linMsgContainer));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_TransmitLinMessage(nullptr, &linMsg));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_TransmitLinMessageContainer(nullptr, &linMsgContainer));

    // FlexRay
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_GetFrControllers(nullptr, &count, &frControllers));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_ReceiveFrMessage(nullptr, &frMsg));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_ReceiveFrMessageContainer(nullptr, &frMsgContainer));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_TransmitFrMessage(nullptr, &frMsg));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_TransmitFrMessageContainer(nullptr, &frMsgContainer));
}

// --- Null Output Pointer Validation (with valid handle) ---

TEST_F(TestDsVeosCoSim, GetConnectionStateNullOutputPointer) {
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_GetConnectionState(_handle, nullptr));
}

TEST_F(TestDsVeosCoSim, GetStepSizeNullOutputPointer) {
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_GetStepSize(_handle, nullptr));
}

TEST_F(TestDsVeosCoSim, GetCurrentSimulationTimeNullOutputPointer) {
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_GetCurrentSimulationTime(_handle, nullptr));
}

TEST_F(TestDsVeosCoSim, GetSimulationStateNullOutputPointer) {
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_GetSimulationState(_handle, nullptr));
}

TEST_F(TestDsVeosCoSim, GetRoundTripTimeNullOutputPointer) {
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_GetRoundTripTime(_handle, nullptr));
}

TEST_F(TestDsVeosCoSim, PollCommandNullOutputPointers) {
    DsVeosCoSim_SimulationTime simulationTime{};
    DsVeosCoSim_Command command{};

    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_PollCommand(_handle, nullptr, &command));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_PollCommand(_handle, &simulationTime, nullptr));
}

TEST_F(TestDsVeosCoSim, PollCommand2NullOutputPointers) {
    DsVeosCoSim_SimulationTime simulationTime{};
    DsVeosCoSim_Command command{};

    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_PollCommand2(_handle, nullptr, &command, 0));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_PollCommand2(_handle, &simulationTime, nullptr, 0));
}

TEST_F(TestDsVeosCoSim, GetIncomingSignalsNullOutputPointers) {
    uint32_t count{};
    const DsVeosCoSim_IoSignal* signals{};

    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_GetIncomingSignals(_handle, nullptr, &signals));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_GetIncomingSignals(_handle, &count, nullptr));
}

TEST_F(TestDsVeosCoSim, GetOutgoingSignalsNullOutputPointers) {
    uint32_t count{};
    const DsVeosCoSim_IoSignal* signals{};

    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_GetOutgoingSignals(_handle, nullptr, &signals));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_GetOutgoingSignals(_handle, &count, nullptr));
}

TEST_F(TestDsVeosCoSim, ReadIncomingSignalNullOutputPointers) {
    uint32_t length{};
    uint8_t value{};

    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_ReadIncomingSignal(_handle, {}, nullptr, &value));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_ReadIncomingSignal(_handle, {}, &length, nullptr));
}

TEST_F(TestDsVeosCoSim, GetCanControllersNullOutputPointers) {
    uint32_t count{};
    const DsVeosCoSim_CanController* controllers{};

    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_GetCanControllers(_handle, nullptr, &controllers));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_GetCanControllers(_handle, &count, nullptr));
}

TEST_F(TestDsVeosCoSim, GetEthControllersNullOutputPointers) {
    uint32_t count{};
    const DsVeosCoSim_EthController* controllers{};

    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_GetEthControllers(_handle, nullptr, &controllers));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_GetEthControllers(_handle, &count, nullptr));
}

TEST_F(TestDsVeosCoSim, GetLinControllersNullOutputPointers) {
    uint32_t count{};
    const DsVeosCoSim_LinController* controllers{};

    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_GetLinControllers(_handle, nullptr, &controllers));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_GetLinControllers(_handle, &count, nullptr));
}

TEST_F(TestDsVeosCoSim, GetFrControllersNullOutputPointers) {
    uint32_t count{};
    const DsVeosCoSim_FrController* controllers{};

    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_GetFrControllers(_handle, nullptr, &controllers));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_GetFrControllers(_handle, &count, nullptr));
}

TEST_F(TestDsVeosCoSim, ReceiveAndTransmitNullMessagePointers) {
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_ReceiveCanMessage(_handle, nullptr));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_ReceiveCanMessageContainer(_handle, nullptr));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_TransmitCanMessage(_handle, nullptr));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_TransmitCanMessageContainer(_handle, nullptr));

    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_ReceiveEthMessage(_handle, nullptr));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_ReceiveEthMessageContainer(_handle, nullptr));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_TransmitEthMessage(_handle, nullptr));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_TransmitEthMessageContainer(_handle, nullptr));

    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_ReceiveLinMessage(_handle, nullptr));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_ReceiveLinMessageContainer(_handle, nullptr));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_TransmitLinMessage(_handle, nullptr));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_TransmitLinMessageContainer(_handle, nullptr));

    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_ReceiveFrMessage(_handle, nullptr));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_ReceiveFrMessageContainer(_handle, nullptr));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_TransmitFrMessage(_handle, nullptr));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_TransmitFrMessageContainer(_handle, nullptr));
}

// --- Connect string handling ---

TEST_F(TestDsVeosCoSim, ConnectWithAllNullStringsInConfig) {
    DsVeosCoSim_ConnectConfig config{};
    config.remoteIpAddress = nullptr;
    config.serverName = nullptr;
    config.clientName = nullptr;
    config.remotePort = 0;
    config.localPort = 0;

    // All null/zero config is rejected as InvalidArgument (empty serverName).
    // The key test: no crash from nullptr→string conversion.
    DsVeosCoSim_Result result = DsVeosCoSim_Connect(_handle, config);
    EXPECT_NE(DsVeosCoSim_Result_Ok, result);
}

// --- WriteOutgoingSignal conditional null ---

TEST_F(TestDsVeosCoSim, WriteOutgoingSignalNullValueWithZeroLengthIsAllowed) {
    // length=0 + value=nullptr should NOT return InvalidArgument.
    // It fails because not connected, but the null check is bypassed.
    DsVeosCoSim_Result result = DsVeosCoSim_WriteOutgoingSignal(_handle, {}, 0, nullptr);
    EXPECT_NE(DsVeosCoSim_Result_InvalidArgument, result);
}

TEST_F(TestDsVeosCoSim, WriteOutgoingSignalNullValueWithNonZeroLengthShouldReturnInvalidArgument) {
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_WriteOutgoingSignal(_handle, {}, 1, nullptr));
}

// --- SetLogCallback ---

namespace {

bool g_logCallbackCalled = false;

void TestLogCallback([[maybe_unused]] DsVeosCoSim_Severity severity, [[maybe_unused]] const char* message) {
    g_logCallbackCalled = true;
}

}  // namespace

TEST(TestDsVeosCoSimLifecycle, SetLogCallbackReceivesMessages) {
    g_logCallbackCalled = false;
    DsVeosCoSim_SetLogCallback(TestLogCallback);

    // Trigger a log message by calling a function with null handle.
    DsVeosCoSim_Disconnect(nullptr);

    EXPECT_TRUE(g_logCallbackCalled);

    // Restore null callback.
    DsVeosCoSim_SetLogCallback(nullptr);
}

TEST(TestDsVeosCoSimLifecycle, SetLogCallbackWithNullShouldNotCrash) {
    DsVeosCoSim_SetLogCallback(nullptr);

    // Trigger a log message — should not crash even with null callback.
    DsVeosCoSim_Disconnect(nullptr);

    DsVeosCoSim_SetLogCallback(nullptr);
}

// --- C ToString Functions ---

TEST(TestDsVeosCoSimToString, ResultToString) {
    EXPECT_STREQ("Ok", DsVeosCoSim_ResultToString(DsVeosCoSim_Result_Ok));
    EXPECT_STREQ("Error", DsVeosCoSim_ResultToString(DsVeosCoSim_Result_Error));
    EXPECT_STREQ("Empty", DsVeosCoSim_ResultToString(DsVeosCoSim_Result_Empty));
    EXPECT_STREQ("Full", DsVeosCoSim_ResultToString(DsVeosCoSim_Result_Full));
    EXPECT_STREQ("InvalidArgument", DsVeosCoSim_ResultToString(DsVeosCoSim_Result_InvalidArgument));
    EXPECT_STREQ("NotConnected", DsVeosCoSim_ResultToString(DsVeosCoSim_Result_Disconnected));
    EXPECT_STREQ("Timeout", DsVeosCoSim_ResultToString(DsVeosCoSim_Result_Timeout));
}

TEST(TestDsVeosCoSimToString, CommandToString) {
    EXPECT_STREQ("None", DsVeosCoSim_CommandToString(DsVeosCoSim_Command_None));
    EXPECT_STREQ("Step", DsVeosCoSim_CommandToString(DsVeosCoSim_Command_Step));
    EXPECT_STREQ("Start", DsVeosCoSim_CommandToString(DsVeosCoSim_Command_Start));
    EXPECT_STREQ("Stop", DsVeosCoSim_CommandToString(DsVeosCoSim_Command_Stop));
    EXPECT_STREQ("Terminate", DsVeosCoSim_CommandToString(DsVeosCoSim_Command_Terminate));
    EXPECT_STREQ("Pause", DsVeosCoSim_CommandToString(DsVeosCoSim_Command_Pause));
    EXPECT_STREQ("Continue", DsVeosCoSim_CommandToString(DsVeosCoSim_Command_Continue));
    EXPECT_STREQ("TerminateFinished", DsVeosCoSim_CommandToString(DsVeosCoSim_Command_TerminateFinished));
    EXPECT_STREQ("Ping", DsVeosCoSim_CommandToString(DsVeosCoSim_Command_Ping));
}

TEST(TestDsVeosCoSimToString, SeverityToString) {
    EXPECT_STREQ("Error", DsVeosCoSim_SeverityToString(DsVeosCoSim_Severity_Error));
    EXPECT_STREQ("Warning", DsVeosCoSim_SeverityToString(DsVeosCoSim_Severity_Warning));
    EXPECT_STREQ("Info", DsVeosCoSim_SeverityToString(DsVeosCoSim_Severity_Info));
    EXPECT_STREQ("Trace", DsVeosCoSim_SeverityToString(DsVeosCoSim_Severity_Trace));
}

TEST(TestDsVeosCoSimToString, TerminateReasonToString) {
    EXPECT_STREQ("Finished", DsVeosCoSim_TerminateReasonToString(DsVeosCoSim_TerminateReason_Finished));
    EXPECT_STREQ("Error", DsVeosCoSim_TerminateReasonToString(DsVeosCoSim_TerminateReason_Error));
}

TEST(TestDsVeosCoSimToString, ConnectionStateToString) {
    EXPECT_STREQ("Disconnected", DsVeosCoSim_ConnectionStateToString(DsVeosCoSim_ConnectionState_Disconnected));
    EXPECT_STREQ("Connected", DsVeosCoSim_ConnectionStateToString(DsVeosCoSim_ConnectionState_Connected));
}

TEST(TestDsVeosCoSimToString, SimulationStateToString) {
    EXPECT_STREQ("Unloaded", DsVeosCoSim_SimulationStateToString(DsVeosCoSim_SimulationState_Unloaded));
    EXPECT_STREQ("Stopped", DsVeosCoSim_SimulationStateToString(DsVeosCoSim_SimulationState_Stopped));
    EXPECT_STREQ("Running", DsVeosCoSim_SimulationStateToString(DsVeosCoSim_SimulationState_Running));
    EXPECT_STREQ("Paused", DsVeosCoSim_SimulationStateToString(DsVeosCoSim_SimulationState_Paused));
    EXPECT_STREQ("Terminated", DsVeosCoSim_SimulationStateToString(DsVeosCoSim_SimulationState_Terminated));
}

TEST(TestDsVeosCoSimToString, DataTypeToString) {
    EXPECT_STREQ("Bool", DsVeosCoSim_DataTypeToString(DsVeosCoSim_DataType_Bool));
    EXPECT_STREQ("Int8", DsVeosCoSim_DataTypeToString(DsVeosCoSim_DataType_Int8));
    EXPECT_STREQ("Int16", DsVeosCoSim_DataTypeToString(DsVeosCoSim_DataType_Int16));
    EXPECT_STREQ("Int32", DsVeosCoSim_DataTypeToString(DsVeosCoSim_DataType_Int32));
    EXPECT_STREQ("Int64", DsVeosCoSim_DataTypeToString(DsVeosCoSim_DataType_Int64));
    EXPECT_STREQ("UInt8", DsVeosCoSim_DataTypeToString(DsVeosCoSim_DataType_UInt8));
    EXPECT_STREQ("UInt16", DsVeosCoSim_DataTypeToString(DsVeosCoSim_DataType_UInt16));
    EXPECT_STREQ("UInt32", DsVeosCoSim_DataTypeToString(DsVeosCoSim_DataType_UInt32));
    EXPECT_STREQ("UInt64", DsVeosCoSim_DataTypeToString(DsVeosCoSim_DataType_UInt64));
    EXPECT_STREQ("Float32", DsVeosCoSim_DataTypeToString(DsVeosCoSim_DataType_Float32));
    EXPECT_STREQ("Float64", DsVeosCoSim_DataTypeToString(DsVeosCoSim_DataType_Float64));
}

TEST(TestDsVeosCoSimToString, SizeKindToString) {
    EXPECT_STREQ("Fixed", DsVeosCoSim_SizeKindToString(DsVeosCoSim_SizeKind_Fixed));
    EXPECT_STREQ("Variable", DsVeosCoSim_SizeKindToString(DsVeosCoSim_SizeKind_Variable));
}

TEST(TestDsVeosCoSimToString, LinControllerTypeToString) {
    EXPECT_STREQ("Responder", DsVeosCoSim_LinControllerTypeToString(DsVeosCoSim_LinControllerType_Responder));
    EXPECT_STREQ("Commander", DsVeosCoSim_LinControllerTypeToString(DsVeosCoSim_LinControllerType_Commander));
}

TEST(TestDsVeosCoSimToString, GetDataTypeSize) {
    EXPECT_EQ(1U, DsVeosCoSim_GetDataTypeSize(DsVeosCoSim_DataType_Bool));
    EXPECT_EQ(1U, DsVeosCoSim_GetDataTypeSize(DsVeosCoSim_DataType_Int8));
    EXPECT_EQ(2U, DsVeosCoSim_GetDataTypeSize(DsVeosCoSim_DataType_Int16));
    EXPECT_EQ(4U, DsVeosCoSim_GetDataTypeSize(DsVeosCoSim_DataType_Int32));
    EXPECT_EQ(8U, DsVeosCoSim_GetDataTypeSize(DsVeosCoSim_DataType_Int64));
    EXPECT_EQ(1U, DsVeosCoSim_GetDataTypeSize(DsVeosCoSim_DataType_UInt8));
    EXPECT_EQ(2U, DsVeosCoSim_GetDataTypeSize(DsVeosCoSim_DataType_UInt16));
    EXPECT_EQ(4U, DsVeosCoSim_GetDataTypeSize(DsVeosCoSim_DataType_UInt32));
    EXPECT_EQ(8U, DsVeosCoSim_GetDataTypeSize(DsVeosCoSim_DataType_UInt64));
    EXPECT_EQ(4U, DsVeosCoSim_GetDataTypeSize(DsVeosCoSim_DataType_Float32));
    EXPECT_EQ(8U, DsVeosCoSim_GetDataTypeSize(DsVeosCoSim_DataType_Float64));
}

// --- Message Container Conversions ---

TEST(TestDsVeosCoSimConversion, CanMessageContainerToMessageRoundTrip) {
    DsVeosCoSim_CanMessageContainer container{};
    container.timestamp = 123456789;
    container.controllerId = 42;
    container.id = 100;
    container.flags = DsVeosCoSim_CanMessageFlags_ExtendedId;
    container.length = 8;
    FillWithRandomData(container.data, container.length);

    DsVeosCoSim_CanMessage message{};
    ASSERT_EQ(DsVeosCoSim_Result_Ok, DsVeosCoSim_WriteCanMessageContainerToMessage(&container, &message));

    EXPECT_EQ(container.timestamp, message.timestamp);
    EXPECT_EQ(container.controllerId, message.controllerId);
    EXPECT_EQ(container.id, message.id);
    EXPECT_EQ(container.flags, message.flags);
    EXPECT_EQ(container.length, message.length);
    EXPECT_EQ(0, std::memcmp(container.data, message.data, container.length));

    DsVeosCoSim_CanMessageContainer roundTripped{};
    ASSERT_EQ(DsVeosCoSim_Result_Ok, DsVeosCoSim_WriteCanMessageToMessageContainer(&message, &roundTripped));

    EXPECT_EQ(container.timestamp, roundTripped.timestamp);
    EXPECT_EQ(container.controllerId, roundTripped.controllerId);
    EXPECT_EQ(container.id, roundTripped.id);
    EXPECT_EQ(container.flags, roundTripped.flags);
    EXPECT_EQ(container.length, roundTripped.length);
    EXPECT_EQ(0, std::memcmp(container.data, roundTripped.data, container.length));
}

TEST(TestDsVeosCoSimConversion, CanMessageConversionNullArgs) {
    DsVeosCoSim_CanMessageContainer container{};
    DsVeosCoSim_CanMessage message{};

    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_WriteCanMessageContainerToMessage(nullptr, &message));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_WriteCanMessageContainerToMessage(&container, nullptr));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_WriteCanMessageToMessageContainer(nullptr, &container));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_WriteCanMessageToMessageContainer(&message, nullptr));
}

TEST(TestDsVeosCoSimConversion, EthMessageContainerToMessageRoundTrip) {
    DsVeosCoSim_EthMessageContainer container{};
    container.timestamp = 987654321;
    container.controllerId = 7;
    container.flags = DsVeosCoSim_EthMessageFlags_Loopback;
    container.length = 6;
    FillWithRandomData(container.data, container.length);

    DsVeosCoSim_EthMessage message{};
    ASSERT_EQ(DsVeosCoSim_Result_Ok, DsVeosCoSim_WriteEthMessageContainerToMessage(&container, &message));

    EXPECT_EQ(container.timestamp, message.timestamp);
    EXPECT_EQ(container.controllerId, message.controllerId);
    EXPECT_EQ(container.flags, message.flags);
    EXPECT_EQ(container.length, message.length);
    EXPECT_EQ(0, std::memcmp(container.data, message.data, container.length));

    DsVeosCoSim_EthMessageContainer roundTripped{};
    ASSERT_EQ(DsVeosCoSim_Result_Ok, DsVeosCoSim_WriteEthMessageToMessageContainer(&message, &roundTripped));

    EXPECT_EQ(container.timestamp, roundTripped.timestamp);
    EXPECT_EQ(container.controllerId, roundTripped.controllerId);
    EXPECT_EQ(container.flags, roundTripped.flags);
    EXPECT_EQ(container.length, roundTripped.length);
    EXPECT_EQ(0, std::memcmp(container.data, roundTripped.data, container.length));
}

TEST(TestDsVeosCoSimConversion, EthMessageConversionNullArgs) {
    DsVeosCoSim_EthMessageContainer container{};
    DsVeosCoSim_EthMessage message{};

    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_WriteEthMessageContainerToMessage(nullptr, &message));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_WriteEthMessageContainerToMessage(&container, nullptr));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_WriteEthMessageToMessageContainer(nullptr, &container));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_WriteEthMessageToMessageContainer(&message, nullptr));
}

TEST(TestDsVeosCoSimConversion, LinMessageContainerToMessageRoundTrip) {
    DsVeosCoSim_LinMessageContainer container{};
    container.timestamp = 555555555;
    container.controllerId = 3;
    container.id = 50;
    container.flags = DsVeosCoSim_LinMessageFlags_Header;
    container.length = 4;
    FillWithRandomData(container.data, container.length);

    DsVeosCoSim_LinMessage message{};
    ASSERT_EQ(DsVeosCoSim_Result_Ok, DsVeosCoSim_WriteLinMessageContainerToMessage(&container, &message));

    EXPECT_EQ(container.timestamp, message.timestamp);
    EXPECT_EQ(container.controllerId, message.controllerId);
    EXPECT_EQ(container.id, message.id);
    EXPECT_EQ(container.flags, message.flags);
    EXPECT_EQ(container.length, message.length);
    EXPECT_EQ(0, std::memcmp(container.data, message.data, container.length));

    DsVeosCoSim_LinMessageContainer roundTripped{};
    ASSERT_EQ(DsVeosCoSim_Result_Ok, DsVeosCoSim_WriteLinMessageToMessageContainer(&message, &roundTripped));

    EXPECT_EQ(container.timestamp, roundTripped.timestamp);
    EXPECT_EQ(container.controllerId, roundTripped.controllerId);
    EXPECT_EQ(container.id, roundTripped.id);
    EXPECT_EQ(container.flags, roundTripped.flags);
    EXPECT_EQ(container.length, roundTripped.length);
    EXPECT_EQ(0, std::memcmp(container.data, roundTripped.data, container.length));
}

TEST(TestDsVeosCoSimConversion, LinMessageConversionNullArgs) {
    DsVeosCoSim_LinMessageContainer container{};
    DsVeosCoSim_LinMessage message{};

    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_WriteLinMessageContainerToMessage(nullptr, &message));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_WriteLinMessageContainerToMessage(&container, nullptr));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_WriteLinMessageToMessageContainer(nullptr, &container));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_WriteLinMessageToMessageContainer(&message, nullptr));
}

TEST(TestDsVeosCoSimConversion, FrMessageContainerToMessageRoundTrip) {
    DsVeosCoSim_FrMessageContainer container{};
    container.timestamp = 111222333;
    container.controllerId = 1;
    container.id = 200;
    container.flags = DsVeosCoSim_FrMessageFlags_Startup | DsVeosCoSim_FrMessageFlags_ChannelA;
    container.length = 8;
    FillWithRandomData(container.data, container.length);

    DsVeosCoSim_FrMessage message{};
    ASSERT_EQ(DsVeosCoSim_Result_Ok, DsVeosCoSim_WriteFrMessageContainerToMessage(&container, &message));

    EXPECT_EQ(container.timestamp, message.timestamp);
    EXPECT_EQ(container.controllerId, message.controllerId);
    EXPECT_EQ(container.id, message.id);
    EXPECT_EQ(container.flags, message.flags);
    EXPECT_EQ(container.length, message.length);
    EXPECT_EQ(0, std::memcmp(container.data, message.data, container.length));

    DsVeosCoSim_FrMessageContainer roundTripped{};
    ASSERT_EQ(DsVeosCoSim_Result_Ok, DsVeosCoSim_WriteFrMessageToMessageContainer(&message, &roundTripped));

    EXPECT_EQ(container.timestamp, roundTripped.timestamp);
    EXPECT_EQ(container.controllerId, roundTripped.controllerId);
    EXPECT_EQ(container.id, roundTripped.id);
    EXPECT_EQ(container.flags, roundTripped.flags);
    EXPECT_EQ(container.length, roundTripped.length);
    EXPECT_EQ(0, std::memcmp(container.data, roundTripped.data, container.length));
}

TEST(TestDsVeosCoSimConversion, FrMessageConversionNullArgs) {
    DsVeosCoSim_FrMessageContainer container{};
    DsVeosCoSim_FrMessage message{};

    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_WriteFrMessageContainerToMessage(nullptr, &message));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_WriteFrMessageContainerToMessage(&container, nullptr));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_WriteFrMessageToMessageContainer(nullptr, &container));
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_WriteFrMessageToMessageContainer(&message, nullptr));
}

// --- Buffer-based ToString Functions ---

// Runs the common two-call (query-size) contract checks against a ToString function.
// The callable is invoked as fn(char* buffer, uint32_t* bufferSize).
template <typename Fn>
void CheckToStringBuffer(const Fn& fn) {
    // Query mode: a null buffer returns the required size including the terminating null.
    uint32_t requiredSize = 0;
    EXPECT_EQ(DsVeosCoSim_Result_Full, fn(nullptr, &requiredSize));
    EXPECT_GT(requiredSize, 0U);

    // A too-small buffer returns Full and reports the required size.
    if (requiredSize > 1) {
        char tooSmall = '\1';
        uint32_t smallSize = 1;
        EXPECT_EQ(DsVeosCoSim_Result_Full, fn(&tooSmall, &smallSize));
        EXPECT_EQ(requiredSize, smallSize);
    }

    // A sufficient buffer succeeds and produces a null-terminated string.
    std::string buffer(requiredSize, '\1');
    uint32_t bufferSize = requiredSize;
    EXPECT_EQ(DsVeosCoSim_Result_Ok, fn(buffer.data(), &bufferSize));
    EXPECT_EQ(requiredSize, bufferSize);
    EXPECT_EQ('\0', buffer[requiredSize - 1]);
    EXPECT_EQ(static_cast<size_t>(requiredSize - 1), std::strlen(buffer.c_str()));

    // A null bufferSize is rejected.
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, fn(buffer.data(), nullptr));
}

TEST(TestDsVeosCoSimToStringBuffer, SimulationTimeToString) {
    CheckToStringBuffer([](char* buffer, uint32_t* bufferSize) {
        return DsVeosCoSim_SimulationTimeToString(1000000000, buffer, bufferSize);
    });
}

TEST(TestDsVeosCoSimToStringBuffer, IoSignalToString) {
    DsVeosCoSim_IoSignal signal{};
    signal.id = 1;
    signal.length = 4;
    signal.dataType = DsVeosCoSim_DataType_Int32;
    signal.sizeKind = DsVeosCoSim_SizeKind_Fixed;
    signal.name = "Signal";

    CheckToStringBuffer([&](char* buffer, uint32_t* bufferSize) {
        return DsVeosCoSim_IoSignalToString(&signal, buffer, bufferSize);
    });

    uint32_t bufferSize = 0;
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_IoSignalToString(nullptr, nullptr, &bufferSize));
}

TEST(TestDsVeosCoSimToStringBuffer, CanControllerToString) {
    DsVeosCoSim_CanController controller{};
    controller.id = 1;
    controller.queueSize = 10;
    controller.bitsPerSecond = 500000;
    controller.flexibleDataRateBitsPerSecond = 2000000;
    controller.name = "CanController";
    controller.channelName = "Channel";
    controller.clusterName = "Cluster";

    CheckToStringBuffer([&](char* buffer, uint32_t* bufferSize) {
        return DsVeosCoSim_CanControllerToString(&controller, buffer, bufferSize);
    });

    uint32_t bufferSize = 0;
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_CanControllerToString(nullptr, nullptr, &bufferSize));
}

TEST(TestDsVeosCoSimToStringBuffer, EthControllerToString) {
    DsVeosCoSim_EthController controller{};
    controller.id = 1;
    controller.queueSize = 10;
    controller.bitsPerSecond = 1000000000;
    controller.name = "EthController";
    controller.channelName = "Channel";
    controller.clusterName = "Cluster";

    CheckToStringBuffer([&](char* buffer, uint32_t* bufferSize) {
        return DsVeosCoSim_EthControllerToString(&controller, buffer, bufferSize);
    });

    uint32_t bufferSize = 0;
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_EthControllerToString(nullptr, nullptr, &bufferSize));
}

TEST(TestDsVeosCoSimToStringBuffer, LinControllerToString) {
    DsVeosCoSim_LinController controller{};
    controller.id = 1;
    controller.queueSize = 10;
    controller.bitsPerSecond = 20000;
    controller.type = DsVeosCoSim_LinControllerType_Commander;
    controller.name = "LinController";
    controller.channelName = "Channel";
    controller.clusterName = "Cluster";

    CheckToStringBuffer([&](char* buffer, uint32_t* bufferSize) {
        return DsVeosCoSim_LinControllerToString(&controller, buffer, bufferSize);
    });

    uint32_t bufferSize = 0;
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_LinControllerToString(nullptr, nullptr, &bufferSize));
}

TEST(TestDsVeosCoSimToStringBuffer, FrControllerToString) {
    DsVeosCoSim_FrController controller{};
    controller.id = 1;
    controller.queueSize = 10;
    controller.bitsPerSecond = 10000000;
    controller.name = "FrController";
    controller.channelName = "Channel";
    controller.clusterName = "Cluster";

    CheckToStringBuffer([&](char* buffer, uint32_t* bufferSize) {
        return DsVeosCoSim_FrControllerToString(&controller, buffer, bufferSize);
    });

    uint32_t bufferSize = 0;
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_FrControllerToString(nullptr, nullptr, &bufferSize));
}

TEST(TestDsVeosCoSimToStringBuffer, ValueToString) {
    int32_t value = 42;
    CheckToStringBuffer([&](char* buffer, uint32_t* bufferSize) {
        return DsVeosCoSim_ValueToString(DsVeosCoSim_DataType_Int32, 1, &value, buffer, bufferSize);
    });
}

TEST(TestDsVeosCoSimToStringBuffer, DataToString) {
    uint8_t data[4] = {0x01, 0x02, 0x03, 0x04};
    CheckToStringBuffer([&](char* buffer, uint32_t* bufferSize) {
        return DsVeosCoSim_DataToString(data, sizeof(data), ' ', buffer, bufferSize);
    });
}

TEST(TestDsVeosCoSimToStringBuffer, IoDataToString) {
    DsVeosCoSim_IoSignal signal{};
    signal.id = 1;
    signal.length = 1;
    signal.dataType = DsVeosCoSim_DataType_Int32;
    signal.sizeKind = DsVeosCoSim_SizeKind_Fixed;
    signal.name = "Signal";

    int32_t value = 7;
    CheckToStringBuffer([&](char* buffer, uint32_t* bufferSize) {
        return DsVeosCoSim_IoDataToString(&signal, 1, &value, buffer, bufferSize);
    });

    uint32_t bufferSize = 0;
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_IoDataToString(nullptr, 1, &value, nullptr, &bufferSize));
}

TEST(TestDsVeosCoSimToStringBuffer, CanMessageToString) {
    uint8_t data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    DsVeosCoSim_CanMessage message{};
    message.timestamp = 123;
    message.controllerId = 1;
    message.id = 100;
    message.flags = DsVeosCoSim_CanMessageFlags_ExtendedId;
    message.length = sizeof(data);
    message.data = data;

    CheckToStringBuffer([&](char* buffer, uint32_t* bufferSize) {
        return DsVeosCoSim_CanMessageToString(&message, buffer, bufferSize);
    });

    uint32_t bufferSize = 0;
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_CanMessageToString(nullptr, nullptr, &bufferSize));
}

TEST(TestDsVeosCoSimToStringBuffer, EthMessageToString) {
    uint8_t data[6] = {1, 2, 3, 4, 5, 6};
    DsVeosCoSim_EthMessage message{};
    message.timestamp = 123;
    message.controllerId = 1;
    message.flags = DsVeosCoSim_EthMessageFlags_Loopback;
    message.length = sizeof(data);
    message.data = data;

    CheckToStringBuffer([&](char* buffer, uint32_t* bufferSize) {
        return DsVeosCoSim_EthMessageToString(&message, buffer, bufferSize);
    });

    uint32_t bufferSize = 0;
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_EthMessageToString(nullptr, nullptr, &bufferSize));
}

TEST(TestDsVeosCoSimToStringBuffer, LinMessageToString) {
    uint8_t data[4] = {1, 2, 3, 4};
    DsVeosCoSim_LinMessage message{};
    message.timestamp = 123;
    message.controllerId = 1;
    message.id = 50;
    message.flags = DsVeosCoSim_LinMessageFlags_Header;
    message.length = sizeof(data);
    message.data = data;

    CheckToStringBuffer([&](char* buffer, uint32_t* bufferSize) {
        return DsVeosCoSim_LinMessageToString(&message, buffer, bufferSize);
    });

    uint32_t bufferSize = 0;
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_LinMessageToString(nullptr, nullptr, &bufferSize));
}

TEST(TestDsVeosCoSimToStringBuffer, FrMessageToString) {
    uint8_t data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    DsVeosCoSim_FrMessage message{};
    message.timestamp = 123;
    message.controllerId = 1;
    message.id = 200;
    message.flags = DsVeosCoSim_FrMessageFlags_Startup;
    message.length = sizeof(data);
    message.data = data;

    CheckToStringBuffer([&](char* buffer, uint32_t* bufferSize) {
        return DsVeosCoSim_FrMessageToString(&message, buffer, bufferSize);
    });

    uint32_t bufferSize = 0;
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_FrMessageToString(nullptr, nullptr, &bufferSize));
}

TEST(TestDsVeosCoSimToStringBuffer, CanMessageContainerToString) {
    DsVeosCoSim_CanMessageContainer container{};
    container.timestamp = 123;
    container.controllerId = 1;
    container.id = 100;
    container.flags = DsVeosCoSim_CanMessageFlags_ExtendedId;
    container.length = 8;
    FillWithRandomData(container.data, container.length);

    CheckToStringBuffer([&](char* buffer, uint32_t* bufferSize) {
        return DsVeosCoSim_CanMessageContainerToString(&container, buffer, bufferSize);
    });

    uint32_t bufferSize = 0;
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_CanMessageContainerToString(nullptr, nullptr, &bufferSize));
}

TEST(TestDsVeosCoSimToStringBuffer, EthMessageContainerToString) {
    DsVeosCoSim_EthMessageContainer container{};
    container.timestamp = 123;
    container.controllerId = 1;
    container.flags = DsVeosCoSim_EthMessageFlags_Loopback;
    container.length = 6;
    FillWithRandomData(container.data, container.length);

    CheckToStringBuffer([&](char* buffer, uint32_t* bufferSize) {
        return DsVeosCoSim_EthMessageContainerToString(&container, buffer, bufferSize);
    });

    uint32_t bufferSize = 0;
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_EthMessageContainerToString(nullptr, nullptr, &bufferSize));
}

TEST(TestDsVeosCoSimToStringBuffer, LinMessageContainerToString) {
    DsVeosCoSim_LinMessageContainer container{};
    container.timestamp = 123;
    container.controllerId = 1;
    container.id = 50;
    container.flags = DsVeosCoSim_LinMessageFlags_Header;
    container.length = 4;
    FillWithRandomData(container.data, container.length);

    CheckToStringBuffer([&](char* buffer, uint32_t* bufferSize) {
        return DsVeosCoSim_LinMessageContainerToString(&container, buffer, bufferSize);
    });

    uint32_t bufferSize = 0;
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_LinMessageContainerToString(nullptr, nullptr, &bufferSize));
}

TEST(TestDsVeosCoSimToStringBuffer, FrMessageContainerToString) {
    DsVeosCoSim_FrMessageContainer container{};
    container.timestamp = 123;
    container.controllerId = 1;
    container.id = 200;
    container.flags = DsVeosCoSim_FrMessageFlags_Startup | DsVeosCoSim_FrMessageFlags_ChannelA;
    container.length = 8;
    FillWithRandomData(container.data, container.length);

    CheckToStringBuffer([&](char* buffer, uint32_t* bufferSize) {
        return DsVeosCoSim_FrMessageContainerToString(&container, buffer, bufferSize);
    });

    uint32_t bufferSize = 0;
    EXPECT_EQ(DsVeosCoSim_Result_InvalidArgument, DsVeosCoSim_FrMessageContainerToString(nullptr, nullptr, &bufferSize));
}

TEST(TestDsVeosCoSimToStringBuffer, CanMessageFlagsToString) {
    CheckToStringBuffer([](char* buffer, uint32_t* bufferSize) {
        return DsVeosCoSim_CanMessageFlagsToString(DsVeosCoSim_CanMessageFlags_ExtendedId, buffer, bufferSize);
    });
}

TEST(TestDsVeosCoSimToStringBuffer, EthMessageFlagsToString) {
    CheckToStringBuffer([](char* buffer, uint32_t* bufferSize) {
        return DsVeosCoSim_EthMessageFlagsToString(DsVeosCoSim_EthMessageFlags_Loopback, buffer, bufferSize);
    });
}

TEST(TestDsVeosCoSimToStringBuffer, LinMessageFlagsToString) {
    CheckToStringBuffer([](char* buffer, uint32_t* bufferSize) {
        return DsVeosCoSim_LinMessageFlagsToString(DsVeosCoSim_LinMessageFlags_Header, buffer, bufferSize);
    });
}

TEST(TestDsVeosCoSimToStringBuffer, FrMessageFlagsToString) {
    CheckToStringBuffer([](char* buffer, uint32_t* bufferSize) {
        return DsVeosCoSim_FrMessageFlagsToString(DsVeosCoSim_FrMessageFlags_Startup, buffer, bufferSize);
    });
}

}  // namespace
