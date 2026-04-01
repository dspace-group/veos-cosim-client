// Copyright dSPACE SE & Co. KG. All rights reserved.

#ifdef _WIN32

#include <array>
#include <thread>

#include "Logger.hpp"
#include "OsUtilities.hpp"
#include "PerformanceTestHelper.hpp"
#include "PerformanceTestServer.hpp"
#include "Result.hpp"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] Result RunServerEventsInternal() {
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

        memcpy(buffer.data(), sharedMemory.GetData(), FrameSize);
        buffer[0]++;
        memcpy(sharedMemory.GetData(), buffer.data(), FrameSize);

        CheckResult(endEvent.Set());
    }

    return CreateOk();
}

void RunServerEvents() {
    if (!IsOk(RunServerEventsInternal())) {
        LogError("Could not run Events Server.");
    }
}

}  // namespace

void ServerEvents() {
    std::thread(RunServerEvents).detach();
}

#endif
