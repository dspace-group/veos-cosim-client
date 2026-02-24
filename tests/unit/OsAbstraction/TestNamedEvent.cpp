// Copyright dSPACE SE & Co. KG. All rights reserved.

#ifdef _WIN32

#include <gtest/gtest.h>

#include <string>
#include <thread>

#include "Helper.hpp"
#include "OsUtilities.hpp"
#include "TestHelper.hpp"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] std::string GenerateName() {
    return GenerateString("Event名前\xF0\x9F\x98\x80");
}

class TestNamedEvent : public testing::Test {};

TEST_F(TestNamedEvent, CreateShouldWork) {
    // Arrange
    std::string name = GenerateName();

    NamedEvent event;

    // Act and assert
    Result result = NamedEvent::CreateOrOpen(name, event);

    // Assert
    AssertOk(result);
}

TEST_F(TestNamedEvent, SetAndWaitOnSameNamedEvent) {
    // Arrange
    std::string name = GenerateName();

    NamedEvent event;
    AssertOk(NamedEvent::CreateOrOpen(name, event));

    // Act
    Result setResult = event.Set();
    Result waitResult = event.Wait();

    // Assert
    AssertOk(setResult);
    AssertOk(waitResult);
}

TEST_F(TestNamedEvent, SetAndWaitOnSameNamedEventWithTimeout) {
    // Arrange
    std::string name = GenerateName();

    NamedEvent event;
    AssertOk(NamedEvent::CreateOrOpen(name, event));

    AssertOk(event.Set());

    // Act
    Result result = event.Wait(1);

    // Assert
    AssertOk(result);
}

TEST_F(TestNamedEvent, WaitTwiceOnNamedEvent) {
    // Arrange
    std::string name = GenerateName();

    NamedEvent event;
    AssertOk(NamedEvent::CreateOrOpen(name, event));

    AssertOk(event.Set());

    AssertOk(event.Wait(1));

    // Act
    Result result = event.Wait(1);

    // Assert
    AssertTimeout(result);
}

TEST_F(TestNamedEvent, SetTwiceOnNamedEvent) {
    // Arrange
    std::string name = GenerateName();

    NamedEvent event;
    AssertOk(NamedEvent::CreateOrOpen(name, event));

    AssertOk(event.Set());

    // Act
    Result setResult = event.Set();  // Second set
    Result waitResult1 = event.Wait(1);
    Result waitResult2 = event.Wait(1);

    // Assert
    AssertOk(setResult);
    AssertOk(waitResult1);
    AssertTimeout(waitResult2);  // Second signal should be false
}

TEST_F(TestNamedEvent, WaitWithoutSetOnNamedEvent) {
    // Arrange
    std::string name = GenerateName();

    NamedEvent event;
    AssertOk(NamedEvent::CreateOrOpen(name, event));

    // Act
    Result result = event.Wait(1);

    // Assert
    AssertTimeout(result);
}

TEST_F(TestNamedEvent, SetAndWaitOnDifferentNamedEvents) {
    // Arrange
    std::string name = GenerateName();

    NamedEvent event1;
    AssertOk(NamedEvent::CreateOrOpen(name, event1));

    NamedEvent event2;
    AssertOk(NamedEvent::CreateOrOpen(name, event2));

    // Act
    Result setResult = event1.Set();
    Result waitResult = event2.Wait();

    // Assert
    AssertOk(setResult);
    AssertOk(waitResult);
}

void WaitAndSet(const std::string& name1, const std::string& name2) {
    NamedEvent event1;
    AssertOk(NamedEvent::CreateOrOpen(name1, event1));

    NamedEvent event2;
    AssertOk(NamedEvent::CreateOrOpen(name2, event2));

    AssertOk(event1.Wait());
    AssertOk(event2.Set());
}

TEST_F(TestNamedEvent, SetAndWaitInDifferentThreads) {
    // Arrange
    std::string name1 = GenerateName();
    std::string name2 = GenerateName();

    NamedEvent event1;
    AssertOk(NamedEvent::CreateOrOpen(name1, event1));

    NamedEvent event2;
    AssertOk(NamedEvent::CreateOrOpen(name2, event2));

    std::thread thread(WaitAndSet, name1, name2);

    // Act
    Result setResult = event1.Set();
    Result waitResult = event2.Wait();

    // Assert
    AssertOk(setResult);
    AssertOk(waitResult);

    // Cleanup
    thread.join();
}

}  // namespace

#endif
