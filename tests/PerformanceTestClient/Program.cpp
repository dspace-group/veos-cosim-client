// Copyright dSPACE GmbH. All rights reserved.

#include <cstdint>
#include <string_view>

#include "Helper.h"

extern void RunTcpTest(std::string_view host);
extern void RunUdpTest(std::string_view host);
extern void RunUdsTest();
extern void RunPipeTest();
extern void RunEventsTest();
extern void RunRemoteCommunicationTest(std::string_view host);
extern void RunLocalCommunicationTest();
extern void RunCoSimCallbackTest(std::string_view host);
extern void RunCoSimPollingTest(std::string_view host);

using namespace DsVeosCoSim;

int32_t main(const int32_t argc, char* argv[]) {
    if (!StartUp()) {
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
        RunUdsTest();
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
