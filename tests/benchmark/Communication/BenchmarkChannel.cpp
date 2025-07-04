// Copyright dSPACE GmbH. All rights reserved.

#include <benchmark/benchmark.h>

#include <string>
#include <thread>
#include <vector>

#include "Channel.h"
#include "Generator.h"
#include "Helper.h"

using namespace DsVeosCoSim;

namespace {

void CounterPart(Channel& channel, const bool& stopThread, size_t size) {
    std::vector<uint8_t> buffer;
    buffer.resize(size);

    MustBeOk(channel.GetReader().Read(buffer.data(), buffer.size()));

    while (!stopThread) {
        MustBeOk(channel.GetWriter().Write(buffer.data(), buffer.size()));
        MustBeOk(channel.GetWriter().EndWrite());
        MustBeOk(channel.GetReader().Read(buffer.data(), buffer.size()));
    }
}

void RunTest(benchmark::State& state, Channel& channel1, Channel& channel2) {
    auto size = static_cast<size_t>(state.range(0));

    bool stopThread{};
    std::thread thread(CounterPart, std::ref(channel1), std::ref(stopThread), size);

    std::vector<uint8_t> buffer;
    buffer.resize(size);

    for (auto _ : state) {
        MustBeOk(channel2.GetWriter().Write(buffer.data(), buffer.size()));
        MustBeOk(channel2.GetWriter().EndWrite());
        MustBeOk(channel2.GetReader().Read(buffer.data(), buffer.size()));
    }

    stopThread = true;
    MustBeOk(channel2.GetWriter().Write(buffer.data(), buffer.size()));
    MustBeOk(channel2.GetWriter().EndWrite());

    thread.join();
}

void TcpChannelRoundtrip(benchmark::State& state) {
    std::unique_ptr<ChannelServer> server;
    MustBeOk(CreateTcpChannelServer(0, false, server));
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectedChannel;
    MustBeOk(TryConnectToTcpChannel("127.0.0.1", port, 0, DefaultTimeout, connectedChannel));
    MustBeTrue(connectedChannel);
    std::unique_ptr<Channel> acceptedChannel;
    MustBeOk(server->TryAccept(acceptedChannel));

    RunTest(state, std::ref(*acceptedChannel), std::ref(*connectedChannel));
}

void UdsChannelRoundtrip(benchmark::State& state) {
    std::string serverName = GenerateString("Server");

    std::unique_ptr<ChannelServer> server;
    MustBeOk(CreateUdsChannelServer(serverName, server));

    std::unique_ptr<Channel> connectedChannel;
    MustBeOk(TryConnectToUdsChannel(serverName, connectedChannel));
    MustBeTrue(connectedChannel);
    std::unique_ptr<Channel> acceptedChannel;
    MustBeOk(server->TryAccept(acceptedChannel));

    RunTest(state, std::ref(*acceptedChannel), std::ref(*connectedChannel));
}

#ifdef _WIN32
void LocalChannelRoundtrip(benchmark::State& state) {
    std::string serverName = GenerateString("Server名前");
    SetEnvVariable("VEOS_COSIM_SPIN_COUNT", "1280");

    std::unique_ptr<ChannelServer> server;
    MustBeOk(CreateLocalChannelServer(serverName, server));

    std::unique_ptr<Channel> connectedChannel;
    MustBeOk(TryConnectToLocalChannel(serverName, connectedChannel));
    MustBeTrue(connectedChannel);
    std::unique_ptr<Channel> acceptedChannel;
    MustBeOk(server->TryAccept(acceptedChannel));

    RunTest(state, std::ref(*acceptedChannel), std::ref(*connectedChannel));
}
#endif

}  // namespace

BENCHMARK(TcpChannelRoundtrip)->Arg(1)->Arg(100)->Arg(10000)->Arg(1000000)->Arg(100000000);
BENCHMARK(UdsChannelRoundtrip)->Arg(1)->Arg(100)->Arg(10000)->Arg(1000000)->Arg(100000000);

#ifdef _WIN32
BENCHMARK(LocalChannelRoundtrip)->Arg(1)->Arg(100)->Arg(10000)->Arg(1000000)->Arg(100000000);
#endif
