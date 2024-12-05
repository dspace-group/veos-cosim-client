// Copyright dSPACE GmbH. All rights reserved.

#include "TestHelper.h"

#include <fmt/format.h>

#include <string>
#include <string_view>

#include "LogHelper.h"
#include "gtest/gtest.h"

using namespace DsVeosCoSim;

[[nodiscard]] CoSimType GetCounterPart(CoSimType coSimType) {
    return coSimType == CoSimType::Client ? CoSimType::Server : CoSimType::Client;
}

[[nodiscard]] std::string GetCounterPart(const std::string& name, ConnectionKind connectionKind) {
    return connectionKind == ConnectionKind::Local ? name : fmt::format("Other{}", name);
}

void AssertByteArray(const void* expected, const void* actual, size_t size) {
    const auto* expectedBytes = static_cast<const uint8_t*>(expected);
    const auto* actualBytes = static_cast<const uint8_t*>(actual);
    for (size_t i = 0; i < size; i++) {
        ASSERT_EQ(expectedBytes[i], actualBytes[i]);
    }
}

void AssertLastMessage(std::string_view message) {
    ASSERT_STREQ(message.data(), GetLastMessage().c_str());
}

void AssertEq(const IoSignal& expected, const IoSignal& actual) {
    ASSERT_EQ(expected.id, actual.id);
    ASSERT_EQ(expected.length, actual.length);
    ASSERT_EQ(expected.dataType, actual.dataType);
    ASSERT_EQ(expected.sizeKind, actual.sizeKind);
    AssertEq(expected.name, actual.name);
}

void AssertEq(const DsVeosCoSim_CanController& expected, const DsVeosCoSim_CanController& actual) {
    ASSERT_EQ(expected.id, actual.id);
    ASSERT_EQ(expected.queueSize, actual.queueSize);
    ASSERT_EQ(expected.bitsPerSecond, actual.bitsPerSecond);
    ASSERT_EQ(expected.flexibleDataRateBitsPerSecond, actual.flexibleDataRateBitsPerSecond);
    ASSERT_STREQ(expected.name, actual.name);
    ASSERT_STREQ(expected.channelName, actual.channelName);
    ASSERT_STREQ(expected.clusterName, actual.clusterName);
}

void AssertEq(const DsVeosCoSim_EthController& expected, const DsVeosCoSim_EthController& actual) {
    ASSERT_EQ(expected.id, actual.id);
    ASSERT_EQ(expected.queueSize, actual.queueSize);
    ASSERT_EQ(expected.bitsPerSecond, actual.bitsPerSecond);
    AssertByteArray(expected.macAddress, actual.macAddress, EthAddressLength);
    ASSERT_STREQ(expected.name, actual.name);
    ASSERT_STREQ(expected.channelName, actual.channelName);
    ASSERT_STREQ(expected.clusterName, actual.clusterName);
}

void AssertEq(const DsVeosCoSim_LinController& expected, const DsVeosCoSim_LinController& actual) {
    ASSERT_EQ(expected.id, actual.id);
    ASSERT_EQ(expected.queueSize, actual.queueSize);
    ASSERT_EQ(expected.bitsPerSecond, actual.bitsPerSecond);
    ASSERT_EQ(expected.type, actual.type);
    ASSERT_STREQ(expected.name, actual.name);
    ASSERT_STREQ(expected.channelName, actual.channelName);
    ASSERT_STREQ(expected.clusterName, actual.clusterName);
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

void AssertEq(std::string_view expected, std::string_view actual) {
    ASSERT_STREQ(expected.data(), actual.data());
}

void AssertEq(const char* expected, const char* actual) {
    ASSERT_STREQ(expected, actual);
}
