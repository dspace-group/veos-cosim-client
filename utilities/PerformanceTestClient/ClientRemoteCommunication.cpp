// Copyright dSPACE GmbH. All rights reserved.

#include <string_view>  // IWYU pragma: keep

#include "CoSimHelper.h"
#include "Helper.h"
#include "LogHelper.h"
#include "PerformanceTestHelper.h"
#include "RunPerformanceTest.h"
#include "SocketChannel.h"

using namespace DsVeosCoSim;

namespace {

void RemoteCommunicationClientRun(std::string_view host,
                                  Event& connectedEvent,
                                  uint64_t& counter,
                                  const bool& isStopped) {
    try {
        std::optional<SocketChannel> channel = *TryConnectToTcpChannel(host, CommunicationPort, 0, Infinite);

        char buffer[BufferSize]{};

        connectedEvent.Set();

        while (!isStopped) {
            MUST_BE_TRUE(channel->GetWriter().Write(buffer, BufferSize));
            MUST_BE_TRUE(channel->GetWriter().EndWrite());

            MUST_BE_TRUE(channel->GetReader().Read(buffer, BufferSize));

            counter++;
        }
    } catch (const std::exception& e) {
        LogError("Exception in remote communication client thread: {}", e.what());
        connectedEvent.Set();
    }
}

}  // namespace

void RunRemoteCommunicationTest(std::string_view host) {
    LogTrace("Remote Communication:");
    RunPerformanceTest(RemoteCommunicationClientRun, host);
    LogTrace("");
}
