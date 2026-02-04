// Copyright dSPACE SE & Co. KG. All rights reserved.

#ifdef ALL_BENCHMARK_TESTS

#include <benchmark/benchmark.h>

#include <memory>
#include <string>
#include <thread>

#include "Event.h"
#include "Helper.h"
#include "IoBuffer.h"
#include "OsUtilities.h"
#include "Protocol.h"

using namespace std::chrono;
using namespace DsVeosCoSim;

namespace {

void Receive(const IoSignalContainer& signal, const IoBuffer& readerIoBuffer, Channel& channel, const bool& stopThread, Event& endEvent) {
    std::vector<uint8_t> readValue = CreateZeroedIoData(signal);

    uint32_t readLength{};

    while (!stopThread) {
        MustBeOk(readerIoBuffer.Deserialize(channel.GetReader(), 0ns, {}));
        MustBeOk(readerIoBuffer.Read(signal.id, readLength, readValue.data()));
        endEvent.Set();
    }
}

void RunTest(benchmark::State& state,
             ConnectionKind connectionKind,
             const std::string& writerName,
             const std::string& readerName,
             Channel& senderChannel,
             Channel& receiverChannel) {
    IoSignalContainer signal = CreateSignal(DataType::Int8, SizeKind::Fixed);
    signal.length = static_cast<uint32_t>(state.range(0));

    std::shared_ptr<IProtocol> protocol;
    FactoryResult result = MakeProtocol(DsVeosCoSim::LATEST_VERSION);
    if (result.error == FactoryError::None && result.protocol) {
        protocol = std::move(result.protocol);
    }

    std::unique_ptr<IoBuffer> writerIoBuffer;
    MustBeOk(CreateIoBuffer(CoSimType::Server, connectionKind, writerName, {signal.Convert()}, {}, protocol, writerIoBuffer));
    std::unique_ptr<IoBuffer> readerIoBuffer;
    MustBeOk(CreateIoBuffer(CoSimType::Client, connectionKind, readerName, {signal.Convert()}, {}, protocol, readerIoBuffer));

    std::vector<uint8_t> writeValue = GenerateIoData(signal);

    bool stopThread{};
    Event endEvent;

    std::thread thread(Receive, std::ref(signal), std::ref(*readerIoBuffer), std::ref(receiverChannel), std::ref(stopThread), std::ref(endEvent));

    for (auto _ : state) {
        writeValue[0]++;
        MustBeOk(writerIoBuffer->Write(signal.id, signal.length, writeValue.data()));

        MustBeOk(writerIoBuffer->Serialize(senderChannel.GetWriter()));
        MustBeOk(senderChannel.GetWriter().EndWrite());

        (void)endEvent.Wait(Infinite);
    }

    stopThread = true;

    // The receiver thread is probably in receive function which is blocking. So we wake it up by sending everything one
    // last time

    MustBeOk(writerIoBuffer->Serialize(senderChannel.GetWriter()));
    MustBeOk(senderChannel.GetWriter().EndWrite());

    thread.join();
}

void TcpIo(benchmark::State& state) {
    std::unique_ptr<ChannelServer> server;
    MustBeOk(CreateTcpChannelServer(0, false, server));
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectedChannel;
    MustBeOk(TryConnectToTcpChannel("127.0.0.1", port, 0, DefaultTimeout, connectedChannel));
    MustBeTrue(connectedChannel);
    std::unique_ptr<Channel> acceptedChannel;
    MustBeOk(server->TryAccept(acceptedChannel));

    std::string writerName = GenerateString("BenchmarkIoWriter名前");
    std::string readerName = GenerateString("BenchmarkIoReader名前");

    RunTest(state, ConnectionKind::Remote, writerName, readerName, *connectedChannel, *acceptedChannel);
}

void LocalIo(benchmark::State& state) {
    std::string serverName = GenerateString("Server名前");

    std::unique_ptr<ChannelServer> server;
    MustBeOk(CreateLocalChannelServer(serverName, server));

    std::unique_ptr<Channel> connectedChannel;
    MustBeOk(TryConnectToLocalChannel(serverName, connectedChannel));
    MustBeTrue(connectedChannel);
    std::unique_ptr<Channel> acceptedChannel;
    MustBeOk(server->TryAccept(acceptedChannel));

    std::string writerName = GenerateString("BenchmarkIoWriter名前");
#ifdef _WIN32
    std::string readerName = writerName;  // NOLINT(performance-unnecessary-copy-initialization)
#else
    std::string readerName = GenerateString("BenchmarkIoReader名前");
#endif

    RunTest(state, ConnectionKind::Local, writerName, readerName, *connectedChannel, *acceptedChannel);
}

}  // namespace

BENCHMARK(TcpIo)->Arg(1)->Arg(100)->Arg(10000)->Arg(1000000);
BENCHMARK(LocalIo)->Arg(1)->Arg(100)->Arg(10000)->Arg(1000000);

#endif
