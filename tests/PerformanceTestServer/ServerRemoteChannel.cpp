// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <array>
#include <thread>

#include "Channel.hpp"
#include "Logger.hpp"
#include "OsUtilities.hpp"
#include "PerformanceTestHelper.hpp"
#include "PerformanceTestServer.hpp"
#include "Result.hpp"

using namespace DsVeosCoSim;

namespace {

Result RunForConnectedRemoteChannel(Channel& channel) {
    std::array<char, FrameSize> buffer{};

    while (true) {
        CheckResult(channel.GetReader().Read(buffer.data(), FrameSize));
        CheckResult(channel.GetWriter().Write(buffer.data(), FrameSize));
        CheckResult(channel.GetWriter().EndWrite());
    }
}

[[nodiscard]] Result RunServerRemoteChannelInternal() {
    LogTrace("Remote channel server is listening ...");

    std::unique_ptr<ChannelServer> server;
    CheckResult(CreateTcpChannelServer(RemoteChannelPort, true, server));

    SetThreadAffinity(std::to_string(RemoteChannelPort));

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

        RunForConnectedRemoteChannel(*acceptedChannel);
    }

    return CreateOk();
}

void RunServerRemoteChannel() {
    if (!IsOk(RunServerRemoteChannelInternal())) {
        LogError("Could not run remote channel server.");
    }
}

}  // namespace

void ServerRemoteChannel() {
    std::thread(RunServerRemoteChannel).detach();
}
