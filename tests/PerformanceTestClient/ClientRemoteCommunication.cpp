// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <array>
#include <string>

#include "Channel.hpp"
#include "Helper.hpp"
#include "Logger.hpp"
#include "OsUtilities.hpp"
#include "PerformanceTestClient.hpp"
#include "PerformanceTestHelper.hpp"
#include "Result.hpp"

namespace DsVeosCoSim {

namespace {

[[nodiscard]] Result Run(const std::string& host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    std::unique_ptr<Channel> channel;
    CheckResult(TryConnectToTcpChannel(host, CommunicationPort, 0, DefaultTimeout, channel));

    SetThreadAffinity(std::to_string(CommunicationPort));

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

void RemoteCommunicationClientRun(const std::string& host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    if (!IsOk(Run(host, connectedEvent, counter, isStopped))) {
        LogError("Could not run remote communication client.");
    }
}

}  // namespace

void RunRemoteCommunicationTest(const std::string& host) {  // NOLINT(misc-use-internal-linkage)
    LogTrace("Remote Communication:");
    RunPerformanceTest(RemoteCommunicationClientRun, host);
    LogTrace("");
}

}  // namespace DsVeosCoSim
