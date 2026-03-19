// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "PerformanceTestClient.hpp"

#include <array>
#include <optional>
#include <string>

#include "CoSimTypes.hpp"
#include "Helper.hpp"
#include "PerformanceTestHelper.hpp"
#include "Socket.hpp"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] Result Run(const std::string& host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    std::optional<Socket> clientSocket;
    CheckResult(Socket::TryConnect(host, TcpPort, 0, 1000, clientSocket));
    CheckBoolResult(clientSocket);

    CheckResult(clientSocket->EnableNoDelay());

    std::array<char, BufferSize> buffer{};

    connectedEvent.Set();

    while (!isStopped) {
        CheckResult(clientSocket->Send(buffer.data(), BufferSize));
        CheckResult(ReceiveComplete(*clientSocket, buffer.data(), BufferSize));

        counter++;
    }

    return Result::Ok;
}

void TcpClientRun(const std::string& host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    if (!IsOk(Run(host, connectedEvent, counter, isStopped))) {
        LogError("Could not run TCP client.");
    }
}

}  // namespace

void RunTcpTest(const std::string& host) {  // NOLINT(misc-use-internal-linkage)
    LogTrace("TCP:");
    RunPerformanceTest(TcpClientRun, host);
    LogTrace("");
}
