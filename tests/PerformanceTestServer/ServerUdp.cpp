// Copyright dSPACE GmbH. All rights reserved.

#include "PerformanceTestServer.h"

#ifdef ALL_COMMUNICATION_TESTS

#include <array>
#include <thread>

#include "LogHelper.h"
#include "OsAbstractionTestHelper.h"
#include "PerformanceTestHelper.h"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] Result Run() {
    LogTrace("UDP server is listening on port {} ...", UdpPort);

    std::array<char, BufferSize> buffer{};

    while (true) {
        UdpSocket serverSocket;
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

#else

void StartUdpServer() {
}

#endif  // ALL_COMMUNICATION_TESTS
