// Copyright dSPACE GmbH. All rights reserved.

#include <array>
#include <thread>

#include "LogHelper.h"
#include "OsAbstractionTestHelper.h"
#include "PerformanceTestHelper.h"

namespace {

void UdpServerRun() {
    try {
        LogTrace("UDP server is listening on port {} ...", UdpPort);

        std::array<char, BufferSize> buffer{};

        while (true) {
            UdpSocket serverSocket;
            serverSocket.Bind("0.0.0.0", UdpPort);

            InternetAddress address("127.0.0.1", 0);

            while (true) {
                if (!serverSocket.ReceiveFrom(buffer.data(), BufferSize, address)) {
                    break;
                }

                if (!serverSocket.SendTo(buffer.data(), BufferSize, address)) {
                    break;
                }
            }
        }
    } catch (const std::exception& e) {
        LogError("Exception in UDP server thread: {}", e.what());
    }
}

}  // namespace

void StartUdpServer() {
    std::thread(UdpServerRun).detach();
}
