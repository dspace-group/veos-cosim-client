// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32
#include <cstdint>
#include <string_view>

#include "CoSimHelper.h"
#include "LogHelper.h"
#include "NamedEvent.h"
#include "PerformanceTestHelper.h"
#include "RunPerformanceTest.h"
#include "SharedMemory.h"

using namespace DsVeosCoSim;

namespace {

void EventsClientRun([[maybe_unused]] std::string_view host,
                     Event& connectedEvent,
                     uint64_t& counter,
                     const bool& isStopped) {
    try {
        NamedEvent beginEvent = NamedEvent::CreateOrOpen(BeginEventName);
        NamedEvent endEvent = NamedEvent::CreateOrOpen(EndEventName);
        SharedMemory sharedMemory = SharedMemory::CreateOrOpen(ShmName, BufferSize);

        char buffer[BufferSize]{};

        connectedEvent.Set();

        while (!isStopped) {
            ::memcpy(sharedMemory.data(), buffer, BufferSize);
            beginEvent.Set();
            endEvent.Wait();
            ::memcpy(buffer, sharedMemory.data(), BufferSize);
            buffer[0]++;

            counter++;
        }
    } catch (const std::exception& e) {
        LogError("Exception in event client thread: {}", e.what());
        connectedEvent.Set();
    }
}

}  // namespace

void RunEventsTest() {
    LogTrace("Event:");
    RunPerformanceTest(EventsClientRun, "");
    LogTrace("");
}
#else
void RunEventsTest() {
}
#endif
