// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "PerformanceTestServer.hpp"

#include <array>
#include <thread>

#include "Helper.hpp"
#include "OsAbstractionTestHelper.hpp"
#include "PerformanceTestHelper.hpp"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] Result Run() {
    LogTrace("UDP server is listening on port {} ...", UdpPort);

    std::array<char, BufferSize> buffer{};

    while (true) {
        UdpSocket serverSocket;
        CheckResult(serverSocket.Initialize());
        CheckResult(serverSocket.Bind("0.0.0.0", UdpPort));

        InternetAddress address;
        CheckResult(address.Initialize("127.0.0.1", 0));

        while (true) {
            if (!IsOk(serverSocket.ReceiveFrom(buffer.data(), BufferSize, address))) {
                break;
            }

            if (!IsOk(serverSocket.SendTo(buffer.data(), BufferSize, address))) {
                break;
            }
        }
    }

    return Result::Ok;
}

void UdpServerRun() {
    if (!IsOk(Run())) {
        LogError("Could not run UDP server.");
    }
}

}  // namespace

void StartUdpServer() {
    std::thread(UdpServerRun).detach();
}
