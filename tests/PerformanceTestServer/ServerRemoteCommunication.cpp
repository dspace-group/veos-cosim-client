// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <array>
#include <thread>

#include "Channel.h"
#include "Helper.h"
#include "OsUtilities.h"
#include "PerformanceTestHelper.h"
#include "PerformanceTestServer.h"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] Result Run() {
    LogTrace("Remote communication server is listening ...");

    std::unique_ptr<ChannelServer> server;
    CheckResult(CreateTcpChannelServer(CommunicationPort, true, server));

    SetThreadAffinity(std::to_string(CommunicationPort));

    std::array<char, BufferSize> buffer{};

    while (true) {
        std::unique_ptr<Channel> acceptedChannel;

        while (true) {
            CheckResult(server->TryAccept(acceptedChannel));
            if (acceptedChannel) {
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        while (true) {
            if (!IsOk(acceptedChannel->GetReader().Read(buffer.data(), BufferSize))) {
                break;
            }

            if (!IsOk(acceptedChannel->GetWriter().Write(buffer.data(), BufferSize))) {
                break;
            }

            if (!IsOk(acceptedChannel->GetWriter().EndWrite())) {
                break;
            }
        }
    }

    return Result::Ok;
}

void RemoteCommunicationServerRun() {
    if (!IsOk(Run())) {
        LogError("Could not run remote communication server.");
    }
}

}  // namespace

void StartRemoteCommunicationServer() {
    std::thread(RemoteCommunicationServerRun).detach();
}
