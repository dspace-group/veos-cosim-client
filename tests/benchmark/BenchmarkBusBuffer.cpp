// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <benchmark/benchmark.h>

#include <chrono>  // IWYU pragma: keep
#include <string>
#include <thread>

#include "BusBuffer.hpp"
#include "Channel.hpp"
#include "Event.hpp"
#include "Helper.hpp"
#include "OsUtilities.hpp"

using namespace std::chrono;
using namespace DsVeosCoSim;

namespace {

struct CanBus {
    using Message = CanMessage;
    using MessageContainer = CanMessageContainer;
    using ControllerContainer = CanControllerContainer;
    static constexpr uint32_t MessageMaxLength = CanMessageMaxLength;
};

struct EthBus {
    using Message = EthMessage;
    using MessageContainer = EthMessageContainer;
    using ControllerContainer = EthControllerContainer;
    static constexpr uint32_t MessageMaxLength = EthMessageMaxLength;
};

struct LinBus {
    using Message = LinMessage;
    using MessageContainer = LinMessageContainer;
    using ControllerContainer = LinControllerContainer;
    static constexpr uint32_t MessageMaxLength = LinMessageMaxLength;
};

struct FrBus {
    using Message = FrMessage;
    using MessageContainer = FrMessageContainer;
    using ControllerContainer = FrControllerContainer;
    static constexpr uint32_t MessageMaxLength = FrMessageMaxLength;
};

template <typename TBus>
void ReceiveMessages(size_t count, BusBuffer& receiverBusBuffer, Channel& channel, bool& stopThread, Event& endEvent) {
    using TMessage = typename TBus::Message;

    while (!stopThread) {
        TMessage receiveMessage{};
        MustBeOk(receiverBusBuffer.Deserialize(channel.GetReader(), SimulationTime{}, {}));

        for (size_t i = 0; i < count; i++) {
            MustBeOk(receiverBusBuffer.Receive(receiveMessage));
        }

        endEvent.Set();
    }
}

template <typename TBus>
void RunTest(benchmark::State& state,
             ConnectionKind connectionKind,
             const std::string& writerName,
             const std::string& readerName,
             Channel& senderChannel,
             Channel& receiverChannel) {
    using TControllerContainer = typename TBus::ControllerContainer;
    using TMessageContainer = typename TBus::MessageContainer;

    std::unique_ptr<IProtocol> protocol;
    MustBeOk(CreateProtocol(ProtocolVersionLatest, protocol));

    TControllerContainer controller{};
    FillWithRandom(controller);

    std::unique_ptr<BusBuffer> transmitterBusBuffer;
    MustBeOk(CreateBusBuffer(CoSimType::Server, connectionKind, writerName, {controller.Convert()}, *protocol, transmitterBusBuffer));
    std::unique_ptr<BusBuffer> receiverBusBuffer;
    MustBeOk(CreateBusBuffer(CoSimType::Client, connectionKind, readerName, {controller.Convert()}, *protocol, receiverBusBuffer));

    auto count = static_cast<size_t>(state.range(0));

    bool stopThread{};
    Event endEvent;

    std::thread thread(ReceiveMessages<TBus>, count, std::ref(*receiverBusBuffer), std::ref(receiverChannel), std::ref(stopThread), std::ref(endEvent));

    TMessageContainer sendMessage{};
    FillWithRandom(sendMessage, controller.id);
    sendMessage.length = TBus::MessageMaxLength;

    for (auto _ : state) {
        for (size_t i = 0; i < count; i++) {
            MustBeOk(transmitterBusBuffer->Transmit(sendMessage));
        }

        MustBeOk(transmitterBusBuffer->Serialize(senderChannel.GetWriter()));
        MustBeOk(senderChannel.GetWriter().EndWrite());

        (void)endEvent.Wait(Infinite);
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

template <typename TBus>
void TcpMessages(benchmark::State& state) {
    std::unique_ptr<ChannelServer> server;
    MustBeOk(CreateTcpChannelServer(0, false, server));
    uint16_t port = server->GetLocalPort();

    std::unique_ptr<Channel> connectedChannel;
    MustBeOk(TryConnectToTcpChannel("127.0.0.1", port, 0, DefaultTimeout, connectedChannel));
    std::unique_ptr<Channel> acceptedChannel;
    MustBeOk(server->TryAccept(acceptedChannel));

    std::string writerName = GenerateString("BenchmarkBusWriter名前");
    std::string readerName = GenerateString("BenchmarkBusReader名前");

    RunTest<TBus>(state, ConnectionKind::Remote, writerName, readerName, *connectedChannel, *acceptedChannel);
}

template <typename TBus>
void LocalMessages(benchmark::State& state) {
    std::string serverName = GenerateString("Server");

    std::unique_ptr<ChannelServer> server;
    MustBeOk(CreateLocalChannelServer(serverName, server));

    std::unique_ptr<Channel> connectedChannel;
    MustBeOk(TryConnectToLocalChannel(serverName, connectedChannel));
    std::unique_ptr<Channel> acceptedChannel;
    MustBeOk(server->TryAccept(acceptedChannel));

    std::string writerName = GenerateString("BenchmarkBusWriter名前");
#ifdef _WIN32
    std::string readerName = writerName;  // NOLINT(performance-unnecessary-copy-initialization)
#else
    std::string readerName = GenerateString("BenchmarkBusReader名前");
#endif

    RunTest<TBus>(state, ConnectionKind::Local, writerName, readerName, *connectedChannel, *acceptedChannel);
}

template <typename TBus>
void LocalOnChannelMessages(benchmark::State& state) {
    std::string serverName = GenerateString("Server");

    std::unique_ptr<ChannelServer> server;
    MustBeOk(CreateLocalChannelServer(serverName, server));

    std::unique_ptr<Channel> connectedChannel;
    MustBeOk(TryConnectToLocalChannel(serverName, connectedChannel));
    std::unique_ptr<Channel> acceptedChannel;
    MustBeOk(server->TryAccept(acceptedChannel));

    std::string writerName = GenerateString("BenchmarkBusWriter名前");
    std::string readerName = GenerateString("BenchmarkBusReader名前");

    RunTest<TBus>(state, ConnectionKind::Remote, writerName, readerName, *connectedChannel, *acceptedChannel);
}

}  // namespace

BENCHMARK_TEMPLATE(TcpMessages, CanBus)->Arg(1)->Arg(10)->Arg(100)->Arg(1000);
BENCHMARK_TEMPLATE(TcpMessages, EthBus)->Arg(1)->Arg(10)->Arg(100)->Arg(1000);
BENCHMARK_TEMPLATE(TcpMessages, LinBus)->Arg(1)->Arg(10)->Arg(100)->Arg(1000);
BENCHMARK_TEMPLATE(TcpMessages, FrBus)->Arg(1)->Arg(10)->Arg(100)->Arg(1000);
BENCHMARK_TEMPLATE(LocalMessages, CanBus)->Arg(1)->Arg(10)->Arg(100)->Arg(1000);
BENCHMARK_TEMPLATE(LocalMessages, EthBus)->Arg(1)->Arg(10)->Arg(100)->Arg(1000);
BENCHMARK_TEMPLATE(LocalMessages, LinBus)->Arg(1)->Arg(10)->Arg(100)->Arg(1000);
BENCHMARK_TEMPLATE(LocalMessages, FrBus)->Arg(1)->Arg(10)->Arg(100)->Arg(1000);
BENCHMARK_TEMPLATE(LocalOnChannelMessages, CanBus)->Arg(1)->Arg(10)->Arg(100)->Arg(1000);
BENCHMARK_TEMPLATE(LocalOnChannelMessages, EthBus)->Arg(1)->Arg(10)->Arg(100)->Arg(1000);
BENCHMARK_TEMPLATE(LocalOnChannelMessages, LinBus)->Arg(1)->Arg(10)->Arg(100)->Arg(1000);
BENCHMARK_TEMPLATE(LocalOnChannelMessages, FrBus)->Arg(1)->Arg(10)->Arg(100)->Arg(1000);
