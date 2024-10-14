// Copyright dSPACE GmbH. All rights reserved.

#include <array>
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

        std::array<char, BufferSize> buffer{};

        while (true) {
            std::optional<Socket> acceptedSocket = serverSocket.TryAccept(Infinite);
            acceptedSocket->EnableNoDelay();

            while (true) {
                if (!ReceiveComplete(*acceptedSocket, buffer.data(), BufferSize)) {
                    break;
                }

                if (!SendComplete(*acceptedSocket, buffer.data(), BufferSize)) {
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
