// Copyright dSPACE GmbH. All rights reserved.

#include "PerformanceTestClient.h"

#ifdef ALL_COMMUNICATION_TESTS

#include <array>
#include <string_view>

#include "CoSimHelper.h"
#include "OsAbstractionTestHelper.h"
#include "PerformanceTestHelper.h"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] Result Run(std::string_view host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    UdpSocket clientSocket;
    CheckResult(clientSocket.Initialize());

    InternetAddress sendAddress;
    CheckResult(sendAddress.Initialize(host, UdpPort));
    InternetAddress receiveAddress;
    CheckResult(receiveAddress.Initialize(host, UdpPort));

    std::array<char, BufferSize> buffer{};

    connectedEvent.Set();

    while (!isStopped) {
        CheckResult(clientSocket.SendTo(buffer.data(), BufferSize, sendAddress));

        CheckResult(clientSocket.ReceiveFrom(buffer.data(), BufferSize, receiveAddress));

        counter++;
    }

    return Result::Ok;
}

void UdpClientRun(std::string_view host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    if (!IsOk(Run(host, connectedEvent, counter, isStopped))) {
        LogError("Could not run UDP client.");
    }
}

}  // namespace

void RunUdpTest(std::string_view host) {  // NOLINT(misc-use-internal-linkage)
    LogTrace("UDP:");
    RunPerformanceTest(UdpClientRun, host);
    LogTrace("");
}

#else

void RunUdpTest([[maybe_unused]] std::string_view host) {  // NOLINT(misc-use-internal-linkage)
}

#endif  // ALL_COMMUNICATION_TESTS
