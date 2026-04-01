// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <array>
#include <string>

#include "Logger.hpp"
#include "OsAbstractionTestHelper.hpp"
#include "PerformanceTestClient.hpp"
#include "PerformanceTestHelper.hpp"
#include "Result.hpp"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] Result RunClientUdpSocketInternal(const std::string& host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    UdpSocket udpSocket;
    CheckResult(UdpSocket::CreateClient(udpSocket));

    InternetAddress sendAddress;
    CheckResult(InternetAddress::Create(host, UdpSocketPort, sendAddress));

    InternetAddress receiveAddress;
    CheckResult(InternetAddress::Create(host, UdpSocketPort, receiveAddress));

    std::array<char, FrameSize> buffer{};

    connectedEvent.Set();

    while (!isStopped) {
        CheckResult(udpSocket.SendTo(buffer.data(), FrameSize, sendAddress));
        CheckResult(udpSocket.ReceiveFrom(buffer.data(), FrameSize, receiveAddress));

        counter++;
    }

    return CreateOk();
}

void RunClientUdpSocket(const std::string& host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    if (!IsOk(RunClientUdpSocketInternal(host, connectedEvent, counter, isStopped))) {
        LogError("Could not run UDP Socket Client.");
    }
}

}  // namespace

void ClientUdpSocket(const std::string& host) {  // NOLINT(misc-use-internal-linkage)
    LogTrace("UDP Socket:");
    RunPerformanceTest(RunClientUdpSocket, host);
    LogTrace("");
}
