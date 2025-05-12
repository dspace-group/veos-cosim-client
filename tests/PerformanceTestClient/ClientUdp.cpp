// Copyright dSPACE GmbH. All rights reserved.

#ifdef ALL_COMMUNICATION_TESTS

#include <array>
#include <string_view>

#include "CoSimHelper.h"
#include "Helper.h"
#include "LogHelper.h"
#include "OsAbstractionTestHelper.h"
#include "PerformanceTestHelper.h"
#include "RunPerformanceTest.h"

using namespace DsVeosCoSim;

namespace {

void UdpClientRun(const std::string_view host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    try {
        const UdpSocket clientSocket;

        const InternetAddress sendAddress(host, UdpPort);
        InternetAddress receiveAddress(host, UdpPort);

        std::array<char, BufferSize> buffer{};

        connectedEvent.Set();

        while (!isStopped) {
            MUST_BE_TRUE(clientSocket.SendTo(buffer.data(), BufferSize, sendAddress));

            MUST_BE_TRUE(clientSocket.ReceiveFrom(buffer.data(), BufferSize, receiveAddress));

            counter++;
        }
    } catch (const std::exception& e) {
        LogError("Exception in UDP client thread: {}", e.what());
        connectedEvent.Set();
    }
}

}  // namespace

void RunUdpTest(const std::string_view host) {  // NOLINT(misc-use-internal-linkage)
    LogTrace("UDP:");
    RunPerformanceTest(UdpClientRun, host);
    LogTrace("");
}

#else

#include <string_view>

void RunUdpTest([[maybe_unused]] const std::string_view host) {  // NOLINT(misc-use-internal-linkage)
}

#endif  // ALL_COMMUNICATION_TESTS
