// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <future>

#include "Helper.hpp"
#include "PerformanceTestServer.hpp"

using namespace DsVeosCoSim;

int main() {
    if (!IsOk(StartUp())) {
        return 1;
    }

    StartEventsServer();
    StartLocalSocketServer();
    StartPipeServer();
    StartShmPipeServer();
    StartTcpSocketServer();
    StartUdpSocketServer();
    StartLocalCommunicationServer();
    StartRemoteCommunicationServer();
    StartCoSimServer();

    std::promise<void>().get_future().wait();

    return 0;
}
