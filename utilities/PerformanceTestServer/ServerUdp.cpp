// Copyright dSPACE GmbH. All rights reserved.

#include <thread>

#include "LogHelper.h"
#include "OsAbstractionTestHelper.h"
#include "PerformanceTestHelper.h"

namespace {

void UdpServerRun() {
    try {
        LogTrace("UDP server is listening on port {} ...", UdpPort);

        char buffer[BufferSize]{};

        while (true) {
            UdpSocket serverSocket;
            serverSocket.Bind("0.0.0.0", UdpPort);

            InternetAddress address("127.0.0.1", 0);

            while (true) {
                if (!serverSocket.ReceiveFrom(buffer, BufferSize, address)) {
                    break;
                }

                if (!serverSocket.SendTo(buffer, BufferSize, address)) {
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
