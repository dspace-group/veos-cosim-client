// Copyright dSPACE GmbH. All rights reserved.

#include <array>
#include <cstdint>
#include <string_view>

#include "CoSimHelper.h"
#include "Helper.h"
#include "LogHelper.h"
#include "OsAbstractionTestHelper.h"
#include "PerformanceTestHelper.h"
#include "RunPerformanceTest.h"

using namespace DsVeosCoSim;

namespace {

void PipeClientRun([[maybe_unused]] std::string_view host,
                   Event& connectedEvent,
                   uint64_t& counter,
                   const bool& isStopped) {
    try {
        Pipe pipe(PipeName);
        pipe.Connect();

        std::array<char, BufferSize> buffer{};

        connectedEvent.Set();

        while (!isStopped) {
            MUST_BE_TRUE(pipe.Write(buffer.data(), BufferSize));

            MUST_BE_TRUE(pipe.Read(buffer.data(), BufferSize));

            counter++;
        }
    } catch (const std::exception& e) {
        LogError("Exception in pipe client thread: {}", e.what());
        connectedEvent.Set();
    }
}

}  // namespace

void RunPipeTest() {
    LogTrace("Pipes:");
    RunPerformanceTest(PipeClientRun, "");
    LogTrace("");
}
