// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32

#include <gtest/gtest.h>

#include <string>
#include <thread>

#include "Helper.h"
#include "OsUtilities.h"
#include "TestHelper.h"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] std::string GenerateName() {
    return GenerateString("Event名前\xF0\x9F\x98\x80");
}

class TestNamedEvent : public testing::Test {};

TEST_F(TestNamedEvent, SetAndWaitOnSameNamedEvent) {
    // Arrange
    std::string name = GenerateName();
    NamedEvent event;
    ExpectOk(NamedEvent::CreateOrOpen(name, event));

    // Act and assert
    AssertOk(event.Set());
    AssertOk(event.Wait());
}

TEST_F(TestNamedEvent, SetAndWaitOnSameNamedEventWithTimeout) {
    // Arrange
    std::string name = GenerateName();
    NamedEvent event;
    ExpectOk(NamedEvent::CreateOrOpen(name, event));
    ExpectOk(event.Set());

    bool waitSignaled{};

    // Act
    AssertOk(event.Wait(1, waitSignaled));

    // Assert
    AssertTrue(waitSignaled);
}

TEST_F(TestNamedEvent, WaitTwiceOnNamedEvent) {
    // Arrange
    std::string name = GenerateName();
    NamedEvent event;
    ExpectOk(NamedEvent::CreateOrOpen(name, event));

    bool waitSignaled1{};
    bool waitSignaled2{};

    // Act
    AssertOk(event.Set());
    AssertOk(event.Wait(1, waitSignaled1));
    AssertOk(event.Wait(1, waitSignaled2));

    // Assert
    AssertTrue(waitSignaled1);
    AssertFalse(waitSignaled2);
}

TEST_F(TestNamedEvent, SetTwiceOnNamedEvent) {
    // Arrange
    std::string name = GenerateName();
    NamedEvent event;
    ExpectOk(NamedEvent::CreateOrOpen(name, event));

    bool waitSignaled1{};
    bool waitSignaled2{};

    // Act
    AssertOk(event.Set());
    AssertOk(event.Set());
    AssertOk(event.Wait(1, waitSignaled1));
    AssertOk(event.Wait(1, waitSignaled2));

    // Assert
    AssertTrue(waitSignaled1);
    AssertFalse(waitSignaled2);
}

TEST_F(TestNamedEvent, WaitWithoutSetOnNamedEvent) {
    // Arrange
    std::string name = GenerateName();
    NamedEvent event;
    ExpectOk(NamedEvent::CreateOrOpen(name, event));

    bool waitSignaled{};

    // Act
    AssertOk(event.Wait(1, waitSignaled));

    // Assert
    AssertFalse(waitSignaled);
}

TEST_F(TestNamedEvent, SetAndWaitOnDifferentNamedEvents) {
    // Arrange
    std::string name = GenerateName();
    NamedEvent event1;
    ExpectOk(NamedEvent::CreateOrOpen(name, event1));
    NamedEvent event2;
    ExpectOk(NamedEvent::CreateOrOpen(name, event2));

    // Act and assert
    AssertOk(event1.Set());
    AssertOk(event2.Wait());
}

TEST_F(TestNamedEvent, ResetOnSettingNamedEvents) {
    // Arrange
    std::string name = GenerateName();
    NamedEvent event1;
    ExpectOk(NamedEvent::CreateOrOpen(name, event1));
    NamedEvent event2;
    ExpectOk(NamedEvent::CreateOrOpen(name, event2));

    // Act and assert
    for (int32_t i = 0; i < 10; i++) {
        AssertOk(event1.Set());
        AssertOk(event2.Wait());
    }
}

TEST_F(TestNamedEvent, ResetOnWaitingNamedEvents) {
    // Arrange
    std::string name = GenerateName();
    NamedEvent event1;
    ExpectOk(NamedEvent::CreateOrOpen(name, event1));
    NamedEvent event2;
    ExpectOk(NamedEvent::CreateOrOpen(name, event2));

    // Act and assert
    for (int32_t i = 0; i < 10; i++) {
        AssertOk(event1.Set());
        AssertOk(event2.Wait());
    }
}

TEST_F(TestNamedEvent, NoResetOnNamedEvents) {
    // Arrange
    std::string name = GenerateName();
    NamedEvent event1;
    ExpectOk(NamedEvent::CreateOrOpen(name, event1));
    NamedEvent event2;
    ExpectOk(NamedEvent::CreateOrOpen(name, event2));

    // Act and assert
    for (int32_t i = 0; i < 10; i++) {
        AssertOk(event1.Set());
        AssertOk(event2.Wait());
    }
}

void WaitAndSet(const std::string& eventName1, const std::string& eventName2) {
    NamedEvent event1;
    ExpectOk(NamedEvent::CreateOrOpen(eventName1, event1));
    NamedEvent event2;
    ExpectOk(NamedEvent::CreateOrOpen(eventName2, event2));

    ExpectOk(event1.Wait());
    ExpectOk(event2.Set());
}

TEST_F(TestNamedEvent, SetAndWaitInDifferentThreads) {
    // Arrange
    std::string firstName = GenerateName();
    std::string secondName = GenerateName();
    NamedEvent event1;
    ExpectOk(NamedEvent::CreateOrOpen(firstName, event1));
    NamedEvent event2;
    ExpectOk(NamedEvent::CreateOrOpen(secondName, event2));

    std::thread thread(WaitAndSet, firstName, secondName);

    // Act and assert
    AssertOk(event1.Set());
    AssertOk(event2.Wait());

    // Cleanup
    thread.join();
}

}  // namespace

#endif
