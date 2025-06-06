// Copyright dSPACE GmbH. All rights reserved.

#if defined(ALL_COMMUNICATION_TESTS) && defined(_WIN32)

#include <array>
#include <cstdint>
#include <string_view>

#include "CoSimHelper.h"
#include "LogHelper.h"
#include "OsUtilities.h"
#include "PerformanceTestHelper.h"
#include "RunPerformanceTest.h"

using namespace DsVeosCoSim;

namespace {

void EventsClientRun([[maybe_unused]] std::string_view host,
                     Event& connectedEvent,
                     uint64_t& counter,
                     const bool& isStopped) {
    try {
        const std::unique_ptr<NamedEvent> beginEvent = CreateOrOpenNamedEvent(BeginEventName);
        const std::unique_ptr<NamedEvent> endEvent = CreateOrOpenNamedEvent(EndEventName);
        const std::unique_ptr<SharedMemory> sharedMemory = CreateOrOpenSharedMemory(ShmName, BufferSize);

        std::array<char, BufferSize> buffer{};

        connectedEvent.Set();

        while (!isStopped) {
            (void)memcpy(sharedMemory->data(), buffer.data(), BufferSize);
            beginEvent->Set();
            endEvent->Wait();
            (void)memcpy(buffer.data(), sharedMemory->data(), BufferSize);
            buffer[0]++;

            counter++;
        }
    } catch (const std::exception& e) {
        LogError("Exception in event client thread: {}", e.what());
        connectedEvent.Set();
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
