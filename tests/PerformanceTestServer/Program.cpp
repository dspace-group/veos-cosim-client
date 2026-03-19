// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <future>

#include "CoSimTypes.hpp"
#include "Helper.hpp"
#include "PerformanceTestServer.hpp"

using namespace DsVeosCoSim;

int main() {
    if (!IsOk(StartUp())) {
        return 1;
    }

    StartTcpServer();
    StartUdpServer();
    StartLocalServer();
    StartPipeServer();
    StartEventsServer();
    StartLocalCommunicationServer();
    StartRemoteCommunicationServer();
    StartCoSimServer();

    std::promise<void>().get_future().wait();

    return 0;
}
