// Copyright dSPACE GmbH. All rights reserved.

#include <benchmark/benchmark.h>

#include <chrono>  // IWYU pragma: keep
#include <string>
#include <thread>

#include "BusBuffer.h"
#include "Event.h"
#include "Generator.h"
#include "Helper.h"
#include "SocketChannel.h"

#ifdef _WIN32
#include "LocalChannel.h"
#endif

using namespace std::chrono;
using namespace DsVeosCoSim;

namespace {

using CanTypes = std::tuple<CanControllerContainer, CanController, CanMessageContainer, CanMessage>;
using EthTypes = std::tuple<EthControllerContainer, EthController, EthMessageContainer, EthMessage>;
using LinTypes = std::tuple<LinControllerContainer, LinController, LinMessageContainer, LinMessage>;

template <typename TypeParam>
void ReceiveMessages(size_t count, BusBuffer& receiverBusBuffer, Channel& channel, bool& stopThread, Event& endEvent) {
    using TMessageExtern = std::tuple_element_t<3, TypeParam>;

    while (!stopThread) {
        TMessageExtern receiveMessage{};
        MUST_BE_TRUE(receiverBusBuffer.Deserialize(channel.GetReader(), 0ns, {}));

        for (size_t i = 0; i < count; i++) {
            MUST_BE_TRUE(receiverBusBuffer.Receive(receiveMessage));
        }

        endEvent.Set();
    }
}

template <typename TypeParam>
void RunTest(benchmark::State& state,
             ConnectionKind connectionKind,
             const std::string& senderName,
             const std::string& receiverName,
             Channel& senderChannel,
             Channel& receiverChannel) {
    using TController = std::tuple_element_t<0, TypeParam>;
    using TControllerExtern = std::tuple_element_t<1, TypeParam>;
    using TMessage = std::tuple_element_t<2, TypeParam>;
    using TMessageExtern = std::tuple_element_t<3, TypeParam>;

    TController controller{};
    FillWithRandom(controller);

    BusBuffer transmitterBusBuffer(CoSimType::Server,
                                   connectionKind,
                                   senderName,
                                   {static_cast<TControllerExtern>(controller)});
    BusBuffer receiverBusBuffer(CoSimType::Client,
                                connectionKind,
                                receiverName,
                                {static_cast<TControllerExtern>(controller)});

    const size_t count = state.range(0);

    bool stopThread{};
    Event endEvent;

    std::thread thread(ReceiveMessages<TypeParam>,
                       count,
                       std::ref(receiverBusBuffer),
                       std::ref(receiverChannel),
                       std::ref(stopThread),
                       std::ref(endEvent));

    TMessage sendMessage{};
    FillWithRandom(sendMessage, controller.id);
    sendMessage.length = static_cast<uint32_t>(sendMessage.data.size());

    for (auto _ : state) {
        for (size_t i = 0; i < count; i++) {
            MUST_BE_TRUE(transmitterBusBuffer.Transmit(static_cast<TMessageExtern>(sendMessage)));
        }

        MUST_BE_TRUE(transmitterBusBuffer.Serialize(senderChannel.GetWriter()));
        MUST_BE_TRUE(senderChannel.GetWriter().EndWrite());

        (void)endEvent.Wait(Infinite);
    }

    stopThread = true;

    // The receiver thread is probably in receive function which is blocking. So we wake it up by sending everything one
    // last time
    for (size_t i = 0; i < count; i++) {
        MUST_BE_TRUE(transmitterBusBuffer.Transmit(static_cast<TMessageExtern>(sendMessage)));
    }

    MUST_BE_TRUE(transmitterBusBuffer.Serialize(senderChannel.GetWriter()));
    MUST_BE_TRUE(senderChannel.GetWriter().EndWrite());

    thread.join();
}

template <typename TypeParam>
void TcpMessages(benchmark::State& state) {
    TcpChannelServer server(0, true);
    uint16_t port = server.GetLocalPort();

    SocketChannel senderChannel = ConnectToTcpChannel("127.0.0.1", port);
    SocketChannel receiverChannel = Accept(server);

    std::string senderName = GenerateString("BenchmarkBusSender名前");
    std::string receiverName = GenerateString("BenchmarkBusReceiver名前");

    RunTest<TypeParam>(state, ConnectionKind::Remote, senderName, receiverName, senderChannel, receiverChannel);
}

template <typename TypeParam>
void UdsMessages(benchmark::State& state) {
    std::string serverName = GenerateString("BusMessages");

    UdsChannelServer server(serverName);

    SocketChannel senderChannel = ConnectToUdsChannel(serverName);
    SocketChannel receiverChannel = Accept(server);

    std::string senderName = GenerateString("BenchmarkBusSender名前");
    std::string receiverName = GenerateString("BenchmarkBusReceiver名前");

    RunTest<TypeParam>(state, ConnectionKind::Remote, senderName, receiverName, senderChannel, receiverChannel);
}

#ifdef _WIN32
template <typename TypeParam>
void LocalMessages(benchmark::State& state) {
    std::string serverName = GenerateString("Server名前");

    LocalChannelServer server(serverName);

    LocalChannel senderChannel = ConnectToLocalChannel(serverName);
    LocalChannel receiverChannel = Accept(server);

    std::string name = GenerateString("BenchmarkBus名前");

    RunTest<TypeParam>(state, ConnectionKind::Local, name, name, senderChannel, receiverChannel);
}
#endif

}  // namespace

BENCHMARK_TEMPLATE(TcpMessages, CanTypes)->Arg(1)->Arg(10)->Arg(100);
BENCHMARK_TEMPLATE(TcpMessages, EthTypes)->Arg(1)->Arg(10)->Arg(100);
BENCHMARK_TEMPLATE(TcpMessages, LinTypes)->Arg(1)->Arg(10)->Arg(100);
BENCHMARK_TEMPLATE(UdsMessages, CanTypes)->Arg(1)->Arg(10)->Arg(100);
BENCHMARK_TEMPLATE(UdsMessages, EthTypes)->Arg(1)->Arg(10)->Arg(100);
BENCHMARK_TEMPLATE(UdsMessages, LinTypes)->Arg(1)->Arg(10)->Arg(100);
#ifdef _WIN32
BENCHMARK_TEMPLATE(LocalMessages, CanTypes)->Arg(1)->Arg(10)->Arg(100);
BENCHMARK_TEMPLATE(LocalMessages, EthTypes)->Arg(1)->Arg(10)->Arg(100);
BENCHMARK_TEMPLATE(LocalMessages, LinTypes)->Arg(1)->Arg(10)->Arg(100);
#endif
