// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32

#include <gtest/gtest.h>

#include <string>
#include <thread>

#include "Generator.h"
#include "NamedEvent.h"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] std::string GenerateName() {
    return GenerateString("Event名前\xF0\x9F\x98\x80");
}

void WaitAndSet(const std::string& eventName1, const std::string& eventName2) {
    const NamedEvent event1 = NamedEvent::OpenExisting(eventName1);
    const NamedEvent event2 = NamedEvent::OpenExisting(eventName2);

    event1.Wait();
    event2.Set();
}

class TestNamedEvent : public testing::Test {};

TEST_F(TestNamedEvent, CreateAndDestroyNamedEvent) {
    // Arrange
    const std::string name = GenerateName();

    // Act
    ASSERT_NO_THROW(const NamedEvent event = NamedEvent::CreateOrOpen(name));

    // Assert
}

TEST_F(TestNamedEvent, SetAndWaitOnSameNamedEvent) {
    // Arrange
    const std::string name = GenerateName();
    const NamedEvent event = NamedEvent::CreateOrOpen(name);

    // Act
    ASSERT_NO_THROW(event.Set());
    ASSERT_NO_THROW(event.Wait());

    // Assert
}

TEST_F(TestNamedEvent, SetAndWaitOnSameNamedEventWithTimeout) {
    // Arrange
    const std::string name = GenerateName();
    const NamedEvent event = NamedEvent::CreateOrOpen(name);

    // Act
    ASSERT_NO_THROW(event.Set());
    const bool result = event.Wait(1);

    // Assert
    ASSERT_TRUE(result);
}

TEST_F(TestNamedEvent, WaitTwiceOnNamedEvent) {
    // Arrange
    const std::string name = GenerateName();
    const NamedEvent event = NamedEvent::CreateOrOpen(name);

    // Act
    ASSERT_NO_THROW(event.Set());
    const bool result1 = event.Wait(1);
    const bool result2 = event.Wait(1);

    // Assert
    ASSERT_TRUE(result1);
    ASSERT_FALSE(result2);
}

TEST_F(TestNamedEvent, SetTwiceOnNamedEvent) {
    // Arrange
    const std::string name = GenerateName();
    const NamedEvent event = NamedEvent::CreateOrOpen(name);

    // Act
    ASSERT_NO_THROW(event.Set());
    ASSERT_NO_THROW(event.Set());
    bool result1 = event.Wait(1);
    bool result2 = event.Wait(1);

    // Assert
    ASSERT_TRUE(result1);
    ASSERT_FALSE(result2);
}

TEST_F(TestNamedEvent, WaitResetAndWaitOnNamedEvent) {
    // Arrange
    const std::string name = GenerateName();
    const NamedEvent event = NamedEvent::CreateOrOpen(name);

    // Act
    ASSERT_NO_THROW(event.Set());
    const bool result1 = event.Wait(1);
    const bool result2 = event.Wait(1);

    // Assert
    ASSERT_TRUE(result1);
    ASSERT_FALSE(result2);
}

TEST_F(TestNamedEvent, WaitWithoutSetOnNamedEvent) {
    // Arrange
    const std::string name = GenerateName();
    const NamedEvent event = NamedEvent::CreateOrOpen(name);

    // Act
    const bool result = event.Wait(1);

    // Assert
    ASSERT_FALSE(result);
}

TEST_F(TestNamedEvent, SetAndWaitOnDifferentNamedEvents) {
    // Arrange
    const std::string name = GenerateName();
    const NamedEvent event1 = NamedEvent::CreateOrOpen(name);
    const NamedEvent event2 = NamedEvent::OpenExisting(name);

    // Act
    ASSERT_NO_THROW(event1.Set());
    ASSERT_NO_THROW(event2.Wait());

    // Assert
}

TEST_F(TestNamedEvent, ResetOnSettingNamedEvents) {
    // Arrange
    const std::string name = GenerateName();
    const NamedEvent event1 = NamedEvent::CreateOrOpen(name);
    const NamedEvent event2 = NamedEvent::OpenExisting(name);

    // Act
    for (int32_t i = 0; i < 10; i++) {
        ASSERT_NO_THROW(event1.Set());
        ASSERT_NO_THROW(event2.Wait());
    }

    // Assert
}

TEST_F(TestNamedEvent, ResetOnWaitingNamedEvents) {
    // Arrange
    const std::string name = GenerateName();
    const NamedEvent event1 = NamedEvent::CreateOrOpen(name);
    const NamedEvent event2 = NamedEvent::OpenExisting(name);

    // Act
    for (int32_t i = 0; i < 10; i++) {
        ASSERT_NO_THROW(event1.Set());
        ASSERT_NO_THROW(event2.Wait());
    }

    // Assert
}

TEST_F(TestNamedEvent, NoResetOnNamedEvents) {
    // Arrange
    const std::string name = GenerateName();
    const NamedEvent event1 = NamedEvent::CreateOrOpen(name);
    const NamedEvent event2 = NamedEvent::OpenExisting(name);

    // Act
    for (int32_t i = 0; i < 10; i++) {
        ASSERT_NO_THROW(event1.Set());
        ASSERT_NO_THROW(event2.Wait());
    }

    // Assert
}

TEST_F(TestNamedEvent, SetAndWaitInDifferentThreads) {
    // Arrange
    const std::string firstName = GenerateName();
    const std::string secondName = GenerateName();
    const NamedEvent event1 = NamedEvent::CreateOrOpen(firstName);
    const NamedEvent event2 = NamedEvent::CreateOrOpen(secondName);

    std::thread thread(WaitAndSet, firstName, secondName);

    // Act
    ASSERT_NO_THROW(event1.Set());
    ASSERT_NO_THROW(event2.Wait());

    // Assert

    // Cleanup
    thread.join();
}

}  // namespace

#endif
