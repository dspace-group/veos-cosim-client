// Copyright dSPACE GmbH. All rights reserved.

#include "TestHelper.h"

#include <fmt/format.h>

#include <gtest/gtest.h>

#include <string>
#include <string_view>

#include "LogHelper.h"

using namespace DsVeosCoSim;

[[nodiscard]] CoSimType GetCounterPart(CoSimType coSimType) {
    return coSimType == CoSimType::Client ? CoSimType::Server : CoSimType::Client;
}

[[nodiscard]] std::string GetCounterPart(std::string_view name, ConnectionKind connectionKind) {
    return connectionKind == ConnectionKind::Local ? std::string(name) : fmt::format("Other{}", name);
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
    ASSERT_STREQ(expected.name, actual.name);
}

void AssertEq(const CanController& expected, const CanController& actual) {
    ASSERT_EQ(expected.id, actual.id);
    ASSERT_EQ(expected.queueSize, actual.queueSize);
    ASSERT_EQ(expected.bitsPerSecond, actual.bitsPerSecond);
    ASSERT_EQ(expected.flexibleDataRateBitsPerSecond, actual.flexibleDataRateBitsPerSecond);
    ASSERT_STREQ(expected.name, actual.name);
    ASSERT_STREQ(expected.channelName, actual.channelName);
    ASSERT_STREQ(expected.clusterName, actual.clusterName);
}

void AssertEq(const EthController& expected, const EthController& actual) {
    ASSERT_EQ(expected.id, actual.id);
    ASSERT_EQ(expected.queueSize, actual.queueSize);
    ASSERT_EQ(expected.bitsPerSecond, actual.bitsPerSecond);
    AssertByteArray(expected.macAddress.data(), actual.macAddress.data(), EthAddressLength);
    ASSERT_STREQ(expected.name, actual.name);
    ASSERT_STREQ(expected.channelName, actual.channelName);
    ASSERT_STREQ(expected.clusterName, actual.clusterName);
}

void AssertEq(const LinController& expected, const LinController& actual) {
    ASSERT_EQ(expected.id, actual.id);
    ASSERT_EQ(expected.queueSize, actual.queueSize);
    ASSERT_EQ(expected.bitsPerSecond, actual.bitsPerSecond);
    ASSERT_EQ(expected.type, actual.type);
    ASSERT_STREQ(expected.name, actual.name);
    ASSERT_STREQ(expected.channelName, actual.channelName);
    ASSERT_STREQ(expected.clusterName, actual.clusterName);
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

void AssertEq(std::string_view expected, std::string_view actual) {
    ASSERT_STREQ(expected.data(), actual.data());
}
