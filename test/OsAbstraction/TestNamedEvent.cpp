// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32

#include <gtest/gtest.h>
#include <string>
#include <string_view>
#include <thread>

#include "Generator.h"
#include "NamedEvent.h"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] std::string GenerateName() {
    return GenerateString("Event名前\xF0\x9F\x98\x80");
}

void WaitAndSet(std::string_view eventName1, std::string_view eventName2) {
    NamedEvent event1 = NamedEvent::OpenExisting(eventName1);
    NamedEvent event2 = NamedEvent::OpenExisting(eventName2);

    event1.Wait();
    event2.Set();
}

class TestNamedEvent : public testing::Test {};

TEST_F(TestNamedEvent, CreateAndDestroyNamedEvent) {
    // Arrange
    std::string name = GenerateName();

    // Act
    ASSERT_NO_THROW(NamedEvent event = NamedEvent::CreateOrOpen(name));

    // Assert
}

TEST_F(TestNamedEvent, SetAndWaitOnSameNamedEvent) {
    // Arrange
    std::string name = GenerateName();
    NamedEvent event = NamedEvent::CreateOrOpen(name);

    // Act
    ASSERT_NO_THROW(event.Set());
    ASSERT_NO_THROW(event.Wait());

    // Assert
}

TEST_F(TestNamedEvent, SetAndWaitOnSameNamedEventWithTimeout) {
    // Arrange
    std::string name = GenerateName();
    NamedEvent event = NamedEvent::CreateOrOpen(name);

    // Act
    ASSERT_NO_THROW(event.Set());
    bool result = event.Wait(1);

    // Assert
    ASSERT_TRUE(result);
}

TEST_F(TestNamedEvent, WaitTwiceOnNamedEvent) {
    // Arrange
    std::string name = GenerateName();
    NamedEvent event = NamedEvent::CreateOrOpen(name);

    // Act
    ASSERT_NO_THROW(event.Set());
    bool result1 = event.Wait(1);
    bool result2 = event.Wait(1);

    // Assert
    ASSERT_TRUE(result1);
    ASSERT_FALSE(result2);
}

TEST_F(TestNamedEvent, SetTwiceOnNamedEvent) {
    // Arrange
    std::string name = GenerateName();
    NamedEvent event = NamedEvent::CreateOrOpen(name);

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
    std::string name = GenerateName();
    NamedEvent event = NamedEvent::CreateOrOpen(name);

    // Act
    ASSERT_NO_THROW(event.Set());
    bool result1 = event.Wait(1);
    bool result2 = event.Wait(1);

    // Assert
    ASSERT_TRUE(result1);
    ASSERT_FALSE(result2);
}

TEST_F(TestNamedEvent, WaitWithoutSetOnNamedEvent) {
    // Arrange
    std::string name = GenerateName();
    NamedEvent event = NamedEvent::CreateOrOpen(name);

    // Act
    bool result = event.Wait(1);

    // Assert
    ASSERT_FALSE(result);
}

TEST_F(TestNamedEvent, SetAndWaitOnDifferentNamedEvents) {
    // Arrange
    std::string name = GenerateName();
    NamedEvent event1 = NamedEvent::CreateOrOpen(name);
    NamedEvent event2 = NamedEvent::OpenExisting(name);

    // Act
    ASSERT_NO_THROW(event1.Set());
    ASSERT_NO_THROW(event2.Wait());

    // Assert
}

TEST_F(TestNamedEvent, ResetOnSettingNamedEvents) {
    // Arrange
    std::string name = GenerateName();
    NamedEvent event1 = NamedEvent::CreateOrOpen(name);
    NamedEvent event2 = NamedEvent::OpenExisting(name);

    // Act
    for (int32_t i = 0; i < 10; i++) {
        ASSERT_NO_THROW(event1.Set());
        ASSERT_NO_THROW(event2.Wait());
    }

    // Assert
}

TEST_F(TestNamedEvent, ResetOnWaitingNamedEvents) {
    // Arrange
    std::string name = GenerateName();
    NamedEvent event1 = NamedEvent::CreateOrOpen(name);
    NamedEvent event2 = NamedEvent::OpenExisting(name);

    // Act
    for (int32_t i = 0; i < 10; i++) {
        ASSERT_NO_THROW(event1.Set());
        ASSERT_NO_THROW(event2.Wait());
    }

    // Assert
}

TEST_F(TestNamedEvent, NoResetOnNamedEvents) {
    // Arrange
    std::string name = GenerateName();
    NamedEvent event1 = NamedEvent::CreateOrOpen(name);
    NamedEvent event2 = NamedEvent::OpenExisting(name);

    // Act
    for (int32_t i = 0; i < 10; i++) {
        ASSERT_NO_THROW(event1.Set());
        ASSERT_NO_THROW(event2.Wait());
    }

    // Assert
}

TEST_F(TestNamedEvent, SetAndWaitInDifferentThreads) {
    // Arrange
    std::string firstName = GenerateName();
    std::string secondName = GenerateName();
    NamedEvent event1 = NamedEvent::CreateOrOpen(firstName);
    NamedEvent event2 = NamedEvent::CreateOrOpen(secondName);

    std::jthread thread(WaitAndSet, firstName, secondName);

    // Act
    ASSERT_NO_THROW(event1.Set());
    ASSERT_NO_THROW(event2.Wait());

    // Assert
}

}  // namespace

#endif
