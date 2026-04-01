// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <array>
#include <cstdint>
#include <string>

#include "Channel.hpp"
#include "Logger.hpp"
#include "OsUtilities.hpp"
#include "PerformanceTestClient.hpp"
#include "PerformanceTestHelper.hpp"
#include "Result.hpp"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] Result RunClientLocalChannelInternal([[maybe_unused]] const std::string& host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    std::unique_ptr<Channel> channel;
    CheckResult(TryConnectToLocalChannel(LocalChannelName, channel));

    SetThreadAffinity(LocalChannelName);

    std::array<char, FrameSize> buffer{};

    connectedEvent.Set();

    while (!isStopped) {
        CheckResult(channel->GetWriter().Write(buffer.data(), FrameSize));
        CheckResult(channel->GetWriter().EndWrite());

        CheckResult(channel->GetReader().Read(buffer.data(), FrameSize));

        counter++;
    }

    return CreateOk();
}

void RunClientLocalChannel(const std::string& host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    if (!IsOk(RunClientLocalChannelInternal(host, connectedEvent, counter, isStopped))) {
        LogError("Could not run local channel client.");
    }
}

}  // namespace

void ClientLocalChannel() {  // NOLINT(misc-use-internal-linkage)
    LogTrace("Local Channel:");
    RunPerformanceTest(RunClientLocalChannel, "");
    LogTrace("");
}
