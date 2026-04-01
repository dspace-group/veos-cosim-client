// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <array>
#include <thread>

#include "Logger.hpp"
#include "OsAbstractionTestHelper.hpp"
#include "PerformanceTestHelper.hpp"
#include "PerformanceTestServer.hpp"
#include "Result.hpp"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] Result RunServerUdpSocketInternal() {
    LogTrace("UDP Socket Server is listening on port {} ...", UdpSocketPort);

    std::array<char, FrameSize> buffer{};

    while (true) {
        UdpSocket udpSocket;
        CheckResult(UdpSocket::CreateServer("0.0.0.0", UdpSocketPort, udpSocket));

        InternetAddress internetAddress;
        CheckResult(InternetAddress::Create("127.0.0.1", UdpSocketPort, internetAddress));

        while (true) {
            CheckResult(udpSocket.ReceiveFrom(buffer.data(), FrameSize, internetAddress));
            CheckResult(udpSocket.SendTo(buffer.data(), FrameSize, internetAddress));
        }
    }

    return CreateOk();
}

void RunServerUdpSocket() {
    if (!IsOk(RunServerUdpSocketInternal())) {
        LogError("Could not run UDP Socket Server.");
    }
}

}  // namespace

void ServerUdpSocket() {
    std::thread(RunServerUdpSocket).detach();
}
