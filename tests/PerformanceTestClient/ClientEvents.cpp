// Copyright dSPACE SE & Co. KG. All rights reserved.

#ifdef _WIN32

#include <array>
#include <cstdint>
#include <string>

#include "Logger.hpp"
#include "OsUtilities.hpp"
#include "PerformanceTestClient.hpp"
#include "PerformanceTestHelper.hpp"
#include "Result.hpp"

namespace DsVeosCoSim {

namespace {

[[nodiscard]] Result Run([[maybe_unused]] const std::string& host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    NamedEvent beginEvent;
    CheckResult(NamedEvent::CreateOrOpen(BeginEventName, beginEvent));

    NamedEvent endEvent;
    CheckResult(NamedEvent::CreateOrOpen(EndEventName, endEvent));

    SharedMemory sharedMemory;
    CheckResult(SharedMemory::TryOpenExisting(ShmName, FrameSize, sharedMemory));

    std::array<char, FrameSize> buffer{};

    connectedEvent.Set();

    while (!isStopped) {
        (void)memcpy(sharedMemory.GetData(), buffer.data(), FrameSize);

        CheckResult(beginEvent.Set());
        CheckResult(endEvent.Wait());

        (void)memcpy(buffer.data(), sharedMemory.GetData(), FrameSize);
        buffer[0]++;

        counter++;
    }

    return CreateOk();
}

void EventsTest(const std::string& host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    if (!IsOk(Run(host, connectedEvent, counter, isStopped))) {
        LogError("Could not run Events Client.");
    }
}

}  // namespace

void RunEventsTest() {  // NOLINT(misc-use-internal-linkage)
    LogTrace("Event:");
    RunPerformanceTest(EventsTest, "");
    LogTrace("");
}

}  // namespace DsVeosCoSim

#endif
