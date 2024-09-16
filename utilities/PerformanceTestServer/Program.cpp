// Copyright dSPACE GmbH. All rights reserved.

#include <future>

#include "Helper.h"

extern void StartAsioBlockingServer();
extern void StartUdpServer();
extern void StartTcpServer();
extern void StartUdsServer();
extern void StartPipeServer();
extern void StartEventsServer();
extern void StartLocalCommunicationServer();
extern void StartRemoteCommunicationServer();
extern void StartCoSimServer();

int32_t main() {
    if (!StartUp()) {
        return 1;
    }

    StartAsioBlockingServer();
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
