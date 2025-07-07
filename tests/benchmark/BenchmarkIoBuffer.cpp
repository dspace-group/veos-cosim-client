// Copyright dSPACE GmbH. All rights reserved.

#ifdef ALL_BENCHMARK_TESTS

#include <benchmark/benchmark.h>

#include <string>
#include <thread>

#include "Event.h"
#include "Helper.h"
#include "IoBuffer.h"
#include "OsUtilities.h"

using namespace std::chrono;
using namespace DsVeosCoSim;

namespace {

void Receive(const IoSignalContainer& signal,
             const IoBuffer& readerIoBuffer,
             Channel& channel,
             const bool& stopThread,
             Event& endEvent) {
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
             std::string_view writerName,
             std::string_view readerName,
             Channel& senderChannel,
             Channel& receiverChannel) {
    IoSignalContainer signal = CreateSignal(DataType::Int8, SizeKind::Fixed);
    signal.length = static_cast<uint32_t>(state.range(0));

    std::unique_ptr<IoBuffer> writerIoBuffer;
    MustBeOk(CreateIoBuffer(CoSimType::Server, connectionKind, writerName, {signal.Convert()}, {}, writerIoBuffer));
    std::unique_ptr<IoBuffer> readerIoBuffer;
    MustBeOk(CreateIoBuffer(CoSimType::Client, connectionKind, readerName, {signal.Convert()}, {}, readerIoBuffer));

    std::vector<uint8_t> writeValue = GenerateIoData(signal);

    bool stopThread{};
    Event endEvent;

    std::thread thread(Receive,
                       std::ref(signal),
                       std::ref(*readerIoBuffer),
                       std::ref(receiverChannel),
                       std::ref(stopThread),
                       std::ref(endEvent));

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

void UdsIo(benchmark::State& state) {
    std::string serverName = GenerateString("Server");

    std::unique_ptr<ChannelServer> server;
    MustBeOk(CreateUdsChannelServer(serverName, server));

    std::unique_ptr<Channel> connectedChannel;
    MustBeOk(TryConnectToUdsChannel(serverName, connectedChannel));
    MustBeTrue(connectedChannel);
    std::unique_ptr<Channel> acceptedChannel;
    MustBeOk(server->TryAccept(acceptedChannel));

    std::string writerName = GenerateString("BenchmarkIoWriter名前");
    std::string readerName = GenerateString("BenchmarkIoReader名前");

    RunTest(state, ConnectionKind::Remote, writerName, readerName, *connectedChannel, *acceptedChannel);
}

#ifdef _WIN32
void LocalIo(benchmark::State& state) {
    std::string serverName = GenerateString("Server名前");

    std::unique_ptr<ChannelServer> server;
    MustBeOk(CreateLocalChannelServer(serverName, server));

    std::unique_ptr<Channel> connectedChannel;
    MustBeOk(TryConnectToLocalChannel(serverName, connectedChannel));
    MustBeTrue(connectedChannel);
    std::unique_ptr<Channel> acceptedChannel;
    MustBeOk(server->TryAccept(acceptedChannel));

    std::string name = GenerateString("BenchmarkIo名前");

    RunTest(state, ConnectionKind::Local, name, name, *connectedChannel, *acceptedChannel);
}
#endif

}  // namespace

BENCHMARK(TcpIo)->Arg(1)->Arg(100)->Arg(10000)->Arg(1000000);
BENCHMARK(UdsIo)->Arg(1)->Arg(100)->Arg(10000)->Arg(1000000);

#ifdef _WIN32
BENCHMARK(LocalIo)->Arg(1)->Arg(100)->Arg(10000)->Arg(1000000);
#endif

#endif
