// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <future>

#include "Helper.hpp"
#include "PerformanceTestServer.hpp"

using namespace DsVeosCoSim;

int main() {
    if (!IsOk(StartUp())) {
        return 1;
    }

    StartLocalSocketServer();
    StartPipeServer();
    StartTcpSocketServer();
    StartUdpSocketServer();
    StartLocalCommunicationServer();
    StartRemoteCommunicationServer();
    StartCoSimServer();

#ifdef _WIN32
    StartEventsServer();
    StartShmPipeServer();
#endif

    std::promise<void>().get_future().wait();

    return 0;
}
