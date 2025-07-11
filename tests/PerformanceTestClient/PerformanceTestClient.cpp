// Copyright dSPACE GmbH. All rights reserved.

#include "PerformanceTestClient.h"

#include <chrono>
#include <cstdint>
#include <string_view>
#include <thread>

#include "Event.h"
#include "Helper.h"

using namespace DsVeosCoSim;

void RunPerformanceTest(const PerformanceTestFunc& function, std::string_view host) {
    Event connected;
    uint64_t counter = 0;
    bool isStopped{};
    std::thread thread(function, host, std::ref(connected), std::ref(counter), std::ref(isStopped));

    (void)connected.Wait(UINT32_MAX);

    for (uint32_t i = 0; i < 5; i++) {
        auto beforeTime = std::chrono::high_resolution_clock::now();
        auto beforeValue = counter;

        std::this_thread::sleep_for(std::chrono::seconds(1));

        auto afterTime = std::chrono::high_resolution_clock::now();
        auto afterValue = counter;

        std::chrono::duration<double> duration = afterTime - beforeTime;
        auto diff = afterValue - beforeValue;
        auto callsPerSecond = static_cast<int64_t>(static_cast<double>(diff) / duration.count());
        LogTrace("{:>10} calls per second", callsPerSecond);
    }

    isStopped = true;
    thread.join();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}
