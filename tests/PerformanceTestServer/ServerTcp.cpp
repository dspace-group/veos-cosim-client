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
    CheckResult(Socket::Create(AddressFamily::Ipv4, serverSocket));
    CheckResult(serverSocket.EnableReuseAddress());
    CheckResult(serverSocket.Bind(TcpPort, true));
    CheckResult(serverSocket.Listen());

    LogTrace("TCP server is listening on port {} ...", TcpPort);

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

        CheckResult(acceptedSocket->EnableNoDelay());

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

void TcpServerRun() {
    if (!IsOk(Run())) {
        LogError("Could not run TCP server.");
    }
}

}  // namespace

void StartTcpServer() {
    std::thread(TcpServerRun).detach();
}

#else

void StartTcpServer() {
}

#endif  // ALL_COMMUNICATION_TESTS
