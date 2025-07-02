// Copyright dSPACE GmbH. All rights reserved.

#include <cstdint>
#include <future>

#include "DsVeosCoSim/CoSimTypes.h"
#include "Helper.h"
#include "PerformanceTestServer.h"

using namespace DsVeosCoSim;

int32_t main() {
    if (!IsOk(StartUp())) {
        return 1;
    }

    StartTcpServer();
    StartUdpServer();
    StartUdsServer();
    StartPipeServer();
    StartEventsServer();
    StartLocalCommunicationServer();
    StartRemoteCommunicationServer();
    StartCoSimServer();

    std::promise<void>().get_future().wait();

    return 0;
}
