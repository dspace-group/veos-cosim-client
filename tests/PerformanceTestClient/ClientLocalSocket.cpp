// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "PerformanceTestClient.hpp"

#include <array>
#include <string>

#include "Error.hpp"
#include "Helper.hpp"
#include "PerformanceTestHelper.hpp"
#include "Socket.hpp"

namespace DsVeosCoSim {

namespace {

[[nodiscard]] Result Run([[maybe_unused]] const std::string& host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
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

void LocalSocketTest(const std::string& host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    if (!IsOk(Run(host, connectedEvent, counter, isStopped))) {
        LogError("Could not run Local Socket Client.");
    }
}

}  // namespace

void RunLocalSocketTest() {  // NOLINT(misc-use-internal-linkage)
    LogTrace("Local Socket:");
    RunPerformanceTest(LocalSocketTest, "");
    LogTrace("");
}

}  // namespace DsVeosCoSim
