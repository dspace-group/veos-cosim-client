// Copyright dSPACE GmbH. All rights reserved.

#include <array>
#include <cstdint>
#include <string>

#include "Channel.h"
#include "CoSimHelper.h"
#include "OsUtilities.h"
#include "PerformanceTestClient.h"
#include "PerformanceTestHelper.h"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] Result Run([[maybe_unused]] const std::string& host,
                         Event& connectedEvent,
                         uint64_t& counter,
                         const bool& isStopped) {
#ifdef _WIN32
    std::unique_ptr<Channel> channel;
    CheckResult(TryConnectToLocalChannel(LocalName, channel));
#else
    std::unique_ptr<Channel> channel;
    CheckResult(TryConnectToUdsChannel(LocalName, channel));
#endif
    CheckBoolResult(channel);

    SetThreadAffinity(LocalName);

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

void LocalCommunicationClientRun(const std::string& host,
                                 Event& connectedEvent,
                                 uint64_t& counter,
                                 const bool& isStopped) {
    if (!IsOk(Run(host, connectedEvent, counter, isStopped))) {
        LogError("Could not run local communication client.");
    }
}

}  // namespace

void RunLocalCommunicationTest() {  // NOLINT(misc-use-internal-linkage)
    LogTrace("Local Communication:");
    RunPerformanceTest(LocalCommunicationClientRun, "");
    LogTrace("");
}
