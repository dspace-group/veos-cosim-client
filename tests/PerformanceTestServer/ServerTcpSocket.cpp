// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <array>
#include <thread>

#include "Helper.hpp"
#include "Logger.hpp"
#include "PerformanceTestHelper.hpp"
#include "PerformanceTestServer.hpp"
#include "Result.hpp"
#include "Socket.hpp"

using namespace DsVeosCoSim;

namespace {

Result RunForConnectedTcpSocket(SocketClient& client) {
    std::array<char, FrameSize> buffer{};

    while (true) {
        CheckResult(ReceiveComplete(client, buffer.data(), FrameSize));
        CheckResult(client.Send(buffer.data(), FrameSize));
    }
}

[[nodiscard]] Result RunServerTcpSocketInternal() {
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

        RunForConnectedTcpSocket(client);
    }

    return CreateOk();
}

void RunServerTcpSocket() {
    if (!IsOk(RunServerTcpSocketInternal())) {
        LogError("Could not run TCP Socket Server.");
    }
}

}  // namespace

void ServerTcpSocket() {
    std::thread(RunServerTcpSocket).detach();
}
