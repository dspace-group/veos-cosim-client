// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <array>
#include <thread>

#include "Logger.hpp"
#include "OsAbstractionTestHelper.hpp"
#include "PerformanceTestHelper.hpp"
#include "PerformanceTestServer.hpp"
#include "Result.hpp"

using namespace DsVeosCoSim;

namespace {

Result RunForConnectedPipe(PipeClient& client) {
    std::array<char, FrameSize> buffer{};

    while (true) {
        CheckResult(client.Read(buffer.data(), FrameSize));
        CheckResult(client.Write(buffer.data(), FrameSize));
    }
}

[[nodiscard]] Result RunServerPipeInternal() {
    LogTrace("Pipe server is listening on pipe {} ...", PipeName);

    while (true) {
        PipeClient client;
        CheckResult(PipeClient::Accept(PipeName, client));

        RunForConnectedPipe(client);
    }

    return CreateOk();
}

void RunServerPipe() {
    if (!IsOk(RunServerPipeInternal())) {
        LogError("Could not run Pipe Server.");
    }
}

}  // namespace

void ServerPipe() {
    std::thread(RunServerPipe).detach();
}
