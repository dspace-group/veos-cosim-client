// Copyright dSPACE GmbH. All rights reserved.

#ifdef ALL_BENCHMARK_TESTS

#include <benchmark/benchmark.h>

#include <chrono>  // IWYU pragma: keep
#include <string>
#include <thread>

#include "BusBuffer.h"
#include "Channel.h"
#include "Event.h"
#include "Helper.h"
#include "OsUtilities.h"

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
        MustBeOk(receiverBusBuffer.Deserialize(channel.GetReader(), 0ns, {}));

        for (size_t i = 0; i < count; i++) {
            MustBeOk(receiverBusBuffer.Receive(receiveMessage));
        }

        endEvent.Set();
    }
}

template <typename TypeParam>
void RunTest(benchmark::State& state,
             ConnectionKind connectionKind,
             std::string_view senderName,
             std::string_view receiverName,
             Channel& senderChannel,
             Channel& receiverChannel) {
    using TController = std::tuple_element_t<0, TypeParam>;
    using TMessageContainer = std::tuple_element_t<2, TypeParam>;

    TController controller{};
    FillWithRandom(controller);

    std::unique_ptr<BusBuffer> transmitterBusBuffer;
    MustBeOk(
        CreateBusBuffer(CoSimType::Server, connectionKind, senderName, {controller.Convert()}, transmitterBusBuffer));
    std::unique_ptr<BusBuffer> receiverBusBuffer;
    MustBeOk(
        CreateBusBuffer(CoSimType::Client, connectionKind, receiverName, {controller.Convert()}, receiverBusBuffer));

    auto count = static_cast<size_t>(state.range(0));

    bool stopThread{};
    Event endEvent;

    std::thread thread(ReceiveMessages<TypeParam>,
                       count,
                       std::ref(*receiverBusBuffer),
                       std::ref(receiverChannel),
                       std::ref(stopThread),
                       std::ref(endEvent));

    TMessageContainer sendMessage{};
    FillWithRandom(sendMessage, controller.id);

    for (auto _ : state) {
        for (size_t i = 0; i < count; i++) {
            MustBeOk(transmitterBusBuffer->Transmit(sendMessage));
        }

        MustBeOk(transmitterBusBuffer->Serialize(senderChannel.GetWriter()));
        MustBeOk(senderChannel.GetWriter().EndWrite());

        MustBeTrue(endEvent.Wait(Infinite));
    }

    stopThread = true;

    // The receiver thread is probably in receive function which is blocking. So we wake it up by sending everything one
    // last time
    for (size_t i = 0; i < count; i++) {
        MustBeOk(transmitterBusBuffer->Transmit(sendMessage));
    }

    MustBeOk(transmitterBusBuffer->Serialize(senderChannel.GetWriter()));
    MustBeOk(senderChannel.GetWriter().EndWrite());

    thread.join();
}

template <typename TypeParam>
void TcpMessages(benchmark::State& state) {
    std::unique_ptr<ChannelServer> server;
    MustBeOk(CreateTcpChannelServer(0, false, server));
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectedChannel;
    MustBeOk(TryConnectToTcpChannel("127.0.0.1", port, 0, DefaultTimeout, connectedChannel));
    MustBeTrue(connectedChannel);
    std::unique_ptr<Channel> acceptedChannel;
    MustBeOk(server->TryAccept(acceptedChannel));

    std::string senderName = GenerateString("BenchmarkBusSender名前");
    std::string receiverName = GenerateString("BenchmarkBusReceiver名前");

    RunTest<TypeParam>(state, ConnectionKind::Remote, senderName, receiverName, *connectedChannel, *acceptedChannel);
}

template <typename TypeParam>
void UdsMessages(benchmark::State& state) {
    std::string serverName = GenerateString("Server");

    std::unique_ptr<ChannelServer> server;
    MustBeOk(CreateUdsChannelServer(serverName, server));

    std::unique_ptr<Channel> connectedChannel;
    MustBeOk(TryConnectToUdsChannel(serverName, connectedChannel));
    MustBeTrue(connectedChannel);
    std::unique_ptr<Channel> acceptedChannel;
    MustBeOk(server->TryAccept(acceptedChannel));

    std::string senderName = GenerateString("BenchmarkBusSender名前");
    std::string receiverName = GenerateString("BenchmarkBusReceiver名前");

    RunTest<TypeParam>(state, ConnectionKind::Remote, senderName, receiverName, *connectedChannel, *acceptedChannel);
}

#ifdef _WIN32
template <typename TypeParam>
void LocalMessages(benchmark::State& state) {
    std::string serverName = GenerateString("Server名前");

    std::unique_ptr<ChannelServer> server;
    MustBeOk(CreateLocalChannelServer(serverName, server));

    std::unique_ptr<Channel> connectedChannel;
    MustBeOk(TryConnectToLocalChannel(serverName, connectedChannel));
    MustBeTrue(connectedChannel);
    std::unique_ptr<Channel> acceptedChannel;
    MustBeOk(server->TryAccept(acceptedChannel));

    std::string name = GenerateString("BenchmarkBus名前");

    RunTest<TypeParam>(state, ConnectionKind::Local, name, name, *connectedChannel, *acceptedChannel);
}

template <typename TypeParam>
void RemoteOnLocalMessages(benchmark::State& state) {
    std::string serverName = GenerateString("Server名前");

    std::unique_ptr<ChannelServer> server;
    MustBeOk(CreateLocalChannelServer(serverName, server));

    std::unique_ptr<Channel> connectedChannel;
    MustBeOk(TryConnectToLocalChannel(serverName, connectedChannel));
    MustBeTrue(connectedChannel);
    std::unique_ptr<Channel> acceptedChannel;
    MustBeOk(server->TryAccept(acceptedChannel));

    std::string senderName = GenerateString("BenchmarkBusSender名前");
    std::string receiverName = GenerateString("BenchmarkBusReceiver名前");

    RunTest<TypeParam>(state, ConnectionKind::Remote, senderName, receiverName, *connectedChannel, *acceptedChannel);
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
BENCHMARK_TEMPLATE(RemoteOnLocalMessages, CanTypes)->Arg(1)->Arg(10)->Arg(100);
BENCHMARK_TEMPLATE(RemoteOnLocalMessages, EthTypes)->Arg(1)->Arg(10)->Arg(100);
BENCHMARK_TEMPLATE(RemoteOnLocalMessages, LinTypes)->Arg(1)->Arg(10)->Arg(100);
#endif

#endif
