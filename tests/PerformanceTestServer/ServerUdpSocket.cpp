// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <array>
#include <thread>

#include "Logger.hpp"
#include "OsAbstractionTestHelper.hpp"
#include "PerformanceTestHelper.hpp"
#include "PerformanceTestServer.hpp"
#include "Result.hpp"

namespace DsVeosCoSim {

namespace {

[[nodiscard]] Result Run() {
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

void UdpSocketServer() {
    if (!IsOk(Run())) {
        LogError("Could not run UDP Socket Server.");
    }
}

}  // namespace

void StartUdpSocketServer() {
    std::thread(UdpSocketServer).detach();
}

}  // namespace DsVeosCoSim
