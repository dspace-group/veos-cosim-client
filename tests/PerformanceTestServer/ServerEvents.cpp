// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "PerformanceTestServer.h"

#if defined(ALL_COMMUNICATION_TESTS) && defined(_WIN32)

#include <array>
#include <thread>

#include "Helper.h"
#include "OsUtilities.h"
#include "PerformanceTestHelper.h"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] Result Run() {
    LogTrace("Events server listening on SHM {} ...", ShmName);

    std::array<char, BufferSize> buffer{};

    NamedEvent beginEvent;
    CheckResult(NamedEvent::CreateOrOpen(BeginEventName, beginEvent));
    NamedEvent endEvent;
    CheckResult(NamedEvent::CreateOrOpen(EndEventName, endEvent));
    SharedMemory sharedMemory;
    CheckResult(SharedMemory::CreateOrOpen(ShmName, BufferSize, sharedMemory));

    while (true) {
        CheckResult(beginEvent.Wait());
        (void)memcpy(buffer.data(), sharedMemory.GetData(), BufferSize);
        buffer[0]++;
        (void)memcpy(sharedMemory.GetData(), buffer.data(), BufferSize);
        CheckResult(endEvent.Set());
    }

    return Result::Ok;
}

void EventsServerRun() {
    if (!IsOk(Run())) {
        LogError("Could not run events server.");
    }
}

}  // namespace

void StartEventsServer() {
    std::thread(EventsServerRun).detach();
}

#else

void StartEventsServer() {
}

#endif
