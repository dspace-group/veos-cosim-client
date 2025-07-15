// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <gtest/gtest.h>

#include <cstddef>
#include <string>
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
[[nodiscard]] std::string GetCounterPart(const std::string& name, DsVeosCoSim::ConnectionKind connectionKind);

void AssertByteArray(const void* expected, const void* actual, size_t size);

void AssertLastMessage(const std::string& message);

void AssertEqHelper(const std::string& expected, const std::string& actual);
void AssertNotEqHelper(const std::string& expected, const std::string& actual);

template <typename T>
void AssertEqHelper(const std::vector<T>& expected, const std::vector<T>& actual) {
    AssertEq(expected.size(), actual.size());
    for (size_t i = 0; i < expected.size(); i++) {
        ASSERT_EQ(expected[i], actual[i]);
    }
}
