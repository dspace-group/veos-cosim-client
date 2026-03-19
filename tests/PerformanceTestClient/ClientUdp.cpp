// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "PerformanceTestClient.hpp"

#include <array>
#include <string>

#include "Helper.hpp"
#include "OsAbstractionTestHelper.hpp"
#include "PerformanceTestHelper.hpp"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] Result Run(const std::string& host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
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

void UdpClientRun(const std::string& host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    if (!IsOk(Run(host, connectedEvent, counter, isStopped))) {
        LogError("Could not run UDP client.");
    }
}

}  // namespace

void RunUdpTest(const std::string& host) {  // NOLINT(misc-use-internal-linkage)
    LogTrace("UDP:");
    RunPerformanceTest(UdpClientRun, host);
    LogTrace("");
}
