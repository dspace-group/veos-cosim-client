// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "PerformanceTestServer.hpp"

#include <array>
#include <thread>

#include "Logger.hpp"
#include "OsAbstractionTestHelper.hpp"
#include "PerformanceTestHelper.hpp"
#include "Result.hpp"

namespace DsVeosCoSim {

namespace {

Result RunForConnected(PipeClient& client) {
    std::array<char, FrameSize> buffer{};

    while (true) {
        CheckResult(client.Read(buffer.data(), FrameSize));
        CheckResult(client.Write(buffer.data(), FrameSize));
    }
}

[[nodiscard]] Result Run() {
    LogTrace("Pipe server is listening on pipe {} ...", PipeName);

    while (true) {
        PipeClient client;
        CheckResult(PipeClient::Accept(PipeName, client));

        RunForConnected(client);
    }

    return CreateOk();
}

void PipeServer() {
    if (!IsOk(Run())) {
        LogError("Could not run Pipe Server.");
    }
}

}  // namespace

void StartPipeServer() {
    std::thread(PipeServer).detach();
}

}  // namespace DsVeosCoSim
