// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "PerformanceTestServer.hpp"

#include <array>
#include <thread>

#include "Helper.hpp"
#include "OsAbstractionTestHelper.hpp"
#include "PerformanceTestHelper.hpp"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] Result Run() {
    LogTrace("Pipe server is listening on pipe {} ...", PipeName);

    std::array<char, BufferSize> buffer{};

    while (true) {
        Pipe pipe;
        CheckResult(pipe.Initialize(PipeName));
        CheckResult(pipe.Accept());

        while (true) {
            if (!IsOk(pipe.Read(buffer.data(), BufferSize))) {
                break;
            }

            if (!IsOk(pipe.Write(buffer.data(), BufferSize))) {
                break;
            }
        }
    }

    return Result::Ok;
}

void PipeServerRun() {
    if (!IsOk(Run())) {
        LogError("Could not run pipe server.");
    }
}

}  // namespace

void StartPipeServer() {
    std::thread(PipeServerRun).detach();
}
