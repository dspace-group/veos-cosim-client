// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32

#include <benchmark/benchmark.h>

#include <string>
#include <thread>

#include "Generator.h"
#include "NamedEvent.h"

using namespace DsVeosCoSim;

namespace {

void EventSet(benchmark::State& state) {
    const std::string name = GenerateString("Event名前");

    const NamedEvent event = NamedEvent::CreateOrOpen(name);

    for (auto _ : state) {
        event.Set();
    }
}

void EventSetAndWait(benchmark::State& state) {
    const std::string name = GenerateString("Event名前");

    const NamedEvent event = NamedEvent::CreateOrOpen(name);

    for (auto _ : state) {
        event.Set();
        event.Wait();
    }
}

bool StopThread;

void WaitAndSet(const std::string& eventName1, const std::string& eventName2) {
    const NamedEvent event1 = NamedEvent::CreateOrOpen(eventName1);
    const NamedEvent event2 = NamedEvent::CreateOrOpen(eventName2);

    while (!StopThread) {
        event1.Wait();
        event2.Set();
    }
}

void EventRoundtrip(benchmark::State& state) {
    const std::string eventName1 = GenerateString("Event名前");
    const std::string eventName2 = GenerateString("Event名前");

    const NamedEvent event1 = NamedEvent::CreateOrOpen(eventName1);
    const NamedEvent event2 = NamedEvent::CreateOrOpen(eventName2);

    StopThread = false;
    std::thread thread(WaitAndSet, eventName1, eventName2);

    for (auto _ : state) {
        event1.Set();
        event2.Wait();
    }

    StopThread = true;
    event1.Set();

    thread.join();
}

}  // namespace

BENCHMARK(EventSet);
BENCHMARK(EventSetAndWait);
BENCHMARK(EventRoundtrip);

#endif
