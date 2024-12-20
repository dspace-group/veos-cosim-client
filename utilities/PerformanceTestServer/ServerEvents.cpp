// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32
#include <array>
#include <thread>

#include "LogHelper.h"
#include "NamedEvent.h"
#include "PerformanceTestHelper.h"
#include "SharedMemory.h"

using namespace DsVeosCoSim;

void EventsServerRun() {
    try {
        LogTrace("Events server listening on SHM {} ...", ShmName);

        std::array<char, BufferSize> buffer{};

        NamedEvent beginEvent = NamedEvent::CreateOrOpen(BeginEventName);
        NamedEvent endEvent = NamedEvent::CreateOrOpen(EndEventName);
        SharedMemory sharedMemory = SharedMemory::CreateOrOpen(ShmName, BufferSize);

        while (true) {
            beginEvent.Wait();
            ::memcpy(buffer.data(), sharedMemory.data(), BufferSize);
            buffer[0]++;
            ::memcpy(sharedMemory.data(), buffer.data(), BufferSize);
            endEvent.Set();
        }
    } catch (const std::exception& e) {
        LogError("Exception in event server thread: {}", e.what());
    }
}

void StartEventsServer() {
    std::thread(EventsServerRun).detach();
}
#else
void StartEventsServer() {
}
#endif
