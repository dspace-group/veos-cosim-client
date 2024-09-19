// Copyright dSPACE GmbH. All rights reserved.

#include <thread>

#include "CoSimHelper.h"
#include "Helper.h"
#include "PerformanceTestHelper.h"
#include "Socket.h"

using namespace DsVeosCoSim;

void UdsServerRun() {
    try {
        Socket serverSocket(AddressFamily::Uds);

        serverSocket.Bind(UdsName);
        serverSocket.Listen();

        LogTrace("UDS server is listening on file {} ...", UdsName);

        char buffer[BufferSize]{};

        while (true) {
            std::optional<Socket> acceptedSocket = serverSocket.TryAccept(Infinite);

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
        LogError("Error in Unix domain socket server thread: {}", e.what());
    }
}

void StartUdsServer() {
    std::thread(UdsServerRun).detach();
}
