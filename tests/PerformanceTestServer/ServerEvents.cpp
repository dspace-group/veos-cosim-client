// Copyright dSPACE GmbH. All rights reserved.

#if defined(ALL_COMMUNICATION_TESTS) && defined(_WIN32)

#include <array>
#include <thread>

#include "LogHelper.h"
#include "OsUtilities.h"
#include "PerformanceTestHelper.h"

using namespace DsVeosCoSim;

namespace {

void EventsServerRun() {
    try {
        LogTrace("Events server listening on SHM {} ...", ShmName);

        std::array<char, BufferSize> buffer{};

        const std::unique_ptr<NamedEvent> beginEvent = CreateOrOpenNamedEvent(BeginEventName);
        const std::unique_ptr<NamedEvent> endEvent = CreateOrOpenNamedEvent(EndEventName);
        const std::unique_ptr<SharedMemory> sharedMemory = CreateOrOpenSharedMemory(ShmName, BufferSize);

        while (true) {
            beginEvent->Wait();
            (void)memcpy(buffer.data(), sharedMemory->data(), BufferSize);
            buffer[0]++;
            (void)memcpy(sharedMemory->data(), buffer.data(), BufferSize);
            endEvent->Set();
        }
    } catch (const std::exception& e) {
        LogError("Exception in event server thread: {}", e.what());
    }
}

}  // namespace

void StartEventsServer() {  // NOLINT(misc-use-internal-linkage)
    std::thread(EventsServerRun).detach();
}

#else

void StartEventsServer() {  // NOLINT(misc-use-internal-linkage)
}

#endif
