// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32

#include <gtest/gtest.h>

#include <string>

#include "Generator.h"
#include "NamedMutex.h"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] std::string GenerateName() {
    return GenerateString("Mutex名前\xF0\x9F\x98\x80");
}

void DifferentThread(const std::string& name, int32_t& counter) {
    const NamedMutex mutex = NamedMutex::CreateOrOpen(name);

    for (int32_t i = 0; i < 10000; i++) {
        ASSERT_NO_THROW(mutex.lock());
        counter++;
        ASSERT_NO_THROW(mutex.unlock());
    }
}

class TestNamedMutex : public testing::Test {};

TEST_F(TestNamedMutex, CreateAndDestroy) {
    // Arrange
    const std::string name = GenerateName();

    // Act and assert
    ASSERT_NO_THROW((void)NamedMutex::CreateOrOpen(name));
}

TEST_F(TestNamedMutex, LockAndUnlockOnSameMutex) {
    // Arrange
    const std::string name = GenerateName();
    const NamedMutex mutex = NamedMutex::CreateOrOpen(name);

    // Act and assert
    ASSERT_NO_THROW(mutex.lock());
    ASSERT_NO_THROW(mutex.unlock());
}

TEST_F(TestNamedMutex, LockAndUnlockOnDifferentMutexes) {
    // Arrange
    const std::string name = GenerateName();
    const NamedMutex mutex = NamedMutex::CreateOrOpen(name);
    int32_t counter{};

    auto thread = std::thread(DifferentThread, name, std::ref(counter));

    // Act
    for (int32_t i = 0; i < 10000; i++) {
        ASSERT_NO_THROW(mutex.lock());
        counter++;
        ASSERT_NO_THROW(mutex.unlock());
    }

    thread.join();

    // Assert
    ASSERT_EQ(counter, 20000);
}

}  // namespace

#endif
