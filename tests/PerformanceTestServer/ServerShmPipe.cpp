// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "PerformanceTestServer.hpp"

#include <array>
#include <thread>

#include "Error.hpp"
#include "Helper.hpp"
#include "OsUtilities.hpp"
#include "PerformanceTestHelper.hpp"

namespace DsVeosCoSim {

namespace {

Result RunForConnected(ShmPipeClient& shmPipeClient) {
    std::array<char, FrameSize> buffer{};

    while (true) {
        CheckResult(ReceiveComplete(shmPipeClient, buffer.data(), FrameSize));
        CheckResult(shmPipeClient.Send(buffer.data(), FrameSize));
    }
}

[[nodiscard]] Result Run() {
    LogTrace("SHM Pipe server is listening on pipe {} ...", ShmPipeName);

    ShmPipeListener listener;
    CheckResult(ShmPipeListener::Create(ShmPipeName, listener));

    while (true) {
        ShmPipeClient client;

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

void ShmPipeServer() {
    if (!IsOk(Run())) {
        LogError("Could not run SHM Pipe Server.");
    }
}

}  // namespace

void StartShmPipeServer() {
    std::thread(ShmPipeServer).detach();
}

}  // namespace DsVeosCoSim
