// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "Helper.hpp"
#include "PerformanceTestClient.hpp"

using namespace DsVeosCoSim;

int main(int argc, char* argv[]) {
    if (!IsOk(StartUp())) {
        return 1;
    }

    if (argc > 1) {
        ClientTcpSocket(argv[1]);
        ClientUdpSocket(argv[1]);

        ClientRemoteChannel(argv[1]);
        ClientCoSimCallback(argv[1]);
        ClientCoSimPolling(argv[1]);
    } else {
        ClientTcpSocket("127.0.0.1");
        ClientUdpSocket("127.0.0.1");
        ClientLocalSocket();
        ClientPipe();

#ifdef _WIN32
        ClientEvents();
        ClientShmPipe();
#endif

        ClientRemoteChannel("127.0.0.1");
        ClientLocalChannel();

        ClientCoSimCallback("127.0.0.1");
        ClientCoSimCallback("");
        ClientCoSimPolling("127.0.0.1");
        ClientCoSimPolling("");
    }

    return 0;
}
