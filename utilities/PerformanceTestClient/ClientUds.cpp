// Copyright dSPACE GmbH. All rights reserved.

#include <string_view>

#include "Helper.h"
#include "Logger.h"
#include "PerformanceTestHelper.h"
#include "RunPerformanceTest.h"
#include "Socket.h"

using namespace DsVeosCoSim;

namespace {

void UdsClientRun([[maybe_unused]] std::string_view host,
                  Event& connectedEvent,
                  uint64_t& counter,
                  const bool& isStopped) {
    try {
        Socket clientSocket(AddressFamily::Uds);
        MUST_BE_TRUE(clientSocket.TryConnect(UdsName));

        char buffer[BufferSize]{};

        connectedEvent.Set();

        while (!isStopped) {
            MUST_BE_TRUE(SendComplete(clientSocket, buffer, BufferSize));
            MUST_BE_TRUE(ReceiveComplete(clientSocket, buffer, BufferSize));

            counter++;
        }
    } catch (const std::exception& e) {
        LogError("Exception in unix domain socket client thread: {}", e.what());
        connectedEvent.Set();
    }
}

}  // namespace

void RunUdsTest() {
    LogTrace("Unix Domain Socket:");
    RunPerformanceTest(UdsClientRun, "");
    LogTrace("");
}
