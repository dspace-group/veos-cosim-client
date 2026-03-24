// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <array>
#include <string>

#include "Helper.hpp"
#include "Logger.hpp"
#include "PerformanceTestClient.hpp"
#include "PerformanceTestHelper.hpp"
#include "Result.hpp"
#include "Socket.hpp"

namespace DsVeosCoSim {

namespace {

[[nodiscard]] Result Run(const std::string& host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    SocketClient client;
    CheckResult(SocketClient::TryConnect(host, TcpSocketPort, 0, 1000, client));

    std::array<char, FrameSize> buffer{};

    connectedEvent.Set();

    while (!isStopped) {
        CheckResult(client.Send(buffer.data(), FrameSize));
        CheckResult(ReceiveComplete(client, buffer.data(), FrameSize));

        counter++;
    }

    return CreateOk();
}

void TcpSocketTest(const std::string& host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    if (!IsOk(Run(host, connectedEvent, counter, isStopped))) {
        LogError("Could not run TCP Socket Client.");
    }
}

}  // namespace

void RunTcpSocketTest(const std::string& host) {  // NOLINT(misc-use-internal-linkage)
    LogTrace("TCP Socket:");
    RunPerformanceTest(TcpSocketTest, host);
    LogTrace("");
}

}  // namespace DsVeosCoSim
