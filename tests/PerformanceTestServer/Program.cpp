// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <future>

#include "Helper.hpp"
#include "PerformanceTestServer.hpp"

using namespace DsVeosCoSim;

int main() {
    if (!IsOk(StartUp())) {
        return 1;
    }

    ServerCoSim();
    ServerLocalChannel();
    ServerLocalSocket();
    ServerPipe();
    ServerRemoteChannel();
    ServerTcpSocket();
    ServerUdpSocket();

#ifdef _WIN32
    ServerEvents();
    ServerShmPipe();
#endif

    std::promise<void>().get_future().wait();

    return 0;
}
