// Copyright dSPACE GmbH. All rights reserved.

#ifdef ALL_BENCHMARK_TESTS

#include <benchmark/benchmark.h>

#include <string>
#include <thread>
#include <vector>

#include "Helper.h"
#include "Socket.h"

using namespace DsVeosCoSim;

namespace {

void CounterPart(const Socket& socket, const bool& stopThread, size_t size) {
    std::vector<uint8_t> buffer;
    buffer.resize(size);

    MustBeOk(ReceiveComplete(socket, buffer.data(), buffer.size()));

    while (!stopThread) {
        MustBeOk(socket.Send(buffer.data(), buffer.size()));
        MustBeOk(ReceiveComplete(socket, buffer.data(), buffer.size()));
    }
}

void RunTest(benchmark::State& state, Socket& socket1, const Socket& socket2) {
    auto size = static_cast<size_t>(state.range(0));

    std::vector<uint8_t> buffer;
    buffer.resize(size);

    bool stopThread{};
    std::thread thread(CounterPart, std::ref(socket1), std::ref(stopThread), size);

    for (auto _ : state) {
        MustBeOk(socket2.Send(buffer.data(), buffer.size()));
        MustBeOk(ReceiveComplete(socket2, buffer.data(), buffer.size()));
    }

    stopThread = true;
    MustBeOk(socket2.Send(buffer.data(), buffer.size()));

    thread.join();
}

void SocketTcpRoundtrip(benchmark::State& state) {
    Socket server;
    MustBeOk(Socket::Create(AddressFamily::Ipv4, server));
    MustBeOk(server.EnableReuseAddress());
    MustBeOk(server.Bind(0, false));
    uint16_t port{};
    MustBeOk(server.GetLocalPort(port));
    MustBeOk(server.Listen());

    std::optional<Socket> connectedSocket;
    MustBeOk(Socket::TryConnect("127.0.0.1", port, 0, DefaultTimeout, connectedSocket));
    MustBeTrue(connectedSocket);
    MustBeOk(connectedSocket->EnableNoDelay());

    std::optional<Socket> acceptedSocket;
    MustBeOk(server.TryAccept(acceptedSocket));
    MustBeTrue(acceptedSocket);
    MustBeOk(acceptedSocket->EnableNoDelay());

    RunTest(state, *connectedSocket, *acceptedSocket);
}

void SocketUdsRoundtrip(benchmark::State& state) {
    std::string path = GenerateString("UdsPath");

    Socket server;
    MustBeOk(Socket::Create(AddressFamily::Uds, server));
    MustBeOk(server.Bind(path));
    MustBeOk(server.Listen());

    std::optional<Socket> connectedSocket;
    MustBeOk(Socket::TryConnect(path, connectedSocket));
    MustBeTrue(connectedSocket);
    std::optional<Socket> acceptedSocket;
    MustBeOk(server.TryAccept(acceptedSocket));
    MustBeTrue(acceptedSocket);

    RunTest(state, *connectedSocket, *acceptedSocket);
}

}  // namespace

BENCHMARK(SocketTcpRoundtrip)->Arg(1)->Arg(100)->Arg(10000)->Arg(1000000);
BENCHMARK(SocketUdsRoundtrip)->Arg(1)->Arg(100)->Arg(10000)->Arg(1000000);

#endif
