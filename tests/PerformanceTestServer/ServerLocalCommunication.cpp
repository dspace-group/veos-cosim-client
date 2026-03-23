// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <array>
#include <chrono>
#include <thread>

#include "Channel.hpp"
#include "Logger.hpp"
#include "OsUtilities.hpp"
#include "PerformanceTestHelper.hpp"
#include "PerformanceTestServer.hpp"
#include "Result.hpp"

namespace DsVeosCoSim {

namespace {

Result RunForConnected(Channel& channel) {
    std::array<char, FrameSize> buffer{};

    while (true) {
        CheckResult(channel.GetReader().Read(buffer.data(), FrameSize));
        CheckResult(channel.GetWriter().Write(buffer.data(), FrameSize));
        CheckResult(channel.GetWriter().EndWrite());
    }
}

[[nodiscard]] Result Run() {
    LogTrace("Local communication server is listening ...");

    std::unique_ptr<ChannelServer> server;
    CheckResult(CreateLocalChannelServer(LocalChannelName, server));

    SetThreadAffinity(LocalChannelName);

    while (true) {
        std::unique_ptr<Channel> acceptedChannel;

        while (true) {
            Result result = server->TryAccept(acceptedChannel);
            if (IsOk(result)) {
                break;
            }

            if (IsNotConnected(result)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            return result;
        }

        RunForConnected(*acceptedChannel);
    }

    return CreateOk();
}

void LocalCommunicationServerRun() {
    if (!IsOk(Run())) {
        LogError("Could not run local communication server.");
    }
}

}  // namespace

void StartLocalCommunicationServer() {
    std::thread(LocalCommunicationServerRun).detach();
}

}  // namespace DsVeosCoSim
