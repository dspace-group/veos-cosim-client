// Copyright dSPACE GmbH. All rights reserved.

#include "TestHelper.h"

#include <fmt/format.h>

#include <gtest/gtest.h>

#include <string>
#include <string_view>

#include "LogHelper.h"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] bool Equals(const void* expected, const void* actual, size_t size) {
    const auto* expectedBytes = static_cast<const uint8_t*>(expected);
    const auto* actualBytes = static_cast<const uint8_t*>(actual);
    for (size_t i = 0; i < size; i++) {
        if (expectedBytes[i] != actualBytes[i]) {
            return false;
        }
    }

    return true;
}

template <typename T, size_t TSize>
[[nodiscard]] bool Equals(const std::array<T, TSize>& expected, const std::array<T, TSize>& actual) {
    for (size_t i = 0; i < TSize; i++) {
        if (expected[i] != actual[i]) {
            return false;
        }
    }

    return true;
}

[[nodiscard]] bool Equals(std::string_view first, std::string_view second) {
    if (first.length() != second.length()) {
        return false;
    }

    return strcmp(first.data(), second.data()) == 0;
}

}  // namespace

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
        AssertEq(expectedBytes[i], actualBytes[i]);
    }
}

void AssertLastMessage(std::string_view message) {
    AssertEqHelper(message, GetLastMessage());
}

namespace DsVeosCoSim {

[[nodiscard]] bool operator==(const IoSignal& first, const IoSignal& second) {
    if (first.id != second.id) {
        return false;
    }

    if (first.length != second.length) {
        return false;
    }

    if (first.dataType != second.dataType) {
        return false;
    }

    if (first.sizeKind != second.sizeKind) {
        return false;
    }

    if (!Equals(first.name, second.name)) {
        return false;
    }

    return true;
}

[[nodiscard]] std::ostream& operator<<(std::ostream& stream, const IoSignal& signal) {
    return stream << ToString(signal);
}

[[nodiscard]] bool operator==(const CanController& first, const CanController& second) {
    if (first.id != second.id) {
        return false;
    }

    if (first.queueSize != second.queueSize) {
        return false;
    }

    if (first.bitsPerSecond != second.bitsPerSecond) {
        return false;
    }

    if (first.flexibleDataRateBitsPerSecond != second.flexibleDataRateBitsPerSecond) {
        return false;
    }

    if (!Equals(first.name, second.name)) {
        return false;
    }

    if (!Equals(first.channelName, second.channelName)) {
        return false;
    }

    if (!Equals(first.clusterName, second.clusterName)) {
        return false;
    }

    return true;
}

[[nodiscard]] std::ostream& operator<<(std::ostream& stream, const CanController& controller) {
    return stream << ToString(controller);
}

[[nodiscard]] bool operator==(const CanMessage& first, const CanMessage& second) {
    if (first.timestamp != second.timestamp) {
        return false;
    }

    if (first.controllerId != second.controllerId) {
        return false;
    }

    if (first.id != second.id) {
        return false;
    }

    if (first.flags != second.flags) {
        return false;
    }

    if (first.length != second.length) {
        return false;
    }

    if (!Equals(first.data, second.data, first.length)) {
        return false;
    }

    return true;
}

[[nodiscard]] std::ostream& operator<<(std::ostream& stream, const CanMessage& message) {
    return stream << ToString(message);
}

[[nodiscard]] bool operator==(const EthController& first, const EthController& second) {
    if (first.id != second.id) {
        return false;
    }

    if (first.queueSize != second.queueSize) {
        return false;
    }

    if (first.bitsPerSecond != second.bitsPerSecond) {
        return false;
    }

    if (!Equals(first.macAddress, second.macAddress)) {
        return false;
    }

    if (!Equals(first.name, second.name)) {
        return false;
    }

    if (!Equals(first.channelName, second.channelName)) {
        return false;
    }

    if (!Equals(first.clusterName, second.clusterName)) {
        return false;
    }

    return true;
}

[[nodiscard]] std::ostream& operator<<(std::ostream& stream, const EthController& controller) {
    return stream << ToString(controller);
}

[[nodiscard]] bool operator==(const EthMessage& first, const EthMessage& second) {
    if (first.timestamp != second.timestamp) {
        return false;
    }

    if (first.controllerId != second.controllerId) {
        return false;
    }

    if (first.flags != second.flags) {
        return false;
    }

    if (first.length != second.length) {
        return false;
    }

    if (!Equals(first.data, second.data, first.length)) {
        return false;
    }

    return true;
}

[[nodiscard]] std::ostream& operator<<(std::ostream& stream, const EthMessage& message) {
    return stream << ToString(message);
}

[[nodiscard]] bool operator==(const LinController& first, const LinController& second) {
    if (first.id != second.id) {
        return false;
    }

    if (first.queueSize != second.queueSize) {
        return false;
    }

    if (first.bitsPerSecond != second.bitsPerSecond) {
        return false;
    }

    if (first.type != second.type) {
        return false;
    }

    if (!Equals(first.name, second.name)) {
        return false;
    }

    if (!Equals(first.channelName, second.channelName)) {
        return false;
    }

    if (!Equals(first.clusterName, second.clusterName)) {
        return false;
    }

    return true;
}

[[nodiscard]] std::ostream& operator<<(std::ostream& stream, const LinController& controller) {
    return stream << ToString(controller);
}

[[nodiscard]] bool operator==(const LinMessage& first, const LinMessage& second) {
    if (first.timestamp != second.timestamp) {
        return false;
    }

    if (first.controllerId != second.controllerId) {
        return false;
    }

    if (first.id != second.id) {
        return false;
    }

    if (first.flags != second.flags) {
        return false;
    }

    if (first.length != second.length) {
        return false;
    }

    if (!Equals(first.data, second.data, first.length)) {
        return false;
    }

    return true;
}

[[nodiscard]] std::ostream& operator<<(std::ostream& stream, const LinMessage& message) {
    return stream << ToString(message);
}

}  // namespace DsVeosCoSim

void AssertEqHelper(std::string_view expected, std::string_view actual) {
    ASSERT_STREQ(expected.data(), actual.data());
}

void AssertNotEqHelper(std::string_view expected, std::string_view actual) {
    ASSERT_STRNE(expected.data(), actual.data());
}
