// Copyright dSPACE SE & Co. KG. All rights reserved.

#ifdef _WIN32

#include <string>
#include <thread>
#include <vector>

#include <benchmark/benchmark.h>

#include "Helper.hpp"
#include "OsUtilities.hpp"

using namespace DsVeosCoSim;

namespace {

void CounterPart(ShmPipeClient& client, const bool& stopThread, size_t size) {
    std::vector<uint8_t> buffer;
    buffer.resize(size);

    MustBeOk(ReceiveComplete(client, buffer.data(), buffer.size()));

    while (!stopThread) {
        MustBeOk(client.Send(buffer.data(), buffer.size()));
        MustBeOk(ReceiveComplete(client, buffer.data(), buffer.size()));
    }
}

void RunTest(benchmark::State& state, ShmPipeClient& client1, ShmPipeClient& client2) {
    auto size = static_cast<size_t>(state.range(0));

    std::vector<uint8_t> buffer;
    buffer.resize(size);

    bool stopThread{};
    std::thread thread(CounterPart, std::ref(client1), std::ref(stopThread), size);

    for (auto _ : state) {
        MustBeOk(client2.Send(buffer.data(), buffer.size()));
        MustBeOk(ReceiveComplete(client2, buffer.data(), buffer.size()));
    }

    stopThread = true;
    MustBeOk(client2.Send(buffer.data(), buffer.size()));

    thread.join();
}

void ShmPipeRoundtrip(benchmark::State& state) {
    std::string path = GenerateString("ShmPipe");

    ShmPipeListener listener;
    MustBeOk(ShmPipeListener::Create(path, listener));

    ShmPipeClient connectClient;
    MustBeOk(ShmPipeClient::TryConnect(path, connectClient));

    ShmPipeClient acceptClient;
    MustBeOk(listener.TryAccept(acceptClient));

    RunTest(state, connectClient, acceptClient);
}

}  // namespace

BENCHMARK(ShmPipeRoundtrip)->Arg(1)->Arg(100)->Arg(10000)->Arg(1000000)->Arg(100000000);

#endif
