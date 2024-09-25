// Copyright dSPACE GmbH. All rights reserved.

#include <thread>

#include "Helper.h"
#include "LogHelper.h"
#include "PerformanceTestHelper.h"
#include "Socket.h"

using namespace DsVeosCoSim;

namespace {

void TcpServerRun() {
    try {
        const Socket serverSocket(AddressFamily::Ipv4);
        serverSocket.EnableReuseAddress();
        serverSocket.Bind(TcpPort, true);
        serverSocket.Listen();

        LogTrace("TCP server is listening on port {} ...", TcpPort);

        char buffer[BufferSize]{};

        while (true) {
            std::optional<Socket> acceptedSocket = serverSocket.TryAccept(Infinite);
            acceptedSocket->EnableNoDelay();

            while (true) {
                if (!ReceiveComplete(*acceptedSocket, buffer, BufferSize)) {
                    break;
                }

                if (!SendComplete(*acceptedSocket, buffer, BufferSize)) {
                    break;
                }
            }
        }
    } catch (const std::exception& e) {
        LogError("Exception in TCP server thread: {}", e.what());
    }
}

}  // namespace

void StartTcpServer() {
    std::thread(TcpServerRun).detach();
}
