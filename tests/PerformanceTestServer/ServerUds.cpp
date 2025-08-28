// Copyright dSPACE GmbH. All rights reserved.

#include "PerformanceTestServer.h"

#ifdef ALL_COMMUNICATION_TESTS

#include <array>
#include <thread>

#include "Helper.h"
#include "PerformanceTestHelper.h"
#include "Socket.h"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] Result Run() {
    Socket serverSocket;
    CheckResult(Socket::Create(AddressFamily::Uds, serverSocket));

    CheckResult(serverSocket.Bind(UdsName));
    CheckResult(serverSocket.Listen());

    LogTrace("Unix Domain Socket server is listening on file {} ...", UdsName);

    std::array<char, BufferSize> buffer{};

    while (true) {
        std::optional<Socket> acceptedSocket;

        while (true) {
            CheckResult(serverSocket.TryAccept(acceptedSocket));
            if (acceptedSocket) {
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        while (true) {
            if (!IsOk(ReceiveComplete(*acceptedSocket, buffer.data(), BufferSize))) {
                break;
            }

            if (!IsOk(acceptedSocket->Send(buffer.data(), BufferSize))) {
                break;
            }
        }
    }

    return Result::Ok;
}

void UdsServerRun() {
    if (!IsOk(Run())) {
        LogError("Could not run Unix Domain Socket server.");
    }
}

}  // namespace

void StartUdsServer() {
    std::thread(UdsServerRun).detach();
}

#else

void StartUdsServer() {
}

#endif
