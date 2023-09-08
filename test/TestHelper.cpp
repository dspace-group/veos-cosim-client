// Copyright dSPACE GmbH. All rights reserved.

#include "TestHelper.h"

#include <string>

namespace DsVeosCoSim {

namespace {

std::string g_lastMessage;

}  // namespace

int Random(int min, int max) {
    static bool first = true;
    if (first) {
        srand(42);
        first = false;
    }

    return (int)((int64_t)min + rand() % (((int64_t)max + 1) - (int64_t)min));  // NOLINT(concurrency-mt-unsafe)
}

void FillWithRandom(uint8_t* data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        data[i] = GenerateU8();
    }
}

void AssertByteArray(const uint8_t* expected, const uint8_t* actual, size_t size) {
    for (size_t i = 0; i < size; i++) {
        ASSERT_EQ(expected[i], actual[i]);
    }
}

uint8_t GenerateU8() {
    return (uint8_t)GenerateRandom((uint8_t)0, (uint8_t)UINT8_MAX);
}

uint16_t GenerateU16() {
    return (uint8_t)GenerateRandom((uint16_t)0, (uint16_t)UINT16_MAX);
}

uint32_t GenerateU32() {
    return (uint32_t)GenerateRandom(0, 123456789);
}

uint64_t GenerateU64() {
    return (((uint64_t)GenerateU32() << sizeof(uint32_t)) + (uint64_t)GenerateU32());
}

int64_t GenerateI64() {
    return (int64_t)GenerateU64();
}

std::string GenerateString(const std::string& prefix) {
    return prefix + std::to_string(GenerateU32());
}

void OnLogCallback(Severity severity, std::string_view message) {
    g_lastMessage = message;
    switch (severity) {
        case Severity::Error:
            printf("ERROR %s\n", message.data());
            break;
        case Severity::Warning:
            printf("WARN  %s\n", message.data());
            break;
        case Severity::Info:
            printf("INFO  %s\n", message.data());
            break;
        case Severity::Trace:
            printf("TRACE %s\n", message.data());
            break;
        default:  // NOLINT(clang-diagnostic-covered-switch-default)
            break;
    }
}

void AssertLastMessage(std::string_view message) {
    AssertEq(message.data(), g_lastMessage.c_str());
}

void ClearLastMessage() {
    g_lastMessage = "";
}

void CreateSignal(IoSignalContainer& container) {
    container.name = GenerateString("Signal");
    container.signal.id = GenerateU32();
    container.signal.length = GenerateRandom(1U, 10U);
    container.signal.dataType = GenerateRandom(DataType::Bool, DataType::Float64);
    container.signal.sizeKind = GenerateRandom(SizeKind::Fixed, SizeKind::Variable);
    container.signal.name = container.name.data();
}

void CreateController(CanControllerContainer& container) {
    container.name = GenerateString("Controller");
    container.channelName = GenerateString("Channel");
    container.clusterName = GenerateString("Cluster");
    container.controller.id = GenerateU32();
    container.controller.queueSize = GenerateRandom(1U, 10U);
    container.controller.bitsPerSecond = GenerateU32();
    container.controller.flexibleDataRateBitsPerSecond = GenerateU32();
    container.controller.name = container.name.data();
    container.controller.channelName = container.channelName.data();
    container.controller.clusterName = container.clusterName.data();
}

void CreateController(EthControllerContainer& container) {
    container.name = GenerateString("Controller");
    container.channelName = GenerateString("Channel");
    container.clusterName = GenerateString("Cluster");
    container.controller.id = GenerateU32();
    container.controller.queueSize = GenerateRandom(1U, 10U);
    container.controller.bitsPerSecond = GenerateU32();
    FillWithRandom(container.controller.macAddress, DSVEOSCOSIM_ETH_ADDRESS_LENGTH);
    container.controller.name = container.name.data();
    container.controller.channelName = container.channelName.data();
    container.controller.clusterName = container.clusterName.data();
}

void CreateController(LinControllerContainer& container) {
    container.name = GenerateString("Controller");
    container.channelName = GenerateString("Channel");
    container.clusterName = GenerateString("Cluster");
    container.controller.id = GenerateU32();
    container.controller.queueSize = GenerateRandom(1U, 10U);
    container.controller.bitsPerSecond = GenerateU32();
    container.controller.type = GenerateRandom(LinControllerType::Responder, LinControllerType::Commander);
    container.controller.name = container.name.data();
    container.controller.channelName = container.channelName.data();
    container.controller.clusterName = container.clusterName.data();
}

std::vector<IoSignalContainer> CreateSignals(uint32_t count) {
    std::vector<IoSignalContainer> containers;
    containers.resize(count);
    for (uint32_t i = 0; i < count; i++) {
        CreateSignal(containers[i]);
    }

    return containers;
}

std::vector<CanControllerContainer> CreateCanControllers(uint32_t count) {
    std::vector<CanControllerContainer> containers;
    containers.resize(count);
    for (uint32_t i = 0; i < count; i++) {
        CreateController(containers[i]);
    }

    return containers;
}

std::vector<EthControllerContainer> CreateEthControllers(uint32_t count) {
    std::vector<EthControllerContainer> containers;
    containers.resize(count);
    for (uint32_t i = 0; i < count; i++) {
        CreateController(containers[i]);
    }

    return containers;
}

std::vector<LinControllerContainer> CreateLinControllers(uint32_t count) {
    std::vector<LinControllerContainer> containers;
    containers.resize(count);
    for (uint32_t i = 0; i < count; i++) {
        CreateController(containers[i]);
    }

    return containers;
}

CanMessageContainer CreateCanMessage(uint32_t controllerId) {
    const uint32_t length = GenerateRandom(1U, CanMessageMaxLength);
    CanMessageContainer container{};
    container.data.resize(length);
    FillWithRandom(container.data.data(), length);
    container.message.controllerId = controllerId;
    container.message.id = GenerateU32();
    container.message.timestamp = GenerateI64();
    container.message.length = length;
    container.message.data = container.data.data();
    return container;
}

EthMessageContainer CreateEthMessage(uint32_t controllerId) {
    const uint32_t length = GenerateRandom(1U, EthMessageMaxLength);
    EthMessageContainer container{};
    container.data.resize(length);
    FillWithRandom(container.data.data(), length);
    container.message.controllerId = controllerId;
    container.message.timestamp = GenerateI64();
    container.message.length = length;
    container.message.data = container.data.data();
    return container;
}

LinMessageContainer CreateLinMessage(uint32_t controllerId) {
    const uint32_t length = GenerateRandom(1U, LinMessageMaxLength);
    LinMessageContainer container{};
    container.data.resize(length);
    FillWithRandom(container.data.data(), length);
    container.message.controllerId = controllerId;
    container.message.id = GenerateU32();
    container.message.timestamp = GenerateI64();
    container.message.length = length;
    container.message.data = container.data.data();
    return container;
}

void AssertEq(const IoSignal& expected, const IoSignal& actual) {
    ASSERT_EQ(expected.id, actual.id);
    ASSERT_EQ(expected.length, actual.length);
    ASSERT_EQ(expected.dataType, actual.dataType);
    ASSERT_EQ(expected.sizeKind, actual.sizeKind);
    AssertEq(expected.name, actual.name);
}

void AssertEq(const IoSignalContainer& expected, const IoSignalContainer& actual) {
    AssertEq(expected.signal, actual.signal);
}

void AssertEq(const CanController& expected, const CanController& actual) {
    ASSERT_EQ(expected.id, actual.id);
    ASSERT_EQ(expected.queueSize, actual.queueSize);
    ASSERT_EQ(expected.bitsPerSecond, actual.bitsPerSecond);
    ASSERT_EQ(expected.flexibleDataRateBitsPerSecond, actual.flexibleDataRateBitsPerSecond);
    AssertEq(expected.name, actual.name);
    AssertEq(expected.channelName, actual.channelName);
    AssertEq(expected.clusterName, actual.clusterName);
}

void AssertEq(const EthController& expected, const EthController& actual) {
    ASSERT_EQ(expected.id, actual.id);
    ASSERT_EQ(expected.queueSize, actual.queueSize);
    ASSERT_EQ(expected.bitsPerSecond, actual.bitsPerSecond);
    AssertByteArray(expected.macAddress, actual.macAddress, EthAddressLength);
    AssertEq(expected.name, actual.name);
    AssertEq(expected.channelName, actual.channelName);
    AssertEq(expected.clusterName, actual.clusterName);
}

void AssertEq(const LinController& expected, const LinController& actual) {
    ASSERT_EQ(expected.id, actual.id);
    ASSERT_EQ(expected.queueSize, actual.queueSize);
    ASSERT_EQ(expected.bitsPerSecond, actual.bitsPerSecond);
    ASSERT_EQ(expected.type, actual.type);
    AssertEq(expected.name, actual.name);
    AssertEq(expected.channelName, actual.channelName);
    AssertEq(expected.clusterName, actual.clusterName);
}

void AssertEq(const CanControllerContainer& expected, const CanControllerContainer& actual) {
    AssertEq(expected.controller, actual.controller);
}

void AssertEq(const EthControllerContainer& expected, const EthControllerContainer& actual) {
    AssertEq(expected.controller, actual.controller);
}

void AssertEq(const LinControllerContainer& expected, const LinControllerContainer& actual) {
    AssertEq(expected.controller, actual.controller);
}

void AssertEq(const CanMessage& expected, const CanMessage& actual) {
    ASSERT_EQ(expected.timestamp, actual.timestamp);
    ASSERT_EQ(expected.controllerId, actual.controllerId);
    ASSERT_EQ(expected.id, actual.id);
    ASSERT_EQ(expected.flags, actual.flags);
    ASSERT_EQ(expected.length, actual.length);
    AssertByteArray(expected.data, actual.data, expected.length);
}

void AssertEq(const EthMessage& expected, const EthMessage& actual) {
    ASSERT_EQ(expected.timestamp, actual.timestamp);
    ASSERT_EQ(expected.controllerId, actual.controllerId);
    ASSERT_EQ(expected.flags, actual.flags);
    ASSERT_EQ(expected.length, actual.length);
    AssertByteArray(expected.data, actual.data, expected.length);
}

void AssertEq(const LinMessage& expected, const LinMessage& actual) {
    ASSERT_EQ(expected.timestamp, actual.timestamp);
    ASSERT_EQ(expected.controllerId, actual.controllerId);
    ASSERT_EQ(expected.id, actual.id);
    ASSERT_EQ(expected.flags, actual.flags);
    ASSERT_EQ(expected.length, actual.length);
    AssertByteArray(expected.data, actual.data, expected.length);
}

void AssertEq(const std::string& expected, const std::string& actual) {
    AssertEq(expected.c_str(), actual.c_str());
}

void AssertEq(const char* expected, const char* actual) {
    ASSERT_STREQ(expected, actual);
}

void AssertEq(Result expected, Result actual) {
    ASSERT_EQ((int)expected, (int)actual);
}

}  // namespace DsVeosCoSim
