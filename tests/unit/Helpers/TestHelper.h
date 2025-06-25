// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <gtest/gtest.h>

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "DsVeosCoSim/CoSimTypes.h"

[[nodiscard]] DsVeosCoSim::CoSimType GetCounterPart(DsVeosCoSim::CoSimType coSimType);
[[nodiscard]] std::string GetCounterPart(std::string_view name, DsVeosCoSim::ConnectionKind connectionKind);

void AssertByteArray(const void* expected, const void* actual, size_t size);

void AssertLastMessage(std::string_view message);

void AssertEq(const DsVeosCoSim::IoSignal& expected, const DsVeosCoSim::IoSignal& actual);
void AssertEq(const DsVeosCoSim::CanController& expected, const DsVeosCoSim::CanController& actual);
void AssertEq(const DsVeosCoSim::EthController& expected, const DsVeosCoSim::EthController& actual);
void AssertEq(const DsVeosCoSim::LinController& expected, const DsVeosCoSim::LinController& actual);
void AssertEq(const DsVeosCoSim::CanMessage& expected, const DsVeosCoSim::CanMessage& actual);
void AssertEq(const DsVeosCoSim::EthMessage& expected, const DsVeosCoSim::EthMessage& actual);
void AssertEq(const DsVeosCoSim::LinMessage& expected, const DsVeosCoSim::LinMessage& actual);
void AssertEq(std::string_view expected, std::string_view actual);

template <typename T>
void AssertEq(const std::vector<T>& expected, const std::vector<T>& actual) {
    ASSERT_EQ(expected.size(), actual.size());
    for (size_t i = 0; i < expected.size(); i++) {
        AssertEq(expected[i], actual[i]);
    }
}
