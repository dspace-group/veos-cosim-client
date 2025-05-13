// Copyright dSPACE GmbH. All rights reserved.

#ifdef ALL_COMMUNICATION_TESTS

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

void TcpClientRun(const std::string_view host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    try {
        const std::optional<Socket> clientSocket = Socket::TryConnect(host, TcpPort, 0, 1000);
        if (!clientSocket) {
            throw std::runtime_error("Could not connect to TCP server.");
        }

        clientSocket->EnableNoDelay();

        std::array<char, BufferSize> buffer{};

        connectedEvent.Set();

        while (!isStopped) {
            MUST_BE_TRUE(SendComplete(*clientSocket, buffer.data(), BufferSize));
            MUST_BE_TRUE(ReceiveComplete(*clientSocket, buffer.data(), BufferSize));

            counter++;
        }
    } catch (const std::exception& e) {
        LogError("Exception in TCP client thread: {}", e.what());
        connectedEvent.Set();
    }
}

}  // namespace

void RunTcpTest(const std::string_view host) {  // NOLINT(misc-use-internal-linkage)
    LogTrace("TCP:");
    RunPerformanceTest(TcpClientRun, host);
    LogTrace("");
}

#else

#include <string_view>

void RunTcpTest([[maybe_unused]] const std::string_view host) {  // NOLINT(misc-use-internal-linkage)
}

#endif  // ALL_COMMUNICATION_TESTS
