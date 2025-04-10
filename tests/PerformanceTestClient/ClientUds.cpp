// Copyright dSPACE GmbH. All rights reserved.

#include <array>
#include <optional>
#include <string_view>

#include "CoSimHelper.h"
#include "Helper.h"
#include "LogHelper.h"
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
        const std::optional<Socket> clientSocket = Socket::TryConnect(UdsName);
        MUST_BE_TRUE(clientSocket);

        std::array<char, BufferSize> buffer{};

        connectedEvent.Set();

        while (!isStopped) {
            MUST_BE_TRUE(SendComplete(*clientSocket, buffer.data(), BufferSize));
            MUST_BE_TRUE(ReceiveComplete(*clientSocket, buffer.data(), BufferSize));

            counter++;
        }
    } catch (const std::exception& e) {
        LogError("Exception in unix domain socket client thread: {}", e.what());
        connectedEvent.Set();
    }
}

}  // namespace

void RunUdsTest() {  // NOLINT
    LogTrace("Unix Domain Socket:");
    RunPerformanceTest(UdsClientRun, "");
    LogTrace("");
}
