// Copyright dSPACE GmbH. All rights reserved.

#include <chrono>
#include <thread>

#include "CoSimHelper.h"
#include "PerformanceTestHelper.h"

#ifdef _WIN32
#include "LocalChannel.h"
#else
#include "SocketChannel.h"
#endif

using namespace DsVeosCoSim;

namespace {

void LocalCommunicationServerRun() {
    try {
        LogTrace("Local communication server is listening ...");

#ifdef _WIN32
        LocalChannelServer server(LocalName);
#else
        UdsChannelServer server(LocalName);
#endif

        char buffer[BufferSize]{};

        while (true) {
#ifdef _WIN32
            std::optional<LocalChannel> acceptedChannel;
#else
            std::optional<SocketChannel> acceptedChannel;
#endif
            while (true) {
                acceptedChannel = server.TryAccept();
                if (acceptedChannel) {
                    break;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            while (true) {
                if (!acceptedChannel->GetReader().Read(buffer, BufferSize)) {
                    break;
                }

                if (!acceptedChannel->GetWriter().Write(buffer, BufferSize)) {
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

void StartLocalCommunicationServer() {
    std::thread(LocalCommunicationServerRun).detach();
}
