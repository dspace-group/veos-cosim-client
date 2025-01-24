// Copyright dSPACE GmbH. All rights reserved.

#include <benchmark/benchmark.h>

#include <string>
#include <thread>
#include <vector>

#include "Generator.h"
#include "Helper.h"
#include "Socket.h"

using namespace DsVeosCoSim;

namespace {

void CounterPart(const Socket& socket, const bool& stopThread, const size_t size) {
    std::vector<uint8_t> buffer;
    buffer.resize(size);

    MUST_BE_TRUE(ReceiveComplete(socket, buffer.data(), buffer.size()));

    while (!stopThread) {
        MUST_BE_TRUE(SendComplete(socket, buffer.data(), buffer.size()));
        MUST_BE_TRUE(ReceiveComplete(socket, buffer.data(), buffer.size()));
    }
}

void RunTest(benchmark::State& state, Socket& socket1, const Socket& socket2) {
    size_t size = state.range(0);

    std::vector<uint8_t> buffer;
    buffer.resize(size);

    bool stopThread{};
    std::thread thread(CounterPart, std::ref(socket1), std::ref(stopThread), size);

    for (auto _ : state) {
        MUST_BE_TRUE(SendComplete(socket2, buffer.data(), buffer.size()));
        MUST_BE_TRUE(ReceiveComplete(socket2, buffer.data(), buffer.size()));
    }

    stopThread = true;
    MUST_BE_TRUE(SendComplete(socket2, buffer.data(), buffer.size()));

    thread.join();
}

void SocketTcpRoundtrip(benchmark::State& state) {
    const Socket server(AddressFamily::Ipv4);
    server.EnableReuseAddress();
    server.Bind(0, false);
    const uint16_t port = server.GetLocalPort();
    server.Listen();

    Socket connectedSocket = ConnectSocket("127.0.0.1", port);
    connectedSocket.EnableNoDelay();

    const Socket acceptedSocket = Accept(server);
    acceptedSocket.EnableNoDelay();

    RunTest(state, connectedSocket, acceptedSocket);
}

void SocketUdsRoundtrip(benchmark::State& state) {
    const std::string path = GenerateString("UdsPath");

    Socket server(AddressFamily::Uds);
    server.Bind(path);
    server.Listen();

    Socket connectedSocket = ConnectSocket(path);
    const Socket acceptedSocket = Accept(server);

    RunTest(state, connectedSocket, acceptedSocket);
}

}  // namespace

BENCHMARK(SocketTcpRoundtrip)->Arg(1)->Arg(100)->Arg(10000)->Arg(1000000);
BENCHMARK(SocketUdsRoundtrip)->Arg(1)->Arg(100)->Arg(10000)->Arg(1000000);
