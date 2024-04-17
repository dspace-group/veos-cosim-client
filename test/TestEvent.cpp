// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32

#include "Event.h"
#include "Generator.h"
#include "Logger.h"
#include "TestHelper.h"

using namespace DsVeosCoSim;

namespace {

void WaitAndSet(const std::string& firstEventName, const std::string& secondEventName) {
    Event firstEvent(firstEventName);
    ASSERT_OK(firstEvent.Create());

    Event secondEvent(secondEventName);
    ASSERT_OK(secondEvent.Create());

    ASSERT_OK(firstEvent.Wait());
    ASSERT_OK(secondEvent.Set());
}

}  // namespace

class TestEvent : public testing::Test {
protected:
    void SetUp() override {
        SetLogCallback(OnLogCallback);
    }
};

TEST_F(TestEvent, CreateAndDestroy) {
    // Arrange
    Event event(GenerateString("Event"));

    // Act
    Result result = event.Create();

    // Assert
    ASSERT_OK(result);
}

TEST_F(TestEvent, SetAndWaitOnSameEvent) {
    // Arrange
    std::string name = GenerateString("Event");
    Event event(name);
    ASSERT_OK(event.Create());

    // Act
    Result result1 = event.Set();
    Result result2 = event.Wait();

    // Assert
    ASSERT_OK(result1);
    ASSERT_OK(result2);
}

TEST_F(TestEvent, SetAndWaitOnDifferentEvents) {
    // Arrange
    std::string name = GenerateString("Event");
    Event event1(name);
    Event event2(name);
    ASSERT_OK(event1.Create());
    ASSERT_OK(event2.Create());

    // Act
    Result result1 = event1.Set();
    Result result2 = event1.Wait();

    // Assert
    ASSERT_OK(result1);
    ASSERT_OK(result2);
}

TEST_F(TestEvent, WaitWithoutSet) {
    // Arrange
    std::string name = GenerateString("Event");
    Event event(name);
    ASSERT_OK(event.Create());

    // Act
    Result result2 = event.Wait(10);

    // Assert
    ASSERT_ERROR(result2);
}

TEST_F(TestEvent, SetAndWaitOnDifferentThreads) {
    // Arrange
    std::string firstName = GenerateString("Event");
    std::string secondName = GenerateString("Event");
    Event event1(firstName);
    Event event2(secondName);
    ASSERT_OK(event1.Create());
    ASSERT_OK(event2.Create());

    std::jthread thread(WaitAndSet, firstName, secondName);

    // Act
    Result result1 = event1.Set();
    Result result2 = event2.Wait();

    // Assert
    ASSERT_OK(result1);
    ASSERT_OK(result2);
}

#endif
