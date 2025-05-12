// Copyright dSPACE GmbH. All rights reserved.

#include <array>
#include <chrono>
#include <thread>

#include "Channel.h"
#include "CoSimHelper.h"
#include "LogHelper.h"
#include "PerformanceTestHelper.h"

using namespace DsVeosCoSim;

namespace {

void LocalCommunicationServerRun() {
    try {
        LogTrace("Local communication server is listening ...");

#ifdef _WIN32
        std::unique_ptr<ChannelServer> server = CreateLocalChannelServer(LocalName);
#else
        std::unique_ptr<ChannelServer> server = CreateUdsChannelServer(LocalName);
#endif

        std::array<char, BufferSize> buffer{};

        while (true) {
            std::unique_ptr<Channel> acceptedChannel;

            while (true) {
                acceptedChannel = server->TryAccept();
                if (acceptedChannel) {
                    break;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            while (true) {
                if (!acceptedChannel->GetReader().Read(buffer.data(), BufferSize)) {
                    break;
                }

                if (!acceptedChannel->GetWriter().Write(buffer.data(), BufferSize)) {
                    break;
                }

                if (!acceptedChannel->GetWriter().EndWrite()) {
                    break;
                }
            }
        }
    } catch (const std::exception& e) {
        LogError("Exception in local communication server thread: {}", e.what());
    }
}

}  // namespace

void StartLocalCommunicationServer() {  // NOLINT(misc-use-internal-linkage)
    std::thread(LocalCommunicationServerRun).detach();
}
