// Copyright dSPACE GmbH. All rights reserved.

#include <array>
#include <string_view>

#include "CoSimHelper.h"
#include "Helper.h"
#include "LogHelper.h"
#include "PerformanceTestHelper.h"
#include "RunPerformanceTest.h"

using namespace DsVeosCoSim;

namespace {

void RemoteCommunicationClientRun(const std::string_view host,
                                  Event& connectedEvent,
                                  uint64_t& counter,
                                  const bool& isStopped) {
    try {
        std::unique_ptr<Channel> channel = ConnectToTcpChannel(host, CommunicationPort);

        std::array<char, BufferSize> buffer{};

        connectedEvent.Set();

        while (!isStopped) {
            MUST_BE_TRUE(channel->GetWriter().Write(buffer.data(), BufferSize));
            MUST_BE_TRUE(channel->GetWriter().EndWrite());

            MUST_BE_TRUE(channel->GetReader().Read(buffer.data(), BufferSize));

            counter++;
        }
    } catch (const std::exception& e) {
        LogError("Exception in remote communication client thread: {}", e.what());
        connectedEvent.Set();
    }
}

}  // namespace

void RunRemoteCommunicationTest(const std::string_view host) {  // NOLINT
    LogTrace("Remote Communication:");
    RunPerformanceTest(RemoteCommunicationClientRun, host);
    LogTrace("");
}
