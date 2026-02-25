// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <benchmark/benchmark.h>

#include <string>
#include <thread>
#include <vector>

#include "Helper.hpp"
#include "Socket.hpp"

using namespace DsVeosCoSim;

namespace {

void CounterPart(const SocketClient& client, const bool& stopThread, size_t size) {
    std::vector<uint8_t> buffer;
    buffer.resize(size);

    MustBeOk(ReceiveComplete(client, buffer.data(), buffer.size()));

    while (!stopThread) {
        MustBeOk(client.Send(buffer.data(), buffer.size()));
        MustBeOk(ReceiveComplete(client, buffer.data(), buffer.size()));
    }
}

void RunTest(benchmark::State& state, SocketClient& client1, const SocketClient& client2) {
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

void SocketTcpRoundtrip(benchmark::State& state) {
    SocketListener listener;
    MustBeOk(SocketListener::Create(AddressFamily::Ipv4, 0, false, listener));

    uint16_t port{};
    MustBeOk(listener.GetLocalPort(port));

    SocketClient connectClient;
    MustBeOk(SocketClient::TryConnect("127.0.0.1", port, 0, 1000, connectClient));

    SocketClient acceptClient;
    MustBeOk(listener.TryAccept(acceptClient));

    RunTest(state, connectClient, acceptClient);
}

void SocketLocalRoundtrip(benchmark::State& state) {
    std::string path = GenerateString("LocalPath");

    SocketListener listener;
    MustBeOk(SocketListener::Create(path, listener));

    SocketClient connectClient;
    MustBeOk(SocketClient::TryConnect(path, connectClient));

    SocketClient acceptClient;
    MustBeOk(listener.TryAccept(acceptClient));

    RunTest(state, connectClient, acceptClient);
}

}  // namespace

BENCHMARK(SocketTcpRoundtrip)->Arg(1)->Arg(100)->Arg(10000)->Arg(1000000)->Arg(100000000);
BENCHMARK(SocketLocalRoundtrip)->Arg(1)->Arg(100)->Arg(10000)->Arg(1000000)->Arg(100000000);
