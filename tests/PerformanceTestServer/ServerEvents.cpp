// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "PerformanceTestServer.hpp"

#ifdef _WIN32

#include <array>
#include <thread>

#include "Helper.hpp"
#include "OsUtilities.hpp"
#include "PerformanceTestHelper.hpp"

namespace DsVeosCoSim {

namespace {

[[nodiscard]] Result Run() {
    LogTrace("Events server listening on SHM {} ...", ShmName);

    NamedEvent beginEvent;
    CheckResult(NamedEvent::CreateOrOpen(BeginEventName, beginEvent));

    NamedEvent endEvent;
    CheckResult(NamedEvent::CreateOrOpen(EndEventName, endEvent));

    SharedMemory sharedMemory;
    CheckResult(SharedMemory::CreateOrOpen(ShmName, FrameSize, sharedMemory));

    std::array<char, FrameSize> buffer{};

    while (true) {
        CheckResult(beginEvent.Wait());

        (void)memcpy(buffer.data(), sharedMemory.GetData(), FrameSize);
        buffer[0]++;
        (void)memcpy(sharedMemory.GetData(), buffer.data(), FrameSize);

        CheckResult(endEvent.Set());
    }

    return CreateOk();
}

void EventsServer() {
    if (!IsOk(Run())) {
        LogError("Could not run Events Server.");
    }
}

}  // namespace

void StartEventsServer() {
    std::thread(EventsServer).detach();
}

}  // namespace DsVeosCoSim

#else

namespace DsVeosCoSim {

void StartEventsServer() {
}

}  // namespace DsVeosCoSim

#endif
