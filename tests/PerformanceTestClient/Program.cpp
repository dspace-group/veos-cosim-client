// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "Helper.hpp"
#include "PerformanceTestClient.hpp"

using namespace DsVeosCoSim;

int main(int argc, char* argv[]) {
    if (!IsOk(StartUp())) {
        return 1;
    }

    if (argc > 1) {
        RunTcpSocketTest(argv[1]);
        RunUdpSocketTest(argv[1]);

        RunRemoteCommunicationTest(argv[1]);
        RunCoSimCallbackTest(argv[1]);
        RunCoSimPollingTest(argv[1]);
    } else {
        RunTcpSocketTest("127.0.0.1");
        RunUdpSocketTest("127.0.0.1");
        RunLocalSocketTest();
        RunEventsTest();
        RunPipeTest();
        RunShmPipeTest();

        RunRemoteCommunicationTest("127.0.0.1");
        RunLocalCommunicationTest();
        RunCoSimCallbackTest("127.0.0.1");
        RunCoSimCallbackTest("");
        RunCoSimPollingTest("127.0.0.1");
        RunCoSimPollingTest("");
    }

    return 0;
}
