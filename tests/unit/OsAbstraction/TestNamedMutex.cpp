// Copyright dSPACE SE & Co. KG. All rights reserved.

#ifdef _WIN32

#include <gtest/gtest.h>

#include <string>

#include "Helper.h"
#include "OsUtilities.h"
#include "TestHelper.h"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] std::string GenerateName() {
    return GenerateString("Mutex名前\xF0\x9F\x98\x80");
}

class TestNamedMutex : public testing::Test {};

TEST_F(TestNamedMutex, LockAndUnlockOnSameMutex) {
    // Arrange
    std::string name = GenerateName();
    NamedMutex mutex;
    ExpectOk(NamedMutex::CreateOrOpen(name, mutex));

    // Act and assert
    AssertOk(mutex.Lock());

    // Cleanup
    mutex.Unlock();
}

void DifferentThread(const std::string& name, int32_t& counter) {
    NamedMutex mutex;
    ExpectOk(NamedMutex::CreateOrOpen(name, mutex));

    for (int32_t i = 0; i < 10000; i++) {
        ExpectOk(mutex.Lock());
        counter++;
        mutex.Unlock();
    }
}

TEST_F(TestNamedMutex, LockAndUnlockOnDifferentMutexes) {
    // Arrange
    std::string name = GenerateName();
    NamedMutex mutex;
    ExpectOk(NamedMutex::CreateOrOpen(name, mutex));
    int32_t counter{};

    auto thread = std::thread(DifferentThread, name, std::ref(counter));

    // Act
    for (int32_t i = 0; i < 10000; i++) {
        ExpectOk(mutex.Lock());
        counter++;
        mutex.Unlock();
    }

    thread.join();

    // Assert
    AssertEq(counter, 20000);
}

}  // namespace

#endif
