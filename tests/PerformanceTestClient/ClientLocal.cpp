// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "PerformanceTestClient.h"

#ifdef ALL_COMMUNICATION_TESTS

#include <array>
#include <optional>
#include <string>

#include "Helper.h"
#include "PerformanceTestHelper.h"
#include "Socket.h"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] Result Run([[maybe_unused]] const std::string& host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    std::optional<Socket> clientSocket;
    CheckResult(Socket::TryConnect(LocalName, clientSocket));
    CheckBoolResult(clientSocket);

    std::array<char, BufferSize> buffer{};

    connectedEvent.Set();

    while (!isStopped) {
        CheckResult(clientSocket->Send(buffer.data(), BufferSize));
        CheckResult(ReceiveComplete(*clientSocket, buffer.data(), BufferSize));

        counter++;
    }

    return Result::Ok;
}

void LocalClientRun(const std::string& host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    if (!IsOk(Run(host, connectedEvent, counter, isStopped))) {
        LogError("Could not run Local client.");
    }
}

}  // namespace

void RunLocalTest() {  // NOLINT(misc-use-internal-linkage)
    LogTrace("Local:");
    RunPerformanceTest(LocalClientRun, "");
    LogTrace("");
}

#else

void RunLocalTest() {  // NOLINT(misc-use-internal-linkage)
}

#endif
