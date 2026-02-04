// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "Helper.h"
#include "PerformanceTestClient.h"

using namespace DsVeosCoSim;

int main(int argc, char* argv[]) {
    if (!IsOk(StartUp())) {
        return 1;
    }

    if (argc > 1) {
        RunTcpTest(argv[1]);
        RunUdpTest(argv[1]);

        RunRemoteCommunicationTest(argv[1]);
        RunCoSimCallbackTest(argv[1]);
        RunCoSimPollingTest(argv[1]);
    } else {
        RunTcpTest("127.0.0.1");
        RunUdpTest("127.0.0.1");
        RunLocalTest();
        RunPipeTest();
        RunEventsTest();

        RunRemoteCommunicationTest("127.0.0.1");
        RunLocalCommunicationTest();
        RunCoSimCallbackTest("127.0.0.1");
        RunCoSimCallbackTest("");
        RunCoSimPollingTest("127.0.0.1");
        RunCoSimPollingTest("");
    }

    return 0;
}
