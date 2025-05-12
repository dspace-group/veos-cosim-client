// Copyright dSPACE GmbH. All rights reserved.

#include <array>
#include <cstdint>
#include <string_view>

#include "Channel.h"
#include "CoSimHelper.h"
#include "Helper.h"
#include "LogHelper.h"
#include "PerformanceTestHelper.h"
#include "RunPerformanceTest.h"

using namespace DsVeosCoSim;

namespace {

void LocalCommunicationClientRun([[maybe_unused]] std::string_view host,
                                 Event& connectedEvent,
                                 uint64_t& counter,
                                 const bool& isStopped) {
    try {
#ifdef _WIN32
        std::unique_ptr<Channel> channel = TryConnectToLocalChannel(LocalName);
#else
        std::unique_ptr<Channel> channel = TryConnectToUdsChannel(LocalName);
#endif
        MUST_BE_TRUE(channel);

        std::array<char, BufferSize> buffer{};

        connectedEvent.Set();

        while (!isStopped) {
            MUST_BE_TRUE(channel->GetWriter().Write(buffer.data(), BufferSize));
            MUST_BE_TRUE(channel->GetWriter().EndWrite());

            MUST_BE_TRUE(channel->GetReader().Read(buffer.data(), BufferSize));

            counter++;
        }
    } catch (const std::exception& e) {
        LogError("Exception in local communication client thread: {}", e.what());
        connectedEvent.Set();
    }
}

}  // namespace

void RunLocalCommunicationTest() {  // NOLINT(misc-use-internal-linkage)
    LogTrace("Local Communication:");
    RunPerformanceTest(LocalCommunicationClientRun, "");
    LogTrace("");
}
