// Copyright dSPACE SE & Co. KG. All rights reserved.

#ifdef _WIN32

#include <gtest/gtest.h>

#include <string>

#include "Helper.hpp"
#include "OsUtilities.hpp"
#include "TestHelper.hpp"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] std::string GenerateName() {
    return GenerateString("Mutex名前\xF0\x9F\x98\x80");
}

class TestNamedLock : public testing::Test {};

void DifferentThread(const std::string& name, int32_t& counter) {
    for (int32_t i = 0; i < 10000; i++) {
        NamedLock mutex;
        AssertOk(NamedLock::Create(name, mutex));
        counter++;
    }
}

TEST_F(TestNamedLock, LockAndUnlockOnDifferentMutexes) {
    // Arrange
    std::string name = GenerateName();

    int32_t counter{};

    auto thread = std::thread(DifferentThread, name, std::ref(counter));

    // Act
    for (int32_t i = 0; i < 10000; i++) {
        NamedLock mutex;
        AssertOk(NamedLock::Create(name, mutex));
        counter++;
    }

    thread.join();

    // Assert
    ASSERT_EQ(counter, 20000);
}

}  // namespace

#endif
