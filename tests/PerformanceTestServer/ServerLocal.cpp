// Copyright dSPACE SE & Co. KG. All rights reserved.

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
    CheckResult(Socket::Create(AddressFamily::Local, serverSocket));

    CheckResult(serverSocket.Bind(LocalName));
    CheckResult(serverSocket.Listen());

    LogTrace("Local server is listening on file {} ...", LocalName);

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

void LocalServerRun() {
    if (!IsOk(Run())) {
        LogError("Could not run Local server.");
    }
}

}  // namespace

void StartLocalServer() {
    std::thread(LocalServerRun).detach();
}

#else

void StartLocalServer() {
}

#endif
