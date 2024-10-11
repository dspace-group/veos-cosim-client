// Copyright dSPACE GmbH. All rights reserved.

#include <string_view>

#include "CoSimHelper.h"
#include "Helper.h"
#include "LogHelper.h"
#include "PerformanceTestHelper.h"
#include "RunPerformanceTest.h"

#ifdef _WIN32
#include "LocalChannel.h"
#else
#include "SocketChannel.h"
#endif

using namespace DsVeosCoSim;

namespace {

void LocalCommunicationClientRun([[maybe_unused]] std::string_view host,
                                 Event& connectedEvent,
                                 uint64_t& counter,
                                 const bool& isStopped) {
    try {
#ifdef _WIN32
        std::optional<LocalChannel> channel = *TryConnectToLocalChannel(LocalName);
#else
        std::optional<SocketChannel> channel = *TryConnectToUdsChannel(LocalName);
#endif

        char buffer[BufferSize]{};

        connectedEvent.Set();

        while (!isStopped) {
            MUST_BE_TRUE(channel->GetWriter().Write(buffer, BufferSize));
            MUST_BE_TRUE(channel->GetWriter().EndWrite());

            MUST_BE_TRUE(channel->GetReader().Read(buffer, BufferSize));

            counter++;
        }
    } catch (const std::exception& e) {
        LogError("Exception in local communication client thread: {}", e.what());
        connectedEvent.Set();
    }
}

}  // namespace

void RunLocalCommunicationTest() {
    LogTrace("Local Communication:");
    RunPerformanceTest(LocalCommunicationClientRun, "");
    LogTrace("");
}
