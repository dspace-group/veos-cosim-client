// Copyright dSPACE GmbH. All rights reserved.

#include <string_view>

#include "CoSimHelper.h"
#include "Helper.h"
#include "OsAbstractionTestHelper.h"
#include "PerformanceTestHelper.h"
#include "RunPerformanceTest.h"

using namespace DsVeosCoSim;

namespace {

void UdpClientRun(std::string_view host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    try {
        const UdpSocket clientSocket;

        const InternetAddress sendAddress(host, UdpPort);
        InternetAddress receiveAddress(host, UdpPort);

        char buffer[BufferSize]{};

        connectedEvent.Set();

        while (!isStopped) {
            MUST_BE_TRUE(clientSocket.SendTo(buffer, BufferSize, sendAddress));

            MUST_BE_TRUE(clientSocket.ReceiveFrom(buffer, BufferSize, receiveAddress));

            counter++;
        }
    } catch (const std::exception& e) {
        LogError("Exception in UDP client thread: {}", e.what());
        connectedEvent.Set();
    }
}

}  // namespace

void RunUdpTest(std::string_view host) {
    LogTrace("UDP:");
    RunPerformanceTest(UdpClientRun, host);
    LogTrace("");
}
