// Copyright dSPACE GmbH. All rights reserved.

#include <benchmark/benchmark.h>

#include <string>
#include <thread>

#include "CoSimTypes.h"
#include "Event.h"
#include "Generator.h"
#include "Helper.h"
#include "IoBuffer.h"
#include "SocketChannel.h"

#ifdef _WIN32
#include "LocalChannel.h"
#endif

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
        MUST_BE_TRUE(readerIoBuffer.Deserialize(channel.GetReader(), 0ns, {}));

        readerIoBuffer.Read(signal.id, readLength, readValue.data());

        endEvent.Set();
    }
}

void RunTest(benchmark::State& state,
             const ConnectionKind connectionKind,
             const std::string& writerName,
             const std::string& readerName,
             Channel& senderChannel,
             Channel& receiverChannel) {
    IoSignalContainer signal = CreateSignal(DataType::Int8, SizeKind::Fixed);
    signal.length = static_cast<uint32_t>(state.range(0));

    const IoBuffer writerIoBuffer(CoSimType::Server, connectionKind, writerName, {static_cast<IoSignal>(signal)}, {});
    const IoBuffer readerIoBuffer(CoSimType::Client, connectionKind, readerName, {static_cast<IoSignal>(signal)}, {});

    std::vector<uint8_t> writeValue = GenerateIoData(signal);

    bool stopThread{};
    Event endEvent;

    std::thread thread(Receive,
                       std::ref(signal),
                       std::ref(readerIoBuffer),
                       std::ref(receiverChannel),
                       std::ref(stopThread),
                       std::ref(endEvent));

    for (auto _ : state) {
        writeValue[0]++;
        writerIoBuffer.Write(signal.id, signal.length, writeValue.data());

        MUST_BE_TRUE(writerIoBuffer.Serialize(senderChannel.GetWriter()));
        MUST_BE_TRUE(senderChannel.GetWriter().EndWrite());

        (void)endEvent.Wait(Infinite);
    }

    stopThread = true;

    // The receiver thread is probably in receive function which is blocking. So we wake it up by sending everything one
    // last time

    MUST_BE_TRUE(writerIoBuffer.Serialize(senderChannel.GetWriter()));
    MUST_BE_TRUE(senderChannel.GetWriter().EndWrite());

    thread.join();
}

void TcpIo(benchmark::State& state) {
    TcpChannelServer server(0, true);
    const uint16_t port = server.GetLocalPort();

    SocketChannel senderChannel = ConnectToTcpChannel("127.0.0.1", port);
    SocketChannel receiverChannel = Accept(server);

    const std::string writerName = GenerateString("BenchmarkIoWriter名前");
    const std::string readerName = GenerateString("BenchmarkIoReader名前");

    RunTest(state, ConnectionKind::Remote, writerName, readerName, senderChannel, receiverChannel);
}

void UdsIo(benchmark::State& state) {
    const std::string serverName = GenerateString("Server");
    UdsChannelServer server(serverName);

    SocketChannel senderChannel = ConnectToUdsChannel(serverName);
    SocketChannel receiverChannel = Accept(server);

    const std::string writerName = GenerateString("BenchmarkIoWriter名前");
    const std::string readerName = GenerateString("BenchmarkIoReader名前");

    RunTest(state, ConnectionKind::Remote, writerName, readerName, senderChannel, receiverChannel);
}

#ifdef _WIN32
void LocalIo(benchmark::State& state) {
    const std::string serverName = GenerateString("Server名前");
    LocalChannelServer server(serverName);

    LocalChannel senderChannel = ConnectToLocalChannel(serverName);
    LocalChannel receiverChannel = Accept(server);

    const std::string name = GenerateString("BenchmarkIo名前");

    RunTest(state, ConnectionKind::Local, name, name, senderChannel, receiverChannel);
}
#endif

}  // namespace

BENCHMARK(TcpIo)->Arg(1)->Arg(100)->Arg(10000)->Arg(1000000);
BENCHMARK(UdsIo)->Arg(1)->Arg(100)->Arg(10000)->Arg(1000000);

#ifdef _WIN32
BENCHMARK(LocalIo)->Arg(1)->Arg(100)->Arg(10000)->Arg(1000000);
#endif
