// Copyright dSPACE GmbH. All rights reserved.

#include <array>
#include <thread>

#include "Helper.h"
#include "LogHelper.h"
#include "PerformanceTestHelper.h"
#include "Socket.h"

using namespace DsVeosCoSim;

namespace {

void UdsServerRun() {
    try {
        Socket serverSocket(AddressFamily::Uds);

        serverSocket.Bind(UdsName);
        serverSocket.Listen();

        LogTrace("UDS server is listening on file {} ...", UdsName);

        std::array<char, BufferSize> buffer{};

        while (true) {
            std::optional<Socket> acceptedSocket = serverSocket.TryAccept(Infinite);

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
        LogError("Error in Unix domain socket server thread: {}", e.what());
    }
}

}  // namespace

void StartUdsServer() {  // NOLINT
    std::thread(UdsServerRun).detach();
}
