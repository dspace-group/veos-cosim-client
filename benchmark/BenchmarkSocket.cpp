// Copyright dSPACE GmbH. All rights reserved.

#include <benchmark/benchmark.h>

#include "BenchmarkHelper.h"
#include "Socket.h"

#include <thread>
#include <vector>

using namespace DsVeosCoSim;

using namespace std::chrono_literals;

namespace {

void Send(const Socket& socket, const std::vector<uint8_t>& buffer) {
    int length = (int)buffer.size();
    const uint8_t* bufferPointer = buffer.data();

    while (length > 0) {
        int sentSize = 0;
        ASSERT_OK(socket.Send(bufferPointer, length, sentSize));

        length -= sentSize;
        bufferPointer += sentSize;
    }
}

void Receive(const Socket& socket, std::vector<uint8_t>& buffer) {
    int length = (int)buffer.size();
    uint8_t* bufferPointer = buffer.data();

    while (length > 0) {
        int receivedSize = 0;
        ASSERT_OK(socket.Receive(bufferPointer, length, receivedSize));

        length -= receivedSize;
        bufferPointer += receivedSize;
    }
}

bool stopThread;

void ReceiveAndSend(uint16_t port, size_t size) {
    Socket client;
    ASSERT_OK(client.Create(AddressFamily::Ipv4));

    ASSERT_OK(client.Connect("127.0.0.1", port, 0));
    ASSERT_OK(client.EnableNoDelay());

    std::vector<uint8_t> buffer;
    buffer.resize(size);

    while (!stopThread) {
        Receive(client, buffer);
        Send(client, buffer);
    }
}

void SocketRoundtrip(benchmark::State& state) {  // NOLINT(readability-function-cognitive-complexity)
    ASSERT_OK(StartupNetwork());

    Socket server;
    ASSERT_OK(server.Create(AddressFamily::Ipv4));
    ASSERT_OK(server.EnableReuseAddress());
    uint16_t port{};
    ASSERT_OK(server.Bind(port, false));
    ASSERT_OK(server.Listen());
    ASSERT_OK(server.GetLocalPort(port));

    size_t size = state.range(0);

    stopThread = false;
    std::jthread thread(ReceiveAndSend, port, size);

    Socket client;
    while (true) {
        Result result = server.Accept(client);
        if (result == Result::TryAgain) {
            continue;
        }

        ASSERT_OK(result);
        break;
    }

    std::vector<uint8_t> buffer;
    buffer.resize(size);

    for (auto _ : state) {
        Send(client, buffer);
        Receive(client, buffer);
    }

    stopThread = true;
    Send(client, buffer);
}

}  // namespace

BENCHMARK(SocketRoundtrip)->Arg(1)->Arg(10)->Arg(100)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);
