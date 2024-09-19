// Copyright dSPACE GmbH. All rights reserved.

#include <benchmark/benchmark.h>
#include <string>
#include <string_view>
#include <thread>

#include "CoSimTypes.h"
#include "DsVeosCoSim/DsVeosCoSim.h"
#include "Event.h"
#include "Generator.h"
#include "Helper.h"
#include "IoBuffer.h"
#include "SocketChannel.h"

#ifdef _WIN32
#include "LocalChannel.h"
#endif

using namespace DsVeosCoSim;

namespace {

void Receive(const IoSignal& signal, IoBuffer& readerIoBuffer, Channel& channel, bool& stopThread, Event& endEvent) {
    std::vector<uint8_t> readValue = CreateZeroedIoData(signal);

    uint32_t readLength{};

    while (!stopThread) {
        MUST_BE_TRUE(readerIoBuffer.Deserialize(channel.GetReader(), 0, {}));

        readerIoBuffer.Read(signal.id, readLength, readValue.data());

        endEvent.Set();
    }
}

void RunTest(benchmark::State& state,
             ConnectionKind connectionKind,
             std::string_view writerName,
             std::string_view readerName,
             Channel& senderChannel,
             Channel& receiverChannel) {
    IoSignal signal = CreateSignal(DsVeosCoSim_DataType_Int8, DsVeosCoSim_SizeKind_Fixed);
    signal.length = static_cast<uint32_t>(state.range(0));

    IoBuffer writerIoBuffer(CoSimType::Server, connectionKind, writerName, {signal}, {});
    IoBuffer readerIoBuffer(CoSimType::Client, connectionKind, readerName, {signal}, {});

    std::vector<uint8_t> writeValue = GenerateIoData(signal);

    bool stopThread{};
    Event endEvent;

    std::jthread thread(Receive,
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
}

void TcpIo(benchmark::State& state) {
    TcpChannelServer server(0, true);
    uint16_t port = server.GetLocalPort();

    SocketChannel senderChannel = ConnectToTcpChannel("127.0.0.1", port);
    SocketChannel receiverChannel = Accept(server);

    std::string writerName = GenerateString("BenchmarkIoWriter名前");
    std::string readerName = GenerateString("BenchmarkIoReader名前");

    RunTest(state, ConnectionKind::Remote, writerName, readerName, senderChannel, receiverChannel);
}

void UdsIo(benchmark::State& state) {
    std::string serverName = GenerateString("Server");
    UdsChannelServer server(serverName);

    SocketChannel senderChannel = ConnectToUdsChannel(serverName);
    SocketChannel receiverChannel = Accept(server);

    std::string writerName = GenerateString("BenchmarkIoWriter名前");
    std::string readerName = GenerateString("BenchmarkIoReader名前");

    RunTest(state, ConnectionKind::Remote, writerName, readerName, senderChannel, receiverChannel);
}

#ifdef _WIN32
void LocalIo(benchmark::State& state) {
    std::string serverName = GenerateString("Server名前");
    LocalChannelServer server(serverName);

    LocalChannel senderChannel = ConnectToLocalChannel(serverName);
    LocalChannel receiverChannel = Accept(server);

    std::string name = GenerateString("BenchmarkIo名前");

    RunTest(state, ConnectionKind::Local, name, name, senderChannel, receiverChannel);
}
#endif

}  // namespace

BENCHMARK(TcpIo)->Arg(1)->Arg(100)->Arg(10000)->Arg(1000000);
BENCHMARK(UdsIo)->Arg(1)->Arg(100)->Arg(10000)->Arg(1000000);

#ifdef _WIN32
BENCHMARK(LocalIo)->Arg(1)->Arg(100)->Arg(10000)->Arg(1000000);
#endif
