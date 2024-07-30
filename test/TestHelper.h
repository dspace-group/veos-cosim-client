// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <gtest/gtest.h>
#include <vector>

#include "CoSimTypes.h"

namespace DsVeosCoSim {

void AssertByteArray(const uint8_t* expected, const uint8_t* actual, size_t size);

void OnLogCallback(Severity severity, std::string_view message);
void AssertLastMessage(std::string_view message);
void ClearLastMessage();

void AssertEq(const IoSignal& expected, const IoSignal& actual);
void AssertEq(const CanController& expected, const CanController& actual);
void AssertEq(const EthController& expected, const EthController& actual);
void AssertEq(const LinController& expected, const LinController& actual);
void AssertEq(const DsVeosCoSim_CanMessage& expected, const DsVeosCoSim_CanMessage& actual);
void AssertEq(const DsVeosCoSim_EthMessage& expected, const DsVeosCoSim_EthMessage& actual);
void AssertEq(const DsVeosCoSim_LinMessage& expected, const DsVeosCoSim_LinMessage& actual);
void AssertEq(const std::string& expected, const std::string& actual);
void AssertEq(const char* expected, const char* actual);
void AssertEq(Result expected, Result actual);

#define ASSERT_OK(actual) ASSERT_EQ((int)Result::Ok, (int)(actual))
#define ASSERT_EMPTY(actual) ASSERT_EQ((int)Result::Empty, (int)(actual))
#define ASSERT_FULL(actual) ASSERT_EQ((int)Result::Full, (int)(actual))
#define ASSERT_ERROR(actual) ASSERT_EQ((int)Result::Error, (int)(actual))
#define ASSERT_INVALID_ARGUMENT(actual) ASSERT_EQ((int)Result::InvalidArgument, (int)(actual))
#define ASSERT_DISCONNECTED(actual) ASSERT_EQ((int)Result::Disconnected, (int)(actual))

template <typename T>
void AssertEq(const std::vector<T>& expected, const std::vector<T>& actual) {
    ASSERT_EQ(expected.size(), actual.size());
    for (size_t i = 0; i < expected.size(); i++) {
        AssertEq(expected[i], actual[i]);
    }
}

}  // namespace DsVeosCoSim
