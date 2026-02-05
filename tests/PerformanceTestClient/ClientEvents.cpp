// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "PerformanceTestClient.h"

#if defined(ALL_COMMUNICATION_TESTS) && defined(_WIN32)

#include <array>
#include <cstdint>
#include <string>

#include "Helper.h"
#include "OsUtilities.h"
#include "PerformanceTestHelper.h"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] Result Run([[maybe_unused]] const std::string& host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    NamedEvent beginEvent;
    CheckResult(NamedEvent::CreateOrOpen(BeginEventName, beginEvent));
    NamedEvent endEvent;
    CheckResult(NamedEvent::CreateOrOpen(EndEventName, endEvent));
    SharedMemory sharedMemory;
    CheckResult(SharedMemory::CreateOrOpen(ShmName, BufferSize, sharedMemory));

    std::array<char, BufferSize> buffer{};

    connectedEvent.Set();

    while (!isStopped) {
        (void)memcpy(sharedMemory.GetData(), buffer.data(), BufferSize);
        CheckResult(beginEvent.Set());
        CheckResult(endEvent.Wait());
        (void)memcpy(buffer.data(), sharedMemory.GetData(), BufferSize);
        buffer[0]++;

        counter++;
    }

    return Result::Ok;
}

void EventsClientRun(const std::string& host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    if (!IsOk(Run(host, connectedEvent, counter, isStopped))) {
        LogError("Could not run events client.");
    }
}

}  // namespace

void RunEventsTest() {  // NOLINT(misc-use-internal-linkage)
    LogTrace("Event:");
    RunPerformanceTest(EventsClientRun, "");
    LogTrace("");
}

#else

void RunEventsTest() {  // NOLINT(misc-use-internal-linkage)
}

#endif
