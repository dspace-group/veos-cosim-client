// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32
#include <array>
#include <thread>

#include "LogHelper.h"
#include "NamedEvent.h"
#include "PerformanceTestHelper.h"
#include "SharedMemory.h"

using namespace DsVeosCoSim;

namespace {

void EventsServerRun() {
    try {
        LogTrace("Events server listening on SHM {} ...", ShmName);

        std::array<char, BufferSize> buffer{};

        const NamedEvent beginEvent = NamedEvent::CreateOrOpen(BeginEventName);
        const NamedEvent endEvent = NamedEvent::CreateOrOpen(EndEventName);
        const SharedMemory sharedMemory = SharedMemory::CreateOrOpen(ShmName, BufferSize);

        while (true) {
            beginEvent.Wait();
            (void)memcpy(buffer.data(), sharedMemory.data(), BufferSize);
            buffer[0]++;
            (void)memcpy(sharedMemory.data(), buffer.data(), BufferSize);
            endEvent.Set();
        }
    } catch (const std::exception& e) {
        LogError("Exception in event server thread: {}", e.what());
    }
}

}  // namespace

void StartEventsServer() {  // NOLINT
    std::thread(EventsServerRun).detach();
}
#else
void StartEventsServer() {  // NOLINT
}
#endif
