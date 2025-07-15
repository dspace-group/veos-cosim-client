// Copyright dSPACE GmbH. All rights reserved.

#include "TestHelper.h"

#include <fmt/format.h>

#include <gtest/gtest.h>

#include <string>

#include "Helper.h"

using namespace DsVeosCoSim;

namespace {}  // namespace

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
        AssertEq(expectedBytes[i], actualBytes[i]);
    }
}

void AssertLastMessage(const std::string& message) {
    AssertEqHelper(message, GetLastMessage());
}

void AssertEqHelper(const std::string& expected, const std::string& actual) {
    ASSERT_STREQ(expected.c_str(), actual.c_str());
}

void AssertNotEqHelper(const std::string& expected, const std::string& actual) {
    ASSERT_STRNE(expected.c_str(), actual.c_str());
}
