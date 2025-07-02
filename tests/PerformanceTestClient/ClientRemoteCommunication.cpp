// Copyright dSPACE GmbH. All rights reserved.

#include <array>
#include <string_view>

#include "Channel.h"
#include "CoSimHelper.h"
#include "Helper.h"
#include "OsUtilities.h"
#include "PerformanceTestClient.h"
#include "PerformanceTestHelper.h"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] Result Run(std::string_view host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    std::unique_ptr<Channel> channel;
    CheckResult(TryConnectToTcpChannel(host, CommunicationPort, 0, DefaultTimeout, channel));
    CheckBoolResult(channel);

    SetThreadAffinity(std::to_string(CommunicationPort));

    std::array<char, BufferSize> buffer{};

    connectedEvent.Set();

    while (!isStopped) {
        CheckResult(channel->GetWriter().Write(buffer.data(), BufferSize));
        CheckResult(channel->GetWriter().EndWrite());

        CheckResult(channel->GetReader().Read(buffer.data(), BufferSize));

        counter++;
    }

    return Result::Ok;
}

void RemoteCommunicationClientRun(std::string_view host,
                                  Event& connectedEvent,
                                  uint64_t& counter,
                                  const bool& isStopped) {
    if (!IsOk(Run(host, connectedEvent, counter, isStopped))) {
        LogError("Could not run remote communication client.");
    }
}

}  // namespace

void RunRemoteCommunicationTest(std::string_view host) {  // NOLINT(misc-use-internal-linkage)
    LogTrace("Remote Communication:");
    RunPerformanceTest(RemoteCommunicationClientRun, host);
    LogTrace("");
}
