// Copyright dSPACE GmbH. All rights reserved.

#include "TestHelper.h"

#include <string>

namespace DsVeosCoSim {

namespace {

std::string g_lastMessage;

}  // namespace

void AssertByteArray(const uint8_t* expected, const uint8_t* actual, size_t size) {
    for (size_t i = 0; i < size; i++) {
        ASSERT_EQ(expected[i], actual[i]);
    }
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

void AssertEq(const IoSignal& expected, const IoSignal& actual) {
    ASSERT_EQ(expected.id, actual.id);
    ASSERT_EQ(expected.length, actual.length);
    ASSERT_EQ(expected.dataType, actual.dataType);
    ASSERT_EQ(expected.sizeKind, actual.sizeKind);
    AssertEq(expected.name, actual.name);
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
    AssertByteArray(expected.macAddress.data(), actual.macAddress.data(), EthAddressLength);
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

void AssertEq(const DsVeosCoSim_CanMessage& expected, const DsVeosCoSim_CanMessage& actual) {
    ASSERT_EQ(expected.timestamp, actual.timestamp);
    ASSERT_EQ(expected.controllerId, actual.controllerId);
    ASSERT_EQ(expected.id, actual.id);
    ASSERT_EQ(expected.flags, actual.flags);
    ASSERT_EQ(expected.length, actual.length);
    AssertByteArray(expected.data, actual.data, expected.length);
}

void AssertEq(const DsVeosCoSim_EthMessage& expected, const DsVeosCoSim_EthMessage& actual) {
    ASSERT_EQ(expected.timestamp, actual.timestamp);
    ASSERT_EQ(expected.controllerId, actual.controllerId);
    ASSERT_EQ(expected.flags, actual.flags);
    ASSERT_EQ(expected.length, actual.length);
    AssertByteArray(expected.data, actual.data, expected.length);
}

void AssertEq(const DsVeosCoSim_LinMessage& expected, const DsVeosCoSim_LinMessage& actual) {
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
    ASSERT_EQ(static_cast<int>(expected), static_cast<int>(actual));
}

}  // namespace DsVeosCoSim
