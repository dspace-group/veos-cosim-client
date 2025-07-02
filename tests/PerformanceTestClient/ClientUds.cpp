// Copyright dSPACE GmbH. All rights reserved.

#include "PerformanceTestClient.h"

#ifdef ALL_COMMUNICATION_TESTS

#include <array>
#include <optional>
#include <string_view>

#include "CoSimHelper.h"
#include "Helper.h"
#include "PerformanceTestHelper.h"
#include "Socket.h"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] Result Run([[maybe_unused]] std::string_view host,
                         Event& connectedEvent,
                         uint64_t& counter,
                         const bool& isStopped) {
    std::optional<Socket> clientSocket;
    CheckResult(Socket::TryConnect(UdsName, clientSocket));
    CheckBoolResult(clientSocket);

    std::array<char, BufferSize> buffer{};

    connectedEvent.Set();

    while (!isStopped) {
        CheckResult(SendComplete(*clientSocket, buffer.data(), BufferSize));
        CheckResult(ReceiveComplete(*clientSocket, buffer.data(), BufferSize));

        counter++;
    }

    return Result::Ok;
}

void UdsClientRun(std::string_view host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    if (!IsOk(Run(host, connectedEvent, counter, isStopped))) {
        LogError("Could not run Unix Domain Socket client.");
    }
}

}  // namespace

void RunUdsTest() {  // NOLINT(misc-use-internal-linkage)
    LogTrace("Unix Domain Socket:");
    RunPerformanceTest(UdsClientRun, "");
    LogTrace("");
}

#else

void RunUdsTest() {  // NOLINT(misc-use-internal-linkage)
}

#endif
