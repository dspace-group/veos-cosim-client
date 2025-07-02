// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <gtest/gtest.h>

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "DsVeosCoSim/CoSimTypes.h"

#define AssertEq(expected, actual) ASSERT_EQ(expected, actual)
#define AssertNotEq(expected, actual) ASSERT_NE(expected, actual)

#define AssertOk(result) ASSERT_EQ(result, DsVeosCoSim::Result::Ok)
#define AssertEmpty(result) ASSERT_EQ(result, DsVeosCoSim::Result::Empty)
#define AssertFull(result) ASSERT_EQ(result, DsVeosCoSim::Result::Full)

#define ExpectOk(result) EXPECT_EQ(result, DsVeosCoSim::Result::Ok)

#define AssertNotOk(result) ASSERT_NE(result, DsVeosCoSim::Result::Ok)

#define AssertTrue(result) ASSERT_TRUE(!!(result))

#define ExpectTrue(result) EXPECT_TRUE(!!(result))

#define AssertFalse(result) ASSERT_FALSE(!!(result))

[[nodiscard]] DsVeosCoSim::CoSimType GetCounterPart(DsVeosCoSim::CoSimType coSimType);
[[nodiscard]] std::string GetCounterPart(std::string_view name, DsVeosCoSim::ConnectionKind connectionKind);

void AssertByteArray(const void* expected, const void* actual, size_t size);

void AssertLastMessage(std::string_view message);

namespace DsVeosCoSim {

[[nodiscard]] bool operator==(const IoSignal& first, const IoSignal& second);
[[nodiscard]] std::ostream& operator<<(std::ostream& stream, const IoSignal& signal);

[[nodiscard]] bool operator==(const CanController& first, const CanController& second);
[[nodiscard]] std::ostream& operator<<(std::ostream& stream, const CanController& controller);
[[nodiscard]] bool operator==(const CanMessage& first, const CanMessage& second);
[[nodiscard]] std::ostream& operator<<(std::ostream& stream, const CanMessage& message);

[[nodiscard]] bool operator==(const EthController& first, const EthController& second);
[[nodiscard]] std::ostream& operator<<(std::ostream& stream, const EthController& controller);
[[nodiscard]] bool operator==(const EthMessage& first, const EthMessage& second);
[[nodiscard]] std::ostream& operator<<(std::ostream& stream, const EthMessage& message);

[[nodiscard]] bool operator==(const LinController& first, const LinController& second);
[[nodiscard]] std::ostream& operator<<(std::ostream& stream, const LinController& controller);
[[nodiscard]] bool operator==(const LinMessage& first, const LinMessage& second);
[[nodiscard]] std::ostream& operator<<(std::ostream& stream, const LinMessage& message);

}  // namespace DsVeosCoSim

void AssertEqHelper(std::string_view expected, std::string_view actual);
void AssertNotEqHelper(std::string_view expected, std::string_view actual);

template <typename T>
void AssertEqHelper(const std::vector<T>& expected, const std::vector<T>& actual) {
    AssertEq(expected.size(), actual.size());
    for (size_t i = 0; i < expected.size(); i++) {
        ASSERT_EQ(expected[i], actual[i]);
    }
}
