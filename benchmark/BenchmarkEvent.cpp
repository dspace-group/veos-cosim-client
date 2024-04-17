// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32

#include <benchmark/benchmark.h>

#include "BenchmarkHelper.h"
#include "Event.h"
#include "Generator.h"

#include <thread>


using namespace DsVeosCoSim;

using namespace std::chrono_literals;

namespace {

void EventSet(benchmark::State& state) {
    Event event(GenerateString("Event"));
    ASSERT_OK(event.Create());

    for (auto _ : state) {
        ASSERT_OK(event.Set());
    }
}

void EventSetAndWait(benchmark::State& state) {
    Event event(GenerateString("Event"));
    ASSERT_OK(event.Create());

    for (auto _ : state) {
        ASSERT_OK(event.Set());
        ASSERT_OK(event.Wait());
    }
}

bool stopThread;

void WaitAndSet(const std::string& eventName1, const std::string& eventName2) {
    Event event1(eventName1);
    Event event2(eventName2);
    ASSERT_OK(event1.Create());
    ASSERT_OK(event2.Create());

    while (!stopThread) {
        ASSERT_OK(event1.Wait());
        ASSERT_OK(event2.Set());
    }
}

void EventRoundtrip(benchmark::State& state) {
    std::string eventName1 = GenerateString("Event");
    std::string eventName2 = GenerateString("Event");
    Event event1(eventName1);
    Event event2(eventName2);
    ASSERT_OK(event1.Create());
    ASSERT_OK(event2.Create());

    stopThread = false;
    std::jthread thread(WaitAndSet, eventName1, eventName2);

    for (auto _ : state) {
        ASSERT_OK(event1.Set());
        ASSERT_OK(event2.Wait());
    }

    stopThread = true;
    ASSERT_OK(event1.Set());
}

}  // namespace

BENCHMARK(EventSet);
BENCHMARK(EventSetAndWait);
BENCHMARK(EventRoundtrip);

#endif
