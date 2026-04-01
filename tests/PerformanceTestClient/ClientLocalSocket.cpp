// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <array>
#include <string>

#include "Helper.hpp"
#include "Logger.hpp"
#include "PerformanceTestClient.hpp"
#include "PerformanceTestHelper.hpp"
#include "Result.hpp"
#include "Socket.hpp"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] Result RunClientLocalSocketInternal([[maybe_unused]] const std::string& host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    SocketClient client;
    CheckResult(SocketClient::TryConnect(LocalSocketPath, client));

    std::array<char, FrameSize> buffer{};

    connectedEvent.Set();

    while (!isStopped) {
        CheckResult(client.Send(buffer.data(), FrameSize));
        CheckResult(ReceiveComplete(client, buffer.data(), FrameSize));

        counter++;
    }

    return CreateOk();
}

void RunClientLocalSocket(const std::string& host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    if (!IsOk(RunClientLocalSocketInternal(host, connectedEvent, counter, isStopped))) {
        LogError("Could not run Local Socket Client.");
    }
}

}  // namespace

void ClientLocalSocket() {  // NOLINT(misc-use-internal-linkage)
    LogTrace("Local Socket:");
    RunPerformanceTest(RunClientLocalSocket, "");
    LogTrace("");
}
