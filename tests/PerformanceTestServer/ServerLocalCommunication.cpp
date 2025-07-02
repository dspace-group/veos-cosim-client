// Copyright dSPACE GmbH. All rights reserved.

#include <array>
#include <chrono>
#include <thread>

#include "Channel.h"
#include "CoSimHelper.h"
#include "OsUtilities.h"
#include "PerformanceTestHelper.h"
#include "PerformanceTestServer.h"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] Result Run() {
    LogTrace("Local communication server is listening ...");

    std::unique_ptr<ChannelServer> server;
#ifdef _WIN32
    CheckResult(CreateLocalChannelServer(LocalName, server));
#else
    CheckResult(CreateUdsChannelServer(LocalName, server));
#endif

    SetThreadAffinity(LocalName);

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

void LocalCommunicationServerRun() {
    if (!IsOk(Run())) {
        LogError("Could not run local communication server.");
    }
}

}  // namespace

void StartLocalCommunicationServer() {
    std::thread(LocalCommunicationServerRun).detach();
}
