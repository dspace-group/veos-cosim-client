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

Result RunForConnectedLocalSocket(SocketClient& client) {
    std::array<char, FrameSize> buffer{};

    while (true) {
        CheckResult(ReceiveComplete(client, buffer.data(), FrameSize));
        CheckResult(client.Send(buffer.data(), FrameSize));
    }
}

[[nodiscard]] Result RunServerLocalSocketInternal() {
    SocketListener listener;
    CheckResult(SocketListener::Create(LocalSocketPath, listener));

    LogTrace("Local Socket Server is listening on file {} ...", LocalSocketPath);

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

        RunForConnectedLocalSocket(client);
    }

    return CreateOk();
}

void RunServerLocalSocket() {
    if (!IsOk(RunServerLocalSocketInternal())) {
        LogError("Could not run Local Socket Server.");
    }
}

}  // namespace

void ServerLocalSocket() {
    std::thread(RunServerLocalSocket).detach();
}
