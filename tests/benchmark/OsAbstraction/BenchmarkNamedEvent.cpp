// Copyright dSPACE SE & Co. KG. All rights reserved.

#ifdef ALL_BENCHMARK_TESTS

#ifdef _WIN32

#include <benchmark/benchmark.h>

#include <string>
#include <thread>

#include "Helper.h"
#include "OsUtilities.h"

using namespace DsVeosCoSim;

namespace {

void EventSet(benchmark::State& state) {
    std::string name = GenerateString("Event名前");

    NamedEvent event;
    MustBeOk(NamedEvent::CreateOrOpen(name, event));

    for (auto _ : state) {
        MustBeOk(event.Set());
    }
}

void EventSetAndWait(benchmark::State& state) {
    std::string name = GenerateString("Event名前");

    NamedEvent event;
    MustBeOk(NamedEvent::CreateOrOpen(name, event));

    for (auto _ : state) {
        MustBeOk(event.Set());
        MustBeOk(event.Wait());
    }
}

bool StopThread;

void WaitAndSet(const std::string& eventName1, const std::string& eventName2) {
    NamedEvent event1;
    MustBeOk(NamedEvent::CreateOrOpen(eventName1, event1));
    NamedEvent event2;
    MustBeOk(NamedEvent::CreateOrOpen(eventName2, event2));

    while (!StopThread) {
        MustBeOk(event1.Wait());
        MustBeOk(event2.Set());
    }
}

void EventRoundtrip(benchmark::State& state) {
    std::string eventName1 = GenerateString("Event名前");
    std::string eventName2 = GenerateString("Event名前");

    NamedEvent event1;
    MustBeOk(NamedEvent::CreateOrOpen(eventName1, event1));
    NamedEvent event2;
    MustBeOk(NamedEvent::CreateOrOpen(eventName2, event2));

    StopThread = false;
    std::thread thread(WaitAndSet, eventName1, eventName2);

    for (auto _ : state) {
        MustBeOk(event1.Set());
        MustBeOk(event2.Wait());
    }

    StopThread = true;
    MustBeOk(event1.Set());

    thread.join();
}

}  // namespace

BENCHMARK(EventSet);
BENCHMARK(EventSetAndWait);
BENCHMARK(EventRoundtrip);

#endif

#endif
