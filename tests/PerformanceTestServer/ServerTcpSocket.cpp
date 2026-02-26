// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "PerformanceTestServer.hpp"

#include <array>
#include <thread>

#include "Helper.hpp"
#include "Logger.hpp"
#include "PerformanceTestHelper.hpp"
#include "Result.hpp"
#include "Socket.hpp"

namespace DsVeosCoSim {

namespace {

Result RunForConnected(SocketClient& client) {
    std::array<char, FrameSize> buffer{};

    while (true) {
        CheckResult(ReceiveComplete(client, buffer.data(), FrameSize));
        CheckResult(client.Send(buffer.data(), FrameSize));
    }
}

[[nodiscard]] Result Run() {
    SocketListener listener;
    CheckResult(SocketListener::Create(AddressFamily::Ipv4, TcpSocketPort, true, listener));

    LogTrace("TCP Socket Server is listening on port {} ...", TcpSocketPort);

    while (true) {
        SocketClient client;

        while (true) {
            Result result = listener.TryAccept(client);
            if (IsOk(result)) {
                break;
            }

            if (IsNotConnected(result)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            return result;
        }

        RunForConnected(client);
    }

    return CreateOk();
}

void TcpSocketServer() {
    if (!IsOk(Run())) {
        LogError("Could not run TCP Socket Server.");
    }
}

}  // namespace

void StartTcpSocketServer() {
    std::thread(TcpSocketServer).detach();
}

}  // namespace DsVeosCoSim
