// Copyright dSPACE GmbH. All rights reserved.

#include <benchmark/benchmark.h>

#include "BenchmarkHelper.h"
#include "Generator.h"
#include "Socket.h"

#include <thread>
#include <vector>

using namespace DsVeosCoSim;

using namespace std::chrono_literals;

namespace {

void SendExactly(const Socket& socket, const void* buffer, size_t length) {
    const auto* bufferPointer = (const uint8_t*)buffer;
    while (length > 0) {
        int sentSize = 0;
        ASSERT_OK(socket.Send(bufferPointer, (int)length, sentSize));

        length -= sentSize;
        bufferPointer += sentSize;
    }
}

void ReceiveExactly(const Socket& socket, void* buffer, size_t length) {
    auto* bufferPointer = (uint8_t*)buffer;
    while (length > 0) {
        int receivedSize = 0;
        ASSERT_OK(socket.Receive(bufferPointer, (int)length, receivedSize));

        length -= receivedSize;
        bufferPointer += receivedSize;
    }
}

bool stopThread;

void ReceiveAndSendTcp(uint16_t port, size_t size) {
    Socket client;
    ASSERT_OK(client.Create(AddressFamily::Ipv4));

    ASSERT_OK(client.Connect("127.0.0.1", port, 0));
    ASSERT_OK(client.EnableNoDelay());

    std::vector<uint8_t> buffer;
    buffer.resize(size);

    ReceiveExactly(client, buffer.data(), buffer.size());

    while (!stopThread) {
        SendExactly(client, buffer.data(), buffer.size());
        ReceiveExactly(client, buffer.data(), buffer.size());
    }
}

void SocketTcpRoundtrip(benchmark::State& state) {  // NOLINT(readability-function-cognitive-complexity)
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
    std::jthread thread(ReceiveAndSendTcp, port, size);

    Socket client;
    while (true) {
        Result result = server.Accept(client);
        if (result == Result::TryAgain) {
            continue;
        }

        ASSERT_OK(result);
        break;
    }

    ASSERT_OK(client.EnableNoDelay());

    std::vector<uint8_t> buffer;
    buffer.resize(size);

    for (auto _ : state) {
        SendExactly(client, buffer.data(), buffer.size());
        ReceiveExactly(client, buffer.data(), buffer.size());
    }

    stopThread = true;
    SendExactly(client, buffer.data(), buffer.size());
}

void ReceiveAndSendUds(const std::string& path, size_t size) {
    Socket client;
    ASSERT_OK(client.Create(AddressFamily::Uds));

    ASSERT_OK(client.Connect(path));

    std::vector<uint8_t> buffer;
    buffer.resize(size);

    ReceiveExactly(client, buffer.data(), buffer.size());

    while (!stopThread) {
        SendExactly(client, buffer.data(), buffer.size());
        ReceiveExactly(client, buffer.data(), buffer.size());
    }
}

void SocketUdsRoundtrip(benchmark::State& state) {  // NOLINT(readability-function-cognitive-complexity)
    ASSERT_OK(StartupNetwork());

    std::string path = GenerateString("Path");
    Socket server;
    ASSERT_OK(server.Create(AddressFamily::Uds));
    ASSERT_OK(server.Bind(path));
    ASSERT_OK(server.Listen());

    size_t size = state.range(0);

    stopThread = false;
    std::jthread thread(ReceiveAndSendUds, path, size);

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
        SendExactly(client, buffer.data(), buffer.size());
        ReceiveExactly(client, buffer.data(), buffer.size());
    }

    stopThread = true;
    SendExactly(client, buffer.data(), buffer.size());
}

}  // namespace

BENCHMARK(SocketTcpRoundtrip)->Arg(1)->Arg(100)->Arg(10000)->Arg(1000000);
BENCHMARK(SocketUdsRoundtrip)->Arg(1)->Arg(100)->Arg(10000)->Arg(1000000);
