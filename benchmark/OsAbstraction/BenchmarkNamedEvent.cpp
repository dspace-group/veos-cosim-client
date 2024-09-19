// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32

#include <benchmark/benchmark.h>
#include <string>
#include <string_view>
#include <thread>

#include "Generator.h"
#include "NamedEvent.h"

using namespace DsVeosCoSim;

namespace {

void EventSet(benchmark::State& state) {
    std::string name = GenerateString("Event名前");

    NamedEvent event = NamedEvent::CreateOrOpen(name);

    for (auto _ : state) {
        event.Set();
    }
}

void EventSetAndWait(benchmark::State& state) {
    std::string name = GenerateString("Event名前");

    NamedEvent event = NamedEvent::CreateOrOpen(name);

    for (auto _ : state) {
        event.Set();
        event.Wait();
    }
}

bool stopThread;

void WaitAndSet(std::string_view eventName1, std::string_view eventName2) {
    NamedEvent event1 = NamedEvent::CreateOrOpen(eventName1);
    NamedEvent event2 = NamedEvent::CreateOrOpen(eventName2);

    while (!stopThread) {
        event1.Wait();
        event2.Set();
    }
}

void EventRoundtrip(benchmark::State& state) {
    std::string eventName1 = GenerateString("Event名前");
    std::string eventName2 = GenerateString("Event名前");

    NamedEvent event1 = NamedEvent::CreateOrOpen(eventName1);
    NamedEvent event2 = NamedEvent::CreateOrOpen(eventName2);

    stopThread = false;
    std::jthread thread(WaitAndSet, eventName1, eventName2);

    for (auto _ : state) {
        event1.Set();
        event2.Wait();
    }

    stopThread = true;
    event1.Set();
}

}  // namespace

BENCHMARK(EventSet);
BENCHMARK(EventSetAndWait);
BENCHMARK(EventRoundtrip);

#endif
