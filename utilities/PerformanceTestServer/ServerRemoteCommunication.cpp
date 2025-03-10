// Copyright dSPACE GmbH. All rights reserved.

#include <array>
#include <thread>

#include "Channel.h"
#include "CoSimHelper.h"
#include "Helper.h"
#include "LogHelper.h"
#include "PerformanceTestHelper.h"

using namespace DsVeosCoSim;

namespace {

void RemoteCommunicationServerRun() {
    try {
        LogTrace("Remote communication server is listening ...");

        const std::unique_ptr<ChannelServer> server = CreateTcpChannelServer(CommunicationPort, true);

        std::array<char, BufferSize> buffer{};

        while (true) {
            std::unique_ptr<Channel> acceptedChannel = server->TryAccept(Infinite);
            if (!acceptedChannel) {
                break;
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
        LogError("Exception in remote communication server thread: {}", e.what());
    }
}

}  // namespace

void StartRemoteCommunicationServer() {  // NOLINT
    std::thread(RemoteCommunicationServerRun).detach();
}
