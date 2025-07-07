// Copyright dSPACE GmbH. All rights reserved.

#include "TestHelper.h"

#include <fmt/format.h>

#include <gtest/gtest.h>

#include <string>
#include <string_view>

#include "Helper.h"

using namespace DsVeosCoSim;

namespace {}  // namespace

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

void AssertEqHelper(std::string_view expected, std::string_view actual) {
    ASSERT_STREQ(expected.data(), actual.data());
}

void AssertNotEqHelper(std::string_view expected, std::string_view actual) {
    ASSERT_STRNE(expected.data(), actual.data());
}
