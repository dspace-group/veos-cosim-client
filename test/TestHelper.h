// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <gtest/gtest.h>
#include <vector>

#include "BusBuffer.h"
#include "CoSimTypes.h"
#include "Communication.h"

namespace DsVeosCoSim {

inline static std::string g_TestServerName = "TheTestServer";

int Random(int min, int max);

void FillWithRandom(uint8_t* data, size_t length);

template <typename T>
T GenerateRandom(T min, T max) {
    return (T)Random((int)min, (int)max);
}

void AssertByteArray(const uint8_t* expected, const uint8_t* actual, size_t size);

uint8_t GenerateU8();
uint16_t GenerateU16();
uint32_t GenerateU32();
uint64_t GenerateU64();
int64_t GenerateI64();
std::string GenerateString(const std::string& prefix);

void OnLogCallback(Severity severity, std::string_view message);
void AssertLastMessage(std::string_view message);
void ClearLastMessage();

void CreateSignal(IoSignalContainer& container);

void CreateController(CanControllerContainer& container);
void CreateController(EthControllerContainer& container);
void CreateController(LinControllerContainer& container);

std::vector<IoSignalContainer> CreateSignals(uint32_t count);

std::vector<CanControllerContainer> CreateCanControllers(uint32_t count);
std::vector<EthControllerContainer> CreateEthControllers(uint32_t count);
std::vector<LinControllerContainer> CreateLinControllers(uint32_t count);

void CreateMessage(uint32_t controllerId, CanMessageContainer& container);
void CreateMessage(uint32_t controllerId, EthMessageContainer& container);
void CreateMessage(uint32_t controllerId, LinMessageContainer& container);

void AssertEq(const IoSignal& expected, const IoSignal& actual);
void AssertEq(const IoSignalContainer& expected, const IoSignalContainer& actual);
void AssertEq(const CanController& expected, const CanController& actual);
void AssertEq(const EthController& expected, const EthController& actual);
void AssertEq(const LinController& expected, const LinController& actual);
void AssertEq(const CanControllerContainer& expected, const CanControllerContainer& actual);
void AssertEq(const EthControllerContainer& expected, const EthControllerContainer& actual);
void AssertEq(const LinControllerContainer& expected, const LinControllerContainer& actual);
void AssertEq(const CanMessage& expected, const CanMessage& actual);
void AssertEq(const EthMessage& expected, const EthMessage& actual);
void AssertEq(const LinMessage& expected, const LinMessage& actual);
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
