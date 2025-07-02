// Copyright dSPACE GmbH. All rights reserved.

#include "PerformanceTestClient.h"

#ifdef ALL_COMMUNICATION_TESTS

#include <array>
#include <optional>
#include <string_view>

#include "CoSimHelper.h"
#include "DsVeosCoSim/CoSimTypes.h"
#include "Helper.h"
#include "PerformanceTestHelper.h"
#include "Socket.h"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] Result Run(std::string_view host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    std::optional<Socket> clientSocket;
    CheckResult(Socket::TryConnect(host, TcpPort, 0, 1000, clientSocket));
    CheckBoolResult(clientSocket);

    CheckResult(clientSocket->EnableNoDelay());

    std::array<char, BufferSize> buffer{};

    connectedEvent.Set();

    while (!isStopped) {
        CheckResult(SendComplete(*clientSocket, buffer.data(), BufferSize));
        CheckResult(ReceiveComplete(*clientSocket, buffer.data(), BufferSize));

        counter++;
    }

    return Result::Ok;
}

void TcpClientRun(std::string_view host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    if (!IsOk(Run(host, connectedEvent, counter, isStopped))) {
        LogError("Could not run TCP client.");
    }
}

}  // namespace

void RunTcpTest(std::string_view host) {  // NOLINT(misc-use-internal-linkage)
    LogTrace("TCP:");
    RunPerformanceTest(TcpClientRun, host);
    LogTrace("");
}

#else

void RunTcpTest([[maybe_unused]] std::string_view host) {  // NOLINT(misc-use-internal-linkage)
}

#endif  // ALL_COMMUNICATION_TESTS
