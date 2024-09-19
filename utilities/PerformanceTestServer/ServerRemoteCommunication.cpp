// Copyright dSPACE GmbH. All rights reserved.

#include <thread>

#include "CoSimHelper.h"
#include "Helper.h"
#include "PerformanceTestHelper.h"
#include "SocketChannel.h"

using namespace DsVeosCoSim;

namespace {

void RemoteCommunicationServerRun() {
    try {
        LogTrace("Remote communication server is listening ...");

        TcpChannelServer server(CommunicationPort, true);

        char buffer[BufferSize]{};

        while (true) {
            std::optional<SocketChannel> acceptedChannel = server.TryAccept(Infinite);

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
        LogError("Exception in remote communication server thread: {}", e.what());
    }
}

}  // namespace

void StartRemoteCommunicationServer() {
    std::thread(RemoteCommunicationServerRun).detach();
}
