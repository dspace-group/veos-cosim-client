// Copyright dSPACE GmbH. All rights reserved.

#include <benchmark/benchmark.h>
#include <string>
#include <thread>
#include <vector>

#include "Generator.h"
#include "Helper.h"
#include "SocketChannel.h"

#ifdef _WIN32
#include "LocalChannel.h"
#endif

using namespace DsVeosCoSim;

namespace {

void CounterPart(Channel& channel, bool& stopThread, size_t size) {
    std::vector<uint8_t> buffer;
    buffer.resize(size);

    MUST_BE_TRUE(channel.GetReader().Read(buffer.data(), buffer.size()));

    while (!stopThread) {
        MUST_BE_TRUE(channel.GetWriter().Write(buffer.data(), buffer.size()));
        MUST_BE_TRUE(channel.GetWriter().EndWrite());
        MUST_BE_TRUE(channel.GetReader().Read(buffer.data(), buffer.size()));
    }
}

void RunTest(benchmark::State& state, Channel& channel1, Channel& channel2) {
    size_t size = state.range(0);

    bool stopThread{};
    std::jthread thread(CounterPart, std::ref(channel1), std::ref(stopThread), size);

    std::vector<uint8_t> buffer;
    buffer.resize(size);

    for (auto _ : state) {  // NOLINT(readability-identifier-length)
        MUST_BE_TRUE(channel2.GetWriter().Write(buffer.data(), buffer.size()));
        MUST_BE_TRUE(channel2.GetWriter().EndWrite());
        MUST_BE_TRUE(channel2.GetReader().Read(buffer.data(), buffer.size()));
    }

    stopThread = true;
    MUST_BE_TRUE(channel2.GetWriter().Write(buffer.data(), buffer.size()));
    MUST_BE_TRUE(channel2.GetWriter().EndWrite());
}

void TcpChannelRoundtrip(benchmark::State& state) {
    TcpChannelServer server(0, true);
    uint16_t port = server.GetLocalPort();

    SocketChannel connectedChannel = ConnectToTcpChannel("127.0.0.1", port);
    SocketChannel acceptedChannel = Accept(server);

    RunTest(state, std::ref(acceptedChannel), std::ref(connectedChannel));
}

void UdsChannelRoundtrip(benchmark::State& state) {
    std::string serverName = GenerateString("Server");

    UdsChannelServer server(serverName);

    SocketChannel connectedChannel = ConnectToUdsChannel(serverName);
    SocketChannel acceptedChannel = Accept(server);

    RunTest(state, std::ref(acceptedChannel), std::ref(connectedChannel));
}

#ifdef _WIN32
void LocalChannelRoundtrip(benchmark::State& state) {
    std::string serverName = GenerateString("Server名前");
    LocalChannelServer server(serverName);

    LocalChannel connectedChannel = ConnectToLocalChannel(serverName);
    LocalChannel acceptedChannel = Accept(server);

    RunTest(state, std::ref(acceptedChannel), std::ref(connectedChannel));
}
#endif

}  // namespace

BENCHMARK(TcpChannelRoundtrip)->Arg(1)->Arg(100)->Arg(10000)->Arg(1000000);
BENCHMARK(UdsChannelRoundtrip)->Arg(1)->Arg(100)->Arg(10000)->Arg(1000000);

#ifdef _WIN32
BENCHMARK(LocalChannelRoundtrip)->Arg(1)->Arg(100)->Arg(10000)->Arg(1000000);
#endif
